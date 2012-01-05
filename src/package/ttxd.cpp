#include "ttxd.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "room.h"
#include "tocheck.h"

GanlinCard::GanlinCard(){
    will_throw = false;
}

void GanlinCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();

    room->moveCardTo(this, target, Player::Hand, false);
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
            room->moveCardTo(card1, song, Player::Hand, false);
            delete card1;
        }
        room->getThread()->delay();

        if(card2){
            room->moveCardTo(card2, source, Player::Hand, false);
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
        ServerPlayer *duck = room->findPlayerBySkillName(objectName());
        if(!duck)
            return false;
        if(evt == Damaged){
            if(duck == player && duck->isWounded() && duck->askForSkillInvoke(objectName())){
                if(duck->getMark("baoguo") == 0)
                    room->playSkillEffect(objectName(), 1);
                duck->drawCards(duck->getLostHp());
            }
            duck->setMark("baoguo", 0);
        }
        else if(duck != player && !duck->isNude() && damage.damage > 0
            && room->askForCard(duck, "..", "@baoguo:" + player->objectName() + ":" + QString::number(damage.damage), data)){
            room->playSkillEffect(objectName(), 2);
            damage.to = duck;
            duck->setMark("baoguo", 1);
            room->damage(damage);
            return true;
        }
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
        return to_select != Self && to_select->getHandcardNum() != to_select->getMaxHP();
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
        target->drawCards(num);
    }
    else if(chaijin->getPhase() == Player::Play){
        target->obtainCard(this);
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

class Yinyu: public PhaseChangeSkill{
public:
    Yinyu():PhaseChangeSkill("yinyu"){

    }

    virtual bool onPhaseChange(ServerPlayer *qing) const{
        Room *room = qing->getRoom();
        if(qing->getPhase() == Player::Start){
            if(qing->askForSkillInvoke(objectName())){
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(.*):(.*)");
                judge.good = true;
                judge.who = qing;
                judge.reason = objectName();
                room->judge(judge);

                LogMessage log;
                log.from = qing;
                switch(judge.card->getSuit()){
                case Card::Heart:{
                        room->playSkillEffect(objectName(), 5);
                        room->setPlayerFlag(qing, "Longest");
                        log.type = "#Yinyu1";
                        break;
                    }
                case Card::Diamond:{
                        //room->playSkillEffect(objectName(), 3);
                        room->setPlayerFlag(qing, "Hitit");
                        log.type = "#Yinyu2";
                        break;
                    }
                case Card::Spade:{
                        room->playSkillEffect(objectName(), 1);
                        room->setPlayerFlag(qing, "SlashbySlash");
                        log.type = "#Yinyu4";
                        break;
                    }
                case Card::Club:{
                        room->playSkillEffect(objectName(), 4);
                        foreach(ServerPlayer *tmp, room->getOtherPlayers(qing))
                            tmp->addMark("qinggang");
                        log.type = "#Yinyu8";
                        break;
                    }
                default:
                    break;
                }
                room->sendLog(log);
            }
        }
        else if(qing->getPhase() == Player::NotActive){
            if(qing->hasFlag("Longest"))
                room->setPlayerFlag(qing, "-Longest");
            if(qing->hasFlag("Hitit"))
                room->setPlayerFlag(qing, "-Hitit");
            if(qing->hasFlag("SlashbySlash"))
                room->setPlayerFlag(qing, "-SlashbySlash");
            foreach(ServerPlayer *tmp, room->getOtherPlayers(qing))
                tmp->removeMark("qinggang");
        }
        return false;
    }
};

class Yueli:public TriggerSkill{
public:
    Yueli():TriggerSkill("yueli"){
        frequency = Frequent;
        events << FinishJudge;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *yuehe, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        CardStar card = judge->card;

        QVariant data_card = QVariant::fromValue(card);
        Room *room = yuehe->getRoom();
        if(judge->card->inherits("BasicCard") && yuehe->askForSkillInvoke(objectName(), data_card)){
            if(card->objectName() == "shit"){
                QString result = room->askForChoice(yuehe, objectName(), "yes+no");
                if(result == "no"){
                    room->playSkillEffect(objectName(), 2);
                    return false;
                }
            }
            yuehe->obtainCard(judge->card);
            if(judge->reason != "taohui")
                room->playSkillEffect(objectName(), 1);
            return true;
        }
        if(judge->reason != "taohui")
            room->playSkillEffect(objectName(), 2);
        return false;
    }
};

class Taohui:public TriggerSkill{
public:
    Taohui():TriggerSkill("taohui"){
        events << PhaseChange << FinishJudge;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yuehe, QVariant &data) const{
        if(event == PhaseChange && yuehe->getPhase() == Player::Finish){
            Room *room = yuehe->getRoom();
            while(yuehe->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());

                JudgeStruct judge;
                judge.reason = objectName();
                judge.who = yuehe;

                room->judge(judge);
                if(judge.card->inherits("BasicCard"))
                    break;
            }
        }
        else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(judge->reason == objectName()){
                if(!judge->card->inherits("BasicCard")){
                    Room *room = yuehe->getRoom();
                    room->throwCard(judge->card->getId());
                    ServerPlayer *target = room->askForPlayerChosen(yuehe, room->getAllPlayers(), objectName());
                    target->drawCards(1);
                    return true;
                }
            }
        }
        return false;
    }
};

class Wuzu: public TriggerSkill{
public:
    Wuzu():TriggerSkill("wuzu"){
        events << CardUsed << CardFinished;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent e, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        Room *room = player->getRoom();
        if(e == CardUsed){
            bool play = false;
            foreach(ServerPlayer *tmp, use.to){
                if(tmp->getArmor()){
                    tmp->addMark("qinggang");
                    LogMessage log;
                    log.type = "$IgnoreArmor";
                    log.from = player;
                    log.to << tmp;
                    log.card_str = tmp->getArmor()->getEffectIdString();
                    room->sendLog(log);

                    play = true;
                }
            }
            if(play)
                room->playSkillEffect(objectName());
        }
        else{
            foreach(ServerPlayer *tmp, use.to)
                tmp->removeMark("qinggang");
        }
        return false;
    }
};

class Huqi: public DistanceSkill{
public:
    Huqi():DistanceSkill("huqi")
    {
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(from->hasSkill(objectName()))
            return -1;
        else
            return 0;
    }
};

class Qiangqu:public TriggerSkill{
public:
    Qiangqu():TriggerSkill("qiangqu"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") && damage.to->getGeneral()->isFemale()
            && damage.to->isWounded() && player->askForSkillInvoke(objectName())){
            Room *room = player->getRoom();
            int card = room->askForCardChosen(damage.from, damage.to, "he", objectName());
            RecoverStruct re;
            re.card = Sanguosha->getCard(card);
            re.who = player;
            room->obtainCard(player, card);

            LogMessage log;
            log.from = player;
            log.type = "#Qiangqu";
            log.to << damage.to;
            room->sendLog(log);
            room->recover(damage.to, re);
            room->playSkillEffect(objectName());
            room->recover(damage.from, re);
            return true;
        }
        return false;
    }
};

class Huatian:public TriggerSkill{
public:
    Huatian():TriggerSkill("huatian"){
        events << Damaged << HpRecovered;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            for(int i = 0; i < damage.damage; i++){
                QList<ServerPlayer *> wounders;
                foreach(ServerPlayer *tmp, room->getOtherPlayers(damage.to)){
                    if(tmp->isWounded())
                        wounders << tmp;
                }
                if(!wounders.isEmpty()){
                    room->setPlayerMark(player, "HBTJ", 1);
                    if(!damage.to->askForSkillInvoke(objectName())){
                        room->setPlayerMark(player, "HBTJ", 0);
                        break;
                    }
                    ServerPlayer *target = room->askForPlayerChosen(player, wounders, objectName());
                    room->setPlayerMark(player, "HBTJ", 0);
                    RecoverStruct recovvv;
                    recovvv.who = player;
                    room->playSkillEffect(objectName(), qrand() % 2 + 1);
                    room->recover(target, recovvv);
                }
            }
            return false;
        }
        RecoverStruct rec = data.value<RecoverStruct>();
        for(int i = rec.recover; i > 0; i--){
            if(!player->askForSkillInvoke(objectName()))
                break;
            room->setPlayerMark(player, "HBTJ", 2);
            ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
            room->setPlayerMark(player, "HBTJ", 0);

            room->playSkillEffect(objectName(), qrand() % 2 + 3);
            DamageStruct damage;
            damage.from = player;
            damage.to = target;
            room->damage(damage);
        }
        return false;
    }
};

HuanshuCard::HuanshuCard(){
    mute = true;
}

bool HuanshuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select != Self;
}

void HuanshuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    bool ichi, me;

    JudgeStruct judge;
    judge.reason = "huanshu1";
    judge.who = effect.to;
    room->judge(judge);
    ichi = judge.card->isRed();

    judge.reason = "huanshu2";
    judge.pattern = ichi ? QRegExp("(.*):(heart|diamond):(.*)"): QRegExp("(.*):(club|spade):(.*)");
    judge.good = false;
    room->judge(judge);
    me = judge.card->isRed();

    DamageStruct damage;
    damage.damage = 2;
    damage.from = effect.from;
    damage.to = effect.to;
    if(ichi && me){
        damage.nature = DamageStruct::Fire;
        room->playSkillEffect("huanshu", qrand() % 2 + 1);
        room->damage(damage);
    }
    else if(!ichi && !me){
        room->playSkillEffect("huanshu", qrand() % 2 + 1);
        damage.nature = DamageStruct::Thunder;
        room->damage(damage);
    }
    else
        room->playSkillEffect("huanshu", 3);
}

class HuanshuViewAsSkill: public ZeroCardViewAsSkill{
public:
    HuanshuViewAsSkill():ZeroCardViewAsSkill("huanshu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@huanshu";
    }

    virtual const Card *viewAs() const{
        return new HuanshuCard;
    }
};

class Huanshu: public MasochismSkill{
public:
    Huanshu():MasochismSkill("huanshu"){
        view_as_skill = new HuanshuViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *qiaodaoq, const DamageStruct &damage) const{
        Room *room = qiaodaoq->getRoom();
        int x = damage.damage, i;
        for(i=0; i<x; i++){
            if(!room->askForUseCard(qiaodaoq, "@@huanshu", "@huanshu"))
                break;
        }
    }
};

class Jishi: public TriggerSkill{
public:
    Jishi():TriggerSkill("jishi"){
        events << TurnStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->isWounded();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        ServerPlayer *lingtianyi = room->findPlayerBySkillName(objectName());
        if(!lingtianyi || lingtianyi->isKongcheng())
            return false;
        if(lingtianyi->askForSkillInvoke(objectName(), QVariant::fromValue(player))){
            RecoverStruct lty;
            lty.card = room->askForCardShow(lingtianyi, player, objectName());
            lty.who = lingtianyi;
            room->throwCard(lty.card);
            room->playSkillEffect(objectName());
            room->recover(player, lty);
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
            int girl = room->getMenorWomen("female").length();
            if(girl > 0 && room->askForSkillInvoke(yinzei, objectName())){
                room->playSkillEffect(objectName());
                yinzei->drawCards(qMin(girl, 2));
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

YixingCard::YixingCard(){
}

bool YixingCard::targetFilter(const QList<const Player *> &targets, const Player *to, const Player *Self) const{
    return targets.isEmpty() && !to->getEquips().isEmpty();
}

void YixingCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    ServerPlayer *target = effect.to;
    int card_id = room->askForCardChosen(effect.from, target, "e", "yixing");
    effect.from->tag["YixingCard"] = card_id;
    effect.from->tag["YixingTarget"] = QVariant::fromValue(effect.to);
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
            if(!tmp->getEquips().isEmpty())
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

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        ServerPlayer *dragon = room->findPlayerBySkillName(objectName());
        if(!dragon)
            return false;
        if(target->getPhase() == Player::NotActive){
            foreach(ServerPlayer *tmp, room->getOtherPlayers(dragon)){
                if(tmp->getMark("Qimen_target") > 0){
                    stopCry(room, tmp);

                    LogMessage log;
                    log.type = "#QimenEnd";
                    log.from = dragon;
                    log.to << tmp;
                    log.arg = objectName();

                    room->sendLog(log);
                    break;
                }
            }
            return false;
        }
        else if(target->getPhase() == Player::Start){
            if(room->askForSkillInvoke(dragon, objectName(), QVariant::fromValue(target))){
                ServerPlayer *superman = room->askForPlayerChosen(dragon, room->getOtherPlayers(dragon), objectName());
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(.*):(.*)");
                judge.reason = objectName();
                judge.who = superman;

                room->judge(judge);
                QString suit_str = judge.card->getSuitString();
                QString pattern = QString("..%1").arg(suit_str.at(0).toUpper());
                QString prompt = QString("@qimen:%1::%2").arg(superman->getGeneralName()).arg(suit_str);
                if(room->askForCard(dragon, pattern, prompt)){
                    room->playSkillEffect(objectName());
                    LogMessage log;
                    log.type = "#Qimen";
                    log.from = dragon;
                    log.to << superman;
                    log.arg = objectName();
                    room->sendLog(log);

                    willCry(room, superman, dragon);
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
        events << Predamaged;
        view_as_skill = new CujuViewAsSkill;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *gaoqiu, QVariant &data) const{
        if(!gaoqiu->isKongcheng() && gaoqiu->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club|spade):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = gaoqiu;

            Room *room = gaoqiu->getRoom();
            room->judge(judge);
            if(judge.isGood()){
                DamageStruct damage = data.value<DamageStruct>();
                gaoqiu->tag["CujuDamage"] = QVariant::fromValue(damage);
                if(room->askForUseCard(gaoqiu, "@@cuju", "@cuju-card"))
                    return true;
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
        return !target->hasSkill(objectName()) && target->getKingdom() == "guan";
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *ply, QVariant &data) const{
        Room *room = ply->getRoom();
        ServerPlayer *gaoqiu = room->findPlayerBySkillName(objectName());
        if(!gaoqiu || !gaoqiu->hasLordSkill(objectName()))
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

    virtual bool trigger(TriggerEvent , ServerPlayer *hu3niang, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.to->isAlive() && damage.to->getGeneral()->isMale()){
            Room *room = hu3niang->getRoom();
            QString ball = room->askForChoice(hu3niang, objectName(), "draw+throw+cancel");
            if(ball == "cancel")
                return false;
            LogMessage log;
            log.type = "#InvokeSkill";
            log.from = hu3niang;
            log.arg = objectName();
            room->sendLog(log);

            if(ball == "throw"){
                room->playSkillEffect(objectName(), 1);
                room->throwCard(room->askForCardChosen(hu3niang, damage.to, "he", objectName()));
            }
            else{
                room->playSkillEffect(objectName(), 2);
                hu3niang->drawCards(1);
            }
        }
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

class Mozhang: public PhaseChangeSkill{
public:
    Mozhang():PhaseChangeSkill("mozhang"){
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool onPhaseChange(ServerPlayer *p) const{
        if(p->getPhase() == Player::NotActive){
            Room *room = p->getRoom();
            if(!p->isChained()){
                p->setChained(true);
                room->playSkillEffect(objectName());
                LogMessage log;
                log.type = "#Mozhang";
                log.from = p;
                log.arg = objectName();
                room->sendLog(log);

                room->broadcastProperty(p, "chained");
            }
        }
        return false;
    }
};

TTXDPackage::TTXDPackage()
    :Package("TTXD")
{
    General *songjiang = new General(this, "songjiang$", "kou");
    songjiang->addSkill(new Ganlin);
    songjiang->addSkill(new Juyi);
    skills << new JuyiViewAsSkill;

    General *lujunyi = new General(this, "lujunyi", "guan");
    lujunyi->addSkill(new Baoguo);

    General *chaijin = new General(this, "chaijin", "guan", 3);
    chaijin->addSkill(new Danshu);
    chaijin->addSkill(new Haoshen);

    General *zhangqing = new General(this, "zhangqing", "guan");
    zhangqing->addSkill(new Yinyu);

    General *yuehe = new General(this, "yuehe", "min", 3);
    yuehe->addSkill(new Yueli);
    yuehe->addSkill(new Taohui);

    General *muhong = new General(this, "muhong", "jiang");
    muhong->addSkill(new Wuzu);
    muhong->addSkill(new Huqi);

    General *zhoutong = new General(this, "zhoutong", "kou", 3);
    zhoutong->addSkill(new Qiangqu);
    zhoutong->addSkill(new Huatian);

    General *qiaodaoqing = new General(this, "qiaodaoqing", "kou", 3);
    qiaodaoqing->addSkill(new Huanshu);
    qiaodaoqing->addSkill(new Mozhang);

    General *andaoquan = new General(this, "andaoquan", "min", 3);
    andaoquan->addSkill(new Jishi);
    andaoquan->addSkill(new Yanshou);
    andaoquan->addSkill(new MarkAssignSkill("@life", 1));
    related_skills.insertMulti("yanshou", "#@life-1");
    andaoquan->addSkill(new Fengyue);

    General *gongsunsheng = new General(this, "gongsunsheng", "kou", 3);
    gongsunsheng->addSkill(new Yixing);
    gongsunsheng->addSkill(new Qimen);
    gongsunsheng->addSkill(new QimenClear);
    related_skills.insertMulti("qimen", "#qimencls");

    General *gaoqiu = new General(this, "gaoqiu$", "guan", 3);
    gaoqiu->addSkill(new Hengxing);
    gaoqiu->addSkill(new Cuju);
    gaoqiu->addSkill(new Panquan);

    General *husanniang = new General(this, "husanniang", "jiang", 3, false);
    husanniang->addSkill(new Hongjin);
    husanniang->addSkill(new Wuji);

    addMetaObject<GanlinCard>();
    addMetaObject<JuyiCard>();
    addMetaObject<HaoshenCard>();
    addMetaObject<HuanshuCard>();
    addMetaObject<CujuCard>();
    addMetaObject<YixingCard>();
    addMetaObject<WujiCard>();
    addMetaObject<YanshouCard>();
}

ADD_PACKAGE(TTXD)
