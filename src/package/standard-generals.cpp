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
}

void GanlinCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();

    room->obtainCard(target, this, false);
    int n = source->getLostHp() - source->getHandcardNum();
    if(n > 0 && source->askForSkillInvoke("ganlin")){
        source->drawCards(n);
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
}

void JuyiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    ServerPlayer *song = room->getLord();
    if(!song->hasLordSkill("juyi") || song == source)
        return;
    if(song->isKongcheng() && source->isKongcheng())
        return;
    if(room->askForChoice(song, "jui", "agree+deny") == "agree"){
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
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent evt, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        QList<ServerPlayer *> ducks = room->findPlayersBySkillName(objectName());
        if(ducks.isEmpty())
            return false;
        foreach(ServerPlayer *duck, ducks){
            if(evt == Damaged){
                if(duck == player && duck->isWounded() && duck->askForSkillInvoke(objectName())){
                    if(duck->getMark("baoguo") == 0)
                        room->playSkillEffect(objectName(), 1);
                    duck->drawCards(duck->getLostHp());
                }
                duck->setMark("baoguo", 0);
            }
            else if(duck != player && !duck->isNude() && damage.damage > 0
                && room->askForCard(duck, ".", "@baoguo:" + player->objectName() + "::" + QString::number(damage.damage), data, CardDiscarded)){
                room->playSkillEffect(objectName(), 2);
                LogMessage log;
                log.type = "#Baoguo";
                log.from = duck;
                log.to << damage.to;
                log.arg = objectName();
                log.arg2 = QString::number(damage.damage);
                room->sendLog(log);

                damage.to = duck;
                duck->setMark("baoguo", 1);
                room->damage(damage);
                return true;
            }
        }
        return false;
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

class Huace:public OneCardViewAsSkill{
public:
    Huace():OneCardViewAsSkill("huace"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("HuaceCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("TrickCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
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

    virtual bool trigger(TriggerEvent event, ServerPlayer *feiwu, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->isNDTrick()){
            Room *room = feiwu->getRoom();
            if(room->askForSkillInvoke(feiwu, objectName())){
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

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
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

    virtual int getPriority() const{
        return 2;
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
        attachskills << "spear" << "axe" << "jui" << "maida0";
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
        else if(target->getPhase() == Player::Start){
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

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
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

    virtual bool trigger(TriggerEvent, ServerPlayer *erge, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(!effect.to->isNude() && effect.jink){
            Room *room = erge->getRoom();
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
}

bool DuijueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    if(to_select->hasSkill("fangzhen") && Self->getHp() > to_select->getHp())
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

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.card || !damage.card->inherits("Slash"))
            return false;
        if(event == Damaged && damage.from == player)
            return false;
        Room *room = player->getRoom();
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
        if(huarong->getPhase() == Player::Start && !huarong->isKongcheng()){
            bool caninvoke = false;
            foreach(const Card *cd, huarong->getHandcards()){
                if(cd->getNumber() <= 5){
                    caninvoke = true;
                    break;
                }
            }
            if(caninvoke && room->askForSkillInvoke(huarong, objectName())){
                const Card *card = room->askForCard(huarong, ".kaixian!", "@kaixian", QVariant(), NonTrigger);
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

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.card->inherits("Slash") && effect.to->isWounded()){
            Room *room = player->getRoom();

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
        if(num > 2)
            room->playSkillEffect("haoshen", 1);
        else
            room->playSkillEffect("haoshen", 3);
        target->drawCards(qMin(5, num));
    }
    else if(chaijin->getPhase() == Player::Play){
        target->obtainCard(this, false);
        if(this->getSubcards().length() > 2)
            room->playSkillEffect("haoshen", 2);
        else
            room->playSkillEffect("haoshen", 4);
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

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to == damage.from || damage.damage < 1)
            return false;
        if(!damage.to->isAllNude() && player->askForSkillInvoke(objectName(), data)){
            Room *room = player->getRoom();
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

    addMetaObject<GanlinCard>();
    addMetaObject<JuyiCard>();
    addMetaObject<HuaceCard>();
    addMetaObject<YixingCard>();
    addMetaObject<DuijueCard>();
    addMetaObject<HaoshenCard>();
    addMetaObject<SijiuCard>();
}

ADD_PACKAGE(Standard)
