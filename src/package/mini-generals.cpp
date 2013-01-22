#include "miniscenarios.h"
#include "mini-generals.h"
#include "general.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"

FangdiaoCard::FangdiaoCard(){
    once = true;
    mute = true;
}

bool FangdiaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return Self->inMyAttackRange(to_select) && to_select != Self && !to_select->isKongcheng();
}

void FangdiaoCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &list) const{
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

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual int getPriority(TriggerEvent) const{
        return 3;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->getGeneral()->isFemale())
            return false;
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
    room->obtainCard(target, this);

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
        target->loseAllMarks("Yinlang");
        return false;
    }
};

BeishuiCard::BeishuiCard(){
    mute = true;
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
                    room->playSkillEffect(objectName(), qrand() % 2 + 1);
                    target->drawCards(x);
                }
                else
                    room->askForUseCard(jie, "@@beishui", "@beishui:::" + QString::number(x), true);
            }
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return qrand() % 2 + 3;
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
    int index = effect.from->getPhase() == Player::Start ? 1 : 3;
    effect.from->playSkillEffect(skill_name, qrand() % 2 + index);
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

class Pushou: public TriggerSkill{
public:
    Pushou():TriggerSkill("pushou"){
        view_as_skill = new PushouViewAsSkill;
        events << Pindian << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual int getPriority(TriggerEvent event) const{
        return event == Pindian ? -1 : 1;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        if(event == PhaseChange){
            if(!player->hasSkill(objectName()) || player->isKongcheng())
                return false;
            if(player->getPhase() == Player::Start ||
               player->getPhase() == Player::Finish)
                room->askForUseCard(player, "@@pushou", "@pushou", true);
            return false;
        }
        PindianStar pindian = data.value<PindianStar>();
        QList<ServerPlayer *> haras = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *hara, haras){
            if(pindian->from != hara && pindian->to != hara)
                continue;
            if(pindian->isSuccess()){
                room->playSkillEffect("pushou", pindian->from == hara ? 5 : 6);
                pindian->from->drawCards(1);
            }else{
                room->playSkillEffect("pushou", pindian->to == hara ? 5 : 6);
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

    virtual bool isEnabledAtNullification(const ServerPlayer *player, bool) const{
        foreach(const Card *card, player->getHandcards()){
            if(card->isBlack() || card->objectName() == "nullification")
                return true;
        }
        return false;
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
    use.to.clear();
    use.to << target;

    if(trick->inherits("Collateral")){
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *tmp, room->getOtherPlayers(target)){
            if(target->canSlash(tmp))
                targets << tmp;
        }
        if(!targets.isEmpty())
            use.to << room->askForPlayerChosen(card_use.from, targets, skill_name);
    }

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
        if(use.card->inherits("Collateral") && !player->getWeapon())
            return false;
        QList<ServerPlayer *> xis = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *xi, xis){
            if(xi != use.to.first() || player == xi)
                continue;
            if(xi->isNude())
                continue;
            if(xi->isProhibited(player, use.card))
                continue;
            xi->tag["Qi6ingData"] = data;
            QString prompt = QString("@qi6ing:%1::%2").arg(player->objectName()).arg(use.card->objectName());
            room->askForUseCard(xi, "@@qi6ing", prompt, true);
            xi->tag.remove("Qi6ingData");
        }
        return false;
    }
};

void MiniScene::addGenerals(int stage, bool show){
    switch(stage){
    case 21: {
            General *zhangbao = new General(this, "zhangbao", "jiang", 4, true, true, show);
            zhangbao->addSkill(new Fangdiao);
            addMetaObject<FangdiaoCard>();
            break;
        }
    case 22: {
            General *liruilan = new General(this, "liruilan", "min", 3, false, true, show);
            liruilan->addSkill(new Chumai);
            liruilan->addSkill(new Yinlang);
            addMetaObject<YinlangCard>();
            break;
        }
    case 20: {
            General *fangjie = new General(this, "fangjie", "jiang", 4, true, true, show);
            fangjie->addSkill(new Beishui);
            addMetaObject<BeishuiCard>();
            break;
        }
    case 23: {
            General *renyuan = new General(this, "renyuan", "jiang", 4, true, true, show);
            renyuan->addSkill(new Pushou);
            addMetaObject<PushouCard>();
            break;
        }
    case 24: {
            General *xisheng = new General(this, "xisheng", "min", 3, true, true, show);
            xisheng->addSkill(new Zhengbing);
            xisheng->addSkill(new Qi6ing);
            addMetaObject<Qi6ingCard>();
            break;
        }
    default:
        ;
    }
}
