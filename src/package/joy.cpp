#include "joy.h"
#include "engine.h"

Shit::Shit(Suit suit, int number):BasicCard(suit, number){
    setObjectName("shit");

    target_fixed = true;
}

QString Shit::getSubtype() const{
    return "disgusting_card";
}

void Shit::onMove(const CardMoveStruct &move) const{
    ServerPlayer *from = move.from;
    if(from && move.from_place == Player::Hand &&
       from->getRoom()->getCurrent() == move.from
       && (move.to_place == Player::DiscardedPile || move.to_place == Player::Special)
       && move.to == NULL
       && from->isAlive()){

        LogMessage log;
        log.card_str = getEffectIdString();
        log.from = from;

        Room *room = from->getRoom();

        if(getSuit() == Spade){            
            log.type = "$ShitLostHp";
            room->sendLog(log);

            room->loseHp(from);

            return;
        }

        DamageStruct damage;
        damage.from = damage.to = from;
        damage.card = this;

        switch(getSuit()){
        case Club: damage.nature = DamageStruct::Thunder; break;
        case Heart: damage.nature = DamageStruct::Fire; break;
        default:
            damage.nature = DamageStruct::Normal;
        }

        log.type = "$ShitDamage";
        room->sendLog(log);

        room->damage(damage);
    }
}

bool Shit::HasShit(const Card *card){
    if(card->isVirtualCard()){
        QList<int> card_ids = card->getSubcards();
        foreach(int card_id, card_ids){
            const Card *c = Sanguosha->getCard(card_id);
            if(c->objectName() == "shit")
                return true;
        }

        return false;
    }else
        return card->objectName() == "shit";
}

class PiPattern: public CardPattern{
public:
    virtual bool match(const Player *, const Card *card) const{
        return card->inherits("Jink") || card->inherits("Assassinate");
    }
};

Stink::Stink(Suit suit, int number):BasicCard(suit, number){
    setObjectName("stink");
    target_fixed = true;
}

QString Stink::getSubtype() const{
    return "disgusting_card";
}

QString Stink::getEffectPath(bool is_male) const{
    return "audio/card/common/stink.ogg";
}

void Stink::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    ServerPlayer *nextfriend = targets.isEmpty() ? source->getNextAlive() : targets.first();
    room->setEmotion(nextfriend, "bad");
    const Card *pipi = room->askForCard(nextfriend, ".haochou", "@haochou:" + source->objectName(), QVariant::fromValue((PlayerStar)source));
    LogMessage log;
    log.from = nextfriend;

    if(!pipi){
        log.type = "#StinkSuccess";
        log.to << nextfriend->getNextAlive();
        room->sendLog(log);
        room->swapSeat(nextfriend, nextfriend->getNextAlive());
    }
    else if(!pipi->inherits("Jink")){
        DamageStruct damage;
        damage.from = nextfriend;
        damage.to = source;
        damage.card = pipi;
        room->setEmotion(nextfriend, "good");
        log.type = "#StinkHit";
        log.to << source;
        room->sendLog(log);
        room->damage(damage);
    }
    else{
        log.type = "#StinkJink";
        log.to << source;
        room->sendLog(log);
        room->setEmotion(nextfriend, "good");
    }
}


KusoPackage::KusoPackage()
    :Package("kuso"){
    QList<Card *> cards;

    cards << new Shit(Card::Club, 1)
            << new Shit(Card::Heart, 8)
            << new Shit(Card::Diamond, 13)
            << new Shit(Card::Spade, 10)
            << new Stink(Card::Diamond, 1);

    foreach(Card *card, cards)
        card->setParent(this);

    patterns[".haochou"] = new PiPattern;
    type = CardPack;
}

class GrabPeach: public TriggerSkill{
public:
    GrabPeach():TriggerSkill("grab_peach"){
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Peach")){
            Room *room = player->getRoom();
            QList<ServerPlayer *> players = room->getOtherPlayers(player);

            foreach(ServerPlayer *p, players){
                if(p->getOffensiveHorse() == parent() &&
                   p->askForSkillInvoke("grab_peach", data))
                {
                    room->throwCard(p->getOffensiveHorse());
                    room->playCardEffect(objectName(), p->getGeneral()->isMale());
                    p->obtainCard(use.card);

                    return true;
                }
            }
        }

        return false;
    }
};

Monkey::Monkey(Card::Suit suit, int number)
    :OffensiveHorse(suit, number)
{
    setObjectName("monkey");

    grab_peach = new GrabPeach;
    grab_peach->setParent(this);
}

void Monkey::onInstall(ServerPlayer *player) const{
    player->getRoom()->getThread()->addTriggerSkill(grab_peach);
}

void Monkey::onUninstall(ServerPlayer *player) const{

}

QString Monkey::getEffectPath(bool ) const{
    return "audio/card/common/monkey.ogg";
}

class GaleShellSkill: public ArmorSkill{
public:
    GaleShellSkill():ArmorSkill("gale-shell"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Fire){
            LogMessage log;
            log.type = "#GaleShellDamage";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            player->getRoom()->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

GaleShell::GaleShell(Suit suit, int number) :Armor(suit, number){
    setObjectName("gale-shell");
    skill = new GaleShellSkill;

    target_fixed = false;
}

bool GaleShell::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->distanceTo(to_select) <= 1;
}

void GaleShell::onUse(Room *room, const CardUseStruct &card_use) const{
    Card::onUse(room, card_use);
}

Poison::Poison(Suit suit, int number)
    : BasicCard(suit, number){
    setObjectName("poison");
}

QString Poison::getSubtype() const{
    return "attack_card";
}

QString Poison::getEffectPath(bool is_male) const{
    return "audio/card/common/poison.ogg";
}

bool Poison::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->distanceTo(to_select) <= 1;
}

void Poison::onEffect(const CardEffectStruct &card_effect) const{
    Room *room = card_effect.from->getRoom();

    LogMessage log;
    log.from = card_effect.to;
    if(card_effect.to->getMark("poison") == 0){
        room->setPlayerMark(card_effect.to, "poison", 1);
        room->setEmotion(card_effect.to, "bad");

        log.type = "#Poison_in";
        room->sendLog(log);
    }
    else{
        room->setPlayerMark(card_effect.to, "poison", 0);
        room->setEmotion(card_effect.to, "good");

        log.type = "#Poison_out";
        room->sendLog(log);
    }
}

JoyPackage::JoyPackage()
    :Package("joy")
{
    QList<Card *> cards;
    cards
                << new Monkey(Card::Diamond, 5)
                << new GaleShell(Card::Heart, 1)
                << new Poison(Card::Heart, 7)
                << new Poison(Card::Club, 9)
                << new Poison(Card::Diamond, 11)
                << new Poison(Card::Spade, 13);

    foreach(Card *card, cards)
            card->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(Kuso)
ADD_PACKAGE(Joy)
