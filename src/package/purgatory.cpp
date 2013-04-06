#include "purgatory.h"
#include "maneuvering.h"

Mastermind::Mastermind(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("mastermind");
}

bool Mastermind::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    return targets.length() == 2;
}

void Mastermind::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    targets.last()->tag["DtoL"] = QVariant::fromValue((PlayerStar)targets.first());
    QString tgs;
    tgs << targets.last();
    TrickCard::use(room, source, tgs);
}

void Mastermind::onEffect(const CardEffectStruct &effect) const{
    PlayerStar death = effect.to;
    PlayerStar life = death->tag["DtoL"].value<PlayerStar>();
    death->gainMark("@death");
    life->gainMark("@life");
}

SpinDestiny::SpinDestiny(Suit suit, int number)
    :GlobalEffect(suit, number)
{
    setObjectName("spin_destiny");
}

bool SpinDestiny::isCancelable(const CardEffectStruct &effect) const{
    return effect.to->isAlive();
}

void SpinDestiny::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct use = card_use;
    use.to = room->getPlayers();
    TrickCard::onUse(room, use);
}

void SpinDestiny::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if(effect.to->isAlive())
        room->loseHp(effect.to);
    else{
        if(effect.to->getMaxHp() <= 0)
            room->setPlayerProperty(effect.to, "maxhp", 1);
        room->setPlayerProperty(effect.to, "hp", 1);
        room->revivePlayer(effect.to);
    }
}

EdoTensei::EdoTensei(Suit suit, int number)
    :SingleTargetTrick(suit, number){
    setObjectName("edo_tensei");
    target_fixed = true;
}

void EdoTensei::onEffect(const CardEffectStruct &effect) const{
    PlayerStar whody = effect.from->tag["EdoSource"].value<PlayerStar>();
    QList<ServerPlayer *> targets = alldead;
    choose dead;
    revive dead;
}

bool EdoTensei::isAvailable(const Player *) const{
    return false;
}

PurgatoryPackage::PurgatoryPackage()
    :CardPackage("purgatory")
{
    QList<Card *> cards;

    cards
            << new Mastermind(Card::Heart, 5)
            << new Mastermind(Card::Heart, 5)
            << new SpinDestiny(Card::Diamond, 5)
            << new EdoTensei(Card::Club, 5)
            << new EdoTensei(Card::Spade, 5)
            ;

    foreach(Card *card, cards)
        card->setParent(this);
}

ADD_PACKAGE(Purgatory)
