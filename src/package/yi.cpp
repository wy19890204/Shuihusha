#include "yi.h"
#include "general.h"
#include "skill.h"
#include "room.h"
#include "carditem.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"

GanlinCard::GanlinCard(){
    will_throw = false;
    once = true;
}

void GanlinCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();

    room->moveCardTo(this, target, Player::Hand, false);
    int n = source->getLostHp() - source->getHandcardNum();
    if(n > 0)
        source->drawCards(n);
};

class Ganlin:public ViewAsSkill{
public:
    Ganlin():ViewAsSkill("ganlin"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("GanlinCard");
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

class Shalu: public TriggerSkill{
public:
    Shalu():TriggerSkill("shalu"){
        events << Damage << PhaseChange;
   }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent e, ServerPlayer *likui, QVariant &data) const{
        Room *room = likui->getRoom();
        if(e == PhaseChange){
            if(likui->getPhase() == Player::NotActive)
                room->setPlayerMark(likui, "shalu", 0);
            return false;
        }
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.card || damage.from != likui)
            return false;
        if(damage.card->inherits("Slash")){
            if(likui->getMark("shalu") > 0 && !likui->hasWeapon("crossbow") && !likui->hasSkill("paoxiao"))
                room->setPlayerMark(likui, "shalu", likui->getMark("shalu") - 1);
            if(!room->askForSkillInvoke(likui, objectName(), data))
                return false;
            room->playSkillEffect(objectName());
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(spade|club):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = likui;

            room->judge(judge);
            if(judge.isGood()){
                room->playSkillEffect(objectName(), 1);
                likui->obtainCard(judge.card);
                room->setPlayerMark(likui, "shalu", likui->getMark("shalu") + 1);
            }
        }
        return false;
    }
};

class Fenhui: public TriggerSkill{
public:
    Fenhui():TriggerSkill("fenhui"){
        frequency = Compulsory;
        events << Predamage << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == Predamage){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature != DamageStruct::Fire){
                Room *room = player->getRoom();
                damage.nature = DamageStruct::Fire;

                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#FenhuiFire";
                log.from = player;
                log.arg = QString::number(damage.damage);
                room->sendLog(log);
                room->playSkillEffect(objectName(), qrand() % 2 + 1);
                return false;
            }
       }else if(event == Predamaged){
           DamageStruct damage = data.value<DamageStruct>();
                  if(damage.nature == DamageStruct::Fire){
                      Room *room = player->getRoom();
                      LogMessage log;
                      log.type = "#FenhuiProtect";
                      log.from = player;
                      log.arg = QString::number(damage.damage);
                      room->sendLog(log);
                      room->playSkillEffect(objectName(), qrand() % 2 + 3);
                      return true;
                  }else
                      return false;

            }


        return false;
    }
};


class Shenhuo: public OneCardViewAsSkill{
public:
    Shenhuo():OneCardViewAsSkill("shenhuo"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->isRed() && card->inherits("TrickCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        FireAttack *fire_attack = new FireAttack(card->getSuit(), card->getNumber());
        fire_attack->addSubcard(card->getId());
        fire_attack->setSkillName(objectName());
        return fire_attack;
    }
};

class Shenhuogetcards:public TriggerSkill{
public:
    Shenhuogetcards():TriggerSkill("#shenhuogetcards"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *weidingguo, QVariant &data) const{
        CardStar card = NULL;

            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;


    if(card->inherits("FireAttack")){
            Room *room = weidingguo->getRoom();
            if(room->askForSkillInvoke(weidingguo, objectName())){
                room->playSkillEffect(objectName());
                weidingguo->drawCards(2);
            }
        }
           return false;
       }
    };

class Huxiao: public OneCardViewAsSkill{
public:
Huxiao():OneCardViewAsSkill("huxiao"){
}

virtual bool viewFilter(const CardItem *to_select) const{
return to_select->getFilteredCard()->inherits("EquipCard");
}

virtual const Card *viewAs(CardItem *card_item) const{
const Card *card = card_item->getCard();
SavageAssault *savage_assault = new SavageAssault
(card->getSuit(), card->getNumber());
savage_assault->addSubcard(card->getId());
savage_assault->setSkillName(objectName());
return savage_assault;
}
};

class Linse: public ProhibitSkill{
public:
    Linse():ProhibitSkill("linse"){
    }
    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->inherits("Snatch") || card->inherits("Dismantlement");
    }
};

class  Halfhp: public GameStartSkill{
public:
     Halfhp():GameStartSkill("#halfhp"){}

     virtual void onGameStart(ServerPlayer *lizhong) const{
        Room *room = lizhong->getRoom();
         room->setPlayerProperty(lizhong, "maxhp", lizhong->getMaxHP() + 1);
     }
};

class Feiyan: public ProhibitSkill{
public:
    Feiyan():ProhibitSkill("feiyan"){
    }
    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->inherits("Snatch") || card->inherits("SupplyShortage");
    }
};


class Shentou: public OneCardViewAsSkill{
public:
    Shentou():OneCardViewAsSkill("shentou"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Snatch *snatch = new Snatch(first->getSuit(), first->getNumber());
        snatch->addSubcard(first->getId());
        snatch->setSkillName(objectName());
        return snatch;
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





YiPackage::YiPackage()
    :Package("yi")
{
    General *songjiang, *likui, *weidingguo, *yanshun, *lizhong, *shiqian, *jiashi;
    songjiang = new General(this, "songjiang$", "qun");
    songjiang->addSkill(new Ganlin);
    songjiang->addSkill(new Juyi);
    skills << new JuyiViewAsSkill;

    likui = new General(this, "likui", "shu");
    likui->addSkill(new Shalu);

    weidingguo = new General(this, "weidingguo", "shu", 3);
    weidingguo->addSkill(new Fenhui);
    weidingguo->addSkill(new Shenhuo);
    weidingguo->addSkill(new Shenhuogetcards);
    related_skills.insertMulti("shenhuo", "#shenhuogetcards");

    yanshun = new General(this, "yanshun", "shu");
    yanshun->addSkill(new Huxiao);

    lizhong = new General(this, "lizhong", "qun", 3);
    lizhong->addSkill(new Linse);
    lizhong->addSkill(new Halfhp);
    related_skills.insertMulti("linse", "#halfhp");

    shiqian = new General(this, "shiqian", "qun", 3);
    shiqian->addSkill(new Feiyan);
    shiqian->addSkill(new Shentou);

    jiashi = new General(this, "jiashi", "wu", 3,false);
    jiashi->addSkill(new Zhuying);
    jiashi->addSkill(new Banzhuang);

    addMetaObject<GanlinCard>();
    addMetaObject<JuyiCard>();
}

ADD_PACKAGE(Yi)
