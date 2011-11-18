#include "ttxd-package.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "room.h"
#include "maneuvering.h"

GanlinCard::GanlinCard(){
    will_throw = false;
}

void GanlinCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();

    room->moveCardTo(this, target, Player::Hand, false);
    int n = source->getLostHp() - source->getHandcardNum();
    if(n > 0 && source->askForSkillInvoke("ganlin")){
        source->drawCards(n);
        source->setFlags("Ganlin");
    }
};

class Ganlin:public ViewAsSkill{
public:
    Ganlin():ViewAsSkill("ganlin"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasFlag("Ganlin");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        GanlinCard *ganlin_card = new GanlinCard;
        ganlin_card->addSubcards(cards);
        return ganlin_card;
    }
};

JuyiCard::JuyiCard(){
    once = true;
}

void JuyiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *song = targets.first();
    if(song->hasSkill("juyi")){
        song->obtainCard(this);
        source->obtainCard(room->askForCardShow(song, source, "juyi"));
    }
}

bool JuyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("juyi") && to_select != Self;
}

class JuyiViewAsSkill: public OneCardViewAsSkill{
public:
    JuyiViewAsSkill():OneCardViewAsSkill("jui"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("JuyiCard") && player->getKingdom() == "qun";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        JuyiCard *card = new JuyiCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Juyi: public GameStartSkill{
public:
    Juyi():GameStartSkill("juyi$"){

    }

    virtual void onGameStart(ServerPlayer *player) const{
        Room *room = player->getRoom();
        foreach(ServerPlayer *tmp, room->getAlivePlayers()){
            room->attachSkillToPlayer(tmp, "jui");
        }
    }
};

class Zhuying: public FilterSkill{
public:
    Zhuying():FilterSkill("zhuying"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->objectName() == "analeptic";
    //视为技用getCard()，表示变化前的牌；当做技用getFilteredCard()，表示调用变化后（比如被视为过了的）牌
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *c = card_item->getCard();
        Peach *peach = new Peach(c->getSuit(), c->getNumber());
        peach->setSkillName(objectName());
        peach->addSubcard(card_item->getCard());

        return peach;
    }
};

class Banzhuang: public OneCardViewAsSkill{
public:
    Banzhuang():OneCardViewAsSkill("banzhuang"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        ExNihilo *ex_nihilo = new ExNihilo(card->getSuit(), card->getNumber());
        ex_nihilo->addSubcard(card->getId());
        ex_nihilo->setSkillName(objectName());
        return ex_nihilo;
    }
};

TTXDPackage::TTXDPackage()
    :Package("TTXD")
{ //guan == wei, jiang == shu, min == wu, kou == qun
    General *songjiang = new General(this, "songjiang$", "qun");
    songjiang->addSkill(new Ganlin);
    songjiang->addSkill(new Juyi);
    skills << new JuyiViewAsSkill;

    General *jiashi = new General(this, "jiashi", "wu", 3, false);
    jiashi->addSkill(new Zhuying);
    jiashi->addSkill(new Banzhuang);

    addMetaObject<GanlinCard>();
    addMetaObject<JuyiCard>();
}

ADD_PACKAGE(TTXD)
