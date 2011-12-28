#include "general.h"
#include "standard.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "serverplayer.h"
#include "room.h"
#include "standard-skillcards.h"
#include "ai.h"

class Yiji:public MasochismSkill{
public:
    Yiji():MasochismSkill("yiji"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *guojia, const DamageStruct &damage) const{
        Room *room = guojia->getRoom();

        if(!room->askForSkillInvoke(guojia, objectName()))
            return;

        room->playSkillEffect(objectName());

        int x = damage.damage, i;
        for(i=0; i<x; i++){
            guojia->drawCards(2);
            QList<int> yiji_cards = guojia->handCards().mid(guojia->getHandcardNum() - 2);

            while(room->askForYiji(guojia, yiji_cards))
                ; // empty loop
        }

    }
};

class Jizhi:public TriggerSkill{
public:
    Jizhi():TriggerSkill("jizhi"){
        frequency = Frequent;
        events << CardUsed << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yueying, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->isNDTrick()){
            Room *room = yueying->getRoom();
            if(room->askForSkillInvoke(yueying, objectName())){
                room->playSkillEffect(objectName());
                yueying->drawCards(1);
            }
        }

        return false;
    }
};

void StandardPackage::addGenerals(){
    General *guojia = new General(this, "guojia", "guan", 3);
    guojia->addSkill(new Yiji);

    General *huangyueying = new General(this, "huangyueying", "jiang", 3, false);
    huangyueying->addSkill(new Jizhi);

    // for skill cards
    addMetaObject<CheatCard>();
}

class Ubuna:public ZeroCardViewAsSkill{
public:
    Ubuna():ZeroCardViewAsSkill("ubuna"){
    }

    virtual const Card *viewAs() const{
        return new UbunaCard;
    }
};

class Ubunb:public ZeroCardViewAsSkill{
public:
    Ubunb():ZeroCardViewAsSkill("ubunb"){
    }

    virtual const Card *viewAs() const{
        return Sanguosha->cloneSkillCard("BuzhenCard");
    }
};

class Ubunc:public ZeroCardViewAsSkill{
public:
    Ubunc():ZeroCardViewAsSkill("ubunc"){
    }

    virtual const Card *viewAs() const{
        return new UbuncCard;
    }
};

class Ubund:public ZeroCardViewAsSkill{
public:
    Ubund():ZeroCardViewAsSkill("ubund"){
    }

    virtual const Card *viewAs() const{
        return new UbundCard;
    }
};

class UbuneVAS: public OneCardViewAsSkill{
public:
    UbuneVAS():OneCardViewAsSkill("ubune"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->isDTE();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        UbuneCard *card = new UbuneCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

class Ubune: public PhaseChangeSkill{
public:
    Ubune():PhaseChangeSkill("ubune"){
        view_as_skill = new UbuneVAS;
    }

    virtual bool onPhaseChange(ServerPlayer *p) const{
        Room *room = p->getRoom();
        if(p->getPhase() == Player::Judge && !p->getJudgingArea().isEmpty() &&
           p->askForSkillInvoke(objectName())){
            ServerPlayer *target = room->askForPlayerChosen(p, room->getOtherPlayers(p), objectName());
            DummyCard *dummy1 = new DummyCard;
            foreach(const Card *card, target->getJudgingArea())
                dummy1->addSubcard(card->getId());
            DummyCard *dummy2 = new DummyCard;
            foreach(const Card *card, p->getJudgingArea())
                dummy2->addSubcard(card->getId());
            room->moveCardTo(dummy2, target, Player::Judging);
            delete dummy2;
            room->moveCardTo(dummy1, p, Player::Judging);
            delete dummy1;
        }
        return false;
    }
};

class Ubunf: public TriggerSkill{
public:
    Ubunf():TriggerSkill("ubunf"){
        events << Dying;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        DyingStruct dying = data.value<DyingStruct>();
        if(dying.who == player && player->askForSkillInvoke(objectName())){
            RecoverStruct rev;
            rev.who = player;
            rev.recover = player->getMaxHP();
            player->getRoom()->recover(player, rev);
            player->getRoom()->setPlayerProperty(player, "hp", player->getMaxHP());
        }
        return false;
    }
};

TestPackage::TestPackage()
    :Package("test")
{    
    General *shenlvbu1 = new General(this, "shenlvbu1", "god", 8, true, true);
    shenlvbu1->addSkill("cuju");
    shenlvbu1->addSkill("wubang");
    shenlvbu1->addSkill("huanshu");

    General *shenlvbu2 = new General(this, "shenlvbu2", "god", 4, true, true);
    shenlvbu2->addSkill("huanshu");
    shenlvbu2->addSkill("shunshui");
    shenlvbu2->addSkill("qibing");
    shenlvbu2->addSkill(new Skill("shenji"));

    General *ubuntenkei = new General(this, "ubuntenkei", "god", 4, false, true);
    ubuntenkei->addSkill(new Ubuna);
    addMetaObject<UbunaCard>();
    ubuntenkei->addSkill(new Ubunb);
    ubuntenkei->addSkill(new Ubunc);
    addMetaObject<UbuncCard>();
    ubuntenkei->addSkill(new Ubund);
    addMetaObject<UbundCard>();
    ubuntenkei->addSkill(new Ubune);
    addMetaObject<UbuneCard>();
    ubuntenkei->addSkill(new Ubunf);

    new General(this, "sujiang", "god", 5, true, true);
    new General(this, "sujiangf", "god", 5, false, true);
}

ADD_PACKAGE(Test)
