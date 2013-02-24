#include "gift.h"
#include "maneuvering.h"

Zongzi::Zongzi(Suit suit, int number):BasicCard(suit, number){
    setObjectName("zongzi");
    target_fixed = true;
}

QString Zongzi::getSubtype() const{
    return "gift_card";
}

QString Zongzi::getEffectPath(bool) const{
    return "audio/card/common/zongzi.ogg";
}

bool Zongzi::isAvailable(const Player *quyuan) const{
    if(ServerInfo.GameMode != "dusong")
        return !quyuan->hasMark("HaveEaten");
    else
        return true;
}

void Zongzi::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    int n = ServerInfo.GameMode != "dusong" ? 1 : -1;
    room->addHpSlot(source, n);
    room->acquireSkill(source, "lisao");
    room->setPlayerMark(source, "HaveEaten", 1);

    // do animation
    room->broadcastInvoke("animate", QString("zongzi:%1")
                          .arg(source->objectName()));
    //room->setEmotion(source, "zongzi");
}

class Lisao: public TriggerSkill{
public:
    Lisao():TriggerSkill("lisao"){
        events << Damaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , Room* room, ServerPlayer *player, QVariant &data) const{
        if(!player)
            return false;

        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") && damage.card->isRed()){
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);

            room->playSkillEffect(objectName(), player->getGeneral()->isMale() ? qrand() % 2 + 1 : qrand() % 2 + 3);
            room->loseMaxHp(player);
        }
        return false;
    }
};

Moonpie::Moonpie(Suit suit, int number):BasicCard(suit, number){
    setObjectName("moonpie");
}

QString Moonpie::getSubtype() const{
    return "gift_card";
}

bool Moonpie::isAvailable(const Player *change) const{
    return ServerInfo.GameMode != "dusong";
}

bool Moonpie::targetFilter(const QList<const Player *> &targets, const Player *houyi, const Player *change) const{
    return targets.isEmpty() && !houyi->hasMark("HaveEaten2") && change->inMyAttackRange(houyi);
}

void Moonpie::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    // do animation
    room->broadcastInvoke("animate", QString("moonpie:%1:%2")
                          .arg(effect.from->objectName())
                          .arg(effect.to->objectName()));

    room->acquireSkill(effect.to, "yaoyue");
    room->acquireSkill(effect.to, "sheri");
    room->setPlayerMark(effect.to, "HaveEaten2", 1);
}

class Yaoyue: public ClientSkill{
public:
    Yaoyue():ClientSkill("yaoyue"){
    }

    virtual int getExtra(const Player *target) const{
        if(!target->hasSkill(objectName()))
            return 0;
        else
            return 2;
    }
};

class YaoyueEffect: public PhaseChangeSkill{
public:
    YaoyueEffect():PhaseChangeSkill("#yaoyue-effect"){
    }

    virtual bool onPhaseChange(ServerPlayer *lz) const{
        if(lz->getPhase() == Player::Discard &&
           lz->getHandcardNum() > lz->getHp() && lz->getHandcardNum() <= lz->getMaxCards())
            lz->playSkillEffect("yaoyue", lz->getGeneral()->isMale() ? 1 : 2);
        return false;
    }
};

class Sheri:public TriggerSkill{
public:
    Sheri():TriggerSkill("sheri"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("TrickCard") && use.card->isRed()
            && !use.card->inherits("Nullification")){
            QList<PlayerStar> targets = use.to;
            if(use.to.isEmpty())
                targets << use.from;

            room->playSkillEffect(objectName(), player->getGeneral()->isMale() ? 1 : 2);
            LogMessage ogg;
            ogg.type = "#BeatJapan";
            ogg.from = player;
            ogg.arg = objectName();
            room->sendLog(ogg);
            room->throwCard(use.card);

            foreach(ServerPlayer *target, targets){
                bool chained = ! target->isChained();
                target->setChained(chained);
                room->broadcastProperty(target, "chained");
                room->setEmotion(target, "chain");
            }

            return true;
        }
        return false;
    }
};

class Change: public OneCardViewAsSkill{
public:
    Change():OneCardViewAsSkill("change"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Moonpie *moon = new Moonpie(card->getSuit(), card->getNumber());
        moon->setSkillName(objectName());
        moon->addSubcard(card_item->getFilteredCard());

        return moon;
    }
};

RiceBall::RiceBall(Suit suit, int number):BasicCard(suit, number){
    setObjectName("rice_ball");
}

QString RiceBall::getSubtype() const{
    return "gift_card";
}

bool RiceBall::isAvailable(const Player *quyuan) const{
    if(ServerInfo.GameMode != "dusong")
        return !quyuan->hasMark("HaveEaten3");
    else
        return true;
}

void RiceBall::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    // do animation
    room->broadcastInvoke("animate", QString("riceball:%1:%2")
                          .arg(effect.from->objectName())
                          .arg(effect.to->objectName()));
    room->getThread()->delay();
    room->jumpSeat(effect.from, effect.to, qrand() % 2);
    room->setPlayerMark(effect.from, "HaveEaten3", 1);
    PlayerStar pre, nex;
    foreach(ServerPlayer *tmp, room->getOtherPlayers(effect.from)){
        if(effect.from->getNextAlive() == tmp)
            nex = tmp;
        if(tmp->getNextAlive() == effect.from)
            pre = tmp;
    }
    LogMessage log;
    log.type = "#RiceBall";
    log.from = effect.from;
    log.to << pre << nex;
    room->sendLog(log);
}

GiftPackage::GiftPackage()
    :CardPackage("gift")
{
    skills << new Lisao << new Yaoyue << new YaoyueEffect << new Sheri
            << new Change;
    related_skills.insertMulti("yaoyue", "#yaoyue-effect");
    QList<Card *> cards;

    cards
            << new Zongzi(Card::Heart, 5)
            << new Moonpie(Card::Diamond, 5)
            << new RiceBall(Card::Club, 5)
            << new RiceBall(Card::Spade, 5)
            ;

    foreach(Card *card, cards)
        card->setParent(this);
}

ADD_PACKAGE(Gift)
