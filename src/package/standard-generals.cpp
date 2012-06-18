#include "general.h"
#include "standard.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "serverplayer.h"
#include "standard-generals.h"
#include "room.h"

GanlinCard::GanlinCard(){
    will_throw = false;
    mute = true;
}

void GanlinCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();

    room->playSkillEffect("ganlin", qrand() % 2 + 1);
    room->obtainCard(target, this, false);
    int n = source->getLostHp() - source->getHandcardNum();
    if(n > 0 && source->askForSkillInvoke("ganlin")){
        source->drawCards(n);
        room->playSkillEffect("ganlin", qrand() % 2 + 3);
        room->setPlayerFlag(source, "Ganlin");
    }
};

class GanlinViewAsSkill:public ViewAsSkill{
public:
    GanlinViewAsSkill():ViewAsSkill("ganlin"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasFlag("Ganlin");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        GanlinCard *ganlin_card = new GanlinCard;
        ganlin_card->addSubcards(cards);
        return ganlin_card;
    }
};

class Ganlin: public PhaseChangeSkill{
public:
    Ganlin():PhaseChangeSkill("ganlin"){
        view_as_skill = new GanlinViewAsSkill;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool onPhaseChange(ServerPlayer *p) const{
        if(p->getPhase() == Player::NotActive){
            Room *room = p->getRoom();
            room->setPlayerFlag(p, "-Ganlin");
        }
        return false;
    }
};

JuyiCard::JuyiCard(){
    once = true;
    target_fixed = true;
    mute = true;
}

void JuyiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    ServerPlayer *song = room->getLord();
    if(!song->hasLordSkill("juyi") || song == source)
        return;
    if(song->isKongcheng() && source->isKongcheng())
        return;
    if(room->askForChoice(song, "jui", "agree+deny") == "agree"){
        room->playSkillEffect("juyi", qrand() % 2 + 1);
        DummyCard *card1 = source->wholeHandCards();
        DummyCard *card2 = song->wholeHandCards();
        if(card1){
            room->obtainCard(song, card1, false);
            delete card1;
        }
        room->getThread()->delay();

        if(card2){
            room->obtainCard(source, card2, false);
            delete card2;
        }
        LogMessage log;
        log.type = "#Juyi";
        log.from = source;
        log.to << song;
        room->sendLog(log);
    }
    else
        room->playSkillEffect("juyi", qrand() % 2 + 3);
}

class JuyiViewAsSkill: public ZeroCardViewAsSkill{
public:
    JuyiViewAsSkill():ZeroCardViewAsSkill("jui"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasLordSkill("juyi")
                && player->getKingdom() == "kou"
                && !player->hasUsed("JuyiCard");
    }

    virtual const Card *viewAs() const{
        return new JuyiCard;
    }
};

class Juyi: public GameStartSkill{
public:
    Juyi():GameStartSkill("juyi$"){

    }

    virtual void onGameStart(ServerPlayer *player) const{
        if(!player->isLord())
            return;
        Room *room = player->getRoom();
        foreach(ServerPlayer *tmp, room->getAlivePlayers()){
            room->attachSkillToPlayer(tmp, "jui");
        }
    }
};

class Baoguo:public TriggerSkill{
public:
    Baoguo():TriggerSkill("baoguo"){
        events << Predamaged << Damaged;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent evt, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        QList<ServerPlayer *> ducks = room->findPlayersBySkillName(objectName());
        if(ducks.isEmpty())
            return false;
        foreach(ServerPlayer *duck, ducks){
            if(evt == Damaged){
                if(duck == player && duck->isWounded() && duck->askForSkillInvoke(objectName())){
                    room->playSkillEffect(objectName());
                    duck->drawCards(duck->getLostHp());
                }
            }
            else if(duck != player && !duck->isNude() && damage.damage > 0
                && room->askForCard(duck, "BasicCard", "@baoguo:" + player->objectName() + "::" + QString::number(damage.damage), data, CardDiscarded)){
                LogMessage log;
                log.type = "#Baoguo";
                log.from = duck;
                log.to << damage.to;
                log.arg = objectName();
                log.arg2 = QString::number(damage.damage);
                room->sendLog(log);

                damage.to = duck;
                room->damage(damage);
                return true;
            }
        }
        return false;
    }
    
    virtual int getPriority() const{
        return 2;
    }
};

HuaceCard::HuaceCard(){
}

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCommandLinkButton>

HuaceDialog *HuaceDialog::GetInstance(){
    static HuaceDialog *instance;
    if(instance == NULL)
        instance = new HuaceDialog;

    return instance;
}

HuaceDialog::HuaceDialog()
{
    setWindowTitle(Sanguosha->translate("huace"));

    group = new QButtonGroup(this);
    QHBoxLayout *mainlayout = new QHBoxLayout;

    QGroupBox *box1 = new QGroupBox(Sanguosha->translate("stt"));
    QVBoxLayout *layout1 = new QVBoxLayout;

    QGroupBox *box2 = new QGroupBox(Sanguosha->translate("mtt"));
    QVBoxLayout *layout2 = new QVBoxLayout;

    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach(const Card *card, cards){
        if(card->isNDTrick() && !map.contains(card->objectName())){
            Card *c = Sanguosha->cloneCard(card->objectName(), Card::NoSuit, 0);
            c->setSkillName("huace");
            c->setParent(this);

            QVBoxLayout *layout = c->inherits("SingleTargetTrick") ? layout1 : layout2;
            layout->addWidget(createButton(c));
        }
    }

    box1->setLayout(layout1);
    box2->setLayout(layout2);

    layout1->addStretch();
    layout2->addStretch();

    mainlayout->addWidget(box1);
    mainlayout->addWidget(box2);

    setLayout(mainlayout);

    connect(group, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(selectCard(QAbstractButton*)));
}

void HuaceDialog::popup(){
    if(ClientInstance->getStatus() != Client::Playing)
        return;

    foreach(QAbstractButton *button, group->buttons()){
        const Card *card = map[button->objectName()];
        button->setEnabled(card->isAvailable(Self));
    }

    Self->tag.remove("Huace");
    exec();
}

void HuaceDialog::selectCard(QAbstractButton *button){
    CardStar card = map.value(button->objectName());
    Self->tag["Huace"] = QVariant::fromValue(card);
    accept();
}

QAbstractButton *HuaceDialog::createButton(const Card *card){
    QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate(card->objectName()));
    button->setObjectName(card->objectName());
    button->setToolTip(card->getDescription());

    map.insert(card->objectName(), card);
    group->addButton(button);

    return button;
}

bool HuaceCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    CardStar card = Self->tag["Huace"].value<CardStar>();
    return card && card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card);
}

bool HuaceCard::targetFixed() const{
    if(ClientInstance->getStatus() == Client::Responsing)
        return true;

    CardStar card = Self->tag["Huace"].value<CardStar>();
    return card && card->targetFixed();
}

bool HuaceCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    CardStar card = Self->tag["Huace"].value<CardStar>();
    return card && card->targetsFeasible(targets, Self);
}

const Card *HuaceCard::validate(const CardUseStruct *card_use) const{
    Room *room = card_use->from->getRoom();
    //room->playSkillEffect("huace");
    const Card *card = Sanguosha->getCard(subcards.first());
    Card *use_card = Sanguosha->cloneCard(user_string, card->getSuit(), card->getNumber());
    use_card->setSkillName("huace");
    use_card->addSubcard(card);
    room->throwCard(this);

    return use_card;
}

const Card *HuaceCard::validateInResposing(ServerPlayer *player, bool *continuable) const{
    *continuable = true;
    Room *room = player->getRoom();
    QString string;
    if(user_string == "nulliplot")
        string = room->askForChoice(player, "huace-nullchoice", "nullification+counterplot");
    else
        string = "nullification";
    const Card *card = Sanguosha->getCard(subcards.first());
    Card *use_card = Sanguosha->cloneCard(string, card->getSuit(), card->getNumber());
    use_card->setSkillName("huace");
    use_card->addSubcard(card);
    room->throwCard(this);

    player->addHistory("HuaceCard", 1);
    Self->addHistory("HuaceCard", 1);
    return use_card;
}

class Huace:public OneCardViewAsSkill{
public:
    Huace():OneCardViewAsSkill("huace"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("HuaceCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return (pattern == "nullification" || pattern == "nulliplot") &&
                !player->hasUsed("HuaceCard") &&
                !player->isKongcheng() &&
                player->getPhase() == Player::Play;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getCard();
        return card->inherits("TrickCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        if(ClientInstance->getStatus() == Client::Responsing){
            HuaceCard *card = new HuaceCard;
            card->setUserString(ClientInstance->getPattern());
            card->addSubcard(card_item->getFilteredCard());
            return card;
        }

        CardStar c = Self->tag["Huace"].value<CardStar>();
        if(c){
            HuaceCard *card = new HuaceCard;
            card->setUserString(c->objectName());
            card->addSubcard(card_item->getFilteredCard());
            return card;
        }else
            return NULL;
    }

    virtual QDialog *getDialog() const{
        return HuaceDialog::GetInstance();
    }
};

class Yunchou:public TriggerSkill{
public:
    Yunchou():TriggerSkill("yunchou"){
        frequency = Frequent;
        events << CardUsed << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *feiwu, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->isNDTrick()){
            if(room->askForSkillInvoke(feiwu, objectName())){
                if(card->getSkillName() != "huace")
                    room->playSkillEffect(objectName());
                feiwu->drawCards(1);
            }
        }
        return false;
    }
};

YixingCard::YixingCard(){
}

bool YixingCard::targetFilter(const QList<const Player *> &targets, const Player *to, const Player *Self) const{
    return targets.isEmpty() && to->hasEquip();
}

void YixingCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    PlayerStar target = effect.to;
    int card_id = target->getEquips().length() == 1 ? target->getEquips().first()->getId() :
                  room->askForCardChosen(effect.from, target, "e", "yixing");
    effect.from->tag["YixingCard"] = card_id;
    effect.from->tag["YixingTarget"] = QVariant::fromValue(target);
}

class YixingViewAsSkill: public ZeroCardViewAsSkill{
public:
    YixingViewAsSkill():ZeroCardViewAsSkill("yixing"){
    }

    virtual const Card *viewAs() const{
        return new YixingCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@yixing";
    }
};

class Yixing: public TriggerSkill{
public:
    Yixing():TriggerSkill("yixing"){
        events << AskForRetrial;
        view_as_skill = new YixingViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        if(!TriggerSkill::triggerable(target))
            return false;
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();

        player->tag["Judge"] = data;
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *tmp, room->getAllPlayers()){
            if(tmp->hasEquip())
                targets << tmp;
        }
        if(targets.isEmpty())
            return false;
        QStringList prompt_list;
        prompt_list << "@yixing" << judge->who->objectName()
                        << "" << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");
        if(room->askForUseCard(player, "@@yixing", prompt)){
            int card_id = player->tag["YixingCard"].toInt();
            ServerPlayer *target = player->tag["YixingTarget"].value<PlayerStar>();
            const Card *card = Sanguosha->getCard(card_id);
            target->obtainCard(judge->card);
            //room->playSkillEffect(objectName());
            judge->card = card;
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = player;
            log.to << judge->who;
            log.card_str = card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);
        }
        return false;
    }
};

QimenStruct::QimenStruct()
    :kingdom("guan"), generalA("gongsunsheng"), generalB("zhuwu"), maxhp(5), skills(NULL)
{
}

class Qimen: public PhaseChangeSkill{
public:
    Qimen():PhaseChangeSkill("qimen"){
    }

    static void willCry(Room *room, ServerPlayer *target, ServerPlayer *gongsun){
        QStringList skills;
        bool has_qimen = target == gongsun;
        foreach(const SkillClass *skill, target->getVisibleSkillList()){
            QString skill_name = skill->objectName();
            skills << skill_name;
            room->detachSkillFromPlayer(target, skill_name);
        }
        QimenStruct Qimen_data;
        Qimen_data.kingdom = target->getKingdom();
        Qimen_data.generalA = target->getGeneralName();
        Qimen_data.maxhp = target->getMaxHP();
        QString to_transfigure = target->getGeneral()->isMale() ? "sujiang" : "sujiangf";
        if(!has_qimen)
            room->transfigure(target, to_transfigure, false, false);
        else{
            room->setPlayerProperty(target, "general", to_transfigure);
            room->acquireSkill(target, "qimen");
        }
        room->setPlayerProperty(target, "maxhp", Qimen_data.maxhp);
        if(target->getGeneral2()){
            Qimen_data.generalB = target->getGeneral2Name();
            room->setPlayerProperty(target, "general2", to_transfigure);
        }
        room->setPlayerProperty(target, "kingdom", Qimen_data.kingdom);
        Qimen_data.skills = skills;
        target->tag["QimenStore"] = QVariant::fromValue(Qimen_data);
        target->setMark("Qimen_target", 1);
    }

    static void stopCry(Room *room, ServerPlayer *player){
        player->setMark("Qimen_target", 0);
        QimenStruct Qimen_data = player->tag.value("QimenStore").value<QimenStruct>();

        QStringList attachskills;
        attachskills << "spear" << "axe" << "jui" << "mAIdao";
        foreach(QString skill_name, Qimen_data.skills){
            if(skill_name == "spear" && (!player->getWeapon() || player->getWeapon()->objectName() != "spear"))
                continue;
            if(skill_name == "axe" && (!player->getWeapon() || player->getWeapon()->objectName() != "axe"))
                continue;
            if(attachskills.contains(skill_name))
                room->attachSkillToPlayer(player, skill_name);
            else
                room->acquireSkill(player, skill_name);
        }
        room->setPlayerProperty(player, "general", Qimen_data.generalA);
        if(player->getGeneral2()){
            room->setPlayerProperty(player, "general2", Qimen_data.generalB);
        }
        room->setPlayerProperty(player, "kingdom", Qimen_data.kingdom);

        player->tag.remove("QimenStore");
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        PlayerStar target = player;
        Room *room = target->getRoom();
        QList<ServerPlayer *> dragons = room->findPlayersBySkillName(objectName());
        if(dragons.isEmpty())
            return false;
        if(target->getPhase() == Player::NotActive){
            foreach(ServerPlayer *tmp, room->getAllPlayers()){
                if(tmp->getMark("Qimen_target") > 0){
                    stopCry(room, tmp);

                    LogMessage log;
                    log.type = "#QimenEnd";
                    log.to << tmp;
                    log.arg = objectName();

                    room->sendLog(log);
                    break;
                }
            }
            return false;
        }
        else if(target->getPhase() == Player::RoundStart){
            foreach(ServerPlayer *dragon, dragons){
                if(!dragon->isNude() && room->askForSkillInvoke(dragon, objectName(), QVariant::fromValue(target))){
                    ServerPlayer *superman = room->askForPlayerChosen(dragon, room->getOtherPlayers(dragon), objectName());
                    JudgeStruct judge;
                    judge.pattern = QRegExp("(.*):(.*):(.*)");
                    judge.reason = objectName();
                    judge.who = superman;

                    room->judge(judge);
                    QString suit_str = judge.card->getSuitString();
                    QString pattern = QString(".|%1").arg(suit_str);
                    QString prompt = QString("@qimen:%1::%2").arg(superman->getGeneralName()).arg(suit_str);
                    if(room->askForCard(dragon, pattern, prompt, QVariant(), CardDiscarded)){
                        if(dragon->getMark("wudao") == 0)
                            room->playSkillEffect(objectName(), qrand() % 2 + 1);
                        else
                            room->playSkillEffect(objectName(), qrand() % 2 + 3);
                        LogMessage log;
                        log.type = "#Qimen";
                        log.from = dragon;
                        log.to << superman;
                        log.arg = objectName();
                        room->sendLog(log);

                        willCry(room, superman, dragon);
                        break;
                    }
                }
            }
        }
        return false;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }
};

class QimenClear: public TriggerSkill{
public:
    QimenClear():TriggerSkill("#qimencls"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill("qimen");
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        foreach(ServerPlayer *tmp, room->getAllPlayers()){
            if(tmp->getMark("Qimen_target") > 0){
                Qimen::stopCry(room, tmp);

                LogMessage log;
                log.type = "#QimenClear";
                log.from = player;
                log.to << tmp;
                log.arg = "qimen";

                room->sendLog(log);
            }
        }
        return false;
    }
};

class Huqi: public DistanceSkill{
public:
    Huqi():DistanceSkill("huqi"){
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(from->hasSkill(objectName()))
            return -1;
        else
            return 0;
    }
};

class Tongwu: public TriggerSkill{
public:
    Tongwu():TriggerSkill("tongwu"){
        events << SlashMissed;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *erge, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(!effect.to->isNude() && effect.jink && !effect.jink->isVirtualCard()){
            if(erge->askForSkillInvoke(objectName(), data)){
                room->playSkillEffect(objectName());
                erge->obtainCard(effect.jink);
                ServerPlayer *target = room->askForPlayerChosen(erge, room->getOtherPlayers(effect.to), objectName());
                target->obtainCard(effect.jink);
            }
        }
        return false;
    }
};

DuijueCard::DuijueCard(){
    mute = true;
}

bool DuijueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return to_select != Self;
}

void DuijueCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(spade):(.*)");
    judge.good = true;
    judge.reason = "duijue";
    judge.who = effect.to;

    room->judge(judge);
    if(judge.isBad()){
        Duel *duel = new Duel(judge.card->getSuit(), judge.card->getNumber());
        duel->setSkillName("duijue");
        duel->setCancelable(false);

        CardUseStruct use;
        use.from = effect.from;
        use.to << effect.to;
        use.card = duel;
        room->useCard(use);
    }
}

class DuijueViewAsSkill:public ZeroCardViewAsSkill{
public:
    DuijueViewAsSkill():ZeroCardViewAsSkill("duijue"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@duijue";
    }

    virtual const Card *viewAs() const{
        return new DuijueCard;
    }
};

class Duijue: public TriggerSkill{
public:
    Duijue():TriggerSkill("duijue"){
        view_as_skill = new DuijueViewAsSkill;
        events << Damage << Damaged;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.card || !damage.card->inherits("Slash"))
            return false;
        if(event == Damaged && damage.from == player)
            return false;
        room->askForUseCard(player, "@@duijue", "@duijue");
        return false;
    }
};

class Jingzhun: public SlashBuffSkill{
public:
    Jingzhun():SlashBuffSkill("jingzhun"){
        frequency = Compulsory;
    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *huarong = effect.from;
        Room *room = huarong->getRoom();

        if(huarong->distanceTo(effect.to) == huarong->getAttackRange()){
            room->playSkillEffect(objectName());
            LogMessage log;
            log.type = "#Jingzhun";
            log.from = huarong;
            log.to << effect.to;
            log.arg = objectName();
            room->sendLog(log);

            room->slashResult(effect, NULL);
            return true;
        }
        return false;
    }
};

class KaixianPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return !player->hasEquip(card) &&
                card->getNumber() <= 5;
    }

    virtual bool willThrow() const{
        return false;
    }
};

class Kaixian: public PhaseChangeSkill{
public:
    Kaixian():PhaseChangeSkill("kaixian"){

    }

    virtual bool onPhaseChange(ServerPlayer *huarong) const{
        Room *room = huarong->getRoom();
        if(huarong->getPhase() == Player::RoundStart && !huarong->isKongcheng()){
            bool caninvoke = false;
            foreach(const Card *cd, huarong->getHandcards()){
                if(cd->getNumber() <= 5){
                    caninvoke = true;
                    break;
                }
            }
            if(caninvoke && room->askForSkillInvoke(huarong, objectName())){
                const Card *card = room->askForCard(huarong, ".kaixian!", "@kaixian", QVariant(), NonTrigger);
                room->showCard(huarong, card->getId());
                room->setPlayerMark(huarong, "kaixian", card->getNumber());
                LogMessage log;
                log.type = "$Kaixian";
                log.from = huarong;
                log.card_str = card->getEffectIdString();
                room->sendLog(log);

                room->playSkillEffect(objectName());
            }
        }
        else if(huarong->getPhase() == Player::NotActive)
            room->setPlayerMark(huarong, "kaixian", 0);

        return false;
    }
};

class Danshu: public TriggerSkill{
public:
    Danshu():TriggerSkill("danshu"){
        events << CardEffected;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.card->inherits("Slash") && effect.to->isWounded()){
            room->playSkillEffect(objectName());
            LogMessage log;
            log.type = "#Danshu";
            log.from = effect.from;
            log.to << effect.to;
            log.arg = objectName();
            log.arg2 = QString::number(effect.to->getLostHp());
            room->sendLog(log);

            return !room->askForDiscard(effect.from, objectName(), effect.to->getLostHp(), true);
        }
        return false;
    }
};

HaoshenCard::HaoshenCard(){
    will_throw = false;
    mute = true;
}

bool HaoshenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    if(Self->getPhase() == Player::Draw)
        return to_select->getHandcardNum() != to_select->getMaxHP();
    else
        return to_select != Self;
    return false;
}

void HaoshenCard::use(Room *room, ServerPlayer *chaijin, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    int num = target->getMaxHP() - target->getHandcardNum();
    if(chaijin->getPhase() == Player::Draw && num > 0){
        room->playSkillEffect("haoshen", qrand() % 2 + 1);
        target->drawCards(qMin(5, num));
    }
    else if(chaijin->getPhase() == Player::Play){
        target->obtainCard(this, false);
        room->playSkillEffect("haoshen", qrand() % 2 + 3);
    }
}

class HaoshenViewAsSkill: public ViewAsSkill{
public:
    HaoshenViewAsSkill():ViewAsSkill("haoshen"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(Self->getPhase() == Player::Draw)
            return selected.isEmpty();
        else{
            int length = (Self->getHandcardNum() + 1) / 2;
            return selected.length() < length;
        }
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(Self->getPhase() == Player::Play && cards.length() != (Self->getHandcardNum() + 1) / 2)
            return NULL;
        HaoshenCard *card = new HaoshenCard;
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@haoshen";
    }
};

class Haoshen: public PhaseChangeSkill{
public:
    Haoshen():PhaseChangeSkill("haoshen"){
        view_as_skill = new HaoshenViewAsSkill;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target);
    }

    virtual bool onPhaseChange(ServerPlayer *chaijin) const{
        Room *room = chaijin->getRoom();

        switch(chaijin->getPhase()){
        case Player::Draw: return room->askForUseCard(chaijin, "@@haoshen", "@haoshen-draw");
        case Player::Play:
            if(!chaijin->isKongcheng())
                return room->askForUseCard(chaijin, "@@haoshen", "@haoshen-play");
        default: return false;
        }

        return false;
    }
};

SijiuCard::SijiuCard()
    :QingnangCard(){
}

bool SijiuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() > 0)
        return false;
    return to_select->isWounded() && to_select != Self;
}

class Sijiu: public OneCardViewAsSkill{
public:
    Sijiu():OneCardViewAsSkill("sijiu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return true;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Peach");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        SijiuCard *qingnang_card = new SijiuCard;
        qingnang_card->addSubcard(card_item->getCard()->getId());
        return qingnang_card;
    }
};

class Yixian: public TriggerSkill{
public:
    Yixian():TriggerSkill("yixian"){
        events << DamageProceed;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to == damage.from || damage.damage < 1)
            return false;
        if(!damage.to->isAllNude() && player->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            int dust = !damage.to->hasEquip() && damage.to->getJudgingArea().isEmpty() ? damage.to->getRandomHandCardId() :
                          room->askForCardChosen(player, damage.to, "hej", objectName());
            room->throwCard(dust);

            LogMessage log;
            log.type = "$Yixian";
            log.from = player;
            log.to << damage.to;
            log.card_str = QString::number(dust);
            room->sendLog(log);
            player->drawCards(1);
            return true;
        }
        return false;
    }
};

class Liba: public TriggerSkill{
public:
    Liba():TriggerSkill("liba"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *luda, QVariant &data) const{
        if(luda->getPhase() != Player::Play)
            return false;
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.card && damage.card->inherits("Slash") && damage.to->isAlive()
            && !damage.to->isKongcheng()){
            if(room->askForSkillInvoke(luda, objectName(), data)){
                room->playSkillEffect(objectName());
                int card_id = damage.to->getRandomHandCardId();
                const Card *card = Sanguosha->getCard(card_id);
                room->showCard(damage.to, card_id);
                room->getThread()->delay();
                if(!card->inherits("BasicCard")){
                    room->throwCard(card_id);
                    LogMessage log;
                    log.type = "$ForceDiscardCard";
                    log.from = luda;
                    log.to << damage.to;
                    log.card_str = card->getEffectIdString();
                    room->sendLog(log);

                    damage.damage ++;
                }
                data = QVariant::fromValue(damage);
            }
        }
        return false;
    }
};

class Fuhu: public TriggerSkill{
public:
    Fuhu():TriggerSkill("fuhu"){
        events << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.from || !damage.card || !damage.card->inherits("Slash"))
            return false;
        QList<ServerPlayer *> wusOng = room->findPlayersBySkillName(objectName());
        if(wusOng.isEmpty())
            return false;

        foreach(ServerPlayer *wusong, wusOng){
            if(damage.from->isDead())
                break;
            if(wusong->canSlash(damage.from, false)
                    && !wusong->isKongcheng() && damage.from != wusong){
                const Card *card = room->askForCard(wusong, ".|.|.|.|black", "@fuhu:" + damage.from->objectName(), data, CardDiscarded);
                if(!card)
                    continue;
                Slash *slash = new Slash(Card::NoSuit, 0);
                slash->setSkillName(objectName());
                CardUseStruct use;
                use.card = slash;
                use.from = wusong;
                use.to << damage.from;

                if(card->inherits("Analeptic") || card->inherits("Weapon")){
                    LogMessage log;
                    log.type = "$Fuhu";
                    log.from = wusong;
                    log.card_str = card->getEffectIdString();
                    room->sendLog(log);

                    room->setPlayerFlag(wusong, "drank");
                }
                room->throwCard(card, wusong);
                room->useCard(use);
            }
        }
        return false;
    }
};

MaidaoCard::MaidaoCard(){
    will_throw = false;
    target_fixed = true;
    mute = true;
}

void MaidaoCard::use(Room *, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    source->getRoom()->playSkillEffect("maidao", qrand() % 2 + 1);
    foreach(int x, getSubcards())
        source->addToPile("knife", x);
}

class MaidaoViewAsSkill: public ViewAsSkill{
public:
    MaidaoViewAsSkill(): ViewAsSkill("maidao"){

    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *to_select) const{
        return to_select->getCard()->inherits("Weapon");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        MaidaoCard *card = new MaidaoCard;
        card->addSubcards(cards);
        return card;
    }
};

class Maidao: public GameStartSkill{
public:
    Maidao():GameStartSkill("maidao"){
        view_as_skill = new MaidaoViewAsSkill;
    }

    virtual void onGameStart(ServerPlayer *yangvi) const{
        Room *room = yangvi->getRoom();
        QList<ServerPlayer *> players = room->getAlivePlayers();
        foreach(ServerPlayer *player, players){
            room->attachSkillToPlayer(player, "mAIdao");
        }
    }
};

FengmangCard::FengmangCard(){
}

bool FengmangCard::targetFilter(const QList<const Player *> &targets, const Player *to_s, const Player *Self) const{
    return targets.isEmpty() && to_s != Self;
}

void FengmangCard::use(Room *room, ServerPlayer *yang, const QList<ServerPlayer *> &targets) const{
    const Card *dmgcard = NULL;
    if(getSubcards().isEmpty()){
        const QList<int> &knife = yang->getPile("knife");
        int card_id;
        if(knife.length() == 1)
            card_id = knife.first();
        else{
            room->fillAG(knife, yang);
            card_id = room->askForAG(yang, knife, false, objectName());
            yang->invoke("clearAG");
        }

        dmgcard = Sanguosha->getCard(card_id);
        room->throwCard(card_id);
    }
    else{
        dmgcard = Sanguosha->getCard(getSubcards().first());
        room->throwCard(this);
    }
    DamageStruct dmg;
    dmg.card = dmgcard;
    dmg.from = yang;
    dmg.to = targets.first();
    room->damage(dmg);
}

class FengmangViewAsSkill: public ViewAsSkill{
public:
    FengmangViewAsSkill():ViewAsSkill("fengmang"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@fengmang";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 1)
            return false;
        return to_select->getCard()->inherits("EventsCard");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty() && Self->getPile("knife").isEmpty())
            return NULL;
        if(cards.length() > 1)
            return NULL;
        FengmangCard *card = new FengmangCard;
        card->addSubcards(cards);
        return card;
    }
};

class Fengmang: public PhaseChangeSkill{
public:
    Fengmang():PhaseChangeSkill("fengmang"){
        view_as_skill = new FengmangViewAsSkill;
    }

    static int getEventsCount(ServerPlayer *player){
        int x = 0;
        foreach(const Card *card, player->getHandcards())
            if(card->inherits("EventsCard"))
                x ++;
        return x;
    }

    virtual bool onPhaseChange(ServerPlayer *yang) const{
        Room *room = yang->getRoom();
        if(yang->getPhase() != Player::Start)
            return false;
        while(!yang->getPile("knife").isEmpty() || getEventsCount(yang) > 0)
            if(!room->askForUseCard(yang, "@@fengmang", "@fengmang"))
                break;
        return false;
    }
};

MAIdaoCard::MAIdaoCard(){
    will_throw = false;
    mute = true;
}

bool MAIdaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty() || to_select == Self)
        return false;
    return to_select->hasSkill("maidao") && !to_select->getPile("knife").isEmpty();
}

void MAIdaoCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    room->playSkillEffect("maidao", qrand() % 2 + 3);
    target->obtainCard(this, false);

    const QList<int> &knife = target->getPile("knife");
    if(knife.isEmpty())
        return;
    int card_id;
    if(knife.length() == 1)
        card_id = knife.first();
    else{
        room->fillAG(knife, source);
        card_id = room->askForAG(source, knife, false, "mAIdao");
        source->invoke("clearAG");
    }
    source->obtainCard(Sanguosha->getCard(card_id));
}

class MAIdao: public ViewAsSkill{
public:
    MAIdao():ViewAsSkill("mAIdao"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() > 2)
            return false;
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        MAIdaoCard *card = new MAIdaoCard();
        card->addSubcards(cards);
        return card;
    }
};

class Goulian: public TriggerSkill{
public:
    Goulian():TriggerSkill("goulian"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.chain || !damage.card || (!damage.card->inherits("Slash") && !damage.card->inherits("Duel")))
            return false;

        QStringList horses;
        if(damage.to->getArmor())
            horses << "armor";
        if(damage.to->getDefensiveHorse())
            horses << "defensive_horse";
        if(damage.to->getOffensiveHorse())
            horses << "offensive_horse";

        if(horses.isEmpty())
            return false;

        if(!player->askForSkillInvoke(objectName(), data))
            return false;
        room->playSkillEffect(objectName());

        QString horse_type;
        if(horses.length() > 1)
            horse_type = room->askForChoice(player, objectName(), horses.join("+"));
        else
            horse_type = horses.first();

        if(horse_type == "defensive_horse")
            room->throwCard(damage.to->getDefensiveHorse(), damage.to);
        else if(horse_type == "offensive_horse")
            room->throwCard(damage.to->getOffensiveHorse(), damage.to);
        else if(horse_type == "armor")
            room->throwCard(damage.to->getArmor(), damage.to);

        return false;
    }
};

class Jinjia: public TriggerSkill{
public:
    Jinjia():TriggerSkill("jinjia"){
        events << Damaged << SlashEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->getArmor() && target->getMark("qinggang") == 0;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == SlashEffected){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(effect.nature != DamageStruct::Normal){
                room->playSkillEffect(objectName(), 1);
                LogMessage log;
                log.from = player;
                log.type = "#JinjiaNullify";
                log.arg = objectName();
                log.arg2 = effect.slash->objectName();
                room->sendLog(log);

                return true;
            }
        }else if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.card && damage.card->inherits("Slash")){
                LogMessage log;
                log.type = "#ThrowJinjiaWeapon";
                log.from = player;
                log.arg = objectName();
                if(damage.from->getWeapon()){
                    room->playSkillEffect(objectName(), 2);
                    room->sendLog(log);
                    room->throwCard(damage.from->getWeapon());
                }
            }
        }
        return false;
    }
};

#include "plough.h"
class Mitan: public OneCardViewAsSkill{
public:
    Mitan():OneCardViewAsSkill("mitan"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("TrickCard") ||
                to_select->getCard()->inherits("EventsCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *c = card_item->getCard();
        Wiretap *wp = new Wiretap(c->getSuit(), c->getNumber());
        wp->setSkillName(objectName());
        wp->addSubcard(card_item->getCard());

        return wp;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 1;
    }
};

class Jibao: public PhaseChangeSkill{
public:
    Jibao():PhaseChangeSkill("jibao"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::RoundStart)
            room->setPlayerMark(player, "jibao", player->getHandcardNum());
        else if(player->getPhase() == Player::NotActive){
            if(player->getMark("jibao") == player->getHandcardNum() &&
               !player->isKongcheng() &&
               room->askForCard(player, ".", "@jibao", QVariant(), CardDiscarded)){
                room->playSkillEffect(objectName());
                LogMessage log;
                log.type = "#Jibao";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);
                room->getThread()->delay();
                player->gainAnExtraTurn(player);
            }
        }
        return false;
    }
};

class Shalu: public TriggerSkill{
public:
    Shalu():TriggerSkill("shalu"){
        events << Damage << PhaseChange;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent e, Room* room, ServerPlayer *likui, QVariant &data) const{
        if(e == PhaseChange){
            if(likui->getPhase() == Player::NotActive)
                room->setPlayerMark(likui, "shalu", 0);
            return false;
        }
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.card || damage.from != likui)
            return false;
        if(damage.card->inherits("Slash")){
            if(likui->getMark("shalu") > 0 && !likui->hasWeapon("crossbow")
                && !likui->hasSkill("paoxiao") && !likui->hasSkill("qinlong")
                && !(likui->hasSkill("yinyu") && likui->getMark("@ylyus") > 0))
                room->setPlayerMark(likui, "shalu", likui->getMark("shalu") - 1);
            if(!room->askForSkillInvoke(likui, objectName(), data))
                return false;
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(spade|club):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = likui;

            room->judge(judge);
            if(judge.isGood()){
                room->playSkillEffect(objectName(), qrand() % 2 + 1);
                likui->obtainCard(judge.card);
                room->setPlayerMark(likui, "shalu", likui->getMark("shalu") + 1);
            }
            else
                room->playSkillEffect(objectName(), 3);
        }
        return false;
    }
};

class Jueming: public ProhibitSkill{
public:
    Jueming():ProhibitSkill("jueming"){
    }

    virtual bool isProhibited(const Player *, const Player *to, const Card *card) const{
        if(to->getHp() == 1)
            return card->inherits("Slash") || card->inherits("Duel") || card->inherits("Assassinate");
        else
            return false;
    }
};

class JuemingEffect:public TriggerSkill{
public:
    JuemingEffect():TriggerSkill("#jueming_effect"){
        events << HpChanged;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *nana, QVariant &data) const{
        if(nana->getHp() == 1 && nana->getPhase() == Player::NotActive)
            room->playSkillEffect("jueming");
        return false;
    }
};

class Jiuhan:public TriggerSkill{
public:
    Jiuhan():TriggerSkill("jiuhan"){
        events << HpRecover;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *nana, QVariant &data) const{
        RecoverStruct rec = data.value<RecoverStruct>();
        if(rec.who == nana && rec.card->inherits("Analeptic") &&
           nana->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            LogMessage log;
            log.type = "#Jiuhan";
            log.from = nana;
            log.arg = objectName();
            log.arg2 = QString::number(1);
            room->sendLog(log);
            rec.recover ++;

            data = QVariant::fromValue(rec);
        }
        return false;
    }
};

class Xingxing: public TriggerSkill{
public:
    Xingxing():TriggerSkill("xingxing"){
        events << Dying;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *poolguy, QVariant &data) const{
        QList<ServerPlayer *>bears = room->findPlayersBySkillName(objectName());
        DyingStruct dying = data.value<DyingStruct>();
        if(!dying.who || dying.who != poolguy)
            return false;
        foreach(ServerPlayer *bear, bears){
            if(bear == poolguy || !bear->inMyAttackRange(poolguy))
                continue;
            if(room->askForCard(bear, ".S", "@xingxing:" + poolguy->objectName(), data, CardDiscarded)){
                room->playSkillEffect(objectName());
                DamageStruct damage;
                damage.from = bear;

                LogMessage log;
                log.type = "#Xingxing";
                log.from = bear;
                log.to << poolguy;
                log.arg = objectName();
                room->sendLog(log);

                room->getThread()->delay(1500);
                room->killPlayer(poolguy, &damage);
                return true;
            }
        }
        return false;
    }
};

DaleiCard::DaleiCard(){
    once = true;
    will_throw = false;
    mute = true;
}

bool DaleiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getGeneral()->isMale() &&
            !to_select->isKongcheng() && to_select != Self;
}

void DaleiCard::use(Room *room, ServerPlayer *xiaoyi, const QList<ServerPlayer *> &targets) const{
    room->playSkillEffect("dalei", qrand() % 2 + 1);
    PlayerStar target = targets.first();
    bool success = xiaoyi->pindian(target, "dalei", this);
    if(success){
        room->playSkillEffect("dalei", 3);
        room->setPlayerFlag(xiaoyi, "dalei_success");
        room->setPlayerProperty(xiaoyi, "dalei_target", QVariant::fromValue(target));
    }else{
        room->playSkillEffect("dalei", 4);
        DamageStruct damage;
        damage.from = targets.first();
        damage.to = xiaoyi;
        room->damage(damage);
    }
}

class DaleiViewAsSkill: public OneCardViewAsSkill{
public:
    DaleiViewAsSkill():OneCardViewAsSkill("dalei"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DaleiCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new DaleiCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Dalei: public TriggerSkill{
public:
    Dalei():TriggerSkill("dalei"){
        view_as_skill = new DaleiViewAsSkill;
        events << Damage << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(!player->hasFlag("dalei_success"))
            return false;
        if(event == PhaseChange){
            if(player->getPhase() == Player::NotActive){
                room->setPlayerFlag(player, "-dalei_success");
                room->setPlayerProperty(player, "dalei_target", QVariant());
            }
            return false;
        }
        DamageStruct damage = data.value<DamageStruct>();
        PlayerStar target = player->property("dalei_target").value<PlayerStar>();
        if(damage.to == target){
            RecoverStruct rev;
            rev.who = player;
            for(int p = 0; p < damage.damage; p++){
                QList<ServerPlayer *> targets;
                foreach(ServerPlayer *tmp, room->getOtherPlayers(target))
                    if(tmp->isWounded())
                        targets << tmp;
                if(!targets.isEmpty() && player->askForSkillInvoke(objectName(), data))
                    room->recover(room->askForPlayerChosen(player, targets, objectName()), rev);
            }
        }
        return false;
    }
};

class Fuqin: public MasochismSkill{
public:
    Fuqin():MasochismSkill("fuqin"){
    }

    virtual QString getDefaultChoice(ServerPlayer *player) const{
        if(player->getLostHp() > 2)
            return "qing";
        else
            return "yan";
    }

    virtual void onDamaged(ServerPlayer *yan, const DamageStruct &damage) const{
        Room *room = yan->getRoom();
        int lstn = yan->getLostHp();
        if(damage.from)
            yan->tag["FuqinSource"] = QVariant::fromValue((PlayerStar)damage.from);
        QString choice = damage.from ?
                         room->askForChoice(yan, objectName(), "yan+qing+nil"):
                         room->askForChoice(yan, objectName(), "qing+nil");
        if(choice == "nil")
            return;
        LogMessage log;
        log.from = yan;
        log.arg = objectName();
        if(choice == "yan"){
            room->playSkillEffect(objectName(), qrand() % 2 + 1);
            if(!damage.from || damage.from->isNude())
                return;
            int i = 0;
            for(; i < lstn; i++){
                int card_id = room->askForCardChosen(yan, damage.from, "he", objectName());
                room->throwCard(card_id);
                if(damage.from->isNude())
                    break;
            }
            log.to << damage.from;
            log.arg2 = QString::number(i);
            log.type = "#FuqinYan";
        }
        else{
            room->playSkillEffect(objectName(), qrand() % 2 + 3);
            ServerPlayer *target = room->askForPlayerChosen(yan, room->getAllPlayers(), objectName());
            target->drawCards(lstn);
            log.to << target;
            log.arg2 = QString::number(lstn);
            log.type = "#FuqinQin";
        }
        room->sendLog(log);
        yan->tag.remove("FuqinSource");
    }
};

class Jishi: public PhaseChangeSkill{
public:
    Jishi():PhaseChangeSkill("jishi"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->isWounded();
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::RoundStart)
            return false;
        Room *room = player->getRoom();
        QList<ServerPlayer *> ltys = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *lingtianyi, ltys){
            if(!player->isWounded())
                break;
            if(lingtianyi->isKongcheng())
                continue;
            const Card *card = room->askForCard(lingtianyi, "BasicCard,TrickCard", "@jishi:" + player->objectName(), QVariant::fromValue((PlayerStar)player), CardDiscarded);
            if(card){
                RecoverStruct lty;
                lty.card = card;
                lty.who = lingtianyi;

                room->playSkillEffect(objectName());
                LogMessage log;
                log.from = lingtianyi;
                log.to << player;
                log.type = "#UseSkill";
                log.arg = objectName();
                room->sendLog(log);

                room->recover(player, lty);
            }
        }
        return false;
    }
};

class Fengyue: public PhaseChangeSkill{
public:
    Fengyue():PhaseChangeSkill("fengyue"){
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool onPhaseChange(ServerPlayer *yinzei) const{
        if(yinzei->getPhase() == Player::Finish){
            Room *room = yinzei->getRoom();
            bool girl = room->getMenorWomen("female").isEmpty();
            if(!girl && room->askForSkillInvoke(yinzei, objectName())){
                room->playSkillEffect(objectName());
                yinzei->drawCards(1);
            }
        }
        return false;
    }
};

YanshouCard::YanshouCard(){
}

bool YanshouCard::targetFilter(const QList<const Player *> &targets, const Player *, const Player *Self) const{
    return targets.isEmpty();
}

void YanshouCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->broadcastInvoke("animate", "lightbox:$yanshou");
    effect.from->loseMark("@life");
    LogMessage log;
    log.type = "#Yanshou";
    log.from = effect.from;
    log.to << effect.to;
    log.arg = QString::number(1);

    room->sendLog(log);
    room->setPlayerProperty(effect.to, "maxhp", effect.to->getMaxHP() + 1);
}

class Yanshou: public ViewAsSkill{
public:
    Yanshou():ViewAsSkill("yanshou"){
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@life") > 0;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 2)
            return false;
        return to_select->getCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;
        YanshouCard *card = new YanshouCard;
        card->addSubcards(cards);
        return card;
    }
};

WujiCard::WujiCard(){
    target_fixed = true;
}

void WujiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    if(source->isAlive())
        room->drawCards(source, subcards.length());
}

class Hongjin: public TriggerSkill{
public:
    Hongjin():TriggerSkill("hongjin"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *hu3niang, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.to->getGeneral()->isMale()){
            hu3niang->tag["HongjinTarget"] = QVariant::fromValue((PlayerStar)damage.to);
            QString voly = damage.to->isDead() || damage.to->isNude() ? "draw+cancel" : "draw+throw+cancel";
            QString ball = room->askForChoice(hu3niang, objectName(), voly);
            if(ball == "cancel")
                return false;
            LogMessage log;
            log.type = "#InvokeSkill";
            log.from = hu3niang;
            log.arg = objectName();
            room->sendLog(log);

            if(ball == "throw"){
                room->playSkillEffect(objectName(), qrand() % 2 + 3);
                int card_id = room->askForCardChosen(hu3niang, damage.to, "he", objectName());
                room->throwCard(card_id);
            }
            else{
                room->playSkillEffect(objectName(), qrand() % 2 + 1);
                hu3niang->drawCards(1);
            }
        }
        hu3niang->tag.remove("HongjinTarget");
        return false;
    }
};

class Wuji:public ViewAsSkill{
public:
    Wuji():ViewAsSkill("wuji"){
    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *to_select) const{
        return to_select->getCard()->inherits("Slash");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        WujiCard *uji_card = new WujiCard;
        uji_card->addSubcards(cards);
        return uji_card;
    }
};

class EquiPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return player->hasEquip(card);
    }
    virtual bool willThrow() const{
        return false;
    }
};

class Heidian: public TriggerSkill{
public:
    Heidian():TriggerSkill("heidian"){
        events << Damaged << CardLost;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent v, Room* room, ServerPlayer *player, QVariant &data) const{
        QList<ServerPlayer *> sunny = room->findPlayersBySkillName(objectName());
        if(sunny.isEmpty())
            return false;
        LogMessage log;
        log.type = "#TriggerSkill";
        log.arg = objectName();
        foreach(ServerPlayer *sun, sunny){
            log.from = sun;
            if(v == Damaged){
                DamageStruct damage = data.value<DamageStruct>();
                if(damage.to == sun && damage.from && damage.from != damage.to &&
                   !damage.from->isKongcheng()){
                    room->playSkillEffect(objectName(), qrand() % 2 + 3);
                    room->sendLog(log);
                    if(!room->askForCard(damage.from, ".", "@heidian1:" + sun->objectName(), data, CardDiscarded))
                        room->throwCard(damage.from->getRandomHandCard());
                }
            }
            else if(v == CardLost){
                if(player == sun)
                    continue;
                if(player->isKongcheng()){
                    CardMoveStar move = data.value<CardMoveStar>();
                    if(move->from_place == Player::Hand && player->isAlive()){
                        room->playSkillEffect(objectName(), qrand() % 2 + 1);
                        room->sendLog(log);

                        const Card *card = player->getEquips().isEmpty() ? NULL :
                                           room->askForCard(player, ".Equi", "@heidian2:" + sun->objectName(), data, NonTrigger);
                        if(card)
                            sun->obtainCard(card);
                        else
                            room->loseHp(player);
                    }
                }
            }
        }
        return false;
    }
};

class Renrou: public TriggerSkill{
public:
    Renrou():TriggerSkill("renrou"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->isNude())
            return false;
        QList<ServerPlayer *> ernian = room->findPlayersBySkillName(objectName());
        if(ernian.isEmpty())
            return false;
        QVariant shiti = QVariant::fromValue((PlayerStar)player);
        foreach(ServerPlayer *erniang, ernian){
            if(erniang->isAlive() && room->askForSkillInvoke(erniang, objectName(), shiti)){
                room->playSkillEffect(objectName(), 1);
                int cardnum = player->getCardCount(true);
                erniang->obtainCard(player->getWeapon());
                erniang->obtainCard(player->getArmor());
                erniang->obtainCard(player->getDefensiveHorse());
                erniang->obtainCard(player->getOffensiveHorse());
                DummyCard *all_cards = player->wholeHandCards();
                if(all_cards){
                    room->moveCardTo(all_cards, erniang, Player::Hand, false);
                    delete all_cards;
                }
                QList<int> yiji_cards = erniang->handCards().mid(erniang->getHandcardNum() - cardnum);
                bool isyiji = false;
                while(room->askForYiji(erniang, yiji_cards))
                    isyiji = true;
                if(isyiji)
                    room->playSkillEffect(objectName(), 2);
            }
        }
        return false;
    }
};

class Hengxing:public DrawCardsSkill{
public:
    Hengxing():DrawCardsSkill("hengxing"){
        frequency = Frequent;
    }

    virtual int getDrawNum(ServerPlayer *qiu, int n) const{
        if(qiu->isWounded())
            return n;
        Room *room = qiu->getRoom();
        int death = room->getPlayers().length() - room->getAlivePlayers().length();
        if(death > 0 && room->askForSkillInvoke(qiu, objectName())){
            room->playSkillEffect(objectName());
            return n + qMin(death, 2);
        }else
            return n;
    }
};

CujuCard::CujuCard(){
}

void CujuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    DamageStruct damage = effect.from->tag["CujuDamage"].value<DamageStruct>();
    damage.to = effect.to;
    damage.chain = true;
    room->damage(damage);
}

class CujuViewAsSkill: public OneCardViewAsSkill{
public:
    CujuViewAsSkill():OneCardViewAsSkill("cuju"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@cuju";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        CujuCard *card = new CujuCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Cuju: public TriggerSkill{
public:
    Cuju():TriggerSkill("cuju"){
        events << DamagedProceed;
        view_as_skill = new CujuViewAsSkill;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *gaoqiu, QVariant &data) const{
        if(gaoqiu->askForSkillInvoke(objectName(), data)){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club|spade):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = gaoqiu;

            room->judge(judge);
            if(judge.isGood()){
                DamageStruct damage = data.value<DamageStruct>();
                gaoqiu->tag["CujuDamage"] = QVariant::fromValue(damage);
                if(room->askForUseCard(gaoqiu, "@@cuju", "@cuju-card"))
                    return true;
                gaoqiu->tag.remove("CujuDamage");
            }
        }
        return false;
    }
};

class Panquan: public TriggerSkill{
public:
    Panquan():TriggerSkill("panquan$"){
        events << HpRecover;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasLordSkill(objectName()) && target->getKingdom() == "guan";
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *ply, QVariant &data) const{
        ServerPlayer *gaoqiu = room->getLord();
        if(!gaoqiu->hasLordSkill(objectName()))
            return false;
        RecoverStruct recover = data.value<RecoverStruct>();
        for(int i = 0; i < recover.recover; i++){
            if(ply->askForSkillInvoke(objectName(), data)){
                gaoqiu->drawCards(2);
                room->playSkillEffect(objectName());
                const Card *ball = room->askForCardShow(gaoqiu, ply, objectName());
                room->moveCardTo(ball, NULL, Player::DrawPile);
            }
        }
        return false;
    }
};

JiashuCard::JiashuCard(){
    will_throw = false;
    once = true;
}

bool JiashuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void JiashuCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this, false);
    Room *room = effect.from->getRoom();

    Card::Suit suit = room->askForSuit(effect.from, "jiashu");
    LogMessage log;
    log.type = "#DeclareSuit";
    log.from = effect.from;
    QString suit_str = Suit2String(suit);
    log.arg = suit_str;
    room->sendLog(log);
    QString pattern = QString(".|%1|.|hand$").arg(suit_str);
    QString prompt = QString("@jiashu:%1::%2").arg(effect.from->objectName()).arg(suit_str);
    const Card *card = room->askForCard(effect.to, pattern, prompt, QVariant(), NonTrigger);
    if(card){
        effect.from->obtainCard(card);
        effect.to->drawCards(1);
    }
    else
        room->loseHp(effect.to);
}

class Jiashu: public OneCardViewAsSkill{
public:
    Jiashu():OneCardViewAsSkill("jiashu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("JiashuCard");
    }

    virtual bool viewFilter(const CardItem *book) const{
        return !book->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        JiashuCard *card = new JiashuCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

class Duoquan: public TriggerSkill{
public:
    Duoquan():TriggerSkill("duoquan"){
        events << Death;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;
        ServerPlayer *caijing = room->findPlayerBySkillName(objectName());
        if(caijing && caijing != killer && caijing->getMark("@quan") > 0){
            QVariant shiti = QVariant::fromValue((PlayerStar)player);
            if(!room->askForSkillInvoke(caijing, objectName(), shiti))
                return false;
            QStringList skills;
            foreach(const Skill *skill, player->getVisibleSkillList()){
                if(skill->parent() && skill->getLocation() == Skill::Right &&
                   skill->getFrequency() != Skill::Limited &&
                   skill->getFrequency() != Skill::Wake &&
                   !skill->isLordSkill()){
                    QString sk = skill->objectName();
                    skills << sk;
                }
            }
            if(!skills.isEmpty()){
                QString skill = room->askForChoice(caijing, objectName(), skills.join("+"));
                room->acquireSkill(caijing, skill);
            }
            room->playSkillEffect(objectName());
            room->broadcastInvoke("animate", "lightbox:$duoquan");
            caijing->loseMark("@quan");

            caijing->obtainCard(player->getWeapon());
            caijing->obtainCard(player->getArmor());
            caijing->obtainCard(player->getDefensiveHorse());
            caijing->obtainCard(player->getOffensiveHorse());
            DummyCard *all_cards = player->wholeHandCards();
            if(all_cards){
                room->obtainCard(caijing, all_cards, false);
                delete all_cards;
            }
        }
        return false;
    }
};

YongleCard::YongleCard(){
}

int YongleCard::getKingdoms(const Player *Self) const{
    QSet<QString> kingdom_set;
    QList<const Player *> players = Self->getSiblings();
    players << Self;
    foreach(const Player *player, players){
        if(player->isDead())
            continue;
        kingdom_set << player->getKingdom();
    }
    return kingdom_set.size();
}

bool YongleCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int x = getKingdoms(Self);
    return targets.length() < x && !to_select->isKongcheng();
}

bool YongleCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    int x = getKingdoms(Self);
    return targets.length() <= x && !targets.isEmpty();
}

void YongleCard::use(Room *room, ServerPlayer *fangla, const QList<ServerPlayer *> &targets) const{
    foreach(ServerPlayer *tmp, targets){
        const Card *card = tmp->getRandomHandCard();
        fangla->obtainCard(card, false);
    }
    foreach(ServerPlayer *tmp, targets){
        if(tmp->isDead())
            continue;
        const Card *card = room->askForCardShow(fangla, tmp, "yongle");
        tmp->obtainCard(card, false);
    }
}

class Yongle: public ZeroCardViewAsSkill{
public:
    Yongle():ZeroCardViewAsSkill("yongle"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YongleCard");
    }

    virtual const Card *viewAs() const{
        return new YongleCard;
    }
};

class Zhiyuan: public TriggerSkill{
public:
    Zhiyuan():TriggerSkill("zhiyuan$"){
        events << CardLost;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill(objectName());
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *fang1a, QVariant &data) const{
        if(fang1a->isKongcheng() && fang1a->isLord()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Hand){
                QList<ServerPlayer *> lieges = room->getLieges("jiang", fang1a);
                if(!lieges.isEmpty())
                    room->playSkillEffect(objectName());
                foreach(ServerPlayer *tmp, lieges){
                    const Card *card = room->askForCard(tmp, ".|.|.|hand$", "@zhiyuan:" + fang1a->objectName(), data, NonTrigger);
                    if(card){
                        LogMessage lo;
                        lo.type = "#InvokeSkill";
                        lo.from = tmp;
                        lo.arg = objectName();
                        room->sendLog(lo);
                        room->obtainCard(fang1a, card, false);
                    }
                }
            }
        }
        return false;
    }
};

class Qibing:public DrawCardsSkill{
public:
    Qibing():DrawCardsSkill("qibing"){
        frequency = Compulsory;
    }

    virtual int getDrawNum(ServerPlayer *wangq, int n) const{
        Room *room = wangq->getRoom();
        int todraw = qMin(wangq->getHp(), 4);
        if(todraw > 2)
            room->playSkillEffect(objectName(), qrand() % 2 + 1);
        else
            room->playSkillEffect(objectName(), qrand() % 2 + 3);

        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = wangq;
        log.arg = objectName();
        room->sendLog(log);

        return todraw;
    }
};

class Jiachu:public MasochismSkill{
public:
    Jiachu():MasochismSkill("jiachu$"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasLordSkill(objectName()) && target->getKingdom() == "min";
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        ServerPlayer *wangqing = room->getLord();
        if(!wangqing || !wangqing->hasLordSkill(objectName()))
            return;
        if(wangqing->isWounded() && player->getKingdom() == "min"
           && room->askForCard(player, ".H", "@jiachu:" + wangqing->objectName(), QVariant::fromValue(damage), CardDiscarded)){
            RecoverStruct rev;
            rev.who = player;
            room->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#InvokeSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);
            room->recover(wangqing, rev);
        }
    }
};

MeihuoCard::MeihuoCard(){
    once = true;
}

bool MeihuoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return to_select->getGeneral()->isMale() && to_select->isWounded();
}

void MeihuoCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;

    room->recover(effect.from, recover, true);
    room->recover(effect.to, recover, true);
}

class Meihuo: public OneCardViewAsSkill{
public:
    Meihuo():OneCardViewAsSkill("meihuo"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("MeihuoCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        MeihuoCard *card = new MeihuoCard;
        card->addSubcard(card_item->getCard()->getId());

        return card;
    }
};

class Zhensha:public TriggerSkill{
public:
    Zhensha():TriggerSkill("zhensha"){
        frequency = Limited;
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *, QVariant &data) const{
        ServerPlayer *xing = room->findPlayerBySkillName(objectName());
        if(!xing || xing->getMark("@vi") < 1)
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        if(!use.card->inherits("Analeptic") || !use.from->isWounded())
            return false;
        if(room->askForCard(xing, ".S", "@zhensha:" + use.from->objectName(), data, CardDiscarded)){
            xing->loseMark("@vi");
            room->playSkillEffect(objectName());
            room->broadcastInvoke("animate", "lightbox:$zhensha:2000");
            LogMessage log;
            log.from = xing;
            log.to << use.from;
            log.arg = objectName();
            room->loseMaxHp(use.from, use.from->getLostHp());
            if(use.from->isAlive())
                log.type = "#ZhenshaA";
            else
                log.type = "#ZhenshaD";
            room->sendLog(log);
            if(use.from->isDead())
                return true;
        }
        return false;
    }
};

class Shengui: public TriggerSkill{
public:
    Shengui():TriggerSkill("shengui"){
        events << CardEffected;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.from && effect.to == player && effect.from->getGeneral()->isMale()
            && effect.card->isNDTrick() && !effect.to->getArmor()){
            LogMessage log;
            log.type = "#ComskillNullify";
            log.from = effect.from;
            log.to << effect.to;
            log.arg = effect.card->objectName();
            log.arg2 = objectName();

            room->sendLog(log);
            room->playSkillEffect(objectName());
            return true;
        }
        return false;
    }
};

class Qinxin: public TriggerSkill{
public:
    Qinxin():TriggerSkill("qinxin"){
        events << PhaseChange;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &) const{
        if(player->getPhase() != Player::RoundStart || !player->askForSkillInvoke(objectName()))
            return false;
        Card::Suit suit = room->askForSuit(player, objectName());
        LogMessage log;
        log.type = "#DeclareSuit";
        log.from = player;
        log.arg = Card::Suit2String(suit);
        room->sendLog(log);

        JudgeStruct judge;
        judge.pattern = QRegExp("(.*):(" + Card::Suit2String(suit) + "):(.*)");
        judge.reason = objectName();
        judge.good = player->isWounded() ? true : false;
        judge.who = player;
        room->judge(judge);

        if(judge.card->getSuit() == suit){
            RecoverStruct rec;
            rec.who = player;
            room->recover(player, rec, true);
        }
        else
            player->obtainCard(judge.card);

        room->playSkillEffect(objectName());
        return false;
    }
};

YinjianCard::YinjianCard(){
    once = true;
    will_throw = false;
}

bool YinjianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    if(targets.length() >= 2 || !to_select->getGeneral()->isMale())
        return false;
    if(targets.length() == 1){
        QString kingdom = targets.first()->getKingdom();
        return to_select->getKingdom() != kingdom;
    }
    return true;
}

bool YinjianCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    return targets.length() == 2;
}

void YinjianCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *from = targets.first();
    ServerPlayer *to = targets.last();

    from->obtainCard(this, false);

    room->obtainCard(to, room->askForCardShow(from, source, "yinjian"), false);
}

class Yinjian: public ViewAsSkill{
public:
    Yinjian():ViewAsSkill("yinjian"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 2)
            return false;
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 2){
            YinjianCard *card = new YinjianCard();
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YinjianCard");
    }
};

SuocaiCard::SuocaiCard(){
    will_throw = false;
    mute = true;
}

bool SuocaiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select->getGeneral()->isMale();
}

void SuocaiCard::use(Room *o, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *yanp = targets.first();
    o->playSkillEffect("suocai", qrand() % 2 + 1);
    source->pindian(yanp, "suocai", this);
}

class SuocaiPindian: public OneCardViewAsSkill{
public:
    SuocaiPindian():OneCardViewAsSkill("suocai"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("SuocaiCard") && !player->isKongcheng();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        SuocaiCard *card = new SuocaiCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return ! to_select->isEquipped();
    }
};

class Suocai: public TriggerSkill{
public:
    Suocai():TriggerSkill("suocai"){
        events << Pindian;
        view_as_skill = new SuocaiPindian;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        if(player == pindian->from && pindian->reason == objectName()){
            if(pindian->isSuccess()){
                room->playSkillEffect(objectName(), 3);
                player->obtainCard(pindian->from_card);
                player->obtainCard(pindian->to_card);
            }
            else{
                room->playSkillEffect(objectName(), 4);
                DamageStruct damage;
                damage.from = pindian->to;
                damage.to = pindian->from;
                room->damage(damage);
            }
        }
        return false;
    }
};

class Huakui: public TriggerSkill{
public:
    Huakui():TriggerSkill("huakui"){
        frequency = Frequent;
        events << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *other, QVariant &data) const{
        QList<ServerPlayer *> lolita = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *loli, lolita){
            if(loli->distanceTo(other) < 2 && loli->askForSkillInvoke(objectName())){
                int card_id = room->drawCard();
                room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special, true);
                room->playSkillEffect(objectName());
                room->setEmotion(loli, "draw-card");
                room->getThread()->delay();

                LogMessage log;
                log.type = "$TakeAG";
                log.from = loli;
                log.card_str = QString::number(card_id);
                room->sendLog(log);

                room->obtainCard(loli, card_id);
            }
        }
        return false;
    }

    virtual int getPriority() const{
        return 2;
    }
};

StandardPackage::StandardPackage()
    :Package("standard")
{
    General *songjiang = new General(this, "songjiang$", "kou");
    songjiang->addSkill(new Ganlin);
    songjiang->addSkill(new Juyi);
    skills << new JuyiViewAsSkill;

    General *lujunyi = new General(this, "lujunyi", "guan");
    lujunyi->addSkill(new Baoguo);

    General *wuyong = new General(this, "wuyong", "kou", 3);
    wuyong->addSkill(new Huace);
    wuyong->addSkill(new Yunchou);

    General *gongsunsheng = new General(this, "gongsunsheng", "kou", 3);
    gongsunsheng->addSkill(new Yixing);
    gongsunsheng->addSkill(new Qimen);
    gongsunsheng->addSkill(new QimenClear);
    related_skills.insertMulti("qimen", "#qimencls");

    General *guansheng = new General(this, "guansheng", "jiang");
    guansheng->addSkill(new Huqi);
    guansheng->addSkill(new Tongwu);

    General *linchong = new General(this, "linchong", "jiang");
    linchong->addSkill(new Duijue);

    General *huarong = new General(this, "huarong", "guan");
    huarong->addSkill(new Jingzhun);
    huarong->addSkill(new Kaixian);
    patterns.insert(".kaixian!", new KaixianPattern);

    General *chaijin = new General(this, "chaijin", "guan", 3);
    chaijin->addSkill(new Danshu);
    chaijin->addSkill(new Haoshen);

    General *zhutong = new General(this, "zhutong", "guan");
    zhutong->addSkill(new Sijiu);
    zhutong->addSkill(new Yixian);

    General *luzhishen = new General(this, "luzhishen", "kou");
    luzhishen->addSkill(new Liba);
    luzhishen->addSkill(new Skill("zuohua", Skill::Compulsory));

    General *wusong = new General(this, "wusong", "kou");
    wusong->addSkill(new Fuhu);

    General *yangzhi = new General(this, "yangzhi", "guan");
    yangzhi->addSkill(new Maidao);
    skills << new MAIdao;
    yangzhi->addSkill(new Fengmang);

    General *xuning = new General(this, "xuning", "jiang");
    xuning->addSkill(new Goulian);
    xuning->addSkill(new Jinjia);

    General *daizong = new General(this, "daizong", "jiang", 3);
    daizong->addSkill(new Mitan);
    daizong->addSkill(new Jibao);

    General *likui = new General(this, "likui", "kou");
    likui->addSkill(new Shalu);

    General *ruanxiaoqi = new General(this, "ruanxiaoqi", "min");
    ruanxiaoqi->addSkill(new Jueming);
    ruanxiaoqi->addSkill(new JuemingEffect);
    related_skills.insertMulti("jueming", "#jueming_effect");
    ruanxiaoqi->addSkill(new Jiuhan);

    General *yangxiong = new General(this, "yangxiong", "jiang");
    yangxiong->addSkill(new Xingxing);

    General *yanqing = new General(this, "yanqing", "min", 3);
    yanqing->addSkill(new Dalei);
    yanqing->addSkill(new Fuqin);

    General *andaoquan = new General(this, "andaoquan", "min", 3);
    andaoquan->addSkill(new Jishi);
    andaoquan->addSkill(new Yanshou);
    andaoquan->addSkill(new MarkAssignSkill("@life", 1));
    related_skills.insertMulti("yanshou", "#@life-1");
    andaoquan->addSkill(new Fengyue);

    General *husanniang = new General(this, "husanniang", "jiang", 3, false);
    husanniang->addSkill(new Hongjin);
    husanniang->addSkill(new Wuji);

    General *sunerniang = new General(this, "sunerniang", "kou", 3, false);
    sunerniang->addSkill(new Heidian);
    sunerniang->addSkill(new Renrou);
    patterns[".Equi"] = new EquiPattern;

    General *gaoqiu = new General(this, "gaoqiu$", "guan", 3);
    gaoqiu->addSkill(new Hengxing);
    gaoqiu->addSkill(new Cuju);
    gaoqiu->addSkill(new Panquan);

    General *caijing = new General(this, "caijing", "guan");
    caijing->addSkill(new Jiashu);
    caijing->addSkill(new Duoquan);
    caijing->addSkill(new MarkAssignSkill("@quan", 1));
    related_skills.insertMulti("duoquan", "#@quan-1");

    General *fangla = new General(this, "fangla$", "jiang");
    fangla->addSkill(new Yongle);
    fangla->addSkill(new Zhiyuan);

    General *wangqing = new General(this, "wangqing$", "min");
    wangqing->addSkill(new Qibing);
    wangqing->addSkill(new Jiachu);

    General *panjinlian = new General(this, "panjinlian", "min", 3, false);
    panjinlian->addSkill(new Meihuo);
    panjinlian->addSkill(new Zhensha);
    panjinlian->addSkill(new Shengui);
    panjinlian->addSkill(new MarkAssignSkill("@vi", 1));
    related_skills.insertMulti("zhensha", "#@vi-1");

    General *lishishi = new General(this, "lishishi", "min", 3, false);
    lishishi->addSkill(new Qinxin);
    lishishi->addSkill(new Yinjian);

    General *yanxijiao = new General(this, "yanxijiao", "min", 3, false);
    yanxijiao->addSkill(new Suocai);
    yanxijiao->addSkill(new Huakui);

    addMetaObject<GanlinCard>();
    addMetaObject<JuyiCard>();
    addMetaObject<HuaceCard>();
    addMetaObject<YixingCard>();
    addMetaObject<DuijueCard>();
    addMetaObject<HaoshenCard>();
    addMetaObject<SijiuCard>();
    addMetaObject<MaidaoCard>();
    addMetaObject<MAIdaoCard>();
    addMetaObject<FengmangCard>();
    addMetaObject<DaleiCard>();
    addMetaObject<WujiCard>();
    addMetaObject<YanshouCard>();
    addMetaObject<CujuCard>();
    addMetaObject<JiashuCard>();
    addMetaObject<YongleCard>();
    addMetaObject<YinjianCard>();
    addMetaObject<MeihuoCard>();
    addMetaObject<SuocaiCard>();
}

ADD_PACKAGE(Standard)
