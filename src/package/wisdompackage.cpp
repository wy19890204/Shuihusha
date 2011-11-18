#include "wisdompackage.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "settings.h"
#include "room.h"
#include "maneuvering.h"

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

WisdomPackage::WisdomPackage()
    :Package("wisdom")
{
    /*
        General *songjiang = new General(this, "songjiang$", "qun");
        songjiang->addSkill(new Ganlin);
        songjiang->addSkill(new Juyi);
        skills << new JuyiViewAsSkill;
    */
    General *jiashi = new General(this, "jiashi", "wu", 3, false);
    jiashi->addSkill(new Zhuying);
    jiashi->addSkill(new Banzhuang);
}

ADD_PACKAGE(Wisdom)
