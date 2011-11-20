#include "ttxd-package.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "room.h"
#include "maneuvering.h"

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
}

void JuyiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *song = targets.first();
    if(song->isKongcheng() && source->isKongcheng())
        return;
    if(song->hasSkill("juyi") && room->askForChoice(song, "jui", "agree+deny") == "agree"){
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

bool JuyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("juyi") && to_select != Self;
}

class JuyiViewAsSkill: public ZeroCardViewAsSkill{
public:
    JuyiViewAsSkill():ZeroCardViewAsSkill("jui"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasLordSkill("juyi")
                && player->getKingdom() == "qun"
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
                room->playSkillEffect(objectName(), 1);
                duck->drawCards(duck->getLostHp());
            }
        }
        else if(duck != player && !duck->isNude() && duck->askForSkillInvoke(objectName())){
            if(room->askForDiscard(duck, objectName(), 1, false, true)){
                room->playSkillEffect(objectName(), 2);
                damage.to = duck;
                room->damage(damage);
                return true;
            }
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
            int length = Self->getHandcardNum() / 2;
            return selected.length() < length;
        }
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(Self->getPhase() == Player::Play && cards.length() != Self->getHandcardNum() / 2)
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
                        room->setPlayerFlag(qing, "tianyi_success");
                        log.type = "#Yinyu1";
                        break;
                    }
                case Card::Diamond:{
                        room->playSkillEffect(objectName(), 3);
                        room->setPlayerFlag(qing, "Hitit");
                        log.type = "#Yinyu2";
                        break;
                    }
                case Card::Spade:{
                        room->playSkillEffect(objectName(), qrand() % 2 + 1);
                        room->attachSkillToPlayer(qing, "paoxiao");
                        qing->tag["Stone"] = "paoxiao";
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
            if(qing->hasFlag("tianyi_success"))
                room->setPlayerFlag(qing, "-tianyi_success");
            if(qing->hasFlag("Hitit"))
                room->setPlayerFlag(qing, "-Hitit");
            foreach(ServerPlayer *tmp, room->getOtherPlayers(qing))
                tmp->removeMark("qinggang");
            room->detachSkillFromPlayer(qing, qing->tag.value("Stone", "").toString());
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
                if(result == "no")
                    return false;
            }
            yuehe->obtainCard(judge->card);
            room->playSkillEffect(objectName(), 1);
            return true;
        }
        else
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
        if(damage.card->inherits("Slash") && damage.to->getGeneral()->isFemale()
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

HuatianAiCard::HuatianAiCard(){
    mute = true;
}

bool HuatianAiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    if(ClientInstance->getPattern().endsWith("ai"))
        return to_select != Self && to_select->isWounded();
    else
        return to_select != Self;
}

void HuatianAiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if(ClientInstance->getPattern().endsWith("ai")){
        RecoverStruct recovvv;
        recovvv.card = this;
        recovvv.who = effect.from;
        room->playSkillEffect("huatian", qrand() % 2 + 1);
        room->recover(effect.to, recovvv);
    }
    else{
        DamageStruct damag;
        damag.from = effect.from;
        damag.to = effect.to;
        damag.card = this;
        room->playSkillEffect("huatian", qrand() % 2 + 3);
        room->damage(damag);
    }
}

class HuatianAi: public ZeroCardViewAsSkill{
public:
    HuatianAi():ZeroCardViewAsSkill("huatian"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.startsWith("@@huatian");
    }

    virtual const Card *viewAs() const{
        return new HuatianAiCard;
    }
};

class Huatian:public TriggerSkill{
public:
    Huatian():TriggerSkill("huatian"){
        view_as_skill = new HuatianAi;
        events << Damaged << HpRecover;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            for(int i = 0; i < damage.damage; i++){
                if(!room->askForUseCard(player, "@@huatianai", "@huatianai"))
                    break;
            }
        }
        else{
            RecoverStruct rec = data.value<RecoverStruct>();
            for(int i = 0; i < rec.recover; i++){
                if(!room->askForUseCard(player, "@@huatiancuo", "@huatiancuo"))
                    break;
            }
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
    judge.reason = "huanshu";
    judge.who = effect.to;
    room->judge(judge);
    ichi = judge.card->isRed();
    room->judge(judge);
    me = judge.card->isRed();

    DamageStruct damage;
    damage.damage = 2;
    damage.from = effect.from;
    damage.to = effect.to;
    if(ichi == true && me == true){
        damage.nature = DamageStruct::Fire;
        room->playSkillEffect("huanshu", qrand() % 2 + 1);
        room->damage(damage);
    }
    else if(ichi == false && me == false){
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
        return !target->hasSkill(objectName()) && target->getKingdom() == "wei";
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *ply, QVariant &data) const{
        Room *room = ply->getRoom();
        ServerPlayer *gaoqiu = room->findPlayerBySkillName(objectName());
        if(!gaoqiu)
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
            else
                hu3niang->drawCards(1);
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
{ //guan == wei, jiang == shu, min == wu, kou == qun
    General *songjiang = new General(this, "songjiang$", "qun");
    songjiang->addSkill(new Ganlin);
    songjiang->addSkill(new Juyi);
    skills << new JuyiViewAsSkill;

    General *lujunyi = new General(this, "lujunyi", "wei");
    lujunyi->addSkill(new Baoguo);

    General *chaijin = new General(this, "chaijin", "wei", 3);
    chaijin->addSkill(new Danshu);
    chaijin->addSkill(new Haoshen);

    General *zhangqing = new General(this, "zhangqing", "wei");
    zhangqing->addSkill(new Yinyu);

    General *yuehe = new General(this, "yuehe", "wu", 3);
    yuehe->addSkill(new Yueli);
    yuehe->addSkill(new Taohui);

    General *muhong = new General(this, "muhong", "shu");
    muhong->addSkill(new Wuzu);
    muhong->addSkill(new Huqi);

    General *zhoutong = new General(this, "zhoutong", "qun", 3);
    zhoutong->addSkill(new Qiangqu);
    zhoutong->addSkill(new Huatian);

    General *qiaodaoqing = new General(this, "qiaodaoqing", "qun", 3);
    qiaodaoqing->addSkill(new Huanshu);
    qiaodaoqing->addSkill(new Mozhang);

    General *andaoquan = new General(this, "andaoquan", "wu", 3);
    General *gongsunsheng = new General(this, "gongsunsheng", "qun", 3);

    General *gaoqiu = new General(this, "gaoqiu$", "wei", 3);
    gaoqiu->addSkill(new Hengxing);
    gaoqiu->addSkill(new Cuju);
    gaoqiu->addSkill(new Panquan);

    General *husanniang = new General(this, "husanniang", "shu", 3, false);
    husanniang->addSkill(new Hongjin);
    husanniang->addSkill(new Wuji);

    addMetaObject<GanlinCard>();
    addMetaObject<JuyiCard>();
    addMetaObject<HaoshenCard>();
    addMetaObject<HuatianAiCard>();
    addMetaObject<HuanshuCard>();
    addMetaObject<CujuCard>();
    addMetaObject<WujiCard>();
}

ADD_PACKAGE(TTXD)
