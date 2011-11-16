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
}

void GanlinCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
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

    room->moveCardTo(this, target, Player::Hand, false);

    int old_value = source->getMark("ganlin");
    int new_value = old_value + subcards.length();
    room->setPlayerMark(source, "ganlin", new_value);

    if(old_value < 2 && new_value >= 2){
        RecoverStruct recover;
        recover.card = this;
        recover.who = source;
        room->recover(source, recover);
    }
};

class GanlinViewAsSkill:public ViewAsSkill{
public:
    GanlinViewAsSkill():ViewAsSkill("ganlin"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(ServerInfo.GameMode == "04_1v3"
           && selected.length() + Self->getMark("ganlin") >= 2)
           return false;
        else
            return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        GanlinCard *ganlin_card = new GanlinCard;
        ganlin_card->addSubcards(cards);
        return ganlin_card;
    }
};

class Ganlin: public PhaseChangeSkill{
public:
    Ganlin():PhaseChangeSkill("ganlin"){
        view_as_skill = new GanlinViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::NotActive
                && target->hasUsed("GanlinCard");
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        target->getRoom()->setPlayerMark(target, "ganlin", 0);

        return false;
    }
};

class Shalu: public TriggerSkill{
public:
    Shalu():TriggerSkill("shalu"){
        events << Damage << FinishJudge << PhaseChange;//伤害事件、判定牌生效后、阶段改变
   }

    virtual bool trigger(TriggerEvent event, ServerPlayer *likui, QVariant &data) const
    {
        if(likui->getPhase() == Player::Finish)
         {    Room *room = likui->getRoom();
            room->setPlayerMark(likui, "shalu_success", 0);
            return false;
           }

        if(likui->getPhase() != Player::Play)
        {
            return false;
        }




        DamageStruct damage = data.value<DamageStruct>();
        if(event == Damage && damage.card
           && damage.card->inherits("Slash"))
        {
            Room *room = likui->getRoom();
            if(room->askForSkillInvoke(likui, objectName(), data))
            {
                room->playSkillEffect(objectName());
                likui->setFlags("shalu"); //发动标记
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(spade|club):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = likui;

                room->judge(judge);
                if(judge.isGood())
                {

                    room->setPlayerMark(likui, "shalu_success",  likui->getMark("shalu_success") + 1);//每有一次判黑，判黑标记+1，
                }
                else
                {

                    likui->setFlags("-shalu");//移除发动标记
                }
            }
        }
        else if(event == FinishJudge){
            if(likui->hasFlag("shalu")){
                JudgeStar judge = data.value<JudgeStar>();
                if(judge->card->isBlack()){
                    likui->obtainCard(judge->card);
                    likui->setFlags("-shalu");
                    return true;
                }
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
    addMetaObject<GanlinCard>();

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

}

ADD_PACKAGE(Yi)
