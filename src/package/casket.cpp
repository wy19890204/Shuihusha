#include "casket.h"
#include "standard.h"

class Qingdu: public TriggerSkill{
public:
    Qingdu():TriggerSkill("qingdu"){
        events << Damage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , Room*, ServerPlayer *, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to->getGender() == General::Male)
            damage.to->gainJur("poison_jur", 5);
        return false;
    }
};

TumiCard::TumiCard(){
}

bool TumiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    return targets.isEmpty() && to_select->getGenderString() == "male" && !to_select->isKongcheng();
}

void TumiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->obtainCard(effect.from, effect.to->wholeHandCards(), false);

    QList<int> card_ids = effect.from->handCards();
    room->fillAG(card_ids);
    //room->askForAG(effect.from, card_ids, true, skill_name);
    QList<const Card *> cards = effect.from->getHandcards();
    foreach(const Card *card, cards){
        if(card->isRed()){
            room->getThread()->delay();
            effect.to->obtainCard(card);
            //room->takeAG(effect.to, card->getId());
        }
    }
    room->broadcastInvoke("clearAG");
}

class TumiViewAsSkill: public ZeroCardViewAsSkill{
public:
    TumiViewAsSkill():ZeroCardViewAsSkill("tumi"){
    }

    virtual const Card *viewAs() const{
        return new TumiCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@tumi";
    }
};

class Tumi: public PhaseChangeSkill{
public:
    Tumi():PhaseChangeSkill("tumi"){
        view_as_skill = new TumiViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *qiaoyun) const{
        Room *room = qiaoyun->getRoom();
        if(qiaoyun->getPhase() == Player::Start)
            room->askForUseCard(qiaoyun, "@@tumi", "@tumi", true);

        return false;
    }
};

class Jueyuan: public TriggerSkill{
public:
    Jueyuan():TriggerSkill("jueyuan"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;

        if(killer){
            LogMessage log;
            log.from = player;
            log.type = "#TriggerSkill";
            log.arg = objectName();
            room->sendLog(log);
            killer->throwAllCards();
            killer->loseAllMarks("poison_jur");
        }
        return false;
    }
};

FanyinCard::FanyinCard(){
}

bool FanyinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.length() < subcardsLength() && to_select != Self;
}

void FanyinCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->gainJur("sleep_jur", 2);
}

class Fanyin: public ViewAsSkill{
public:
    Fanyin(): ViewAsSkill("fanyin"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("FanyinCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        foreach(CardItem *item, selected){
            if(to_select->getFilteredCard()->getTypeId() == item->getFilteredCard()->getTypeId())
                return false;
        }
        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() < 2)
            return NULL;
        FanyinCard *card = new FanyinCard;
        card->addSubcards(cards);
        return card;
    }
};

class Lichen: public DrawCardsSkill{
public:
    Lichen():DrawCardsSkill("lichen"){
    }

    virtual int getDrawNum(ServerPlayer *tulv, int n) const{
        Room *room = tulv->getRoom();
        int x = 0;
        foreach(ServerPlayer *tmp, room->getAllPlayers()){
            if(tmp->hasMark("sleep_jur"))
                x++;
        }
        x = qMin(x, 4);
        if(x > 0 && room->askForSkillInvoke(tulv, objectName())){
            room->setPlayerMark(tulv, objectName(), x);
            return n + x;
        }else
            return n;
    }

    virtual void drawDone(ServerPlayer *heshang, int) const{
        Room *room = heshang->getRoom();
        int x = heshang->getMark(objectName());
        if(x > 2){
            ServerPlayer *target = room->askForPlayerChosen(heshang, room->getOtherPlayers(heshang), objectName());
            DamageStruct daah;
            daah.from = target;
            daah.to = heshang;
            room->damage(daah);
        }
        room->setPlayerMark(heshang, objectName(), 0);
    }
};

class Lunhui: public TriggerSkill{
public:
    Lunhui():TriggerSkill("lunhui"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;
        if(killer){
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = objectName();
            room->sendLog(log);
            room->loseMaxHp(killer);
            room->acquireSkill(killer, objectName());
            killer->playSkillEffect(objectName());
        }
        return false;
    }
};

CasketPackage::CasketPackage()
    :GeneralPackage("casket")
{
    General *moon_panqiaoyun = new General(this, "moon_panqiaoyun", "moon", 3, false);
    moon_panqiaoyun->addSkill(new Qingdu);
    moon_panqiaoyun->addSkill(new Tumi);
    moon_panqiaoyun->addSkill(new Jueyuan);

    General *sun_peiruhai = new General(this, "sun_peiruhai", "sun", 4);
    sun_peiruhai->addSkill("#hp-1");
    sun_peiruhai->addSkill(new Fanyin);
    sun_peiruhai->addSkill(new Lichen);
    sun_peiruhai->addSkill(new Lunhui);

    addMetaObject<TumiCard>();
    addMetaObject<FanyinCard>();
}

ADD_PACKAGE(Casket)
