#include "gift.h"
#include "skill.h"
#include "clientstruct.h"

Zongzi::Zongzi(Suit suit, int number):BasicCard(suit, number){
    setObjectName("zongzi");
    target_fixed = true;
}

QString Zongzi::getSubtype() const{
    return "gift_card";
}

QString Zongzi::getEffectPath(bool is_male) const{
    return "audio/card/common/zongzi.ogg";
}

bool Zongzi::isAvailable(const Player *quyuan) const{
    if(ServerInfo.GameMode != "dusong")
        return quyuan->getMark("HaveEaten") == 0;
    else
        return true;
}

void Zongzi::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    int n = ServerInfo.GameMode != "dusong" ? 1 : -1;
    room->setPlayerProperty(source, "maxhp", source->getMaxHp() + n);
    room->setPlayerProperty(source, "hp", source->getHp() + n);
    room->acquireSkill(source, "lisao");
    room->setPlayerMark(source, "HaveEaten", 1);

    room->setEmotion(source, "zongzi");
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

QString Moonpie::getEffectPath(bool is_male) const{
    return "audio/card/common/moonpie.ogg";
}

bool Moonpie::isAvailable(const Player *change) const{
    return ServerInfo.GameMode != "dusong";
}

bool Moonpie::targetFilter(const QList<const Player *> &targets, const Player *houyi, const Player *change) const{
    return targets.isEmpty() && houyi->getMark("HaveEaten2") == 0 && change->inMyAttackRange(houyi);
}

void Moonpie::onEffect(const CardEffectStruct &effect) const{
    //room->throwCard(this);
    Room *room = effect.from->getRoom();
    // do animation
    room->broadcastInvoke("animate", QString("moonpie:%1:%2")
                          .arg(effect.from->objectName())
                          .arg(effect.to->objectName()));

    room->acquireSkill(effect.to, "beatjapan");
    //room->acquireSkill(effect.from, "beatjapan");
    room->setPlayerMark(effect.to, "HaveEaten2", 1);
}

class BeatJapan: public ClientSkill{
public:
    BeatJapan():ClientSkill("beatjapan"){
    }

    virtual int getExtra(const Player *target) const{
        if(!target->hasSkill(objectName()))
            return 0;
        else
            return 2;
    }
};

#include "maneuvering.h"
class Beatjapan:public TriggerSkill{
public:
    Beatjapan():TriggerSkill("#beat-japan"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("TrickCard") && use.card->isRed()){
            QList<PlayerStar> targets = use.to;
            if(use.to.isEmpty())
                targets << use.from;

            room->playSkillEffect("beatjapan");
            LogMessage ogg;
            ogg.type = "#BeatJapan";
            ogg.from = player;
            ogg.arg = "beatjapan";
            ogg.arg2 = "iron_chain";
            ogg.to = targets;
            room->sendLog(ogg);
            room->throwCard(use.card);

            foreach(ServerPlayer *target, targets){
                bool chained = ! target->isChained();
                target->setChained(chained);
                room->broadcastProperty(target, "chained");
                room->setEmotion(target, "chain");
            }

            /*Card *card = new IronChain(use.card->getSuit(), use.card->getNumber());
            card->addSubcard(use.card);
            card->setSkillName("beatjapan");
            CardUseStruct usechange = use;
            usechange.card = card;
            room->useCard(usechange);*/
            return true;
        }
        return false;
    }
};

GiftPackage::GiftPackage()
    :Package("gift")
{
    skills << new Lisao << new BeatJapan << new Beatjapan;
    related_skills.insertMulti("beatjapan", "#beat-japan");
    QList<Card *> cards;

    cards
            << new Zongzi(Card::Heart, 5)
            << new Zongzi(Card::Club, 5)
            << new Moonpie(Card::Diamond, 5)
            << new Moonpie(Card::Spade, 5)
            ;

    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(Gift)
