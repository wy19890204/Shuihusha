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
    mute = true;
}

bool FangdiaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return Self->inMyAttackRange(to_select) && to_select != Self && !to_select->isKongcheng();
}

void FangdiaoCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &list) const{
    room->throwCard(this, source);
    room->playSkillEffect(skill_name, 1);
    PlayerStar target = list.first();
    const Card *spade = room->askForCard(target, ".S", "@fangdiao:" + source->objectName(), QVariant::fromValue((PlayerStar)source), NonTrigger);
    if(spade)
        room->obtainCard(source, spade);
    else{
        QString choice = room->askForChoice(source, skill_name, "fang+diao", QVariant::fromValue(target));
        if(choice == "fang"){
            room->playSkillEffect(skill_name, 2);
            room->swapHandcards(source, target);
        }
        else{
            foreach(ServerPlayer *tmp, room->getAlivePlayers()){
                if(source->distanceTo(tmp) <= 1)
                    tmp->drawCards(1);
            }
            room->playSkillEffect(skill_name, 3);
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
                    room->askForUseCard(jie, "@@beishui", "@beishui:::" + QString::number(x), true);
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

void PushouCard::onEffect(const CardEffectStruct &effect) const{
    effect.from->playSkillEffect(skill_name, qrand() % 2 + 1);
    effect.from->pindian(effect.to, skill_name, effect.card);
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

class PushouPindian: public TriggerSkill{
public:
    PushouPindian():TriggerSkill("#pushou_pd"){
        events << Pindian;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, Room *room, ServerPlayer *, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        QList<ServerPlayer *> haras = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *hara, haras){
            if(pindian->from != hara && pindian->to != hara)
                continue;
            if(pindian->isSuccess()){
                room->playSkillEffect("pushou", 3);
                pindian->from->drawCards(1);
            }else{
                room->playSkillEffect("pushou", 4);
                pindian->to->drawCards(1);
            }
        }
        return false;
    }
};

class Zhengbing: public OneCardViewAsSkill{
public:
    Zhengbing():OneCardViewAsSkill("zhengbing"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isBlack() && !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "nullification" || pattern == "nulliplot";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getFilteredCard();
        Card *ncard = new Nullification(first->getSuit(), first->getNumber());
        ncard->addSubcard(first);
        ncard->setSkillName(objectName());

        return ncard;
    }
};

Qi6ingCard::Qi6ingCard(){
    target_fixed = true;
    mute = true;
}

void Qi6ingCard::onUse(Room *room, const CardUseStruct &card_use) const{
    CardUseStruct table = card_use.from->tag["Qi6ingData"].value<CardUseStruct>();
    PlayerStar target = table.from;
    QString card_name = table.card->objectName();

    CardStar trick = Sanguosha->getCard(getSubcards().first());
    Card *card = Sanguosha->cloneCard(card_name, trick->getSuit(), trick->getNumber());
    card->setSkillName(skill_name);
    card->addSubcard(trick);

    CardUseStruct use = card_use;
    use.card = card;
    use.to << target;
    room->useCard(use);
}

class Qi6ingViewAsSkill: public OneCardViewAsSkill{
public:
    Qi6ingViewAsSkill():OneCardViewAsSkill("qi6ing"){
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Qi6ingCard *card = new Qi6ingCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@qi6ing";
    }
};

class Qi6ing: public TriggerSkill{
public:
    Qi6ing():TriggerSkill("qi6ing"){
        events << CardFinished;
        view_as_skill = new Qi6ingViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(!use.card->isNDTrick() || (use.to.length() != 1 && !use.card->inherits("Collateral")))
            return false;
        QList<ServerPlayer *> xis = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *xi, xis){
            if(xi != use.to.first() || player == xi)
                continue;
            if(xi->isNude())
                continue;
            xi->tag["Qi6ingData"] = data;
            QString prompt = QString("@qi6ing:%1::%2").arg(player->objectName()).arg(use.card->objectName());
            room->askForUseCard(xi, "@@qi6ing", prompt, true);
            xi->tag.remove("Qi6ingData");
        }
        return false;
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
            renyuan->addSkill(new PushouPindian);
            related_skills.insertMulti("pushou", "#pushou_pd");
            addMetaObject<PushouCard>();
            break;
        }
    case 5: {
            General *xisheng = new General(this, "xisheng", "min", 3, true, true);
            xisheng->addSkill(new Zhengbing);
            xisheng->addSkill(new Qi6ing);
            addMetaObject<Qi6ingCard>();
            break;
        }
    default:
        ;
    }
}
