#include "aux-skills.h"
#include "client.h"
#include "carditem.h"
#include "standard.h"
#include "clientplayer.h"
#include "common-skillcards.h"
#include "engine.h"

DiscardSkill::DiscardSkill()
    :ViewAsSkill("discard"), card(new DummyCard),
    num(0), include_equip(false)
{
    card->setParent(this);
}

void DiscardSkill::setNum(int num){
    this->num = num;
}

void DiscardSkill::setIncludeEquip(bool include_equip){
    this->include_equip = include_equip;
}

bool DiscardSkill::viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
    if(selected.length() >= num)
        return false;

    if(!include_equip && to_select->isEquipped())
        return false;

    if(Self->isJilei(to_select->getFilteredCard()))
        return false;

    return true;
}

const Card *DiscardSkill::viewAs(const QList<CardItem *> &cards) const{
    if(cards.length() == num){
        card->clearSubcards();
        card->addSubcards(cards);
        return card;
    }else
        return NULL;
}

// -------------------------------------------

ResponseSkill::ResponseSkill()
    :OneCardViewAsSkill("response-skill")
{

}

void ResponseSkill::setPattern(const QString &pattern){
    this->pattern = Sanguosha->getPattern(pattern);
}

bool ResponseSkill::matchPattern(const Player *player, const Card *card) const{
    if(player->isJilei(card))
        return false;

    return pattern && pattern->match(player, card);
}

bool ResponseSkill::viewFilter(const CardItem *to_select) const{
    const Card *card = to_select->getFilteredCard();
    return matchPattern(Self, card);
}

const Card *ResponseSkill::viewAs(CardItem *card_item) const{
    return card_item->getFilteredCard();
}

// -------------------------------------------

FreeRegulateCard::FreeRegulateCard(){
    will_throw = false;
}

bool FreeRegulateCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

bool FreeRegulateCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() <= 1;
}

void FreeRegulateCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(targets.isEmpty()){
        if(getSubcards().isEmpty()){
            int card_id = room->getDiscardPile().last();
            room->obtainCard(source, card_id);
        }
        else
            room->throwCard(this, source);
    }
    else{
        PlayerStar target = targets.first();
        if(getSubcards().isEmpty()){
            int card_id = room->askForCardChosen(source, target, "hej", "free-regulate");
            room->obtainCard(source, card_id);
        }
        else
            room->obtainCard(target, this, false);
    }
}

FreeRegulateSkill::FreeRegulateSkill(QObject *parent)
    :ViewAsSkill("free-regulate")
{
    setParent(parent);
    card = new DummyCard;
}

bool FreeRegulateSkill::isEnabledAtPlay(const Player *) const{
    return true;
}

bool FreeRegulateSkill::viewFilter(const QList<CardItem *> &, const CardItem *) const{
    return true;
}

const Card *FreeRegulateSkill::viewAs(const QList<CardItem *> &cards) const{
    FreeRegulateCard *card = new FreeRegulateCard;
    card->addSubcards(cards);
    return card;
}

// -------------------------------------------
RendeCard::RendeCard(){
    will_throw = false;
}

void RendeCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = NULL;
    if(targets.isEmpty()){
        foreach(ServerPlayer *player, room->getAlivePlayers()){
            if(player != source){
                target = player;
                break;
            }
        }
    }else
        target = targets.first();

    room->obtainCard(target, this, false);

    int old_value = source->getMark("rende");
    int new_value = old_value + subcards.length();
    room->setPlayerMark(source, "rende", new_value);

    if(old_value < 2 && new_value >= 2){
        RecoverStruct recover;
        recover.card = this;
        recover.who = source;
        room->recover(source, recover);
    }
}

YijiViewAsSkill::YijiViewAsSkill()
    :ViewAsSkill("yiji")
{
    card = new RendeCard;
}

void YijiViewAsSkill::setCards(const QString &card_str){
    QStringList cards = card_str.split("+");
    ids = Card::StringsToIds(cards);
}

bool YijiViewAsSkill::viewFilter(const QList<CardItem *> &, const CardItem *to_select) const{
    return ids.contains(to_select->getCard()->getId());
}

const Card *YijiViewAsSkill::viewAs(const QList<CardItem *> &cards) const{
    if(cards.isEmpty())
        return NULL;

    card->clearSubcards();
    card->addSubcards(cards);

    return card;
}

// ------------------------------------------------

class ChoosePlayerCard: public DummyCard{
public:
    ChoosePlayerCard(){
        target_fixed = false;
    }

    void setPlayerNames(const QStringList &names){
        set = names.toSet();
    }

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
        return targets.isEmpty() && set.contains(to_select->objectName());
    }

private:
    QSet<QString> set;
};

ChoosePlayerSkill::ChoosePlayerSkill()
    :ZeroCardViewAsSkill("choose_player")
{
    card = new ChoosePlayerCard;
    card->setParent(this);
}

void ChoosePlayerSkill::setPlayerNames(const QStringList &names){
    card->setPlayerNames(names);
}

const Card *ChoosePlayerSkill::viewAs() const{
    return card;
}
