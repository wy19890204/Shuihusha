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
    if(song->isKongcheng() && source->isKongcheng())
        return;
    if(song->hasSkill("juyi") && room->askForChoice(song, "jui", "agree+deny") == "agree"){
        DummyCard *card1 = source->wholeHandCards();
        DummyCard *card2 = song->wholeHandCards();
        if(card1){
            room->moveCardTo(card1, song, Player::Hand, false);
            delete card1;
        }
        room->getThread()->delay();

        if(card2){
            room->moveCardTo(card2, source, Player::Hand, false);
            delete card2;
        }
        LogMessage log;
        log.type = "#Juyi";
        log.from = source;
        log.to << song;
        room->sendLog(log);
    }
}

bool JuyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("juyi") && to_select != Self;
}

class JuyiViewAsSkill: public ZeroCardViewAsSkill{
public:
    JuyiViewAsSkill():ZeroCardViewAsSkill("jui"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasLordSkill("juyi")
                && player->getKingdom() == "qun"
                && !player->hasUsed("JuyiCard");
    }

    virtual const Card *viewAs() const{
        return new JuyiCard;
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

class Hengxing:public DrawCardsSkill{
public:
    Hengxing():DrawCardsSkill("hengxing"){
        frequency = Frequent;
    }

    virtual int getDrawNum(ServerPlayer *qiu, int n) const{
        if(qiu->isWounded())
            return n;
        Room *room = qiu->getRoom();
        int death = room->getPlayers().length() - room->getAlivePlayers().length();
        if(death > 0 && room->askForSkillInvoke(qiu, objectName())){
            room->playSkillEffect(objectName());
            return n + qMin(death, 2);
        }else
            return n;
    }
};

CujuCard::CujuCard(){
}

void CujuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    DamageStruct damage = effect.from->tag["CujuDamage"].value<DamageStruct>();
    damage.to = effect.to;
    damage.chain = true;
    room->damage(damage);
}

class CujuViewAsSkill: public OneCardViewAsSkill{
public:
    CujuViewAsSkill():OneCardViewAsSkill("cuju"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@cuju";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        CujuCard *card = new CujuCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Cuju: public TriggerSkill{
public:
    Cuju():TriggerSkill("cuju"){
        events << Predamaged;
        view_as_skill = new CujuViewAsSkill;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *gaoqiu, QVariant &data) const{
        if(!gaoqiu->isKongcheng() && gaoqiu->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club|spade):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = gaoqiu;

            Room *room = gaoqiu->getRoom();
            room->judge(judge);
            if(judge.isGood()){
                DamageStruct damage = data.value<DamageStruct>();
                gaoqiu->tag["CujuDamage"] = QVariant::fromValue(damage);
                if(room->askForUseCard(gaoqiu, "@@cuju", "@cuju-card"))
                    return true;
            }
        }
        return false;
    }
};

class Panquan: public TriggerSkill{
public:
    Panquan():TriggerSkill("panquan$"){
        events << HpRecover;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName()) && target->getKingdom() == "wei";
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *ply, QVariant &data) const{
        Room *room = ply->getRoom();
        ServerPlayer *gaoqiu = room->findPlayerBySkillName(objectName());
        if(!gaoqiu)
            return false;
        RecoverStruct recover = data.value<RecoverStruct>();
        for(int i = 0; i < recover.recover; i++){
            if(ply->askForSkillInvoke(objectName(), data)){
                gaoqiu->drawCards(2);
                room->playSkillEffect(objectName());
                const Card *ball = room->askForCardShow(gaoqiu, ply, objectName());
                room->moveCardTo(ball, NULL, Player::DrawPile);
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

    General *gaoqiu = new General(this, "gaoqiu$", "wei", 3);
    gaoqiu->addSkill(new Hengxing);
    gaoqiu->addSkill(new Cuju);
    gaoqiu->addSkill(new Panquan);

    addMetaObject<GanlinCard>();
    addMetaObject<JuyiCard>();
    addMetaObject<CujuCard>();
}

ADD_PACKAGE(TTXD)
