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
        room->setPlayerFlag(source, "Ganlin");
    }
};

class GanlinViewAsSkill:public ViewAsSkill{
public:
    GanlinViewAsSkill():ViewAsSkill("ganlin"){
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

class Ganlin: public PhaseChangeSkill{
public:
    Ganlin():PhaseChangeSkill("ganlin"){
        view_as_skill = new GanlinViewAsSkill;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool onPhaseChange(ServerPlayer *p) const{
        if(p->getPhase() == Player::NotActive){
            Room *room = p->getRoom();
            room->setPlayerFlag(p, "-Ganlin");
        }
        return false;
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

class Yueli:public TriggerSkill{
public:
    Yueli():TriggerSkill("yueli"){
        frequency = Frequent;
        events << FinishJudge;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *yuehe, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        CardStar card = judge->card;

        QVariant data_card = QVariant::fromValue(card);
        Room *room = yuehe->getRoom();
        if(judge->card->inherits("BasicCard") && yuehe->askForSkillInvoke(objectName(), data_card)){
            if(card->objectName() == "shit"){
                QString result = room->askForChoice(yuehe, objectName(), "yes+no");
                if(result == "no")
                    return false;
            }
            yuehe->obtainCard(judge->card);
            room->playSkillEffect(objectName(), 1);
            return true;
        }
        else
            room->playSkillEffect(objectName(), 2);
        return false;
    }
};

class Taohui:public TriggerSkill{
public:
    Taohui():TriggerSkill("taohui"){
        events << PhaseChange << FinishJudge;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yuehe, QVariant &data) const{
        if(event == PhaseChange && yuehe->getPhase() == Player::Finish){
            Room *room = yuehe->getRoom();
            while(yuehe->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());

                JudgeStruct judge;
                judge.reason = objectName();
                judge.who = yuehe;

                room->judge(judge);
                if(judge.card->inherits("BasicCard"))
                    break;
            }
        }
        else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(judge->reason == objectName()){
                if(!judge->card->inherits("BasicCard")){
                    Room *room = yuehe->getRoom();
                    ServerPlayer *target = room->askForPlayerChosen(yuehe, room->getAllPlayers(), objectName());
                    target->drawCards(1);
                    return true;
                }
            }
        }
        return false;
    }
};

TTXDPackage::TTXDPackage()
    :Package("TTXD")
{ //guan == wei, jiang == shu, min == wu, kou == qun
    General *songjiang = new General(this, "songjiang$", "qun");
    songjiang->addSkill(new Ganlin);
    songjiang->addSkill(new Juyi);
    skills << new JuyiViewAsSkill;

    General *yuehe = new General(this, "yuehe", "wu", 3);
    yuehe->addSkill(new Yueli);
    yuehe->addSkill(new Taohui);

    addMetaObject<GanlinCard>();
    addMetaObject<JuyiCard>();
}

ADD_PACKAGE(TTXD)
