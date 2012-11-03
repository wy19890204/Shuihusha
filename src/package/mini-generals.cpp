#include "miniscenarios.h"
#include "mini-generals.h"
#include "general.h"
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

BeishuiCard::BeishuiCard(){
}

bool BeishuiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int n = Self->getMark("BeishuiNum");
    return targets.length() < n && to_select != Self;
}

void BeishuiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName(skill_name);
    CardUseStruct use;
    use.card = slash;
    use.from = card_use.from;
    use.to = card_use.to;
    room->useCard(use);
}

class BeishuiViewAsSkill: public ZeroCardViewAsSkill{
public:
    BeishuiViewAsSkill(): ZeroCardViewAsSkill("Beishui"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual const Card *viewAs() const{
        return new BeishuiCard;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@beishui";
    }
};

class Beishui: public TriggerSkill{
public:
    Beishui():TriggerSkill("beishui"){
        events << CardLost;
        view_as_skill = new BeishuiViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *jie, QVariant &data) const{
        if(jie->isKongcheng()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Hand){
                QString choice = room->askForChoice(jie, objectName(), "bei+shui");
                int x = qMax(1, jie->getLostHp());
                room->setPlayerMark(jie, "BeishuiNum", x);
                if(choice == "bei"){
                    LogMessage log;
                    log.type = "#InvokeSkill";
                    log.from = jie;
                    log.arg = objectName();
                    room->sendLog(log);
                    ServerPlayer *target = room->askForPlayerChosen(jie, room->getAlivePlayers(), objectName());
                    room->playSkillEffect(objectName(), 1);
                    target->drawCards(x);
                }
                else
                    room->askForUseCard(jie, "@@beishui", "@beishui:" + QString::number(x), true);
            }
        }
        return false;
    }
};

PushouCard::PushouCard(){
    will_throw = false;
    mute = true;
}

bool PushouCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void PushouCard::use(Room *room, ServerPlayer *sbren, const QList<ServerPlayer *> &targets) const{
    room->playSkillEffect(skill_name, qrand() % 2 + 1);
    PlayerStar target = targets.first();
    bool success = sbren->pindian(target, skill_name, this);
    if(success){
        room->playSkillEffect(skill_name, 3);
        sbren->drawCards(1);
    }else{
        room->playSkillEffect(skill_name, 4);
        target->drawCards(1);
    }
}

class PushouViewAsSkill: public OneCardViewAsSkill{
public:
    PushouViewAsSkill():OneCardViewAsSkill("pushou"){
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@pushou";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        PushouCard *card = new PushouCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Pushou: public PhaseChangeSkill{
public:
    Pushou():PhaseChangeSkill("pushou"){
        view_as_skill = new PushouViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Start ||
           target->getPhase() == Player::Finish){
            target->getRoom()->askForUseCard(target, "@@pushou", "@pushou", true);
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

class Cihu: public MasochismSkill{
public:
    Cihu():MasochismSkill("cihu"){
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

void MiniScene::addGenerals(int stage){
    switch(stage){
    case 1: {
            General *zhangbao = new General(this, "zhangbao", "jiang", 4, true, true);
            zhangbao->addSkill(new Fangdiao);
            addMetaObject<FangdiaoCard>();
            break;
        }
    case 2: {
            General *liruilan = new General(this, "liruilan", "min", 3, false, true);
            liruilan->addSkill(new Chumai);
            liruilan->addSkill(new Yinlang);
            addMetaObject<YinlangCard>();
            break;
        }
    case 3: {
            General *fangjie = new General(this, "fangjie", "jiang", 4, true, true);
            fangjie->addSkill(new Beishui);
            addMetaObject<BeishuiCard>();
            break;
        }
    case 4: {
            General *renyuan = new General(this, "renyuan", "jiang", 4, true, true);
            renyuan->addSkill(new Pushou);
            addMetaObject<PushouCard>();
            break;
        }
    default:
        ;
    }
}
