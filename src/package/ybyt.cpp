#include "ybyt.h"
#include "standard.h"
#include "skill.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"

class SWPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return card->inherits("Slash") || card->inherits("Weapon");
    }
    virtual bool willThrow() const{
        return false;
    }
};

YuanpeiCard::YuanpeiCard(){
}

bool YuanpeiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select->getGeneral()->isMale() &&
            (!to_select->isKongcheng() || (to_select->isKongcheng() && to_select->getWeapon()));
}

void YuanpeiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    const Card *card = room->askForCard(effect.to, ".Yuanp", "@yuanpei:" + effect.from->objectName());
    if(card){
        effect.from->obtainCard(card);
        effect.from->drawCards(1);
        effect.to->drawCards(1);
    }
    else
        room->acquireSkill(effect.from, "yuanpei_slash");
}

class Yuanpei: public ZeroCardViewAsSkill{
public:
    Yuanpei():ZeroCardViewAsSkill("yuanpei"){
    }

    virtual const Card *viewAs() const{
        return new YuanpeiCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YuanpeiCard");
    }
};

class YuanpeiS1ash:public OneCardViewAsSkill{
public:
    YuanpeiS1ash():OneCardViewAsSkill(""){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "slash";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Card *slash = new Slash(card->getSuit(), card->getNumber());
        slash->addSubcard(card->getId());
        slash->setSkillName(objectName());
        return slash;
    }
};

class YuanpeiSlash: public PhaseChangeSkill{
public:
    YuanpeiSlash():PhaseChangeSkill("yuanpei_slash"){
        view_as_skill = new YuanpeiS1ash;
    }

    virtual bool onPhaseChange(ServerPlayer *p) const{
        if(p->getPhase() == Player::NotActive){
            p->getRoom()->detachSkillFromPlayer(p, "yuanpei_slash");
            p->loseSkill("yuanpei_slash");
        }
        return false;
    }
};

class Mengshi: public PhaseChangeSkill{
public:
    Mengshi():PhaseChangeSkill("mengshi"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("mengshi") == 0
                && target->getPhase() == Player::Start
                && target->getHandcardNum() < target->getAttackRange();
    }

    virtual bool onPhaseChange(ServerPlayer *qyyy) const{
        Room *room = qyyy->getRoom();

        LogMessage log;
        log.type = "#WakeUp";
        log.from = qyyy;
        log.arg = objectName();
        room->sendLog(log);
        room->playSkillEffect(objectName());
        room->broadcastInvoke("animate", "lightbox:$mengshi:1500");
        room->getThread()->delay(1500);

        qyyy->drawCards(3);
        room->acquireSkill(qyyy, "yinyu");
        room->setPlayerMark(qyyy, "mengshi", 1);
        return false;
    }
};

YBYTPackage::YBYTPackage()
    :Package("YBYT")
{
    General *qiongying = new General(this, "qiongying", "jiang", 3, false);
    qiongying->addSkill(new Yuanpei);
    patterns[".Yuanp"] = new SWPattern;
    qiongying->addSkill(new Mengshi);
    skills << new YuanpeiSlash;

    addMetaObject<YuanpeiCard>();
}

ADD_PACKAGE(YBYT);
