#include "standard.h"
#include "skill.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "qlfd.h"
#include "tocheck.h"

YushuiCard::YushuiCard(){
    once = true;
}

bool YushuiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return to_select->getGeneral()->isMale() && to_select->isWounded();
}

void YushuiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;

    room->recover(effect.from, recover, true);
    room->recover(effect.to, recover, true);
/*
    int index = -1;
    if(effect.from->getGeneral()->isMale()){
        if(effect.from == effect.to)
            index = 5;
        else if(effect.from->getHp() >= effect.to->getHp())
            index = 3;
        else
            index = 4;
    }else{
        index = 1 + qrand() % 2;
    }

    room->playSkillEffect("Yushui", index);
*/
    effect.from->drawCards(2);
    effect.to->drawCards(2);
    effect.from->turnOver();
    effect.to->turnOver();
}

class Yushui: public OneCardViewAsSkill{
public:
    Yushui():OneCardViewAsSkill("yushui"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("YushuiCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        YushuiCard *card = new YushuiCard;
        card->addSubcard(card_item->getCard()->getId());

        return card;
    }
};

class Shengui: public ProhibitSkill{
public:
    Shengui():ProhibitSkill("shengui"){
    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        return !to->faceUp() && from->getGeneral()->isMale() && card->inherits("TrickCard") && !card->inherits("Collateral");
    }
};

class Zhensha:public TriggerSkill{
public:
    Zhensha():TriggerSkill("zhensha"){
        frequency = Limited;
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *xing = room->findPlayerBySkillName(objectName());
        if(!xing || xing->getMark("@vi") < 1)
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Analeptic") && room->askForCard(xing, ".S", "@zhensha:" + use.from->objectName(), data)){
            xing->loseMark("@vi");
            room->broadcastInvoke("animate", "lightbox:$zhensha:2000");
            LogMessage log;
            log.type = "#Zhensha";
            log.from = xing;
            log.to << use.from;
            log.arg = objectName();
            room->sendLog(log);
            room->killPlayer(use.from);
        }
        return false;
    }
};

FanwuCard::FanwuCard(){
    will_throw = false;
}

bool FanwuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select->getGeneral()->isMale();
}

void FanwuCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);
    DamageStruct damage = effect.from->tag["FanwuStruct"].value<DamageStruct>();
    damage.from = effect.to;
    effect.from->tag["FanwuStruct"] = QVariant::fromValue(damage);
}

class FanwuViewAsSkill: public OneCardViewAsSkill{
public:
    FanwuViewAsSkill():OneCardViewAsSkill("fanwu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@fanwu";
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        FanwuCard *card = new FanwuCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

class Fanwu: public TriggerSkill{
public:
    Fanwu():TriggerSkill("fanwu"){
        events << Predamage;
        view_as_skill = new FanwuViewAsSkill;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!player->isNude() && damage.to != damage.from){
            Room *room = player->getRoom();
            player->tag["FanwuStruct"] = data;
            if(room->askForUseCard(player, "@@fanwu", "@fanwu")){
                DamageStruct damage2 = player->tag["FanwuStruct"].value<DamageStruct>();
                if(damage2.from){
                    damage.from = damage2.from;
                    data = QVariant::fromValue(damage);
                }
            }
        }
        player->tag.remove("FanwuStruct");
        return false;
    }
};

class Foyuan: public TriggerSkill{
public:
    Foyuan():TriggerSkill("foyuan"){
        events << CardEffected;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.from && effect.to == player && effect.from->getGeneral()->isMale() && effect.from->getEquips().isEmpty()
            && (effect.card->isNDTrick() || effect.card->inherits("Slash"))){
            LogMessage log;
            log.type = "#Foyuan";
            log.from = effect.from;
            Room *room = player->getRoom();
            log.to << effect.to;
            log.arg = effect.card->objectName();
            log.arg2 = objectName();

            room->sendLog(log);
            room->playSkillEffect(objectName());
            return true;
        }
        return false;
    }
};

class Panxin: public TriggerSkill{
public:
    Panxin():TriggerSkill("panxin"){
        events << PreDeath;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        ServerPlayer *qiaoyun = room->findPlayerBySkillName(objectName());
        if(!qiaoyun || qiaoyun->isLord() || player->isLord() || qiaoyun == player)
            return false;
        if(qiaoyun->getMark("@pfxl") && qiaoyun->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());
            room->broadcastInvoke("animate", "lightbox:$panxin:2000");
            QString role = player->getRole();
            player->setRole(qiaoyun->getRole());
            room->setPlayerProperty(player, "panxin", true);
            qiaoyun->loseMark("@pfxl");
            qiaoyun->setRole(role);
            qiaoyun->drawCards(1);
            room->getThread()->delay(1500);
        }
        return false;
    }
};

class Meicha: public OneCardViewAsSkill{
public:
    Meicha():OneCardViewAsSkill("meicha"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Analeptic::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("analeptic");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Analeptic *analeptic = new Analeptic(card->getSuit(), card->getNumber());
        analeptic->setSkillName(objectName());
        analeptic->addSubcard(card->getId());
        return analeptic;
    }
};

QianxianCard::QianxianCard(){
    once = true;
}

bool QianxianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select == Self)
        return false;
    if(targets.isEmpty())
        return true;
    if(targets.length() == 1){
        int max1 = targets.first()->getMaxHP();
        return to_select->getMaxHP() != max1;
    }
    return false;
}

bool QianxianCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(targets.length() != 2)
        return false;
    int max1 = targets.first()->getMaxHP();
    int max2 = targets.last()->getMaxHP();
    return max1 != max2;
}

void QianxianCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    const Card *club = room->askForCard(effect.to, ".C", "@qianxian:" + effect.from->objectName(), QVariant::fromValue(effect));
    if(club){
        effect.from->obtainCard(club);
        if(!effect.to->faceUp())
            effect.to->turnOver();
        room->setPlayerProperty(effect.to, "chained", false);
    }
    else{
        if(effect.to->faceUp())
            effect.to->turnOver();
        room->setPlayerProperty(effect.to, "chained", true);
    }
}

class Qianxian: public OneCardViewAsSkill{
public:
    Qianxian():OneCardViewAsSkill("qianxian"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QianxianCard");
    }

    virtual bool viewFilter(const CardItem *blackfeiyanshitrick) const{
        const Card *card = blackfeiyanshitrick->getCard();
        return card->isBlack() && card->isNDTrick();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        QianxianCard *card = new QianxianCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

class Zhongzhen: public TriggerSkill{
public:
    Zhongzhen():TriggerSkill("zhongzhen"){
        events << Predamaged;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *linko, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.from || !damage.from->getGeneral()->isMale() || damage.damage <= 0)
            return false;
        if(!linko->isKongcheng() && !damage.from->isKongcheng() &&
           linko->askForSkillInvoke(objectName(), data)){
            linko->getRoom()->playSkillEffect(objectName());
            bool success = linko->pindian(damage.from, objectName());
            if(success){
                LogMessage log;
                log.type = "#Zhongzhen";
                log.from = damage.from;
                log.to << linko;
                log.arg = QString::number(damage.damage);
                linko->getRoom()->sendLog(log);
                return true;
            }
        }
        return false;
    }
};

ZiyiCard::ZiyiCard(){
}

bool ZiyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select->getGeneral()->isMale() && to_select->isWounded();
}

void ZiyiCard::onEffect(const CardEffectStruct &effect) const{
    effect.from->loseAllMarks("@ziyi");
    RecoverStruct r;
    Room *o = effect.from->getRoom();
    o->broadcastInvoke("animate", "lightbox:$ziyi:3000");
    r.who = effect.from;
    r.recover = 2;
    o->recover(effect.to, r);
    o->getThread()->delay(3000);
    o->killPlayer(effect.from);
}

class Ziyi: public ZeroCardViewAsSkill{
public:
    Ziyi():ZeroCardViewAsSkill("ziyi"){
        frequency = Limited;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@ziyi") > 0 && player->isWounded();
    }

    virtual const Card *viewAs() const{
        return new ZiyiCard;
    }
};

ShouwangCard::ShouwangCard(){
    once = true;
}

bool ShouwangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select->getGeneral()->isMale();
}

void ShouwangCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);
    if(effect.from->getRoom()->askForChoice(effect.from, "shouwang", "tian+zi") == "tian")
        effect.to->drawCards(1);
    else
        effect.from->drawCards(1);
}

class Shouwang: public OneCardViewAsSkill{
public:
    Shouwang():OneCardViewAsSkill("shouwang"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ShouwangCard");
    }

    virtual bool viewFilter(const CardItem *watch) const{
        return watch->getCard()->inherits("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ShouwangCard *card = new ShouwangCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

ZishiCard::ZishiCard(){
    target_fixed = true;
}

void ZishiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    int num = this->getSubcards().length();
    room->throwCard(this);
    source->tag["ZiShi"] = num;
}

class ZishiViewAsSkill: public ViewAsSkill{
public:
    ZishiViewAsSkill():ViewAsSkill("zishi"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 3 && to_select->getCard()->isBlack();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@zishi";
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() > 2 && !cards.isEmpty())
            return NULL;
        ZishiCard *card = new ZishiCard;
        card->addSubcards(cards);
        return card;
    }
};

class Zishi:public DrawCardsSkill{
public:
    Zishi():DrawCardsSkill("zishi"){
        view_as_skill = new ZishiViewAsSkill;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getGeneral()->isMale();
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        Room *room = player->getRoom();
        ServerPlayer *duan3niang = room->findPlayerBySkillName(objectName());
        if(duan3niang && room->askForUseCard(duan3niang, "@@zishi", "@zishi:" + player->objectName())){
            int delta = duan3niang->tag["ZiShi"].toInt();
            if(delta > 0){
                QString choice = room->askForChoice(duan3niang, objectName(), "duo+shao");
                LogMessage log;
                log.type = "#Zishi";
                log.from = duan3niang;
                log.to << player;
                log.arg = QString::number(delta);
                log.arg2 = choice == "duo" ? "duo" : "shao";
                if(choice == "duo")
                    n = n + delta;
                else
                    n = n - delta;
                room->sendLog(log);
            }
            duan3niang->tag.remove("ZiShi");
        }
        return n;
    }
};

SuocaiCard::SuocaiCard(){
    will_throw = false;
}

bool SuocaiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select->getGeneral()->isMale();
}

void SuocaiCard::use(Room *, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *yanp = targets.first();
    source->pindian(yanp, "suocai", this);
}

class SuocaiPindian: public OneCardViewAsSkill{
public:
    SuocaiPindian():OneCardViewAsSkill("suocai"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("SuocaiCard") && !player->isKongcheng();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        SuocaiCard *card = new SuocaiCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return ! to_select->isEquipped();
    }
};

class Suocai: public TriggerSkill{
public:
    Suocai():TriggerSkill("suocai"){
        events << Pindian;
        view_as_skill = new SuocaiPindian;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        if(player == pindian->from && pindian->reason == objectName() &&
           pindian->isSuccess()){
            player->obtainCard(pindian->from_card);
            player->obtainCard(pindian->to_card);
        }
        return false;
    }
};

class Huakui: public MasochismSkill{
public:
    Huakui():MasochismSkill("huakui"){
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual void onDamaged(ServerPlayer *other, const DamageStruct &damage) const{
        Room *room = other->getRoom();
        ServerPlayer *loli = room->findPlayerBySkillName(objectName());
        if(loli){
            if(loli->distanceTo(other) < 2 && loli->askForSkillInvoke(objectName()))
                loli->drawCards(1);
        }
    }
};

EyanCard::EyanCard(){
    once = true;
}

bool EyanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select->getGeneral()->isMale() && to_select->inMyAttackRange(Self);
}

void EyanCard::onEffect(const CardEffectStruct &effect) const{
    PlayerStar target = effect.to;
    Room *room = effect.from->getRoom();
    const Card *slash = NULL;
    if(effect.from->canSlash(target)){
        QVariant source = QVariant::fromValue((PlayerStar)effect.from);
        slash = room->askForCard(target, "slash", "@eyan:" + effect.from->objectName(), source);
    }
    if(slash){
        CardUseStruct use;
        use.card = slash;
        use.from = target;
        use.to << effect.from;
        room->useCard(use);
    }
    else{
        room->attachSkillToPlayer(effect.from, "eyanslash");
        room->setPlayerFlag(target, "EyanTarget");
    }
}

class Eyan: public ZeroCardViewAsSkill{
public:
    Eyan():ZeroCardViewAsSkill("eyan"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("EyanCard");
    }

    virtual const Card *viewAs() const{
        return new EyanCard;
    }
};

EyanSlashCard::EyanSlashCard(){
}

bool EyanSlashCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select->hasFlag("EyanTarget");
}

void EyanSlashCard::onEffect(const CardEffectStruct &effect) const{
    CardUseStruct use;
    use.from = effect.from;
    use.to << effect.to;
    use.card = Sanguosha->getCard(this->getSubcards().first());
    effect.from->getRoom()->useCard(use, false);
}

class EyanSlash: public OneCardViewAsSkill{
public:
    EyanSlash():OneCardViewAsSkill("eyanslash"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        EyanSlashCard *card = new EyanSlashCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

ZhangshiCard::ZhangshiCard(){

}

bool ZhangshiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->canSlash(to_select);
}

void ZhangshiCard::use(Room *room, ServerPlayer *white, const QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> men = room->getMenorWomen("male");
    const Card *slash = NULL;
    QVariant tohelp = QVariant::fromValue((PlayerStar)white);
    foreach(ServerPlayer *man, men){
        slash = room->askForCard(man, "slash", "@zhangshi:" + white->objectName(), tohelp);
        if(slash){
            CardUseStruct card_use;
            card_use.card = slash;
            card_use.from = white;
            card_use.to << targets.first();
            room->useCard(card_use);
            return;
        }
    }
}

class ZhangshiViewAsSkill:public ZeroCardViewAsSkill{
public:
    ZhangshiViewAsSkill():ZeroCardViewAsSkill("zhangshi$"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual const Card *viewAs() const{
        return new ZhangshiCard;
    }
};

class Zhangshi: public TriggerSkill{
public:
    Zhangshi():TriggerSkill("zhangshi"){
        events << CardAsked;
        view_as_skill = new ZhangshiViewAsSkill;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *white, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "slash")
            return false;
        Room *room = white->getRoom();
        QList<ServerPlayer *> men = room->getMenorWomen("male");
        if(men.isEmpty() || !white->askForSkillInvoke(objectName()))
            return false;
        room->playSkillEffect(objectName());
        QVariant tohelp = QVariant::fromValue((PlayerStar)white);
        foreach(ServerPlayer *man, men){
            const Card *slash = room->askForCard(man, "slash", "@zhangshi:" + white->objectName(), tohelp);
            if(slash){
                room->provide(slash);
                return true;
            }
        }
        return false;
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

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *ran = room->findPlayerBySkillName(objectName());
        if(!ran || room->getCurrent() == ran)
            return false;
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->from->isAlive() && move->to_place == Player::DiscardedPile){
            const Card *equ = Sanguosha->getCard(move->card_id);
            QVariant chu = QVariant::fromValue((PlayerStar)player);
            if(move->from->getHp() > 0 && (equ->inherits("Weapon") || equ->inherits("Armor")) &&
               room->askForCard(ran, ".black", "@chumai:" + player->objectName(), chu)){
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
    room->moveCardTo(this, target, Player::Hand, false);

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

class Chiyuan:public TriggerSkill{
public:
    Chiyuan():TriggerSkill("chiyuan"){
        events << HpRecovered;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getGeneral()->isMale();
    }

    virtual QString getDefaultChoice(ServerPlayer *player) const{
        if(player->getLostHp() > 1)
            return "qiao";
        else
            return "nu";
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *anu = room->findPlayerBySkillName(objectName());
        if(!anu || anu->isKongcheng())
            return false;
        RecoverStruct rec = data.value<RecoverStruct>();
        for(int i = rec.recover; i > 0; i--){
            if(!room->askForCard(anu, "..", "@chiyuan:" + player->objectName(), data))
                break;
            LogMessage age;
            age.type = "#Chiyuan";
            age.from = anu;
            age.arg = objectName();
            age.to << player;
            room->sendLog(age);

            JudgeStruct jd;
            jd.reason = objectName();
            jd.who = rec.who;
            room->judge(jd);

            if(jd.card->isBlack()){
                room->playSkillEffect(objectName(), 2);
                int cardnum = player->getCardCount(true);
                room->askForDiscard(player, objectName(), qMin(2, cardnum), false, true);
            }
            else{
                room->playSkillEffect(objectName(), 1);
                if(!anu->isWounded() || room->askForChoice(anu, objectName(), "qiao+nu") == "nu")
                    anu->drawCards(2);
                else{
                    RecoverStruct r;
                    room->recover(anu, r);
                }
            }
        }
        return false;
    }
};

#include "plough.h"
class Huoshui: public FilterSkill{
public:
    Huoshui():FilterSkill("huoshui"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Weapon") ||
                to_select->getCard()->inherits("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *c = card_item->getCard();
        Drivolt *drive = new Drivolt(c->getSuit(), c->getNumber());
        drive->setSkillName(objectName());
        drive->addSubcard(card_item->getCard());

        return drive;
    }
};

class Baoen:public TriggerSkill{
public:
    Baoen():TriggerSkill("baoen"){
        events << HpRecover;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *cuilian, QVariant &data) const{
        Room *room = cuilian->getRoom();
        RecoverStruct rec = data.value<RecoverStruct>();
        if(!rec.who || rec.who == cuilian)
            return false;
        for(int i = rec.recover; i > 0; i--){
            if(!room->askForCard(cuilian, "..", "@baoen:" + rec.who->objectName(), data))
                break;
            room->playSkillEffect(objectName());
            LogMessage s;
            s.type = "#Baoen";
            s.from = cuilian;
            s.to << rec.who;
            s.arg = objectName();
            room->sendLog(s);

            rec.who->drawCards(qMin(3, rec.who->getHp()));
        }
        return false;
    }
};

class Zhiyu: public MasochismSkill{
public:
    Zhiyu():MasochismSkill("zhiyu"){
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getGeneral()->isMale();
    }

    virtual void onDamaged(ServerPlayer *masata, const DamageStruct &damage) const{
        Room *room = masata->getRoom();
        ServerPlayer *loli = room->findPlayerBySkillName(objectName());
        if(loli){
            QVariant whiter = QVariant::fromValue((PlayerStar)masata);
            if(masata->isKongcheng() || !loli->askForSkillInvoke(objectName(), whiter))
                return;
            room->playSkillEffect(objectName());
            QList<int> card_ids = masata->handCards();
            room->fillAG(card_ids, loli);
            int card_id = room->askForAG(loli, card_ids, false, objectName());
            //card_ids.removeOne(card_id);
            //room->takeAG(loli, card_id);
            room->broadcastInvoke("clearAG");
            room->obtainCard(loli, card_id);
            room->getThread()->delay(300);
            room->obtainCard(masata, room->askForCardShow(loli, masata, objectName()));

            LogMessage log;
            log.type = "#Zhiyu";
            log.from = loli;
            log.to << masata;
            log.arg = objectName();
            log.arg2 = QString::number(1);
            room->sendLog(log);
        }
    }
};

class Zhuying: public FilterSkill{
public:
    Zhuying():FilterSkill("zhuying"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->objectName() == "analeptic";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *c = card_item->getCard();
        Peach *peach = new Peach(c->getSuit(), c->getNumber());
        peach->setSkillName(objectName());
        peach->addSubcard(card_item->getCard());

        return peach;
    }
};

class Banzhuang: public OneCardViewAsSkill{
public:
    Banzhuang():OneCardViewAsSkill("banzhuang"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        ExNihilo *ex_nihilo = new ExNihilo(card->getSuit(), card->getNumber());
        ex_nihilo->addSubcard(card->getId());
        ex_nihilo->setSkillName(objectName());
        return ex_nihilo;
    }
};

class Caiquan: public TriggerSkill{
public:
    Caiquan():TriggerSkill("caiquan"){
        events << CardEffected;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.from && effect.to == player && !player->getEquips().isEmpty()){
            QStringList suits;
            foreach(const Card *rmp, player->getEquips()){
                if(!suits.contains(rmp->getSuitString()))
                    suits << rmp->getSuitString();
            }
            if(!effect.card->inherits("Slash") && !effect.card->inherits("Duel") && !effect.card->inherits("Ecstasy"))
                return false;
            QString suit = effect.card->getSuitString();
            if(!suits.contains(suit))
                return false;

            LogMessage log;
            log.type = "#Caiquan";
            log.from = effect.from;
            Room *room = player->getRoom();
            log.to << effect.to;
            log.arg = effect.card->objectName();
            log.arg2 = objectName();

            room->sendLog(log);
            room->playSkillEffect(objectName());
            return true;
        }
        return false;
    }
};

QLFDPackage::QLFDPackage()
    :Package("QLFD")
{
    General *panjinlian = new General(this, "panjinlian", "min", 3, false);
    panjinlian->addSkill(new Yushui);
    panjinlian->addSkill(new Zhensha);
    panjinlian->addSkill(new Shengui);
    panjinlian->addSkill(new MarkAssignSkill("@vi", 1));
    related_skills.insertMulti("zhensha", "#@vi-1");

    General *panqiaoyun = new General(this, "panqiaoyun", "min", 3, false);
    panqiaoyun->addSkill(new Fanwu);
    panqiaoyun->addSkill(new Panxin);
    panqiaoyun->addSkill(new MarkAssignSkill("@pfxl", 1));
    related_skills.insertMulti("panxin", "#@pfxl-1");
    panqiaoyun->addSkill(new Foyuan);

    General *wangpo = new General(this, "wangpo", "min", 3, false);
    wangpo->addSkill(new Qianxian);
    wangpo->addSkill(new Meicha);

    General *linniangzi = new General(this, "linniangzi", "min", 3, false);
    linniangzi->addSkill(new Shouwang);
    linniangzi->addSkill(new Ziyi);
    linniangzi->addSkill(new MarkAssignSkill("@ziyi", 1));
    related_skills.insertMulti("ziyi", "#@ziyi-1");
    linniangzi->addSkill(new Zhongzhen);

    General *duansanniang = new General(this, "duansanniang", "min", 4, false);
    duansanniang->addSkill(new Zishi);

    General *yanxijiao = new General(this, "yanxijiao", "min", 3, false);
    yanxijiao->addSkill(new Suocai);
    yanxijiao->addSkill(new Huakui);

    General *baixiuying = new General(this, "baixiuying", "min", 3, false);
    baixiuying->addSkill(new Eyan);
    skills << new EyanSlash;
    baixiuying->addSkill(new Zhangshi);

    General *liruilan = new General(this, "liruilan", "min", 3, false);
    liruilan->addSkill(new Chumai);
    liruilan->addSkill(new Yinlang);

    General *liqiaonu = new General(this, "liqiaonu", "min", 3, false);
    liqiaonu->addSkill(new Chiyuan);
    liqiaonu->addSkill(new Huoshui);

    General *jincuilian = new General(this, "jincuilian", "min", 3, false);
    jincuilian->addSkill(new Baoen);
    jincuilian->addSkill(new Zhiyu);

    General *jiashi = new General(this, "jiashi", "min", 3, false);
    jiashi->addSkill(new Banzhuang);
    jiashi->addSkill(new Zhuying);

    General *ximenqing = new General(this, "ximenqing$", "min", 4, true, true);
    ximenqing->addSkill(new Caiquan);

    addMetaObject<YushuiCard>();
    addMetaObject<FanwuCard>();
    addMetaObject<QianxianCard>();
    addMetaObject<ZiyiCard>();
    addMetaObject<ShouwangCard>();
    addMetaObject<ZishiCard>();
    addMetaObject<SuocaiCard>();
    addMetaObject<EyanCard>();
    addMetaObject<EyanSlashCard>();
    addMetaObject<ZhangshiCard>();
    addMetaObject<YinlangCard>();
}

ADD_PACKAGE(QLFD);
