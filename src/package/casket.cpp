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
            damage.to->gainJur("poison_jur", 3);
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
    foreach(int card_id, card_ids){
        const Card *card = Sanguosha->getCard(card_id);
        if(card->isRed()){
            room->takeAG(effect.to, card_id);
            //card_ids.removeOne(card_id);
            //effect.to->obtainCard(card);
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

    virtual bool trigger(TriggerEvent, Room*, ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;

        if(killer){
            killer->throwAllCards();
            killer->loseAllMarks("poison_jur");
        }
        return false;
    }
};

CasketPackage::CasketPackage()
    :GeneralPackage("casket")
{
    General *moon_panqiaoyun = new General(this, "moon_panqiaoyun", "min", 3, false);
    moon_panqiaoyun->addSkill(new Qingdu);
    moon_panqiaoyun->addSkill(new Tumi);
    moon_panqiaoyun->addSkill(new Jueyuan);

    addMetaObject<TumiCard>();
}

ADD_PACKAGE(Casket)
