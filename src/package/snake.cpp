#include "snake.h"
#include "standard.h"
#include "skill.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"
#include "plough.h"
#include "maneuvering.h"

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

    virtual bool trigger(TriggerEvent e, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
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

class Zhanchi:public PhaseChangeSkill{
public:
    Zhanchi():PhaseChangeSkill("zhanchi"){
        frequency = Limited;
    }

    virtual bool onPhaseChange(ServerPlayer *opt) const{
        if(opt->hasMark("@wings") && opt->getPhase() == Player::Judge){
            Room *room = opt->getRoom();
            if(opt->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName(), 1);
                room->broadcastInvoke("animate", "lightbox:$zhanchi");
                while(!opt->getJudgingArea().isEmpty())
                    room->throwCard(opt->getJudgingArea().first()->getId());
                room->acquireSkill(opt, "tengfei");
                opt->loseMark("@wings");
            }
        }
        return false;
    }
};

class Tengfei: public ClientSkill{
public:
    Tengfei():ClientSkill("tengfei"){
    }

    virtual int getAtkrg(const Player *op) const{
        return op->getHp();
    }
};

class TengfeiMain:public PhaseChangeSkill{
public:
    TengfeiMain():PhaseChangeSkill("#tengfei_main"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *opt) const{
        if(opt->getPhase() == Player::NotActive){
            Room *room = opt->getRoom();
            if(opt->getMaxHP() > 3)
                room->playSkillEffect(objectName(), 1);
            else if(opt->getMaxHP() > 1)
                room->playSkillEffect(objectName(), 2);
            room->loseMaxHp(opt);

            if(opt->isAlive()){
                LogMessage log;
                log.type = "#Tengfei";
                log.from = opt;
                log.arg = objectName();
                room->sendLog(log);

                opt->gainAnExtraTurn(opt);
            }
        }
        return false;
    }
};

class Paohong: public FilterSkill{
public:
    Paohong():FilterSkill("paohong"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getCard();
        return card->objectName() == "slash" && card->isBlack();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *c = card_item->getCard();
        ThunderSlash *bs = new ThunderSlash(c->getSuit(), c->getNumber());
        bs->setSkillName(objectName());
        bs->addSubcard(card_item->getCard());
        return bs;
    }
};

SinueCard::SinueCard(){
}

bool SinueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return Self->distanceTo(to_select) == 1;
}

void SinueCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    LogMessage log;
    log.type = "#UseSkill";
    log.from = effect.from;
    log.to << effect.to;
    log.arg = "sinue";
    room->sendLog(log);

    DamageStruct damage;
    damage.damage = 2;
    damage.from = effect.from;
    damage.to = effect.to;
    room->damage(damage);
}

class SinueViewAsSkill: public OneCardViewAsSkill{
public:
    SinueViewAsSkill():OneCardViewAsSkill("sinue"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@sinue";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        SinueCard *card = new SinueCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return ! to_select->isEquipped();
    }
};

class Sinue: public TriggerSkill{
public:
    Sinue():TriggerSkill("sinue"){
        events << Death;
        view_as_skill = new SinueViewAsSkill;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;
        if(!killer || !killer->hasSkill(objectName()) || killer == player)
            return false;
        if(killer->getPhase() == Player::Play && !killer->isKongcheng())
            room->askForUseCard(killer, "@@sinue", "@sinue", true);
        return false;
    }
};

class Kongmen: public TriggerSkill{
public:
    Kongmen():TriggerSkill("kongmen"){
        events << CardLost;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *mowang, QVariant &data) const{
        if(mowang->isKongcheng()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Hand){
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = mowang;
                log.arg = objectName();
                room->playSkillEffect(objectName());
                room->sendLog(log);
                RecoverStruct o;
                o.card = Sanguosha->getCard(move->card_id);
                room->recover(mowang, o, true);
            }
        }
        return false;
    }
};

class Wudao: public PhaseChangeSkill{
public:
    Wudao():PhaseChangeSkill("wudao"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        if(target->getPhase() != Player::Start)
            return false;
        Room *room = target->getRoom();
        QList<ServerPlayer *> fanruis = room->findPlayersBySkillName(objectName());
        if(!fanruis.isEmpty()){
            foreach(ServerPlayer *fanrui, fanruis){
                if(!fanrui->hasMark("wudao_wake") && fanrui->isKongcheng())
                    return true;
            }
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> fanruis = room->findPlayersBySkillName(objectName());
        if(fanruis.isEmpty())
            return false;

        LogMessage log;
        log.type = "#WakeUp";
        log.arg = objectName();
        foreach(ServerPlayer *fanrui, fanruis){
            log.from = fanrui;
            room->sendLog(log);
            room->playSkillEffect(objectName());
            room->broadcastInvoke("animate", "lightbox:$wudao:2500");
            room->getThread()->delay(2500);

            room->drawCards(fanrui, 2);
            room->setPlayerMark(fanrui, "wudao_wake", 1);
            room->loseMaxHp(fanrui);
            room->acquireSkill(fanrui, "butian");
            room->acquireSkill(fanrui, "qimen");
        }
        return false;
    }
};

FangzaoCard::FangzaoCard(){
    once = true;
}

bool FangzaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void FangzaoCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    room->playSkillEffect("fangzao", 1);
    const Card *card = room->askForCardShow(effect.to, effect.from, "fangzao");
    int card_id = card->getEffectiveId();
    room->showCard(effect.to, card_id);

    if(card->getTypeId() == Card::Basic || card->isNDTrick()){
        room->setPlayerMark(effect.from, "fangzao", card_id);
        room->setPlayerFlag(effect.from, "fangzao");
    }else{
        room->setPlayerFlag(effect.from, "-fangzao");
    }
}

class Fangzao: public ViewAsSkill{
public:
    Fangzao():ViewAsSkill("fangzao"){

    }

    virtual int getEffectIndex(ServerPlayer *, const Card *card) const{
        if(card->getTypeId() == Card::Basic)
            return 2;
        else
            return 3;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->hasUsed("FangzaoCard") && player->hasFlag("fangzao")){
            int card_id = player->getMark("fangzao");
            const Card *card = Sanguosha->getCard(card_id);
            return card->isAvailable(player);
        }else
            return true;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if(player->getPhase() == Player::NotActive)
            return false;

        if(!player->hasFlag("fangzao"))
            return false;

        if(player->hasUsed("FangzaoCard")){
            int card_id = player->getMark("fangzao");
            const Card *card = Sanguosha->getCard(card_id);
            return pattern.contains(card->objectName());
        }else
            return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(Self->hasUsed("FangzaoCard") && selected.isEmpty() && Self->hasFlag("fangzao")){
            return !to_select->isEquipped();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(Self->hasUsed("FangzaoCard")){
            if(!Self->hasFlag("fangzao"))
                return false;

            if(cards.length() != 1)
                return NULL;

            int card_id = Self->getMark("fangzao");
            const Card *card = Sanguosha->getCard(card_id);
            const Card *first = cards.first()->getFilteredCard();

            Card *new_card = Sanguosha->cloneCard(card->objectName(), first->getSuit(), first->getNumber());
            new_card->addSubcards(cards);
            new_card->setSkillName(objectName());
            return new_card;
        }else{
            return new FangzaoCard;
        }
    }
};

class FangzaoMark: public TriggerSkill{
public:
    FangzaoMark():TriggerSkill("#fangzao_mark"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *jindajian, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->getSkillName() == "fangzao"){
            room->setPlayerFlag(jindajian, "-fangzao");
        }
        return false;
    }
};

class Jiangxin:public TriggerSkill{
public:
    Jiangxin():TriggerSkill("jiangxin"){
        frequency = Frequent;
        events << AskForRetrial;
    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *jindajian, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        CardStar card = judge->card;

        if(card->inherits("BasicCard") && jindajian->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            jindajian->drawCards(1);
            return true;
        }
        return false;
    }
};

class Longao: public TriggerSkill{
public:
    Longao():TriggerSkill("longao"){
        events << CardEffected;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *zouyuan = room->findPlayerBySkillName(objectName());
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!zouyuan || !effect.from || effect.from == zouyuan ||
           effect.multiple || !effect.card->isNDTrick())
            return false;

        if(!zouyuan->isNude() && room->askForSkillInvoke(zouyuan, objectName(), data)){
            room->askForDiscard(zouyuan, objectName(), 1, false, true);

            QList<ServerPlayer *> players = room->getOtherPlayers(effect.from), targets;
            foreach(ServerPlayer *player, players){
                if(player != effect.to)
                    targets << player;
            }

            QString choice = room->askForChoice(zouyuan, objectName(), "zhuan+qi");
            if(choice == "zhuan" && targets.length() > 0){
                room->playSkillEffect(objectName(), 1);
                ServerPlayer *target = room->askForPlayerChosen(zouyuan, targets, objectName());

                effect.from = effect.from;
                effect.to = target;
                data = QVariant::fromValue(effect);
            }
            if(choice == "qi" && !effect.from->isNude()){
                room->playSkillEffect(objectName(), 2);
                int to_throw = room->askForCardChosen(zouyuan, effect.from, "he", objectName());
                room->throwCard(to_throw, effect.from, zouyuan);
            }
        }
        return false;
    }
};

SnakePackage::SnakePackage()
    :Package("snake")
{
    General *muhong = new General(this, "muhong", "jiang");
    muhong->addSkill(new Wuzu);
    muhong->addSkill("huqi");

    General *oupeng = new General(this, "oupeng", "jiang", 5);
    oupeng->addSkill("#hp-1");
    oupeng->addSkill(new Zhanchi);
    oupeng->addSkill(new MarkAssignSkill("@wings", 1));
    related_skills.insertMulti("zhanchi", "#@wings-1");
    oupeng->addRelateSkill("tengfei");
    skills << new Tengfei << new TengfeiMain;
    related_skills.insertMulti("tengfei", "#tengfei_main");

    General *lingzhen = new General(this, "lingzhen", "jiang");
    lingzhen->addSkill(new Paohong);

    General *baoxu = new General(this, "baoxu", "kou");
    baoxu->addSkill(new Sinue);

    General *fanrui = new General(this, "fanrui", "kou", 3);
    fanrui->addSkill(new Kongmen);
    fanrui->addSkill(new Wudao);

    General *jindajian = new General(this, "jindajian", "min", 3);
    jindajian->addSkill(new Fangzao);
    jindajian->addSkill(new FangzaoMark);
    jindajian->addSkill(new Jiangxin);
    related_skills.insertMulti("fangzao", "#fangzao_mark");

    General *zouyan = new General(this, "zouyuan", "min");
    zouyan->addSkill(new Longao);

    addMetaObject<SinueCard>();
    addMetaObject<FangzaoCard>();
}

ADD_PACKAGE(Snake);
