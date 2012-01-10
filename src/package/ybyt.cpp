#include "ybyt.h"
#include "standard.h"
#include "skill.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"
#include "plough.h"
#include "tocheck.h"

class SWPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return card->inherits("Slash") || card->inherits("Weapon");
    }
    virtual bool willThrow() const{
        return false;
    }
};

YuanpeiCard::YuanpeiCard(){
}

bool YuanpeiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select->getGeneral()->isMale() &&
            (!to_select->isKongcheng() || (to_select->isKongcheng() && to_select->getWeapon()));
}

void YuanpeiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    const Card *card = room->askForCard(effect.to, ".Yuanp", "@yuanpei:" + effect.from->objectName());
    if(card){
        effect.from->obtainCard(card);
        effect.from->drawCards(1);
        effect.to->drawCards(1);
    }
    else
        room->acquireSkill(effect.from, "yuanpei_slash");
}

class Yuanpei: public ZeroCardViewAsSkill{
public:
    Yuanpei():ZeroCardViewAsSkill("yuanpei"){
    }

    virtual const Card *viewAs() const{
        return new YuanpeiCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YuanpeiCard");
    }
};

class YuanpeiS1ash:public OneCardViewAsSkill{
public:
    YuanpeiS1ash():OneCardViewAsSkill(""){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "slash";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Card *slash = new Slash(card->getSuit(), card->getNumber());
        slash->addSubcard(card->getId());
        slash->setSkillName(objectName());
        return slash;
    }
};

class YuanpeiSlash: public PhaseChangeSkill{
public:
    YuanpeiSlash():PhaseChangeSkill("yuanpei_slash"){
        view_as_skill = new YuanpeiS1ash;
    }

    virtual bool onPhaseChange(ServerPlayer *p) const{
        if(p->getPhase() == Player::NotActive){
            p->getRoom()->detachSkillFromPlayer(p, "yuanpei_slash");
            p->loseSkill("yuanpei_slash");
        }
        return false;
    }
};

class Mengshi: public PhaseChangeSkill{
public:
    Mengshi():PhaseChangeSkill("mengshi"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("mengshi") == 0
                && target->getPhase() == Player::Start
                && target->getHandcardNum() < target->getAttackRange();
    }

    virtual bool onPhaseChange(ServerPlayer *qyyy) const{
        Room *room = qyyy->getRoom();

        LogMessage log;
        log.type = "#WakeUp";
        log.from = qyyy;
        log.arg = objectName();
        room->sendLog(log);
        room->playSkillEffect(objectName());
        room->broadcastInvoke("animate", "lightbox:$mengshi:1500");
        room->getThread()->delay(1500);

        qyyy->drawCards(3);
        room->acquireSkill(qyyy, "yinyu");
        room->setPlayerMark(qyyy, "mengshi", 1);
        return false;
    }
};

GuibingCard::GuibingCard(){

}

bool GuibingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->canSlash(to_select);
}

void GuibingCard::use(Room *room, ServerPlayer *gaolian, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *to = targets.first();

    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(club|spade):(.*)");
    judge.good = true;
    judge.reason = objectName();
    judge.who = gaolian;

    room->judge(judge);

    if(judge.isGood()){
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("guibing");

        CardUseStruct use;
        use.from = gaolian;
        use.to << to;
        use.card = slash;
        room->useCard(use);
    }
    else
        room->setPlayerFlag(gaolian, "guibing");
}

class GuibingViewAsSkill:public ZeroCardViewAsSkill{
public:
    GuibingViewAsSkill():ZeroCardViewAsSkill("guibing"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->hasSkill("guibing") && !player->hasFlag("guibing") && Slash::IsAvailable(player);
    }

    virtual const Card *viewAs() const{
        return new GuibingCard;
    }
};

class Guibing: public TriggerSkill{
public:
    Guibing():TriggerSkill("guibing"){
        events << CardAsked;
        view_as_skill = new GuibingViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill("guibing");
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *gaolian, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "slash")
            return false;

        Room *room = gaolian->getRoom();
        if(gaolian->askForSkillInvoke(objectName())){

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club|spade):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = gaolian;

            room->judge(judge);

            if(judge.isGood()){
                Slash *slash = new Slash(Card::NoSuit, 0);
                slash->setSkillName(objectName());
                room->provide(slash);
                return true;
        }
     }
        if(gaolian->getPhase() == Player::Finish){
            room->setPlayerFlag(gaolian, "-guibing");
        }

        return false;
    }
};

HeiwuCard::HeiwuCard(){
    target_fixed = true;
    once = true;
}

void HeiwuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QString result = room->askForChoice(source, "heiwu", "put+dis");
    if(result == "put")
        room->moveCardTo(this, NULL, Player::DrawPile, true);
    else{
        room->throwCard(this);
    }
};

class Heiwu:public ViewAsSkill{
public:
    Heiwu():ViewAsSkill("heiwu"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
            return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        HeiwuCard *heiwu_card = new HeiwuCard;
        heiwu_card->addSubcards(cards);

        return heiwu_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng();
    }
};

class Goulian: public TriggerSkill{
public:
    Goulian():TriggerSkill("goulian"){
        events << Predamage;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *xuning = damage.from;
        if(damage.card && damage.card->inherits("Slash")
            && damage.to != xuning
            && damage.card->objectName() != "fire_slash"
            && damage.card->objectName() != "thunder_slash"
            && damage.to->isChained()
            && xuning->askForSkillInvoke(objectName())
            )
        {
            Room *room = xuning->getRoom();

            damage.to->setChained(false);
            room->broadcastProperty(damage.to, "chained");

            room->throwCard(damage.to->getDefensiveHorse());
            room->throwCard(damage.to->getOffensiveHorse());

            }

            return false;
    }
};

class Jinjia: public TriggerSkill{
public:
    Jinjia():TriggerSkill("jinjia"){
        events << Predamaged;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getArmor() == NULL && target->getMark("qinggang") == 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.damage > 1){
            LogMessage log;
            log.type = "#SilverLion";
            log.from = player;
            log.arg = QString::number(damage.damage);
            player->getRoom()->sendLog(log);

            damage.damage = 1;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

class Sinue: public TriggerSkill{
public:
    Sinue():TriggerSkill("sinue"){
        events << Death;
    }
    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;

        if(killer && killer->hasSkill("sinue")
            && !killer->hasFlag("sinue")
            && !killer->isKongcheng()
            && killer->getPhase() == Player::Play
            && killer->askForSkillInvoke(objectName())){

            Room *room = killer->getRoom();
            room->askForDiscard(killer, "sinue", 1);
            room->setPlayerFlag(killer, "sinue");

            QList<ServerPlayer *> players = room->getOtherPlayers(killer), targets;
            foreach(ServerPlayer *player, players){
                if(killer->distanceTo(player) <= 1)
                    targets << player;
            }
            ServerPlayer *target = room->askForPlayerChosen(killer, targets, "sinue");

            DamageStruct damage;
            damage.card = NULL;
            damage.damage = 2;
            damage.from = killer;
            damage.to = target;
            room->damage(damage);
        }

    if(killer->getPhase() == Player::Finish){
        Room *room = killer->getRoom();
        room->setPlayerFlag(killer, "-sinue");
    }
        return false;
    }
};

class Xuandao: public TriggerSkill{
public:
    Xuandao():TriggerSkill("xuandao"){
        events << SlashMissed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        SlashEffectStruct slash_effect = data.value<SlashEffectStruct>();
        ServerPlayer *target = slash_effect.to;

        if(target->hasFlag("triggered")){
            target->setFlags("-triggered");
            return true;
        }

        ServerPlayer *next = target->getNextAlive();

            room->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#Xuandao";
            log.from = player;
            log.to << target;
            log.arg = next->getGeneralName();
            room->sendLog(log);

            slash_effect.to = next;
            slash_effect.to->setFlags("triggered");
            room->setEmotion(next, "victim");
            room->slashEffect(slash_effect);

        return true;
    }
};

WeizaoCard::WeizaoCard(){
    once = true;
    mute = true;
}

bool WeizaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void WeizaoCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    room->playSkillEffect("weizao", 1);
    const Card *card = room->askForCardShow(effect.to, effect.from, "weizao");
    int card_id = card->getEffectiveId();
    room->showCard(effect.to, card_id);

    if(card->getTypeId() == Card::Basic || card->isNDTrick()){
        room->setPlayerMark(effect.from, "weizao", card_id);
        room->setPlayerFlag(effect.from, "weizao");
    }else{
        room->setPlayerFlag(effect.from, "-weizao");
    }
}

class Weizao: public ViewAsSkill{
public:
    Weizao():ViewAsSkill("weizao"){

    }

    virtual int getEffectIndex(ServerPlayer *, const Card *card) const{
        if(card->getTypeId() == Card::Basic)
            return 2;
        else
            return 3;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->hasUsed("WeizaoCard") && player->hasFlag("weizao")){
            int card_id = player->getMark("weizao");
            const Card *card = Sanguosha->getCard(card_id);
            return card->isAvailable(player);
        }else
            return true;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if(player->getPhase() == Player::NotActive)
            return false;

        if(!player->hasFlag("weizao"))
            return false;

        if(player->hasUsed("WeizaoCard")){
            int card_id = player->getMark("weizao");
            const Card *card = Sanguosha->getCard(card_id);
            return  pattern.contains(card->objectName());
        }else
            return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(Self->hasUsed("WeizaoCard") && selected.isEmpty() && Self->hasFlag("weizao")){
            return !to_select->isEquipped();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(Self->hasUsed("WeizaoCard")){
            if(!Self->hasFlag("weizao"))
                return false;

            if(cards.length() != 1)
                return NULL;

            int card_id = Self->getMark("weizao");
            const Card *card = Sanguosha->getCard(card_id);
            const Card *first = cards.first()->getFilteredCard();

            Card *new_card = Sanguosha->cloneCard(card->objectName(), first->getSuit(), first->getNumber());
            new_card->addSubcards(cards);
            new_card->setSkillName(objectName());
            return new_card;
        }else{
            return new WeizaoCard;
        }
    }
};

class WeizaoMark: public TriggerSkill{
public:
    WeizaoMark():TriggerSkill("#weizao_mark"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *jindajian, QVariant &data) const{
        Room *room = jindajian->getRoom();
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->getSkillName() == "weizao"){
                    room->setPlayerFlag(jindajian, "-weizao");
        }
        return false;
    }
};

class Jiangxin:public TriggerSkill{
public:
    Jiangxin():TriggerSkill("jiangxin"){
        frequency = Frequent;
        events << AskForRetrial;
    }
    virtual int getPriority() const{
        return -2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *jindajian, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        CardStar card = judge->card;

        Room *room = jindajian->getRoom();

        if(card->inherits("BasicCard") && jindajian->askForSkillInvoke(objectName(), data)){

            room->playSkillEffect(objectName());
            jindajian->drawCards(1);

            return true;
        }

        return false;
    }
};

ShexinCard::ShexinCard(){
    once = true;
}

bool ShexinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng();
}

void ShexinCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    ServerPlayer *target = targets.value(0, source);

    CardEffectStruct effect;
    effect.card = this;
    effect.from = source;
    effect.to = target;

    room->cardEffect(effect);
}

void ShexinCard::onEffect(const CardEffectStruct &effect) const{
        Room *room = effect.from->getRoom();
        QList<int> cardes = effect.to->handCards();
        room->fillAG(cardes, effect.from);
        QList<const Card *> cards = effect.to->getHandcards();
        foreach(const Card *card, cards){
            if(card->inherits("EquipCard") || card->inherits("TrickCard")){
                room->showCard(effect.to, card->getEffectiveId());
                room->getThread()->delay();
                room->throwCard(card);

            }else{
            room->showCard(effect.to, card->getEffectiveId());
            room->getThread()->delay();
        }
            effect.from->invoke("clearAG");
    }
}

class Shexin: public OneCardViewAsSkill{
public:
    Shexin():OneCardViewAsSkill("shexin"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ShexinCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("EquipCard") || to_select->getCard()->isNDTrick();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ShexinCard *card = new ShexinCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Sheyan: public OneCardViewAsSkill{
public:
    Sheyan():OneCardViewAsSkill("sheyan"){

    }
    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasFlag("sheyan");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        AmazingGrace *amazingGrace = new AmazingGrace(first->getSuit(), first->getNumber());
        amazingGrace->addSubcard(first->getId());
        amazingGrace->setSkillName(objectName());
        return amazingGrace;
    }
};

class SheyanMark: public TriggerSkill{
public:
    SheyanMark():TriggerSkill("#sheyan_mark"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *songqing, QVariant &data) const{
        Room *room = songqing->getRoom();
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card->getSkillName() == "sheyan"){
                    room->setPlayerFlag(songqing, "sheyan");
        }
            if(songqing->getPhase() == Player::Finish){
                room->setPlayerFlag(songqing, "-sheyan");
            }
        return false;
    }
};

MaiyiCard::MaiyiCard(){

}

bool MaiyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.length() < 2;
}

void MaiyiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
   room->throwCard(this);
   room->playSkillEffect(objectName());
   foreach(ServerPlayer *target, targets){
       CardEffectStruct effect;
       effect.card = this;
       effect.from = source;
       effect.to = target;
       effect.multiple = true;

       room->cardEffect(effect);

   if(targets.length() <= 1)
       effect.to->addMark("maiyi");
   else
       effect.to->drawCards(2);
   }
}

class MaiyiViewAsSkill: public ViewAsSkill{
public:
    MaiyiViewAsSkill(): ViewAsSkill("maiyi"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("MaiyiCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 3)
            return false;

        foreach(CardItem *item, selected){
            if(to_select->getFilteredCard()->getSuit() == item->getFilteredCard()->getSuit())
                return false;
        }

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 3)
            return NULL;

        MaiyiCard *card = new MaiyiCard;
        card->addSubcards(cards);

        return card;
    }
};

class Maiyi: public PhaseChangeSkill{
public:
    Maiyi():PhaseChangeSkill("maiyi"){
        view_as_skill = new MaiyiViewAsSkill;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *xueyong) const{
        Room *room = xueyong->getRoom();

        QList<ServerPlayer *> players = room->getOtherPlayers(xueyong), maiyis;
            foreach(ServerPlayer *player, players){
                if(player->getMark("maiyi") > 0)
                    maiyis << player;
            }

        if(maiyis.length() > 0 && xueyong->getPhase() == Player::Finish){

        ServerPlayer *maiyier = room->askForPlayerChosen(xueyong, maiyis, objectName());
        maiyier->setMark("maiyi", 0);

        LogMessage log;
        log.type = "#MaiyiCanInvoke";
        log.from = maiyier;
        room->sendLog(log);

        room->setCurrent(maiyier);
        room->getThread()->trigger(TurnStart, maiyier);
        room->setCurrent(xueyong);
    }

        return false;
    }
};

class Longao: public TriggerSkill{
public:
    Longao():TriggerSkill("longao"){
        events << CardEffected;
    }

    virtual bool triggerable(const ServerPlayer *target) const{

        return true;
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        ServerPlayer *zouyuan = room->findPlayerBySkillName(objectName());
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.from == zouyuan)
            return false;

        if(effect.multiple || effect.from == NULL)
            return false;

        if(!effect.card->isNDTrick())
            return false;

        if(zouyuan && !zouyuan->isNude() && room->askForSkillInvoke(zouyuan, objectName(), data)){
            room->askForDiscard(zouyuan, "longao", 1, false, true);

            QList<ServerPlayer *> players = room->getOtherPlayers(effect.from), targets;
            foreach(ServerPlayer *player, players){
                if(player != effect.to)
                    targets << player;
            }

            QString choice = room->askForChoice(zouyuan, objectName(), "zhuan+qi");
            if(choice == "zhuan" && targets.length() > 0){

                ServerPlayer *target = room->askForPlayerChosen(zouyuan, targets, "sinue");

                effect.from = effect.from;
                effect.to = target;
                data = QVariant::fromValue(effect);
        }
        if(choice == "qi" && !effect.from->isNude()){
            int to_throw = room->askForCardChosen(zouyuan, effect.from, "hej", objectName());
            room->throwCard(to_throw);
        }
      }

        return false;
    }
};

class Hunjiu:public OneCardViewAsSkill{
public:
    Hunjiu():OneCardViewAsSkill("hunjiu"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                return card->inherits("Peach") || card->inherits("Analeptic") || card->inherits("Ecstasy");
            }

        case Client::Responsing:{
                QString pattern = ClientInstance->getPattern();
                if(pattern.contains("analeptic"))
                    return card->inherits("Ecstasy");
            }

        default:
            return false;
        }
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("analeptic");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        if(card->inherits("Peach") || card->inherits("Analeptic")){
            Ecstasy *ecstasy = new Ecstasy(card->getSuit(), card->getNumber());
            ecstasy->setSkillName(objectName());
            ecstasy->addSubcard(card->getId());
            return ecstasy;
        }else if(card->inherits("Ecstasy")){
            Analeptic *analeptic = new Analeptic(card->getSuit(), card->getNumber());
            analeptic->setSkillName(objectName());
            analeptic->addSubcard(card->getId());
            return analeptic;
        }else
            return NULL;
    }
};

class HeartPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return card->getSuit() == Card::Heart;
    }
};

class Guitai: public TriggerSkill{
public:
    Guitai():TriggerSkill("guitai"){
        events << CardEffected;
    }

    virtual bool triggerable(const ServerPlayer *target) const{

        return true;
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        ServerPlayer *zhufu = room->findPlayerBySkillName(objectName());
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.from == zhufu)
            return false;

        if(zhufu->getPhase() != Player::NotActive)
            return false;

        if(!effect.card->inherits("Peach"))
            return false;

        if(zhufu && !zhufu->isNude() && zhufu->isWounded() && room->askForSkillInvoke(zhufu, objectName(), data)){
            const Card *card = room->askForCard(zhufu, ".heart", "@guitai-ask:");
        if(card){
                effect.from = effect.from;
                effect.to = zhufu;
                data = QVariant::fromValue(effect);
      }
    }

        return false;
    }
};

YBYTPackage::YBYTPackage()
    :Package("YBYT")
{
    General *qiongying = new General(this, "qiongying", "jiang", 3, false);
    qiongying->addSkill(new Yuanpei);
    patterns[".Yuanp"] = new SWPattern;
    qiongying->addSkill(new Mengshi);
    skills << new YuanpeiSlash;

    General *gaolian = new General(this, "gaolian", "guan", 3);
    gaolian->addSkill(new Guibing);
    gaolian->addSkill(new Heiwu);

    General *xuning = new General(this, "xuning", "jiang");
    xuning->addSkill(new Goulian);
    xuning->addSkill(new Jinjia);

    General *baoxu = new General(this, "baoxu", "kou");
    baoxu->addSkill(new Sinue);

    General *xiangchong = new General(this, "xiangchong", "jiang");
    xiangchong->addSkill(new Xuandao);

    General *jindajian = new General(this, "jindajian", "min", 3);
    jindajian->addSkill(new Weizao);
    jindajian->addSkill(new WeizaoMark);
    jindajian->addSkill(new Jiangxin);
    related_skills.insertMulti("weizao", "#weizao_mark");

    General *yangchun = new General(this, "yangchun", "kou");
    yangchun->addSkill(new Shexin);

    General *songqing = new General(this, "songqing", "min", 3);
    songqing->addSkill(new Sheyan);
    songqing->addSkill(new SheyanMark);
    songqing->addSkill(new Skill("jiayao"));
    related_skills.insertMulti("sheyan", "#sheyan_mark");

    General *xueyong = new General(this, "xueyong", "min");
    xueyong->addSkill(new Maiyi);

    General *zouyan = new General(this, "zouyuan", "min");
    zouyan->addSkill(new Longao);

    General *zhufu = new General(this, "zhufu", "min", 3);
    zhufu->addSkill(new Hunjiu);
    zhufu->addSkill(new Guitai);
    patterns[".heart"] = new HeartPattern;

    addMetaObject<YuanpeiCard>();
    addMetaObject<GuibingCard>();
    addMetaObject<HeiwuCard>();
    addMetaObject<WeizaoCard>();
    addMetaObject<ShexinCard>();
    addMetaObject<MaiyiCard>();
}

ADD_PACKAGE(YBYT);
