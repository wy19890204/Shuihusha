#include "standard.h"
#include "skill.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "qlfd-package.h"

YushuiCard::YushuiCard(){
    once = true;
}

bool YushuiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return to_select->getGeneral()->isMale() && to_select->isWounded();
}

void YushuiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;

    room->recover(effect.from, recover, true);
    room->recover(effect.to, recover, true);
/*
    int index = -1;
    if(effect.from->getGeneral()->isMale()){
        if(effect.from == effect.to)
            index = 5;
        else if(effect.from->getHp() >= effect.to->getHp())
            index = 3;
        else
            index = 4;
    }else{
        index = 1 + qrand() % 2;
    }

    room->playSkillEffect("Yushui", index);
*/
    effect.from->drawCards(2);
    effect.to->drawCards(2);
    effect.from->turnOver();
    effect.to->turnOver();
}

class Yushui: public OneCardViewAsSkill{
public:
    Yushui():OneCardViewAsSkill("yushui"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("YushuiCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        YushuiCard *card = new YushuiCard;
        card->addSubcard(card_item->getCard()->getId());

        return card;
    }
};

class Shengui: public ProhibitSkill{
public:
    Shengui():ProhibitSkill("shengui"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        return !to->faceUp() && from->getGeneral()->isMale() && card->inherits("TrickCard") && !card->inherits("Collateral");
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

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *xing = room->findPlayerBySkillName(objectName());
        if(!xing || xing->getMark("@vi") < 1)
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Analeptic") && room->askForCard(xing, ".S", "@zhensha:" + use.from->objectName(), data)){
            xing->loseMark("@vi");

            LogMessage log;
            log.type = "#Zhensha";
            log.from = xing;
            log.to << use.from;
            log.arg = objectName();
            room->sendLog(log);
            room->killPlayer(use.from);
        }
        return false;
    }
};

FanwuCard::FanwuCard(){
}

bool FanwuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select->getGeneral()->isMale();
}

void FanwuCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);
    effect.from->tag["Fanwu"] = QVariant::fromValue(effect.to);
}

class FanwuViewAsSkill: public OneCardViewAsSkill{
public:
    FanwuViewAsSkill():OneCardViewAsSkill("fanwu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@fanwu";
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        FanwuCard *card = new FanwuCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

class Fanwu: public TriggerSkill{
public:
    Fanwu():TriggerSkill("fanwu"){
        events << Predamage;
        view_as_skill = new FanwuViewAsSkill;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!player->isNude() && damage.to != damage.from){
            Room *room = player->getRoom();
            if(room->askForUseCard(player, "@@fanwu", "@fanwu")){
                ServerPlayer *target = player->tag["Fanwu"].value<PlayerStar>();
                if(target){
                    damage.from = target;
                    data = QVariant::fromValue(damage);
                }
                player->tag.remove("Fanwu");
            }
        }
        return false;
    }
};

class Foyuan: public TriggerSkill{
public:
    Foyuan():TriggerSkill("foyuan"){
        events << CardEffected;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.to == player && effect.from->getGeneral()->isMale() && effect.from->getEquips().isEmpty()
            && (effect.card->isNDTrick() || effect.card->inherits("Slash"))){
            LogMessage log;
            log.type = "#Foyuan";
            log.from = effect.from;
            Room *room = player->getRoom();
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

class Panxin: public TriggerSkill{
public:
    Panxin():TriggerSkill("panxin"){
        events << PreDeath;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        ServerPlayer *qiaoyun = room->findPlayerBySkillName(objectName());
        if(!qiaoyun || qiaoyun->isLord() || player->isLord())
            return false;
        if(qiaoyun->getMark("@pfxl") && qiaoyun->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());
            room->broadcastInvoke("animate", "lightbox:$panxin");
            QString role = player->getRole();
            player->setRole(qiaoyun->getRole());
            room->setPlayerProperty(player, "panxin", true);
            qiaoyun->loseMark("@pfxl");
            qiaoyun->setRole(role);
            qiaoyun->drawCards(1);
        }
        return false;
    }
};

QLFDPackage::QLFDPackage()
    :Package("QLFD")
{
    General *panjinlian = new General(this, "panjinlian", "min", 3, false);
    panjinlian->addSkill(new Yushui);
    panjinlian->addSkill(new Zhensha);
    panjinlian->addSkill(new Shengui);
    panjinlian->addSkill(new MarkAssignSkill("@vi", 1));
    related_skills.insertMulti("zhensha", "#@vi-1");

    General *panqiaoyun = new General(this, "panqiaoyun", "min", 3, false);
    panqiaoyun->addSkill(new Fanwu);
    panqiaoyun->addSkill(new Panxin);
    panqiaoyun->addSkill(new MarkAssignSkill("@pfxl", 1));
    related_skills.insertMulti("panxin", "#@pfxl-1");
    panqiaoyun->addSkill(new Foyuan);

    addMetaObject<YushuiCard>();
    addMetaObject<FanwuCard>();
}

ADD_PACKAGE(QLFD);
