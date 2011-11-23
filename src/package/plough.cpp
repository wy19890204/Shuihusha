#include "plough.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "god.h"
#include "standard.h"
#include "tocheck.h"

Ecstasy::Ecstasy(Suit suit, int number): BasicCard(suit, number)
{
    setObjectName("ecstasy");
}

bool Ecstasy::isAvailable(const Player *player) const{
    return !player->hasUsed("Ecstasy");
}

QString Ecstasy::getSubtype() const{
    return "attack_card";
}

bool Ecstasy::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return !targets.isEmpty();
}

bool Ecstasy::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return to_select != Self && Self->inMyAttackRange(to_select);
}

void Ecstasy::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->setPlayerFlag(effect.to, "Ecstasy");
}

Drivolt::Drivolt(Suit suit, int number)
    :SingleTargetTrick(suit, number, true) {
    setObjectName("drivolt");
}

bool Drivolt::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    if(to_select->getKingdom() == Self->getKingdom())
        return false;

    return to_select->getCardCount(true) >= 2;
}

void Drivolt::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    room->loseHp(effect.to);
    room->askForDiscard(effect.to, "Drivolt", 2, false, true);
    effect.to->drawCards(3);
}

Wiretap::Wiretap(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("wiretap");
}

bool Wiretap::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;
    return true;
}

void Wiretap::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->showAllCards(effect.to, effect.from);
}

Assassinate::Assassinate(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("assassinate");
}

bool Assassinate::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;
    return true;
}

void Assassinate::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    const Card *card1 = room->askForCard(effect.to, "jink", "Assassinate");
    const Card *card2;
    if(card1)
        card2 = room->askForCard(effect.to, "jink", "Assassinate");
    if(card1 && card2)
        effect.from->turnOver();
    else{
        DamageStruct dmae;
        dmae.card = this;
        dmae.from = effect.from;
        dmae.to = effect.to;
        room->damage(dmae);
    }
}

Counterplot::Counterplot(Suit suit, int number)
    :Nullification(suit, number)
{
    setObjectName("counterplot");
}

Provistore::Provistore(Suit suit, int number)
    :DelayedTrick(suit, number)
{
    setObjectName("provistore");
    target_fixed = false;

    judge.pattern = QRegExp("(.*):(diamond):(.*)");
    judge.good = true;
    judge.reason = objectName();
}

bool Provistore::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if(!targets.isEmpty())
        return false;
    if(to_select->containsTrick(objectName()))
        return false;
    return true;
}

void Provistore::takeEffect(ServerPlayer *target) const{
    target->skip(Player::Discard);
}

Treasury::Treasury(Suit suit, int number):Disaster(suit, number){
    setObjectName("treasury");

    judge.pattern = QRegExp("(.*):(heart|diamond):([JQKA])");
    judge.good = false;
    judge.reason = objectName();
}

void Treasury::takeEffect(ServerPlayer *target) const{
    //room->broadcastInvoke("animate", "treasury:" + target->objectName());
    target->getRoom()->broadcastInvoke("playAudio", "treasury");
    target->drawCards(5);
}

Tsunami::Tsunami(Suit suit, int number):Disaster(suit, number){
    setObjectName("tsunami");

    judge.pattern = QRegExp("(.*):(club|spade):([JQKA])");
    judge.good = false;
    judge.reason = objectName();
}

void Tsunami::takeEffect(ServerPlayer *target) const{
    //room->broadcastInvoke("animate", "tsunami:" + target->objectName());
    target->getRoom()->broadcastInvoke("playAudio", "tsunami");
    target->throwAllCards();
}

class DoubleWhipSkill : public WeaponSkill{
public:
    DoubleWhipSkill():WeaponSkill("double_whip"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardUseStruct effect = data.value<CardUseStruct>();
        Room *room = player->getRoom();
        if(effect.card->inherits("Slash") && player->askForSkillInvoke("double_whip")){
            foreach(ServerPlayer *effecto, effect.to){
                bool chained = ! effecto->isChained();
                effecto->setChained(chained);
                room->broadcastProperty(effecto, "chained");
            }
        }
        return false;
    }
};

DoubleWhip::DoubleWhip(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("double_whip");
    skill = new DoubleWhipSkill;
}

class MeteorSwordSkill : public WeaponSkill{
public:
    MeteorSwordSkill():WeaponSkill("meteor_sword"){
        events << Predamage;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        if(damage.card->inherits("Slash") && damage.to->isAlive()){
            room->loseHp(damage.to, damage.damage);
            return true;
        }
        return false;
    }
};

MeteorSword::MeteorSword(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("meteor_sword");
    skill = new MeteorSwordSkill;
}

SunBow::SunBow(Suit suit, int number)
    :Weapon(suit, number, 5)
{
    setObjectName("sun_bow");
}

class GoldArmorSkill: public ArmorSkill{
public:
    GoldArmorSkill():ArmorSkill("gold_armor"){
        events << Damaged << SlashEffected;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == SlashEffected){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(effect.nature != DamageStruct::Normal){
                LogMessage log;
                log.from = player;
                log.type = "#ArmorNullify";
                log.arg = objectName();
                log.arg2 = effect.slash->objectName();
                room->sendLog(log);

                return true;
            }
        }else if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.card->inherits("Slash")){
                LogMessage log;
                log.type = "#ThrowWeapon";
                log.from = player;
                log.arg = objectName();
                if(damage.from->getWeapon()){
                    room->sendLog(log);
                    room->throwCard(damage.from->getWeapon());
                }
            }
        }
        return false;
    }
};

GoldArmor::GoldArmor(Suit suit, int number):Armor(suit, number){
    setObjectName("gold_armor");
    skill = new GoldArmorSkill;
}

PloughPackage::PloughPackage()
    :Package("plough")
{
    type = CardPack;
    QList<Card *> cards;

    cards
    // spade
            << new Assassinate(Card::Spade, 1)
            << new SilverLion(Card::Spade, 2)
            << new MeteorSword(Card::Spade, 3)
            << new ThunderSlash(Card::Spade, 5)
            << new ThunderSlash(Card::Spade, 6)
            << new Counterplot(Card::Spade, 7)
            << new IronChain(Card::Spade, 8)
            << new Ecstasy(Card::Spade, 9)
            << new GoldArmor(Card::Spade, 10)
            << new Wiretap(Card::Spade, 11)
            << new IronChain(Card::Spade, 12)
            << new Counterplot(Card::Spade, 13)

    // diamond
            << new Tsunami(Card::Diamond, 1)
            << new Peach(Card::Diamond, 2)
            << new Peach(Card::Diamond, 3)
            << new FireSlash(Card::Diamond, 4)
            << new Jink(Card::Diamond, 5)
            << new Tsunami(Card::Diamond, 6)
            << new Wiretap(Card::Diamond, 7)
            << new Treasury(Card::Diamond, 8)
            << new Analeptic(Card::Diamond, 9)
            << new Jink(Card::Diamond, 10)
            << new SunBow(Card::Diamond, 11)
            << new Assassinate(Card::Diamond, 12)
            << new Counterplot(Card::Diamond, 13)

    // club
            << new Tsunami(Card::Club, 1)
            << new Ecstasy(Card::Club, 2)
            << new Ecstasy(Card::Club, 3)
            << new Analeptic(Card::Club, 4)
            << new Ecstasy(Card::Club, 5)
            << new Provistore(Card::Club, 6)
            << new DoubleWhip(Card::Club, 7)
            << new IronChain(Card::Club, 8)
            << new ThunderSlash(Card::Club, 9)
            << new GoldArmor(Card::Club, 10)
            << new IronChain(Card::Club, 11)
            << new Drivolt(Card::Club, 12)
            << new ArcheryAttack(Card::Club, 13)

    // heart
            << new Provistore(Card::Heart, 1)
            << new Jink(Card::Heart, 2)
            << new Analeptic(Card::Heart, 3)
            << new FireSlash(Card::Heart, 4)
            << new Peach(Card::Heart, 5)
            << new Jink(Card::Heart, 6)
            << new Wiretap(Card::Heart, 7)
            << new Ecstasy(Card::Heart, 8)
            << new Ecstasy(Card::Heart, 9)
            << new Peach(Card::Heart, 10)
            << new Counterplot(Card::Heart, 11)
            << new Drivolt(Card::Heart, 13);

    DefensiveHorse *white = new DefensiveHorse(Card::Heart, 12);
    white->setObjectName("white");
    OffensiveHorse *brown = new OffensiveHorse(Card::Spade, 4);
    brown->setObjectName("brown");

    cards << white << brown;
    foreach(Card *card, cards)
        card->setParent(this);
}

ADD_PACKAGE(Plough)
