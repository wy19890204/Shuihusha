#include "mini.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"

FangdiaoCard::FangdiaoCard(){
    will_throw = false;
    once = true;
}

bool FangdiaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return Self->inMyAttackRange(to_select) && to_select != Self && !to_select->isKongcheng();
}

void FangdiaoCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &list) const{
    room->throwCard(this, source);
    PlayerStar target = list.first();
    const Card *spade = room->askForCard(target, ".S$", "@fangdiao:" + source->objectName(), QVariant::fromValue((PlayerStar)source));
    if(spade)
        source->obtainCard(spade, false);
    else{
        QString choice = room->askForChoice(source, skill_name, "fang+diao", QVariant::fromValue(target));
        if(choice == "fang"){
            room->swapHandcards(source, target);
        }
        else{
            foreach(ServerPlayer *tmp, room->getAlivePlayers()){
                if(target->distanceTo(tmp) <= 1)
                    tmp->drawCards(1);
            }
        }
    }
}

class Fangdiao: public OneCardViewAsSkill{
public:
    Fangdiao():OneCardViewAsSkill("fangdiao"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("FangdiaoCard");
    }

    virtual bool viewFilter(const CardItem *shoupai) const{
        return !shoupai->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        FangdiaoCard *card = new FangdiaoCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};


class Chumai: public TriggerSkill{
public:
    Chumai():TriggerSkill("chumai"){
        events << CardLost;
    }

    virtual bool triggerable(const ServerPlayer *ta) const{
        return ta->getGeneral()->isMale();
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *ran = room->findPlayerBySkillName(objectName());
        if(!ran || room->getCurrent() == ran)
            return false;
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->from->isAlive() && move->to_place == Player::DiscardedPile){
            const Card *equ = Sanguosha->getCard(move->card_id);
            QVariant chu = QVariant::fromValue((PlayerStar)player);
            if(move->from->getHp() > 0 && (equ->inherits("Weapon") || equ->inherits("Armor")) &&
               room->askForCard(ran, ".|.|.|hand|black", "@chumai:" + player->objectName(), true, chu, CardDiscarded)){
                room->playSkillEffect(objectName());
                LogMessage log;
                log.type = "#InvokeSkill";
                log.from = ran;
                log.arg = objectName();
                room->sendLog(log);
                room->loseHp(player);
            }
        }
        return false;
    }
};

YinlangCard::YinlangCard(){
    will_throw = false;
}

bool YinlangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getGeneral()->isMale();
}

void YinlangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
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
    room->obtainCard(target, this, false);

    int num = 0;
    foreach(int x, this->getSubcards()){
        if(Sanguosha->getCard(x)->inherits("Weapon") ||
           Sanguosha->getCard(x)->inherits("Armor"))
            num ++;
    }

    int old_value = source->getMark("Yinlang");
    int new_value = old_value + num;
    room->setPlayerMark(source, "Yinlang", new_value);

    if(old_value < 1 && new_value >= 1){
        RecoverStruct recover;
        recover.card = this;
        recover.who = source;
        room->recover(source, recover);
    }
}

class YinlangViewAsSkill:public ViewAsSkill{
public:
    YinlangViewAsSkill():ViewAsSkill("yinlang"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->inherits("EquipCard");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        YinlangCard *ard = new YinlangCard;
        ard->addSubcards(cards);
        return ard;
    }
};

class Yinlang: public PhaseChangeSkill{
public:
    Yinlang():PhaseChangeSkill("yinlang"){
        view_as_skill = new YinlangViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::NotActive
                && target->hasUsed("YinlangCard");
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        target->getRoom()->setPlayerMark(target, "Yinlang", 0);
        return false;
    }
};

class Longjiao:public TriggerSkill{
public:
    Longjiao():TriggerSkill("longjiao"){
        events << CardUsed;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *, QVariant &data) const{
        ServerPlayer *zou = room->findPlayerBySkillName(objectName());
        if(!zou)
            return false;
        CardUseStruct effect = data.value<CardUseStruct>();
        bool caninvoke = false;
        if(effect.card->isNDTrick()){
            if(effect.to.contains(zou))
                caninvoke = true; //指定自己为目标
            //if(effect.card->inherits("GlobalEffect"))
            //    caninvoke = true; //指定所有人为目标
            //if(effect.card->inherits("AOE") && effect.from != zou)
            //    caninvoke = true; //其他人使用的AOE
            if(effect.from == zou && effect.to.isEmpty() && effect.card->inherits("ExNihilo"))
                caninvoke = true; //自己使用的无中生有
        }
        if(caninvoke && room->askForSkillInvoke(zou, objectName(), data)){
            zou->drawCards(2);
            room->playSkillEffect(objectName());
            QList<int> card_ids = zou->handCards().mid(zou->getHandcardNum() - 2);
            room->fillAG(card_ids, zou);
            int card_id = room->askForAG(zou, card_ids, false, objectName());
            room->broadcastInvoke("clearAG");
            room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::DrawPile);
        }
        return false;
    }
};

class Juesi: public TriggerSkill{
public:
    Juesi():TriggerSkill("juesi"){
        events << DamageProceed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *caifu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.to->isAlive() && damage.to->getHp() <= 1){
            LogMessage log;
            log.type = "#JuesiBuff";
            log.from = caifu;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            room->sendLog(log);

            damage.damage ++;
            room->playSkillEffect(objectName());
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class Tuzai: public TriggerSkill{
public:
    Tuzai():TriggerSkill("tuzai"){
        events << Damage;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") &&
           damage.to && !damage.to->isKongcheng()
            && player->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            int dust = damage.to->getRandomHandCardId();
            room->showCard(damage.to, dust);

            if(Sanguosha->getCard(dust)->isRed()){
                room->throwCard(dust, damage.to, player);
                player->drawCards(1);
            }
        }
        return false;
    }
};

CihuCard::CihuCard(){
}

bool CihuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getGeneral()->isFemale() && to_select->isWounded();
}

bool CihuCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() < 2;
}

void CihuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this, source);
    ServerPlayer *ogami = source->tag["CihuOgami"].value<PlayerStar>();
    DamageStruct damage;
    damage.from = source;
    damage.to = ogami;
    room->damage(damage);
    PlayerStar target = !targets.isEmpty() ? targets.first() :
                        (source->getGeneral()->isFemale() && source->isWounded()) ?
                        source : NULL;
    if(target){
        RecoverStruct recover;
        recover.who = source;
        room->recover(target, recover, true);
    }
}

class CihuViewAsSkill: public ViewAsSkill{
public:
    CihuViewAsSkill(): ViewAsSkill("Cihu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        int n = Self->getMark("CihuNum");
        if(selected.length() >= n)
            return false;
        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        int n = Self->getMark("CihuNum");
        if(cards.length() != n)
            return NULL;

        CihuCard *card = new CihuCard;
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@cihu";
    }
};

class Cihu: public MasochismSkill{
public:
    Cihu():MasochismSkill("cihu"){
        view_as_skill = new CihuViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getGeneral()->isFemale();
    }

    virtual void onDamaged(ServerPlayer *akaziki, const DamageStruct &damage) const{
        Room *room = akaziki->getRoom();
        ServerPlayer *tiger = room->findPlayerBySkillName(objectName());
        if(!tiger || !damage.card || !damage.card->inherits("Slash"))
            return;
        PlayerStar ogami = damage.from;
        if(!ogami || !ogami->getGeneral()->isMale())
            return;
        if(tiger->getCardCount(true) >= akaziki->getHp()){
            room->setPlayerMark(tiger, "CihuNum", akaziki->getHp());
            tiger->tag["CihuOgami"] = QVariant::fromValue(ogami);
            QString prompt = QString("@cihu:%1::%2").arg(ogami->getGeneralName()).arg(akaziki->getGeneralName());
            room->askForUseCard(tiger, "@@cihu", prompt, true);
            tiger->tag.remove("CihuOgami");
            room->setPlayerMark(tiger, "CihuNum", 0);
        }
    }
};
