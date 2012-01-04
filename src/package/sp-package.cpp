#include "standard.h"
#include "skill.h"
#include "sp-package.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"

class Shemi: public TriggerSkill{
public:
    Shemi():TriggerSkill("shemi"){
        events << PhaseChange << TurnOvered;
    }

    virtual bool trigger(TriggerEvent e, ServerPlayer *emperor, QVariant &data) const{
        //Room *room = emperor->getRoom();
        if(e == PhaseChange){
            if(emperor->getPhase() == Player::Discard &&
               emperor->askForSkillInvoke(objectName(), data)){
                emperor->turnOver();
                return true;
            }
        }
        else{
            if(!emperor->hasFlag("NongQ")){
                int index = emperor->faceUp() ? 2: 1;
                emperor->getRoom()->playSkillEffect(objectName(), index);
            }
            int x = emperor->getLostHp();
            x = qMax(qMin(x,2),1);
            emperor->drawCards(x);
        }
        return false;
    }
};

class Lizheng: public DistanceSkill{
public:
    Lizheng():DistanceSkill("lizheng"){
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(!to->faceUp())
            return +1;
        else
            return 0;
    }
};

class Nongquan:public PhaseChangeSkill{
public:
    Nongquan():PhaseChangeSkill("nongquan$"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getKingdom() == "guan" && !target->hasLordSkill(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *otherguan) const{
        Room *room = otherguan->getRoom();
        if(otherguan->getPhase() != Player::Draw)
            return false;
        ServerPlayer *head = room->getLord();
        if(head->hasLordSkill(objectName()) && otherguan->getKingdom() == "guan"
           && otherguan->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());
            room->setPlayerFlag(head, "NongQ");
            head->turnOver();
            room->setPlayerFlag(head, "-NongQ");
            return true;
        }
        return false;
    }
};

class Shuntian: public TriggerSkill{
public:
    Shuntian():TriggerSkill("shuntian"){
        events << GameStart;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room *rom = player->getRoom();
        QString kim;
        if(!player->isLord())
            kim = rom->getLord()->getKingdom();
        rom->setPlayerProperty(player, "kingdom", kim);
        return false;
    }
};

YuzhongCard::YuzhongCard(){

}

bool YuzhongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int num = Self->getMark("YuZy");
    return targets.length() < num;
}

bool YuzhongCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    int num = Self->getMark("YuZy");
    return targets.length() <= num;
}

void YuzhongCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->drawCards(1);
}

class YuzhongViewAsSkill: public ZeroCardViewAsSkill{
public:
    YuzhongViewAsSkill():ZeroCardViewAsSkill("Yuzhong"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@yuzhong";
    }

    virtual const Card *viewAs() const{
        return new YuzhongCard;
    }
};

class Yuzhong: public TriggerSkill{
public:
    Yuzhong():TriggerSkill("yuzhong"){
        events << Death;
        view_as_skill = new YuzhongViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    static int getKingdoms(Room *room){
        QSet<QString> kingdom_set;
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            kingdom_set << p->getKingdom();
        }
        return kingdom_set.size();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        int num = getKingdoms(room);
        DamageStar damage = data.value<DamageStar>();
        if(damage->from != damage->to && damage->from->hasSkill(objectName())){
            ServerPlayer *source = damage->from;
            QString choice = room->askForChoice(source, objectName(), "hp+card+cancel");
            if(choice != "cancel"){
                LogMessage log;
                log.type = "#InvokeSkill";
                log.from = source;
                log.arg = "yuzhong";
                room->sendLog(log);
            }
            if(choice == "hp"){
                RecoverStruct rev;
                rev.who = source;
                rev.recover = num;
                room->recover(room->getLord(), rev);
            }
            else if(choice == "card"){
                room->getLord()->drawCards(num);
            }
        }
        return false;
    }
};

class Yuzhong2: public TriggerSkill{
public:
    Yuzhong2():TriggerSkill("#yuzh0ng"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        room->setPlayerMark(player, "YuZy", Yuzhong::getKingdoms(room));
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.to->isLord())
            return false;
        QString choice = room->askForChoice(player, "yuzhong", "all+me+cancel");
        if(choice == "cancel")
            return false;
        if(choice == "all")
            if(!room->askForUseCard(player, "@@yuzhong", "@yuzhong"))
                choice = "me";
        if(choice == "me"){
            LogMessage log;
            log.type = "#InvokeSkill";
            log.from = player;
            log.arg = "yuzhong";
            room->sendLog(log);
            player->drawCards(2);
        }

        room->setPlayerMark(player, "YuZy", 0);
        return false;
    }
};

SPPackage::SPPackage()
    :Package("sp")
{
    General *zhaoji = new General(this, "zhaoji$", "guan", 3);
    zhaoji->addSkill(new Shemi);
    zhaoji->addSkill(new Lizheng);
    zhaoji->addSkill(new Nongquan);

    General *jiangsong = new General(this, "jiangsong", "guan");
    jiangsong->addSkill(new Yuzhong);
    jiangsong->addSkill(new Yuzhong2);
    related_skills.insertMulti("yuzhong", "#yuzh0ng");
    jiangsong->addSkill(new Shuntian);

    addMetaObject<YuzhongCard>();
}

ADD_PACKAGE(SP);
