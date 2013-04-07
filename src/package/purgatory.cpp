#include "purgatory.h"
#include "maneuvering.h"

Mastermind::Mastermind(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("mastermind");
}

bool Mastermind::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    return targets.length() == 2;
}

bool Mastermind::targetFilter(const QList<const Player *> &targets, const Player *, const Player *) const{
    return targets.length() <= 1;
}

void Mastermind::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    targets.last()->tag["DtoL"] = QVariant::fromValue((PlayerStar)targets.first());
    QList<ServerPlayer *> tgs;
    tgs << targets.last();
    TrickCard::use(room, source, tgs);
}

void Mastermind::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    PlayerStar death = effect.to;
    PlayerStar life = death->tag["DtoL"].value<PlayerStar>();

    foreach(ServerPlayer *t, room->getAllPlayers()){
        t->loseAllMarks("@death");
        t->loseAllMarks("@life");
    }

    death->gainMark("@death");
    life->gainMark("@life");
}

SpinDestiny::SpinDestiny(Suit suit, int number)
    :GlobalEffect(suit, number)
{
    setObjectName("spin_destiny");
}

void SpinDestiny::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct use = card_use;
    use.to = room->getAllPlayers(false);
    TrickCard::onUse(room, use);
    foreach(ServerPlayer *dead, room->getAllPlayers(true)){
        if(dead->isDead()){
            if(dead->getMaxHp() <= 0)
                room->setPlayerProperty(dead, "maxhp", 1);
            room->setPlayerProperty(dead, "hp", 1);
            room->revivePlayer(dead);
        }
    }
}

void SpinDestiny::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->loseHp(effect.to);
}

EdoTensei::EdoTensei(Suit suit, int number)
    :SingleTargetTrick(suit, number, false){
    setObjectName("edo_tensei");
    target_fixed = true;
}

void EdoTensei::onEffect(const CardEffectStruct &effect) const{
    PlayerStar whody = effect.from->tag["EdoSource"].value<PlayerStar>();
    //QList<ServerPlayer *> targets = alldead;
    //choose dead;
    //revive dead;
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
