#include "plough.h"
#include "maneuvering.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"

Ecstasy::Ecstasy(Suit suit, int number): BasicCard(suit, number)
{
    setObjectName("ecstasy");
}

bool Ecstasy::IsAvailable(const Player *player){
    return !player->hasUsed("Ecstasy");
}

bool Ecstasy::isAvailable(const Player *player) const{
    return IsAvailable(player);
}

QString Ecstasy::getSubtype() const{
    return "attack_card";
}

bool Ecstasy::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int cst_targets = 1;
    if(Self->hasSkill("xiayao"))
        cst_targets ++;

    if(targets.length() >= cst_targets)
        return false;
    return to_select != Self && Self->inMyAttackRange(to_select);
}

void Ecstasy::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    QString animation_str = QString("ecstasy:%1:%2")
                            .arg(effect.from->objectName()).arg(effect.to->objectName());
    room->broadcastInvoke("animate", animation_str);

    room->setPlayerFlag(effect.to, "ecst");
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

    return to_select->getKingdom() != Self->getKingdom();
}

void Drivolt::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    room->loseHp(effect.to);
    if(effect.to->isAlive()){
        room->askForDiscard(effect.to, "drivolt", qMin(2, effect.to->getCardCount(true)), false, true);
        effect.to->drawCards(3);
    }
}

Wiretap::Wiretap(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("wiretap");
}

void Wiretap::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    room->setTag("Wiretap", QVariant::fromValue(effect));
    QList<int> all = effect.to->handCards();
    //room->showAllCards(effect.to, effect.from);
    room->fillAG(all, effect.from);
    int mitan = room->askForAG(effect.from, all, true, "wiretap");
    if(effect.from->hasSkill("mitan") && mitan > -1){
        if(getSkillName() != "mitan")
            room->playSkillEffect("mitan", 2);
        room->showCard(effect.to, mitan);
    }
    effect.from->invoke("clearAG");
    room->removeTag("Wiretap");
}

Assassinate::Assassinate(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("assassinate");
}

void Assassinate::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    room->setEmotion(effect.from, "assassinate-b");
    room->setEmotion(effect.to, "assassinate-a");

    const Card *card1 = room->askForCard(effect.to, "jink", "@assas1:" + effect.from->objectName());
    const Card *card2;
    if(card1)
        card2 = room->askForCard(effect.to, "jink", "@assas2:" + effect.from->objectName());
    if(card1 && card2)
        effect.from->turnOver();
    else{
        DamageStruct dmae;
        dmae.card = this;
        dmae.from = effect.from;
        if(effect.to->hasSkill("huoshui")){
            room->broadcastInvoke("playAudio", "scream");
            LogMessage ogg;
            ogg.type = "#Huoshui";
            ogg.from = effect.to;
            ogg.arg = "huoshui";
            ogg.arg2 = objectName();
            room->sendLog(ogg);
            dmae.damage = 2;
        }
        dmae.to = effect.to;
        room->damage(dmae);
    }
}

Counterplot::Counterplot(Suit suit, int number)
    :Nullification(suit, number){
    setObjectName("counterplot");
}

Provistore::Provistore(Suit suit, int number)
    :DelayedTrick(suit, number)
{
    setObjectName("provistore");
    target_fixed = false;

    judge.pattern = QRegExp("(.*):(diamond):(.*)");
    judge.good = false;
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

bool Provistore::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(targets.isEmpty() && !Self->containsTrick(objectName()))
        return true;
    return targets.length() == 1;
}

void Provistore::takeEffect(ServerPlayer *target, bool good) const{
    if(good)
        target->skip(Player::Discard);
}

Treasury::Treasury(Suit suit, int number):Disaster(suit, number){
    setObjectName("treasury");

    judge.pattern = QRegExp("(.*):(heart|diamond):([JQKA])");
    judge.good = true;
    judge.reason = objectName();
}

void Treasury::takeEffect(ServerPlayer *target, bool good) const{
    if(good){
        //room->broadcastInvoke("animate", "treasury:" + target->objectName());
        target->getRoom()->broadcastInvoke("playAudio", "treasury");
        target->drawCards(5);
    }
}

Tsunami::Tsunami(Suit suit, int number):Disaster(suit, number){
    setObjectName("tsunami");

    judge.pattern = QRegExp("(.*):(club|spade):([JQKA])");
    judge.good = false;
    judge.reason = objectName();
}

void Tsunami::takeEffect(ServerPlayer *target, bool good) const{
    if(!good){
        Room *room = target->getRoom();
        room->broadcastInvoke("playAudio", "tsunami");
        room->setEmotion(target, "tsunami");
        target->throwAllCards();
    }
}

class DoubleWhipSkill : public WeaponSkill{
public:
    DoubleWhipSkill():WeaponSkill("double_whip"){
        events << CardEffect;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->inherits("Slash") && player->askForSkillInvoke("double_whip", data)){
            if(!effect.to->isChained())
                player->playCardEffect("Edouble_whip1", "weapon");
            else
                player->playCardEffect("Edouble_whip2", "weapon");
            bool chained = ! effect.to->isChained();
            effect.to->setChained(chained);
            room->broadcastProperty(effect.to, "chained");
            room->setEmotion(effect.to, "chain");
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

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card->inherits("Slash") && damage.to->isAlive()){
            player->playCardEffect("Emeteor_sword", "weapon");
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

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == SlashEffected){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(effect.nature != DamageStruct::Normal){
                player->playCardEffect("Egold_armor1", "armor");

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
            if(damage.card && damage.card->inherits("Slash")){
                LogMessage log;
                log.type = "#ThrowWeapon";
                log.from = player;
                log.arg = objectName();
                if(damage.from->getWeapon()){
                    player->playCardEffect("Egold_armor2", "armor");
                    room->sendLog(log);
                    room->throwCard(damage.from->getWeapon(), damage.from);
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
    :CardPackage("plough")
{
    QList<Card *> cards;

    cards
    // spade
            << new Assassinate(Card::Spade, 1)
            << new Tsunami(Card::Spade, 2)
            << new MeteorSword(Card::Spade, 3)
            << new Ecstasy(Card::Spade, 4)
            << new Ecstasy(Card::Spade, 5)
            << new Ecstasy(Card::Spade, 6)
            << new Slash(Card::Spade, 7)
            << new Slash(Card::Spade, 8)
            << new ThunderSlash(Card::Spade, 9)
            << new ThunderSlash(Card::Spade, 10)
            << new ThunderSlash(Card::Spade, 11)
            << new Wiretap(Card::Spade, 12)
            << new Drivolt(Card::Spade, 13)

    // diamond
            << new Dismantlement(Card::Diamond, 1)
            << new Peach(Card::Diamond, 2)
            << new FireSlash(Card::Diamond, 3)
            << new Slash(Card::Diamond, 4)
            << new Slash(Card::Diamond, 5)
            << new Jink(Card::Diamond, 6)
            << new Jink(Card::Diamond, 7)
            << new Treasury(Card::Diamond, 8)
            << new Analeptic(Card::Diamond, 9)
            << new Slash(Card::Diamond, 10)
            << new SunBow(Card::Diamond, 11)
            << new Assassinate(Card::Diamond, 12)
            << new Counterplot(Card::Diamond, 13)

    // club
            << new Provistore(Card::Club, 1)
            << new Ecstasy(Card::Club, 2)
            << new Ecstasy(Card::Club, 3)
            << new Slash(Card::Club, 4)
            << new Slash(Card::Club, 5)
            << new ThunderSlash(Card::Club, 6)
            << new DoubleWhip(Card::Club, 7)
            << new Analeptic(Card::Club, 9)
            << new GoldArmor(Card::Club, 10)
            << new Wiretap(Card::Club, 11)
            << new IronChain(Card::Club, 12)
            << new IronChain(Card::Club, 13)

    // heart
            << new Drivolt(Card::Heart, 1)
            << new FireSlash(Card::Heart, 2)
            << new Slash(Card::Heart, 3)
            << new Provistore(Card::Heart, 4)
            << new Jink(Card::Heart, 5)
            << new Jink(Card::Heart, 6)
            << new Jink(Card::Heart, 7)
            << new Jink(Card::Heart, 8)
            << new Analeptic(Card::Heart, 9)
            << new Peach(Card::Heart, 10)
            << new Peach(Card::Heart, 11)
            << new Counterplot(Card::Heart, 13);

    DefensiveHorse *jade = new DefensiveHorse(Card::Heart, 12);
    jade->setObjectName("jade");
    OffensiveHorse *brown = new OffensiveHorse(Card::Club, 8);
    brown->setObjectName("brown");

    cards << jade << brown;
    foreach(Card *card, cards)
        card->setParent(this);
}

// ex
Inspiration::Inspiration(Suit suit, int number)
    :GlobalEffect(suit, number)
{
    setObjectName("inspiration");
}

bool Inspiration::isCancelable(const CardEffectStruct &effect) const{
    return effect.to->isWounded();
}

void Inspiration::onEffect(const CardEffectStruct &effect) const{
    int x = qMin(3, effect.to->getLostHp());
    if(x > 0)
        effect.to->drawCards(x);
}

Haiqiu::Haiqiu(Card::Suit suit, int number)
    :OffensiveHorse(suit, number)
{
    setObjectName("haiqiu");
}

QString Haiqiu::getEffectPath(bool ) const{
    return "audio/card/common/haiqiu.ogg";
}

ADD_PACKAGE(Plough)
