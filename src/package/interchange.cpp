#include "standard.h"
#include "skill.h"
#include "interchange.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"

class Xianxi: public TriggerSkill{
public:
    Xianxi():TriggerSkill("xianxi"){
        events << SlashMissed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        LogMessage log;
        log.from = player;
        log.type = "#Xianxi";
        log.arg = objectName();
        room->sendLog(log);
        if(player->getCardCount(true) >= 2){
            if(!room->askForDiscard(player, objectName(), 2, true, true))
                room->loseHp(player);
        }
        else
            room->loseHp(player);
        return false;
    }
};

class Lianzang: public TriggerSkill{
public:
    Lianzang():TriggerSkill("lianzang"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *tenkei = room->findPlayerBySkillName(objectName());
        if(!tenkei)
            return false;
        QVariantList eatdeath_skills = tenkei->tag["EatDeath"].toList();
        if(room->askForSkillInvoke(tenkei, objectName(), data)){
            QStringList eatdeaths;
            foreach(QVariant tmp, eatdeath_skills)
                eatdeaths << tmp.toString();
            if(!eatdeaths.isEmpty()){
                QString choice = room->askForChoice(tenkei, objectName(), eatdeaths.join("+"));
                room->detachSkillFromPlayer(tenkei, choice);
                eatdeath_skills.removeOne(choice);
            }
            room->loseMaxHp(tenkei);
            QList<const Skill *> skills = player->getVisibleSkillList();
            foreach(const Skill *skill, skills){
                if(skill->parent()){
                    QString sk = skill->objectName();
                    room->acquireSkill(tenkei, sk);
                    eatdeath_skills << sk;
                }
            }
            tenkei->tag["EatDeath"] = eatdeath_skills;
        }

        return false;
    }
};

ShensuanCard::ShensuanCard(){
}

bool ShensuanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.length() < subcardsLength() && to_select->isWounded();
}

bool ShensuanCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() <= subcardsLength();
}

void ShensuanCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(targets.isEmpty()){
        QList<ServerPlayer *> to;
        to << source;
        SkillCard::use(room, source, to);
    }else
        SkillCard::use(room, source, targets);
}

void ShensuanCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    if(effect.to->isWounded()){
        RecoverStruct recover;
        recover.card = this;
        recover.who = effect.from;
        room->recover(effect.to, recover);
    }else
        effect.to->drawCards(2);
}

class ShensuanViewAsSkill: public ViewAsSkill{
public:
    ShensuanViewAsSkill():ViewAsSkill("shensuan"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@shensuan";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 3)
            return false;
        int sum = 0;
        foreach(CardItem *item, selected){
            sum += item->getCard()->getNumber();
        }
        sum += to_select->getCard()->getNumber();
        return sum <= Self->getMark("shensuan");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        int sum = 0;
        foreach(CardItem *item, cards){
            sum += item->getCard()->getNumber();
        }

        if(sum == Self->getMark("shensuan")){
            ShensuanCard *card = new ShensuanCard;
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }
};

class Shensuan: public MasochismSkill{
public:
    Shensuan():MasochismSkill("shensuan"){
        view_as_skill = new ShensuanViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *jiangjing, const DamageStruct &damage) const{
        const Card *card = damage.card;
        if(card == NULL)
            return;

        int point = card->getNumber();
        if(point == 0)
            return;

        if(jiangjing->isNude())
            return;

        Room *room = jiangjing->getRoom();
        room->setPlayerMark(jiangjing, objectName(), point);

        QString prompt = QString("@shensuan:::%1").arg(point);
        room->askForUseCard(jiangjing, "@@shensuan", prompt);
    }
};

class Gunzhu:public TriggerSkill{
public:
    Gunzhu():TriggerSkill("gunzhu"){
        frequency = Frequent;
        events << CardUsed << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *jiangjing, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->isNDTrick()){
            int num = card->getNumber();
            Room *room = jiangjing->getRoom();
            if(room->askForSkillInvoke(jiangjing, objectName())){
                room->playSkillEffect(objectName());
                JudgeStruct judge;
                judge.reason = objectName();
                judge.who = jiangjing;

                room->judge(judge);
                if(qAbs(judge.card->getNumber() - num) < 3)
                    jiangjing->drawCards(2);
            }
        }
        return false;
    }
};

InterChangePackage::InterChangePackage()
    :Package("interchange")
{
    General *qinming = new General(this, "qinming", "guan");
    qinming->addSkill(new Xianxi);

    General *caiqing = new General(this, "caiqing", "jiang", 5);
    caiqing->addSkill(new Lianzang);

    General *jiangjing = new General(this, "jiangjing", "kou", 3);
    jiangjing->addSkill(new Shensuan);
    jiangjing->addSkill(new Gunzhu);

    addMetaObject<ShensuanCard>();
}

ADD_PACKAGE(InterChange);
