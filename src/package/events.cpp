#include "events.h"
#include "skill.h"
#include "room.h"
#include "carditem.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "engine.h"

QString EventsCard::getType() const{
    return "events";
}

QString EventsCard::getSubtype() const{
    return "events";
}

QString EventsCard::getEffectPath(bool is_male) const{
    return QString();
}

Card::CardType EventsCard::getTypeId() const{
    return Events;
}

Jiefachang::Jiefachang(Suit suit, int number):EventsCard(suit, number){
    setObjectName("jiefachang");
}

bool Jiefachang::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    if(Self->getPhase() == Player::Play){
        return !to_select->getJudgingArea().isEmpty();
    }
    return !to_select->faceUp();
}

void Jiefachang::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    ServerPlayer *target = targets.first();
    if(source->getPhase() == Player::Play && !target->getJudgingArea().isEmpty()){
        source->playCardEffect("@jiefachang2");
        if(target->getJudgingArea().length() > 1)
            room->throwCard(room->askForCardChosen(source, target, "j", "jiefachang"));
        else
            room->throwCard(target->getJudgingArea().last());
    }
    else{
        source->playCardEffect("@jiefachang1");
        target->turnOver();
    }
}

Daojia::Daojia(Suit suit, int number):EventsCard(suit, number){
    setObjectName("daojia");
}

bool Daojia::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!Self->hasFlag("Daojia")){
        return !targets.isEmpty();
    }
    return targets.isEmpty() && to_select != Self && to_select->getArmor();
}

bool Daojia::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(Self->hasFlag("Daojia"))
        return targets.length() == 1;
    else
        return targets.length() == 0;
}

void Daojia::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    if(targets.isEmpty()){
        source->playCardEffect("@daojia2");
        source->drawCards(1);
        room->moveCardTo(this, NULL, Player::DrawPile, true);
    }
    else{
        source->playCardEffect("@daojia1");
        ServerPlayer *target = targets.first();
        source->obtainCard(target->getArmor());
    }
}

Tifanshi::Tifanshi(Suit suit, int number):EventsCard(suit, number){
    setObjectName("tifanshi");
}

bool Tifanshi::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(Self->hasFlag("Tifanshi"))
        return false;
    else
        return targets.isEmpty() && to_select != Self;
}

bool Tifanshi::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(Self->hasFlag("Tifanshi"))
        return true;
    else
        return !targets.isEmpty();
}

void Tifanshi::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    if(targets.isEmpty()){
        source->playCardEffect("@tifanshi1");
        int reco = 0;
        foreach(ServerPlayer *tmp, room->getAlivePlayers())
            if(tmp->getRole() == "rebel")
                reco ++;
        if(reco > 0)
            source->drawCards(reco);
    }
    else{
        source->playCardEffect("@tifanshi2");
        ServerPlayer *target = targets.first();
        room->askForDiscard(target, "tifanshi", 1, false, true);
    }
}

NinedayGirl::NinedayGirl(Suit suit, int number):EventsCard(suit, number){
    setObjectName("ninedaygirl");
}

bool NinedayGirl::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(Self->hasFlag("NineGirl"))
        return false;
    else
        return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

bool NinedayGirl::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(Self->hasFlag("NineGirl"))
        return true;
    else
        return !targets.isEmpty();
}

void NinedayGirl::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    if(!targets.isEmpty()){
        source->playCardEffect("@ninedaygirl2");
        ServerPlayer *target = targets.first();
        int card_id = target->getRandomHandCardId();
        room->showCard(target, card_id);
        room->obtainCard(source, card_id);
    }
    else
        source->playCardEffect("@ninedaygirl1");
}

FuckGaolian::FuckGaolian(Suit suit, int number):EventsCard(suit, number){
    setObjectName("fuckgaolian");
}

bool FuckGaolian::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(Self->hasFlag("FuckGao"))
        return targets.isEmpty() && to_select != Self;
    else // if(Self->hasFlag("FuckLian"))
        return false;
}

bool FuckGaolian::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(Self->hasFlag("FuckGao"))
        return !targets.isEmpty();
    else
        return true;
}

bool FuckGaolian::isAvailable(const Player *) const{
    return false;
}

void FuckGaolian::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    if(!targets.isEmpty()){
        source->playCardEffect("@fuckgaolian1");
        ServerPlayer *target = targets.first();
        DamageStruct damage;
        damage.from = source;
        damage.to = target;
        damage.nature = DamageStruct::Thunder;
        room->damage(damage);
    }
}

Jiangjieshi::Jiangjieshi(Suit suit, int number):EventsCard(suit, number){
    setObjectName("jiangjieshi");
    target_fixed = true;
}

void Jiangjieshi::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    source->playCardEffect("@jiangjieshi1");
    room->setPlayerFlag(source, "drunken");
}

bool Jiangjieshi::isAvailable(const Player *) const{
    return false;
}

NanaStars::NanaStars(Suit suit, int number):EventsCard(suit, number){
    setObjectName("nanastars");
}

bool NanaStars::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->containsTrick("treasury");
}

void NanaStars::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    source->playCardEffect("@nanastars1");
    foreach(const Card *card, targets.first()->getJudgingArea()){
        if(card->inherits("Treasury")){
            room->throwCard(card);
            source->drawCards(5);
            break;
        }
    }
}

bool NanaStars::isAvailable(const Player *) const{
    return false;
}

EventsPackage::EventsPackage()
    :Package("events_package")
{
    QList<Card *> cards;
    cards
            << new Jiefachang(Card::Diamond, 13)
            << new Tifanshi(Card::Spade, 7)
            << new Daojia(Card::Club, 12)
            << new NinedayGirl(Card::Heart, 2)
            << new FuckGaolian(Card::Heart, 8)
            << new Jiangjieshi(Card::Club, 9)
            << new NanaStars(Card::Diamond, 10);

    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(Events)
