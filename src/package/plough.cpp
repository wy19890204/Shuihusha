#include "plough.h"
#include "maneuvering.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "standard.h"
#include "standard-equips.h"

Discuss::Discuss(Suit suit, int number)
    :AOE(suit, number)
{
    setObjectName("discuss");
}

void Discuss::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *trick = room->askForCard(effect.to, "TrickCard", "discuss-trick:" + effect.from->objectName(), QVariant::fromValue(effect));
    if(!trick)
        room->loseHp(effect.to);
}

Burn::Burn(Suit suit, int number)
    :SingleTargetTrick(suit, number, true) {
    setObjectName("burn");
}

bool Burn::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select->hasEquip() && to_select != Self;
}

void Burn::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    QString choice = room->askForChoice(effect.to, objectName(), "throw+cancel");
    if(choice == "throw")
        effect.to->throwAllEquips();
    else{
        DamageStruct damage;
        damage.from = effect.from;
        damage.to = effect.to;
        room->damage(damage);
    }
}

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
    return targets.length() <= 1;
}

void Provistore::takeEffect(ServerPlayer *target, bool good) const{
    if(good)
        target->skip(Player::Discard);
}

class SnowStopSkill : public WeaponSkill{
public:
    SnowStopSkill():WeaponSkill("snow_stop"){
        events << SlashMissed;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if(!player->askForSkillInvoke(objectName()))
            return false;
        const Card *card;
        if(player->isKongcheng())
            card = Sanguosha->getCard(room->drawCard());
        else
            card = room->askForCardShow(effect.from, effect.from, objectName());
        room->showCard(effect.from, card->getEffectiveId());

        if(card->getSuit() == Card::Heart){
            room->throwCard(card, effect.from);
            room->loseHp(effect.to);
        }

        return false;
    }
};

SnowStop::SnowStop(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("snow_stop");
    skill = new SnowStopSkill;
}

class SpaceAskSkill : public WeaponSkill{
public:
    SpaceAskSkill():WeaponSkill("space_ask"){
        events << Damaged;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.from && !damage.from->isKongcheng() && !damage.to->isKongcheng() &&
           player->askForSkillInvoke(objectName())){
            bool success = player->pindian(damage.from, objectName());
            if(success){
                ThunderSlash *slash = new ThunderSlash(Card::NoSuit, 0);
                slash->setSkillName(objectName());
                CardUseStruct use;
                use.card = slash;
                use.from = player;
                use.to << damage.from;

                room->useCard(use);
            }
        }
        return false;
    }
};

SpaceAsk::SpaceAsk(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("space_ask");
    skill = new SpaceAskSkill;
}

void SpaceAsk::onMove(const CardMoveStruct &move) const{
    if(move.from_place == Player::Equip && move.from->isAlive()){
        Room *room = move.from->getRoom();

        bool invoke = move.from->askForSkillInvoke("space-lost");
        if(!invoke)
            return;

        ServerPlayer *target = room->askForPlayerChosen(move.from, room->getOtherPlayers(move.from), "space-lost");
        DamageStruct damage;
        damage.from = move.from;
        damage.to = target;
        damage.nature = DamageStruct::Thunder;
        damage.card = this;

        room->damage(damage);
    }
}

class SevenStarSkill: public PhaseChangeSkill{
public:
    SevenStarSkill():PhaseChangeSkill("seven_star"){
    }

    virtual int getPriority() const{
        return -1;
    }

    static void Exchange(ServerPlayer *shenzhuge){
        Room *room = shenzhuge->getRoom();
        QList<int> stars;
        foreach(QString tmp, room->getTag("stars").toString().split("+"))
            stars << tmp.toInt();
        if(stars.isEmpty())
            return;

        room->fillAG(stars, shenzhuge);

        int n = 0;
        while(!stars.isEmpty()){
            int card_id = room->askForAG(shenzhuge, stars, true, "seven_star");
            if(card_id == -1)
                break;

            stars.removeOne(card_id);
            ++n;

            room->obtainCard(shenzhuge, card_id, false);
        }

        shenzhuge->invoke("clearAG");

        if(n == 0)
            return;

        const Card *exchange_card = room->askForExchange(shenzhuge, "seven_star", n);

        foreach(int card_id, exchange_card->getSubcards()){
            room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special);
            stars << card_id;
        }

        QStringList data;
        foreach(int n, stars)
            data << QString::number(n);
        room->setTag("stars", QVariant::fromValue(data.join("+")));

        LogMessage log;
        log.type = "#QixingExchange";
        log.from = shenzhuge;
        log.arg = QString::number(n);
        log.arg2 = "qixing";
        room->sendLog(log);

        delete exchange_card;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::RoundStart){
            Exchange(player);
        }

        return false;
    }
};

SevenStar::SevenStar(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("seven_star");
    skill = new SevenStarSkill;
}

void SevenStar::onInstall(ServerPlayer *player) const{
    Room *room = player->getRoom();
    QList<int> stars;
    foreach(QString tmp, room->getTag("stars").toString().split("+"))
        stars << tmp.toInt();
    if(stars.isEmpty()){
        QList<int> card_ids = room->getNCards(7);
        foreach(int card_id, card_ids){
            room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special);
            stars << card_id;
        }
        QStringList data;
        foreach(int n, stars)
            data << QString::number(n);
        room->setTag("stars", QVariant::fromValue(data.join("+")));
    }
    room->getThread()->addTriggerSkill(skill);
}

class RainbowSkill: public WeaponSkill{
public:
    RainbowSkill():WeaponSkill("rainbow"){
        events << CardFinished << CardResponsed << FinishJudge;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
            return false;

        CardStar card = NULL;
        bool caninvoke = false;
        if(event == CardFinished){
            CardUseStruct card_use = data.value<CardUseStruct>();
            card = card_use.card;

            if(card == player->tag["MoonSpearSlash"].value<CardStar>()){
                card = NULL;
            }
            if(card->isBlack()){
                if(!player->hasFlag("retrial"))
                    caninvoke = true;
                else
                    room->setPlayerFlag(player, "invokelater");
            }
        }else if(event == CardResponsed){
            card = data.value<CardStar>();
            player->tag["MoonSpearSlash"] = data;
            if(card != NULL && card->isBlack()){
                if(!player->hasFlag("retrial"))
                    caninvoke = true;
                else
                    room->setPlayerFlag(player, "invokelater");
            }
        }else if(event == FinishJudge){
            if(player->hasFlag("invokelater")){
                room->setPlayerFlag(player, "-invokelater");
                caninvoke = true;
            }
        }
        if(caninvoke){
            if(!room->askForSkillInvoke(player, objectName(), data))
                return false;
            QList<ServerPlayer *> targets;
            foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
                if(player->inMyAttackRange(tmp))
                    targets << tmp;
            }
            if(targets.isEmpty()) return false;
            ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
            if(!room->askForCard(target, "jink", "@moon-spear-jink")){
                DamageStruct damage;
                damage.from = player;
                damage.to = target;
                room->damage(damage);
            }
        }
        return false;
    }
};

Rainbow::Rainbow(Suit suit, int number):Weapon(suit, number, 3){
    setObjectName("rainbow");
    skill = new RainbowSkill;
}

class PendantSkill: public ArmorSkill{
public:
    PendantSkill():ArmorSkill("pendant"){
        events << PhaseChange;
    }

    virtual bool trigger(TriggerEvent , Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() == Player::Start){
            foreach(const Card *card, player->getJudgingArea())
                room->throwCard(card);
        }
        return false;
    }
};

Pendant::Pendant(Suit suit, int number):Armor(suit, number){
    setObjectName("pendant");
    skill = new PendantSkill;
}

void Pendant::onUninstall(ServerPlayer *player) const{
    if(player->isAlive() && !player->hasMark("qinggang")){
        player->drawCards(2);
    }
}

Scroll::Scroll(Suit suit, int number):Armor(suit, number){
    setObjectName("scroll");
    //skill = new ScrollSkill;
}

class SquareSkill: public ArmorSkill{
public:
    SquareSkill():ArmorSkill("square"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Fire){
            LogMessage log;
            log.type = "#SquareDamage";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            room->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

Square::Square(Suit suit, int number) :Armor(suit, number){
    setObjectName("square");
    skill = new SquareSkill;
    target_fixed = false;
}

bool Square::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->distanceTo(to_select) <= 1;
}

void Square::onUse(Room *room, const CardUseStruct &card_use) const{
    Card::onUse(room, card_use);
}

Mirage::Mirage(Card::Suit suit, int number)
    :DefensiveHorse(suit, number)
{
    setObjectName("mirage");
}

class WeaselSkill: public TriggerSkill{
public:
    WeaselSkill():TriggerSkill("weasel"){
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Peach")){
            QList<ServerPlayer *> players = room->getOtherPlayers(player);

            foreach(ServerPlayer *p, players){
                if(p->getOffensiveHorse() == parent() &&
                   p->askForSkillInvoke(objectName(), data))
                {
                    room->throwCard(p->getOffensiveHorse());
                    p->playCardEffect(objectName());
                    p->obtainCard(use.card);

                    return true;
                }
            }
        }

        return false;
    }
};

Weasel::Weasel(Card::Suit suit, int number)
    :OffensiveHorse(suit, number)
{
    setObjectName("weasel");

    skill = new WeaselSkill;
    skill->setParent(this);
}

void Weasel::onInstall(ServerPlayer *player) const{
    player->getRoom()->getThread()->addTriggerSkill(skill);
}

void Weasel::onUninstall(ServerPlayer *player) const{

}

QString Weasel::getEffectPath(bool ) const{
    return "audio/card/common/monkey.ogg";
}

PloughPackage::PloughPackage()
    :Package("plough")
{
    type = CardPack;
    QList<Card *> cards;

    DefensiveHorse *mirage = new DefensiveHorse(Card::Diamond, 13);
    mirage->setObjectName("mirage");
    cards
    // spade
            << new SpaceAsk(Card::Spade, 1)
            << new Rainbow(Card::Spade, 2)
            << new Analeptic(Card::Spade, 3)
            << new Wiretap(Card::Spade, 4)
            << new FireSlash(Card::Spade, 5)
            << new Slash(Card::Spade, 6)
            << new Scroll(Card::Spade, 7)
            << new Slash(Card::Spade, 8)
            << new Slash(Card::Spade, 9)
            << new Discuss(Card::Spade, 10)
            << new ThunderSlash(Card::Spade, 11)
            << new Burn(Card::Spade, 12)
            << new Counterplot(Card::Spade, 13)

    // diamond
            << new Counterplot(Card::Diamond, 1)
            << new Assassinate(Card::Diamond, 2)
            << new Ecstasy(Card::Diamond, 3)
            << new Ecstasy(Card::Diamond, 4)
            << new Peach(Card::Diamond, 5)
            << new Burn(Card::Diamond, 6)
            << new Slash(Card::Diamond, 7)
            << new Jink(Card::Diamond, 8)
            << new Jink(Card::Diamond, 9)
            << new Jink(Card::Diamond, 10)
            << new Jink(Card::Diamond, 11)
            << new Jink(Card::Diamond, 12)
            << mirage

    // club
            << new Pendant(Card::Club, 1)
            << new Assassinate(Card::Club, 2)
            << new Analeptic(Card::Club, 3)
            << new Wiretap(Card::Club, 4)
            << new Weasel(Card::Club, 5)
            << new Slash(Card::Club, 6)
            << new Slash(Card::Club, 7)
            << new FireSlash(Card::Club, 8)
            << new Slash(Card::Club, 9)
            << new Discuss(Card::Club, 10)
            << new Counterplot(Card::Club, 11)
            << new ThunderSlash(Card::Club, 12)
            << new ThunderSlash(Card::Club, 13)

    // heart
            << new Crossbow(Card::Heart, 1)
            << new Assassinate(Card::Heart, 2)
            << new Slash(Card::Heart, 3)
            << new Jink(Card::Heart, 4)
            << new Peach(Card::Heart, 5)
            << new Square(Card::Heart, 6)
            << new SevenStar(Card::Heart, 7)
            << new Slash(Card::Heart, 8)
            << new Analeptic(Card::Heart, 9)
            << new Peach(Card::Heart, 10)
            << new ThunderSlash(Card::Heart, 11)
            << new Ecstasy(Card::Heart, 12)
            << new Jink(Card::Heart, 13);

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
