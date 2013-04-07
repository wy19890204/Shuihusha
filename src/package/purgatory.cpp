#include "purgatory.h"
#include "maneuvering.h"

Shit::Shit(Suit suit, int number):
    BasicCard(suit, number){
    setObjectName("shit");
    target_fixed = true;
}

QString Shit::getSubtype() const{
    return "specially";
}

void Shit::onUse(Room *room, const CardUseStruct &) const{
    room->moveCardTo(this, NULL, Player::DiscardedPile);
}

void Shit::onMove(const CardMoveStruct &move) const{
    PlayerStar from = move.from;
    Room *room = from->getRoom();
    if(from && move.from_place == Player::Hand &&
       from->getRoom()->getCurrent() == move.from
       && (move.to_place == Player::DiscardedPile || move.to_place == Player::Special)
       && move.to == NULL
       && from->isAlive()){

        LogMessage log;
        log.type = "$ShitHint";
        log.card_str = getEffectIdString();
        log.from = from;
        room->sendLog(log);

        if(getSuit() == Spade)
            room->loseHp(from);
        else{
            DamageStruct damage;
            damage.from = damage.to = from;
            damage.card = this;

            switch(getSuit()){
            case Club: damage.nature = DamageStruct::Thunder; break;
            case Heart: damage.nature = DamageStruct::Fire; break;
            default: damage.nature = DamageStruct::Normal;
            }
            room->damage(damage);
        }
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
            /*<< new Shit(Card::Club, 1)
            << new Shit(Card::Heart, 8)
            << new Shit(Card::Diamond, 13)*/
            << new Shit(Card::Spade, 10)
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
