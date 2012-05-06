#include "ttxd.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "room.h"
#include "maneuvering.h"

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

class Yinyu: public TriggerSkill{
public:
    Yinyu():TriggerSkill("yinyu"){
        events << PhaseChange << SlashProceed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *qing, QVariant &data) const{
        Room *room = qing->getRoom();
        if(event == SlashProceed){
            if(qing->hasFlag("Hitit")){
                SlashEffectStruct effect = data.value<SlashEffectStruct>();
                int index = effect.from->getMark("mengshi") > 0 ? 8: 3;
                room->playSkillEffect("yinyu", index);
                room->slashResult(effect, NULL);
                return true;
            }
            return false;
        }
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
                int index = 0;
                switch(judge.card->getSuit()){
                case Card::Heart:{
                        index = qing->getMark("mengshi") > 0 ? 10: 5;
                        room->playSkillEffect(objectName(), index);
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
                        index = qing->getMark("mengshi") > 0 ? 6: 1;
                        room->playSkillEffect(objectName(), index);
                        room->setPlayerFlag(qing, "SlashbySlash");
                        log.type = "#Yinyu4";
                        break;
                    }
                case Card::Club:{
                        index = qing->getMark("mengshi") > 0 ? 9: 4;
                        room->playSkillEffect(objectName(), index);
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
                judge.time_consuming = true;

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

    static bool isWuzuEffectCard(CardStar card){
        return card->inherits("Slash") ||
                card->inherits("AOE") ||
                card->inherits("FireAttack");
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
            if(play && isWuzuEffectCard(use.card))
                room->playSkillEffect(objectName());
        }
        else{
            foreach(ServerPlayer *tmp, use.to)
                tmp->removeMark("qinggang");
        }
        return false;
    }
};

class Qiangqu:public TriggerSkill{
public:
    Qiangqu():TriggerSkill("qiangqu"){
        events << DamageProceed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") && damage.to->getGeneral()->isFemale()
            && damage.to->isWounded() && !damage.to->isNude() && player->askForSkillInvoke(objectName(), data)){
            Room *room = player->getRoom();
            int card_id = room->askForCardChosen(damage.from, damage.to, "he", objectName());
            RecoverStruct re;
            re.card = Sanguosha->getCard(card_id);
            re.who = player;
            room->obtainCard(player, card_id, false);

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

    virtual bool trigger(TriggerEvent , ServerPlayer *target, QVariant &) const{
        PlayerStar player = target;
        Room *room = player->getRoom();
        ServerPlayer *lingtianyi = room->findPlayerBySkillName(objectName());
        if(!lingtianyi || lingtianyi->isKongcheng())
            return false;
        const Card *card = room->askForCard(lingtianyi, ".", "@jishi:" + target->objectName(), QVariant::fromValue(player), CardDiscarded);
        if(!card)
            return false;
        RecoverStruct lty;
        lty.card = card;
        lty.who = lingtianyi;

        room->playSkillEffect(objectName());
        LogMessage log;
        log.from = lingtianyi;
        log.to << target;
        log.type = "#UseSkill";
        log.arg = objectName();
        room->sendLog(log);

        room->recover(player, lty);
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
        if(!gaoqiu->isKongcheng() && gaoqiu->askForSkillInvoke(objectName(), data)){
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

    virtual bool trigger(TriggerEvent , ServerPlayer *ply, QVariant &data) const{
        Room *room = ply->getRoom();
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
            hu3niang->tag["HongjinTarget"] = QVariant::fromValue((PlayerStar)damage.to);
            QString voly = damage.to->isNude() ? "draw+cancel" : "draw+throw+cancel";
            QString ball = room->askForChoice(hu3niang, objectName(), voly);
            if(ball == "cancel")
                return false;
            LogMessage log;
            log.type = "#InvokeSkill";
            log.from = hu3niang;
            log.arg = objectName();
            room->sendLog(log);

            if(ball == "throw"){
                room->playSkillEffect(objectName(), 1);
                int card_id = room->askForCardChosen(hu3niang, damage.to, "he", objectName());
                room->throwCard(card_id);
            }
            else{
                room->playSkillEffect(objectName(), 2);
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
    muhong->addSkill("huqi");

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

    General *gaoqiu = new General(this, "gaoqiu$", "guan", 3);
    gaoqiu->addSkill(new Hengxing);
    gaoqiu->addSkill(new Cuju);
    gaoqiu->addSkill(new Panquan);

    General *husanniang = new General(this, "husanniang", "jiang", 3, false);
    husanniang->addSkill(new Hongjin);
    husanniang->addSkill(new Wuji);

    addMetaObject<HaoshenCard>();
    addMetaObject<HuanshuCard>();
    addMetaObject<CujuCard>();
    addMetaObject<WujiCard>();
    addMetaObject<YanshouCard>();
}

ADD_PACKAGE(TTXD)
