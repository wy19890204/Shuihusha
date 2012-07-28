#include "ox.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"

GuibingCard::GuibingCard(){
}

bool GuibingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->canSlash(to_select);
}

void GuibingCard::use(Room *room, ServerPlayer *gaolian, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *to = targets.first();

    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(heart):(.*)");
    judge.good = false;
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
    }else
        room->setPlayerFlag(gaolian, "Guibing");
}

class GuibingViewAsSkill:public ZeroCardViewAsSkill{
public:
    GuibingViewAsSkill():ZeroCardViewAsSkill("guibing"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
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

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *gaolian, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "slash")
            return false;

        if(gaolian->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart):(.*)");
            judge.good = false;
            judge.reason = objectName();
            judge.who = gaolian;

            room->playSkillEffect(objectName());
            room->judge(judge);

            if(judge.isGood()){
                Slash *slash = new Slash(Card::NoSuit, 0);
                slash->setSkillName(objectName());
                room->provide(slash);
                return true;
            }
            else
                room->setPlayerFlag(gaolian, "Guibing");
        }
        return false;
    }
};

HeiwuCard::HeiwuCard(){
    target_fixed = true;
}

void HeiwuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    int num = getSubcards().length();
    room->moveCardTo(this, NULL, Player::DrawPile, true);
    QList<int> fog = room->getNCards(num, false);
    room->askForGuanxing(source, fog, true);
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

class Aoxiang: public PhaseChangeSkill{
public:
    Aoxiang():PhaseChangeSkill("aoxiang"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("aoxiang") == 0
                && target->getPhase() == Player::RoundStart
                && target->getHp() <= 2;
    }

    virtual bool onPhaseChange(ServerPlayer *tg) const{
        Room *room = tg->getRoom();

        LogMessage log;
        log.type = "#WakeUp";
        log.from = tg;
        log.arg = objectName();
        room->sendLog(log);
        room->playSkillEffect(objectName());
        room->broadcastInvoke("animate", "lightbox:$aoxiang:1500");
        room->getThread()->delay(1500);

        room->loseMaxHp(tg, 1);
        if(tg->getGeneralName() == "tongguan")
            room->setPlayerProperty(tg, "general", "tongguanf");
        else if(tg->getGeneral2Name() == "tongguan")
            room->setPlayerProperty(tg, "general2", "tongguanf");
        room->acquireSkill(tg, "wanghuan");
        room->setPlayerMark(tg, "aoxiang", 1);
        return false;
    }
};

class Wanghuan: public TriggerSkill{
public:
    Wanghuan():TriggerSkill("wanghuan"){
        events << CardEffected << CardEffect;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(!effect.card->inherits("Slash"))
            return false;
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = player;
        log.arg = objectName();
        if(event == CardEffect){
            if(effect.to->getGender() == General::Male && !effect.to->isKongcheng()){
                room->sendLog(log);
                room->playSkillEffect(objectName(), 1);
                room->askForDiscard(effect.to, objectName(), 1);
            }
        }
        else{
            if(effect.from->getGender() == General::Male){
                room->sendLog(log);
                room->playSkillEffect(objectName(), 2);
                player->drawCards(1);
            }
        }
        return false;
    }
};

ZhengfaCard::ZhengfaCard(){
    will_throw = false;
    mute = true;
}

bool ZhengfaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!Self->hasFlag("Zhengfa") && getSubcards().length() == 1)
        return targets.isEmpty() && to_select->getKingdom() != Self->getKingdom()
            && !to_select->isKongcheng() && to_select != Self;
    else if(Self->hasFlag("Zhengfa") && getSubcards().isEmpty())
        return targets.length() < Self->getKingdoms() && to_select != Self;
    else
        return false;
}

void ZhengfaCard::use(Room *room, ServerPlayer *tonguan, const QList<ServerPlayer *> &targets) const{
    if(getSubcards().isEmpty()){
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("zhengfa");
        CardUseStruct uset;
        uset.card = slash;
        uset.mute = true;
        uset.from = tonguan;
        uset.to = targets;
        room->playSkillEffect("zhengfa", tonguan->getGeneral()->isMale()? 2: 4);
        room->useCard(uset);
    }
    else{
        bool success = tonguan->pindian(targets.first(), "zhengfa", this);
        if(success){
            room->playSkillEffect("zhengfa", tonguan->getGeneral()->isMale()? 1: 3);
            room->setPlayerFlag(tonguan, "Zhengfa");
            room->askForUseCard(tonguan, "@@zhengfa", "@zhengfa-effect", true);
        }else{
            room->playSkillEffect("zhengfa", tonguan->getGeneral()->isMale()? 5: 6);
            tonguan->turnOver();
        }
    }
}

class ZhengfaViewAsSkill: public ViewAsSkill{
public:
    ZhengfaViewAsSkill():ViewAsSkill("zhengfa"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !Self->hasFlag("Zhengfa")? selected.isEmpty() && !to_select->isEquipped(): false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.startsWith("@@zhengfa");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        Card *zhengfcard = new ZhengfaCard;
        if(cards.length() == 1)
            zhengfcard->addSubcard(cards.first()->getCard());
        else if(cards.length() > 1)
            return NULL;
        return zhengfcard;
    }
};

class Zhengfa: public PhaseChangeSkill{
public:
    Zhengfa():PhaseChangeSkill("zhengfa"){
        view_as_skill = new ZhengfaViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *tong) const{
        Room *room = tong->getRoom();
        if(tong->getPhase() == Player::Finish && !tong->isKongcheng())
            room->askForUseCard(tong, "@@zhengfa", "@zhengfa", true);
        return false;
    }
};

LianmaCard::LianmaCard(){
    target_fixed = true;
    once = true;
}

void LianmaCard::use(Room *room, ServerPlayer *huyanzhuo, const QList<ServerPlayer *> &) const{
    QList<ServerPlayer *> players = room->getAlivePlayers();

    //room->broadcastSkillInvoke(objectName());
    QString choice = room->askForChoice(huyanzhuo, "lianma", "lian+ma");
    LogMessage log;
    log.from = huyanzhuo;
    log.arg = "lianma";
    if(choice == "lian"){
        log.type = "#LianmaOn";
        foreach(ServerPlayer *player, players){
            if(player->hasEquip("Horse", true)){
                if(!player->isChained()){
                    player->setChained(true);
                    room->broadcastProperty(player, "chained");
                    room->setEmotion(player, "chain");
                    log.to << player;
                }
            }
        }
    }else{
        log.type = "#LianmaOff";
        foreach(ServerPlayer *player, players){
            if(!player->hasEquip("Horse", true)){
                if(player->isChained()){
                    room->setPlayerProperty(player, "chained", false);
                    log.to << player;
                }
            }
        }
    }
    if(!log.to.isEmpty())
        room->sendLog(log);
};

class Lianma: public ZeroCardViewAsSkill{
public:
    Lianma():ZeroCardViewAsSkill("lianma"){
        default_choice = "lian";
    }

    virtual const Card *viewAs() const{
        return new LianmaCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("LianmaCard");
    }
};

class Zhongjia: public ClientSkill{
public:
    Zhongjia():ClientSkill("zhongjia"){
    }

    virtual int getExtra(const Player *target) const{
        if(!target->hasSkill(objectName()))
            return 0;
        else{
            int extra = target->isChained() ? 1 : 0;
            foreach(const Player *player, target->getSiblings()){
                if(player->isAlive() && player->isChained())
                    extra ++;
            }
            return extra;
        }
    }
};

SheruCard::SheruCard(){
    once = true;
}

bool SheruCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return to_select->isWounded() && to_select != Self;
}

void SheruCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    //room->throwCard(this);
    QString choice = room->askForChoice(effect.from, "sheru", "she+ru");
    int x = effect.to->getLostHp();
    if(choice == "she"){
        effect.to->drawCards(x);
        room->loseHp(effect.to);
    }else{
        if(effect.to->getCardCount(true) <= x){
            effect.to->throwAllHandCards();
            effect.to->throwAllEquips();
        }else{
            int card_id = -1;
            for(int i=1; i<=x; i++){
                card_id = room->askForCardChosen(effect.from, effect.to, "he", "sheru");
                room->throwCard(card_id);
                if(effect.to->isNude())
                    break;
            }
        }
        RecoverStruct recover;
        recover.who = effect.from;
        room->recover(effect.to, recover, true);
    }
    //room->broadcastSkillInvoke("sheru");
}

class Sheru: public OneCardViewAsSkill{
public:
    Sheru():OneCardViewAsSkill("sheru"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("SheruCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->isBlack() && card->inherits("BasicCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        SheruCard *sheru_card = new SheruCard;
        sheru_card->addSubcard(card_item->getFilteredCard());

        return sheru_card;
    }
};

LianzhuCard::LianzhuCard(){
    once = true;
    target_fixed = true;
}

void LianzhuCard::onUse(Room *room, const CardUseStruct &card_use) const{
    card_use.from->turnOver();
    ArcheryAttack *ar = new ArcheryAttack(Card::NoSuit, 0);
    ar->setSkillName("lianzhu");
    CardUseStruct use;
    use.card = ar;
    use.from = card_use.from;
    use.to = card_use.to;
    room->useCard(use);
}

class Lianzhu: public ZeroCardViewAsSkill{
public:
    Lianzhu():ZeroCardViewAsSkill("lianzhu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("LianzhuCard");
    }

    virtual const Card *viewAs() const{
        return new LianzhuCard;
    }
};

class Tongxia: public PhaseChangeSkill{
public:
    Tongxia():PhaseChangeSkill("tongxia"){

    }

    virtual bool onPhaseChange(ServerPlayer *hx) const{
        Room *room = hx->getRoom();
        if(hx->getPhase() == Player::Draw && hx->askForSkillInvoke(objectName())){
            QList<int> card_ids = room->getNCards(3);
            room->fillAG(card_ids);
            room->playSkillEffect(objectName());

            while(!card_ids.isEmpty()){
                int card_id = room->askForAG(hx, card_ids, false, "tongxia");
                CardStar card = Sanguosha->getCard(card_id);
                hx->tag["TongxiaCard"] = QVariant::fromValue(card);
                ServerPlayer *target = room->askForPlayerChosen(hx, room->getAllPlayers(), objectName());
                if(!target)
                    target = hx;
                //room->takeAG(target, card_id);
                if(card->inherits("EquipCard")){
                    const EquipCard *equipped = qobject_cast<const EquipCard *>(card);
                    QList<ServerPlayer *> targets;
                    targets << target;
                    equipped->use(room, hx, targets);
                }
                else
                    target->obtainCard(card);

                card_ids.removeOne(card_id);
                room->broadcastInvoke("clearAG");
                room->fillAG(card_ids);
            }
            room->broadcastInvoke("clearAG");

            return true;
        }
        hx->tag.remove("TongxiaCard");
        return false;
    }
};

ButianCard::ButianCard(){
    target_fixed = true;
    mute = true;
}

void ButianCard::use(Room *, ServerPlayer *, const QList<ServerPlayer *> &) const{
}

class ButianViewAsSkill:public OneCardViewAsSkill{
public:
    ButianViewAsSkill():OneCardViewAsSkill("butian"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@butian";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new ButianCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Butian: public TriggerSkill{
public:
    Butian():TriggerSkill("butian"){
        view_as_skill = new ButianViewAsSkill;
        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->isNude();
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@butian-card" << judge->who->objectName()
                << "" << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");

        player->tag["Judge"] = data;
        const Card *card = room->askForCard(player, "@butian", prompt, true, data, CardDiscarded);

        if(card){
            int index = qrand() % 2 + 1;
            if(player->getMark("wudao") == 0)
                room->playSkillEffect(objectName(), index);
            else
                room->playSkillEffect(objectName(), index + 2);
            room->throwCard(judge->card);

            QList<int> card_ids = room->getNCards(3);
            room->fillAG(card_ids, player);
            int card_id = room->askForAG(player, card_ids, false, objectName());
            if(card_id == -1)
                return false;
            int locat = card_ids.indexOf(card_id);
            card_ids.replace(locat, judge->card->getId());
            player->invoke("clearAG");

            card_ids.swap(0, 2);
            foreach(int tmp, card_ids){
                room->throwCard(tmp);
                room->moveCardTo(Sanguosha->getCard(tmp), NULL, Player::DrawPile);
            }
            room->getThread()->delay();

            judge->card = Sanguosha->getCard(card_id);
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = player;
            log.to << judge->who;
            log.card_str = QString::number(card_id);
            room->sendLog(log);

            room->sendJudgeResult(judge);
        }
        return false;
    }
};

class Huaxian: public TriggerSkill{
public:
    Huaxian():TriggerSkill("huaxian"){
        events << Dying;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *ren, QVariant &data) const{
        DyingStruct dying = data.value<DyingStruct>();
        if(dying.who == ren && ren->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = ren;

            room->judge(judge);
            if(judge.isGood()){
                RecoverStruct rev;
                rev.card = judge.card;
                rev.recover = ren->getLostHp(false) - ren->getMaxHP() + 1;
                rev.who = ren;
                room->recover(ren, rev);
                if(ren->getHp() != 1)
                    room->setPlayerProperty(ren, "hp", 1);
            }
        }
        return false;
    }
};

DuomingCard::DuomingCard(){
    target_fixed = true;
}

PlayerStar DuomingCard::findPlayerByFlag(Room *room, const QString &flag) const{
    const QList<ServerPlayer *> &list = room->getAlivePlayers();
    foreach(ServerPlayer *player, list){
        if(player->hasFlag(flag))
            return player;
    }
    return NULL;
}

void DuomingCard::use(Room *m, ServerPlayer *, const QList<ServerPlayer *> &) const{
    PlayerStar target = findPlayerByFlag(m, "Duoming");
    if(target)
        target->obtainCard(this, false);
}

class DuomingViewAsSkill: public ViewAsSkill{
public:
    DuomingViewAsSkill():ViewAsSkill("duoming"){
        frequency = Limited;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@duoming";
    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 2)
            return false;
        return !to_select->isEquipped() && to_select->getCard()->isBlack();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;
        DuomingCard *card = new DuomingCard;
        card->addSubcards(cards);
        return card;
    }
};

class Duoming: public TriggerSkill{
public:
    Duoming(): TriggerSkill("duoming"){
        view_as_skill = new DuomingViewAsSkill;
        events << HpRecovered;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() == Player::NotActive)
            return false;
        QList<ServerPlayer *> lily = room->findPlayersBySkillName(objectName());
        if(lily.isEmpty())
            return false;
        room->setPlayerFlag(player, "Duoming");
        foreach(ServerPlayer *lili, lily){
            if(lili->getHandcardNum() > 1 && room->askForUseCard(lili, "@@duoming", "@duoming:" + player->objectName(), true)){
                DamageStruct damage;
                damage.from = lili;
                damage.to = player;
                room->damage(damage);
            }
        }
        room->setPlayerFlag(player, "-Duoming");
        return false;
    }
};

class Moucai: public MasochismSkill{
public:
    Moucai():MasochismSkill("moucai"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> lily = room->findPlayersBySkillName(objectName());
        if(lily.isEmpty())
            return;
        foreach(ServerPlayer *lili, lily){
            if(lili && player->getHandcardNum() > lili->getHp() && lili->askForSkillInvoke(objectName(), QVariant::fromValue((PlayerStar)player))){
                room->playSkillEffect(objectName());
                const Card *wolegequ = player->getRandomHandCard();
                lili->obtainCard(wolegequ, false);
            }
        }
    }
};

class Wubang: public TriggerSkill{
public:
    Wubang():TriggerSkill("wubang"){
        events << CardLost << FinishJudge;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *jiuwenlong = room->findPlayerBySkillName(objectName());
        if(!jiuwenlong || player == jiuwenlong)
            return false;
        if(event == CardLost){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->to_place == Player::DiscardedPile){
                const Card *weapon = Sanguosha->getCard(move->card_id);
                if(weapon->inherits("Weapon") &&
                   jiuwenlong->askForSkillInvoke(objectName())){
                    room->playSkillEffect(objectName());
                    jiuwenlong->obtainCard(weapon);
                }
            }
        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(room->getCardPlace(judge->card->getEffectiveId()) == Player::DiscardedPile &&
               judge->card->inherits("Weapon") &&
               jiuwenlong->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());
                jiuwenlong->obtainCard(judge->card);
            }
        }
        return false;
    }
};

class Xiagu: public TriggerSkill{
public:
    Xiagu():TriggerSkill("xiagu"){
        events << Predamage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *, QVariant &data) const{
        QList<ServerPlayer *> jiuwennong = room->findPlayersBySkillName(objectName());
        if(jiuwennong.isEmpty())
            return false;

        DamageStruct damage = data.value<DamageStruct>();
        foreach(ServerPlayer *jiuwenlong, jiuwennong){
            if(!jiuwenlong->isNude() && damage.nature == DamageStruct::Normal &&
               damage.to->isAlive() && damage.damage > 0){
                bool caninvoke = false;
                foreach(const Card *cd, jiuwenlong->getCards("he")){
                    if(cd->getTypeId() == Card::Equip){
                        caninvoke = true;
                        break;
                    }
                }
                if(caninvoke){
                    const Card *card = room->askForCard(jiuwenlong, "EquipCard", "@xiagu", true, data, CardDiscarded);
                    if(card){
                        LogMessage log;
                        log.type = "$Xiagu";
                        log.from = jiuwenlong;
                        log.to << damage.to;
                        log.card_str = card->getEffectIdString();
                        room->sendLog(log);
                        room->playSkillEffect(objectName());

                        damage.damage --;
                    }
                    data = QVariant::fromValue(damage);
                }
            }
        }
        return false;
    }
};

DingceCard::DingceCard(){
    will_throw = false;
    once = true;
}

void DingceCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);
    Room *room = effect.from->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "he", "dingce");
    room->obtainCard(effect.from, card_id, room->getCardPlace(card_id) != Player::Hand);
    room->showCard(effect.from, card_id);
    if(Sanguosha->getCard(card_id)->inherits("TrickCard") && effect.from->askForSkillInvoke("dingce")){
        room->throwCard(card_id);
        RecoverStruct tec;
        room->recover(effect.from, tec, true);
    }
}

class Dingce: public OneCardViewAsSkill{
public:
    Dingce():OneCardViewAsSkill("dingce"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("DingceCard");
    }

    virtual bool viewFilter(const CardItem *lij) const{
        return lij->getCard()->inherits("TrickCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        DingceCard *card = new DingceCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

XunlieCard::XunlieCard(){
}

bool XunlieCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int x = getSubcards().isEmpty() ? qMax(Self->getEquips().count(), 1) : 1;
    if(targets.length() >= x)
        return false;
    return !to_select->isKongcheng() && to_select != Self;
}

void XunlieCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if(!effect.to->isKongcheng()){
        int card_id = room->askForCardChosen(effect.from, effect.to, "h", "xunlie");
        room->obtainCard(effect.from, card_id, false);
        if(!getSubcards().isEmpty() && !effect.to->isKongcheng())
            room->obtainCard(effect.from, effect.to->getRandomHandCardId(), false);
        room->setEmotion(effect.to, "bad");
        room->setEmotion(effect.from, "good");
    }
}

class XunlieViewAsSkill: public ViewAsSkill{
public:
    XunlieViewAsSkill(): ViewAsSkill("xunlie"){
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return new XunlieCard;
        XunlieCard *card = new XunlieCard;
        card->addSubcards(cards);
        return card;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_selec) const{
        if(to_selec->getCard()->isRed())
            return true;
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@xunlie";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
};

class Xunlie:public PhaseChangeSkill{
public:
    Xunlie():PhaseChangeSkill("xunlie"){
        view_as_skill = new XunlieViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *xiezhen) const{
        Room *room = xiezhen->getRoom();
        if(xiezhen->getPhase() == Player::Draw &&
           room->askForUseCard(xiezhen, "@@xunlie", "@xunlie", true))
            return true;
        else
            return false;
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

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *linko, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.from || !damage.from->getGeneral()->isMale() || damage.damage <= 0)
            return false;
        if(!linko->isKongcheng() && !damage.from->isKongcheng() &&
           linko->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            bool success = linko->pindian(damage.from, objectName());
            if(success){
                LogMessage log;
                log.type = "#Zhongzhen";
                log.from = damage.from;
                log.to << linko;
                log.arg = QString::number(damage.damage);
                room->sendLog(log);
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
    effect.from->loseAllMarks("@rope");
    RecoverStruct r;
    r.who = effect.from;
    r.recover = 2;
    effect.from->getRoom()->recover(effect.to, r, true);
    effect.from->setFlags("Hanging");
}

class ZiyiViewAsSkill: public ZeroCardViewAsSkill{
public:
    ZiyiViewAsSkill():ZeroCardViewAsSkill("ziyi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@rope") > 0 && player->isWounded();
    }

    virtual const Card *viewAs() const{
        return new ZiyiCard;
    }
};

class Ziyi:public PhaseChangeSkill{
public:
    Ziyi():PhaseChangeSkill("ziyi"){
        frequency = Limited;
        view_as_skill = new ZiyiViewAsSkill;
    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool onPhaseChange(ServerPlayer *lnz) const{
        if(lnz->hasFlag("Hanging") && lnz->getPhase() == Player::Finish){
            Room *o = lnz->getRoom();
            o->broadcastInvoke("animate", "lightbox:$ziyi:3000");
            o->getThread()->delay(3000);
            DamageStruct damage;
            damage.from = lnz;
            o->killPlayer(lnz, &damage);
        }
        return false;
    }
};

ShouwangCard::ShouwangCard(){
    will_throw = false;
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

OxPackage::OxPackage()
    :Package("ox")
{
    General *gaolian = new General(this, "gaolian", "guan", 3);
    gaolian->addSkill(new Guibing);
    gaolian->addSkill(new Heiwu);

    General *tongguan = new General(this, "tongguan", "guan");
    tongguan->addSkill(new Aoxiang);
    skills << new Wanghuan;
    tongguan->addRelateSkill("wanghuan");
    tongguan->addSkill(new Zhengfa);

    tongguan = new General(this, "tongguanf", "yan", 4, false, true);
    tongguan->addSkill("aoxiang");
    tongguan->addSkill("zhengfa");
    tongguan->addSkill("zhengfa");

    General *huyanzhuo = new General(this, "huyanzhuo", "guan");
    huyanzhuo->addSkill(new Lianma);
    huyanzhuo->addSkill(new Zhongjia);

    General *dongchaoxueba = new General(this, "dongchaoxueba", "jiang");
    dongchaoxueba->addSkill(new Sheru);

    General *pangwanchun = new General(this, "pangwanchun", "jiang");
    pangwanchun->addSkill(new Lianzhu);

    General *huangxin = new General(this, "huangxin", "jiang");
    huangxin->addSkill(new Tongxia);

    General *luozhenren = new General(this, "luozhenren", "kou", 3);
    luozhenren->addSkill(new Butian);
    luozhenren->addSkill(new Huaxian);

    General *lili = new General(this, "lili", "kou", 3);
    lili->addSkill(new Duoming);
    lili->addSkill(new Moucai);

    General *shijin = new General(this, "shijin", "kou");
    shijin->addSkill(new Wubang);
    shijin->addSkill(new Xiagu);

    General *lijun = new General(this, "lijun", "min");
    lijun->addSkill(new Skill("nizhuan"));
    lijun->addSkill(new Dingce);

    General *xiezhen = new General(this, "xiezhen", "min");
    xiezhen->addSkill(new Xunlie);

    General *linniangzi = new General(this, "linniangzi", "min", 3, false);
    linniangzi->addSkill(new Shouwang);
    linniangzi->addSkill(new Ziyi);
    linniangzi->addSkill(new MarkAssignSkill("@rope", 1));
    related_skills.insertMulti("ziyi", "#@rope-1");
    linniangzi->addSkill(new Zhongzhen);

    addMetaObject<GuibingCard>();
    addMetaObject<HeiwuCard>();
    addMetaObject<ZhengfaCard>();
    addMetaObject<LianmaCard>();
    addMetaObject<SheruCard>();
    addMetaObject<LianzhuCard>();
    addMetaObject<ButianCard>();
    addMetaObject<DuomingCard>();
    addMetaObject<DingceCard>();
    addMetaObject<XunlieCard>();
    addMetaObject<ZiyiCard>();
    addMetaObject<ShouwangCard>();
}

ADD_PACKAGE(Ox)
