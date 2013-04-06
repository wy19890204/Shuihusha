#include "snake.h"
#include "standard.h"
#include "plough.h"
#include "maneuvering.h"

class Wuzu: public TriggerSkill{
public:
    Wuzu():TriggerSkill("wuzu"){
        events << CardUsed << CardFinished;
        frequency = Compulsory;
    }

    static bool isWuzuEffectCard(CardStar card){
        return card->inherits("Slash") ||
                card->inherits("AOE") ||
                card->inherits("FireAttack");
    }

    virtual bool trigger(TriggerEvent e, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(e == CardUsed){
            bool play = false;
            foreach(ServerPlayer *tmp, use.to){
                if(tmp->getArmor()){
                    tmp->addMark("wuzuc");
                    LogMessage log;
                    log.type = "$IgnoreArmor";
                    log.from = player;
                    log.to << tmp;
                    log.card_str = tmp->getArmor()->getEffectIdString();
                    room->sendLog(log);

                    play = true;
                }
            }
            if(play && isWuzuEffectCard(use.card))
                room->playSkillEffect(objectName());
        }
        else{
            foreach(ServerPlayer *tmp, use.to)
                tmp->removeMark("wuzuc");
        }
        return false;
    }
};

class WuzuSlash: public ClientSkill{
public:
    WuzuSlash():ClientSkill("#wuzu_slash"){
    }

    virtual bool isSlashPenetrate(const Player *, const Player *to, const Card *) const{
        return to->hasMark("wuzuc");
    }
};

class WuzuDistance: public DistanceSkill{
public:
    WuzuDistance(): DistanceSkill("#wuzu_distance"){
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(from->hasSkill("wuzu"))
            return - qMax(from->getLostHp(), 1);
        else
            return 0;
    }
};

class Zhanchi:public PhaseChangeSkill{
public:
    Zhanchi():PhaseChangeSkill("zhanchi"){
        frequency = Limited;
    }

    virtual bool onPhaseChange(ServerPlayer *opt) const{
        if(opt->hasMark("@wings") && opt->getPhase() == Player::Judge){
            Room *room = opt->getRoom();
            if(opt->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName(), 1);
                while(!opt->getJudgingArea().isEmpty())
                    room->throwCard(opt->getJudgingArea().first()->getId());
                room->playLightbox(opt, "zhanchi", "");
                room->acquireSkill(opt, "tengfei");
                opt->loseMark("@wings");
            }
        }
        return false;
    }
};

class Tengfei:public PhaseChangeSkill{
public:
    Tengfei():PhaseChangeSkill("tengfei"){
        frequency = Compulsory;
    }

    virtual int getPriority(TriggerEvent) const{
        return -1;
    }

    virtual bool onPhaseChange(ServerPlayer *opt) const{
        if(opt->getPhase() == Player::NotActive){
            Room *room = opt->getRoom();
            if(opt->getMaxHP() > 4)
                room->playSkillEffect(objectName(), 1);
            else if(opt->getMaxHP() > 2)
                room->playSkillEffect(objectName(), 2);
            else if(opt->getMaxHP() > 1)
                room->playSkillEffect(objectName(), 3);
            room->loseMaxHp(opt);

            if(opt->isAlive()){
                LogMessage log;
                log.type = "#Tengfei";
                log.from = opt;
                log.arg = objectName();
                room->sendLog(log);

                opt->gainAnExtraTurn(opt);
            }
        }
        return false;
    }
};

class TengfeiSlash: public SlashSkill{
public:
    TengfeiSlash():SlashSkill("#tengfei-slash"){
    }

    virtual int getSlashRange(const Player *op, const Player *, const Card *) const{
        if(op->hasSkill("tengfei"))
            return op->getHp();
        else
            return 0;
    }
};

class Paohong: public FilterSkill{
public:
    Paohong():FilterSkill("paohong"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getCard();
        return card->objectName() == "slash" && card->isBlack();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *c = card_item->getCard();
        ThunderSlash *bs = new ThunderSlash(c->getSuit(), c->getNumber());
        bs->setSkillName(objectName());
        bs->addSubcard(c);
        return bs;
    }
};

class PaohongSlash: public SlashSkill{
public:
    PaohongSlash():SlashSkill("#paohong-slash"){
    }

    virtual int getSlashRange(const Player *from, const Player *, const Card *card) const{
        if(from->hasSkill("paohong") && card){
            const Slash *slash = qobject_cast<const Slash*>(card);
            if(slash->getNature() == DamageStruct::Thunder)
                return 998;
        }
        return 0;
    }
};

SinueCard::SinueCard(){
}

bool SinueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return Self->distanceTo(to_select) == 1;
}

void SinueCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    LogMessage log;
    log.type = "#UseSkill";
    log.from = effect.from;
    log.to << effect.to;
    log.arg = "sinue";
    room->sendLog(log);

    DamageStruct damage;
    damage.damage = 2;
    damage.from = effect.from;
    damage.to = effect.to;
    room->damage(damage);
}

class SinueViewAsSkill: public OneCardViewAsSkill{
public:
    SinueViewAsSkill():OneCardViewAsSkill("sinue"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@sinue";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        SinueCard *card = new SinueCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return ! to_select->isEquipped();
    }
};

class Sinue: public TriggerSkill{
public:
    Sinue():TriggerSkill("sinue"){
        events << Death;
        view_as_skill = new SinueViewAsSkill;
    }

    virtual int getPriority(TriggerEvent) const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;
        if(!killer || !killer->hasSkill(objectName()) || killer == player)
            return false;
        if(killer->getPhase() == Player::Play && !killer->isKongcheng())
            room->askForUseCard(killer, "@@sinue", "@sinue", true);
        return false;
    }
};

class Liuxing: public TriggerSkill{
public:
    Liuxing():TriggerSkill("liuxing"){
        events << Damaged << AskForRetrial;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->hasMark("wudao_wake"))
            return false;
        if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.damage > 0){
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = player;
                log.arg = objectName();
                room->playSkillEffect(objectName());
                room->sendLog(log);
            }
            for(int i = 0; i < damage.damage; i++){
                if(player->isDead())
                    break;
                const Card *card = room->peek();
                room->showCard(player, card->getEffectiveId());
                room->getThread()->delay();

                if(!(card->isRed() && card->isKindOf("BasicCard")))
                    player->addToPile("shang", card);
            }
        }
        else{
            JudgeStar judge = data.value<JudgeStar>();
            player->tag["Judge"] = data;
            if(player->getPile("shang").isEmpty() || !player->askForSkillInvoke(objectName(), data))
                return false;

            QList<int> card_ids = player->getPile("shang");
            room->fillAG(card_ids, player);
            int card_id = room->askForAG(player, card_ids, false, objectName());
            player->invoke("clearAG");
            if(card_id > 0){
                player->obtainCard(judge->card);
                judge->card = Sanguosha->getCard(card_id);
                room->moveCardTo(judge->card, NULL, Player::Special);

                LogMessage log;
                log.type = "$ChangedJudge";
                log.from = player;
                log.to << judge->who;
                log.card_str = QString::number(card_id);
                room->sendLog(log);

                room->sendJudgeResult(player);
            }
        }
        return false;
    }
};

class Hunyuan: public PhaseChangeSkill{
public:
    Hunyuan():PhaseChangeSkill("hunyuan"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && !target->hasMark("hunyuan_wake")
                && target->getPhase() == Player::Finish
                && target->getPile("shang").count() >= 3;
    }

    virtual bool onPhaseChange(ServerPlayer *fanrui) const{
        Room *room = fanrui->getRoom();

        room->awake(fanrui, objectName(), "2500", 2500);

        room->loseMaxHp(fanrui);
        room->drawCards(fanrui, 2);
        room->acquireSkill(fanrui, "qimen");
        return false;
    }
};

class Kongmen: public TriggerSkill{
public:
    Kongmen():TriggerSkill("kongmen"){
        events << CardLost;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *mowang, QVariant &data) const{
        if(mowang->isKongcheng()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Hand){
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = mowang;
                log.arg = objectName();
                room->playSkillEffect(objectName());
                room->sendLog(log);
                RecoverStruct o;
                o.card = Sanguosha->getCard(move->card_id);
                room->recover(mowang, o);
            }
        }
        return false;
    }
};

class Wudao: public PhaseChangeSkill{
public:
    Wudao():PhaseChangeSkill("wudao"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target && target->getPhase() == Player::RoundStart;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> fanruis = room->findPlayersBySkillName(objectName());
        if(fanruis.isEmpty())
            return false;

        foreach(ServerPlayer *fanrui, fanruis){
            if(fanrui->hasMark("wudao_wake") || fanrui->getPile("shang").count() < 5)
                continue;
            room->awake(fanrui, objectName(), "2500", 2500);
            room->loseMaxHp(fanrui);
            room->detachSkillFromPlayer(fanrui, "liuxing");
            room->acquireSkill(fanrui, "butian");
            room->acquireSkill(fanrui, "kongmen");
        }
        return false;
    }
};

FangzaoCard::FangzaoCard(){
    once = true;
}

bool FangzaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void FangzaoCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    room->playSkillEffect("fangzao", 1);
    const Card *card = room->askForCardShow(effect.to, effect.from, "fangzao");
    int card_id = card->getEffectiveId();
    room->showCard(effect.to, card_id);

    if(card->getTypeId() == Card::Basic || card->isNDTrick()){
        room->setPlayerMark(effect.from, "fangzao", card_id);
        room->setPlayerFlag(effect.from, "fangzao");
    }else{
        room->setPlayerFlag(effect.from, "-fangzao");
    }
}

class FangzaoViewAsSkill: public ViewAsSkill{
public:
    FangzaoViewAsSkill():ViewAsSkill("fangzao"){

    }

    virtual int getEffectIndex(ServerPlayer *, const Card *card) const{
        if(card->getTypeId() == Card::Basic)
            return 2;
        else
            return 3;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->hasUsed("FangzaoCard") && player->hasFlag("fangzao")){
            int card_id = player->getMark("fangzao");
            const Card *card = Sanguosha->getCard(card_id);
            return card->isAvailable(player);
        }else
            return true;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if(player->getPhase() == Player::NotActive)
            return false;

        if(!player->hasFlag("fangzao"))
            return false;

        if(player->hasUsed("FangzaoCard")){
            int card_id = player->getMark("fangzao");
            const Card *card = Sanguosha->getCard(card_id);
            return pattern.contains(card->objectName());
        }else
            return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(Self->hasUsed("FangzaoCard") && selected.isEmpty() && Self->hasFlag("fangzao")){
            return !to_select->isEquipped();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(Self->hasUsed("FangzaoCard")){
            if(!Self->hasFlag("fangzao"))
                return false;

            if(cards.length() != 1)
                return NULL;

            int card_id = Self->getMark("fangzao");
            const Card *card = Sanguosha->getCard(card_id);
            const Card *first = cards.first()->getFilteredCard();

            Card *new_card = Sanguosha->cloneCard(card->objectName(), first->getSuit(), first->getNumber());
            new_card->addSubcards(cards);
            new_card->setSkillName(objectName());
            return new_card;
        }else{
            return new FangzaoCard;
        }
    }
};

class Fangzao: public TriggerSkill{
public:
    Fangzao():TriggerSkill("fangzao"){
        view_as_skill = new FangzaoViewAsSkill;
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *jindajian, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->getSkillName() == "fangzao"){
            room->setPlayerFlag(jindajian, "-fangzao");
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

    virtual int getPriority(TriggerEvent) const{
        return -2;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *jindajian, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        CardStar card = judge->card;

        if(card->inherits("BasicCard") && jindajian->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            jindajian->drawCards(1);
            return true;
        }
        return false;
    }
};

FeizhenCard::FeizhenCard(){
    mute = true;
    will_throw = false;
}

bool FeizhenCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(targets.length() == 2)
        return true;
    return targets.length() == 1 && Self->canSlash(targets.first());
}

bool FeizhenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.isEmpty()){
        return to_select->getWeapon() && to_select != Self;
    }else if(targets.length() == 1){
        const Player *first = targets.first();
        return to_select != Self && first->getWeapon() && Self->canSlash(to_select);
    }else
        return false;
}

void FeizhenCard::onUse(Room *room, const CardUseStruct &card_use) const{
    QList<ServerPlayer *> targets = card_use.to;
    PlayerStar target; PlayerStar source = card_use.from;
    if(targets.length() > 1)
        target = targets.at(1);
    else if(targets.length() == 1 && source->canSlash(targets.first())){
        target = targets.first();
    }
    else
        return;

    const Card *weapon = target->getWeapon();
    if(weapon){
        Slash *slash = new Slash(weapon->getSuit(), weapon->getNumber());
        slash->setSkillName(skill_name);
        slash->addSubcard(weapon);
        room->throwCard(weapon->getId(), target, source);
        CardUseStruct use;
        use.card = slash;
        use.from = source;
        use.to << target;
        room->useCard(use);
    }
}

class FeizhenViewAsSkill:public ZeroCardViewAsSkill{
public:
    FeizhenViewAsSkill():ZeroCardViewAsSkill("feizhen"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual const Card *viewAs() const{
        return new FeizhenCard;
    }
};

class Feizhen: public TriggerSkill{
public:
    Feizhen():TriggerSkill("feizhen"){
        events << CardAsked;
        view_as_skill = new FeizhenViewAsSkill;
    }

    virtual int getPriority(TriggerEvent) const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        QString asked = data.toString();
        if(asked != "slash" && asked != "jink")
            return false;
        QList<ServerPlayer *> playerAs, playerBs;
        foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
            if(asked == "slash" && tmp->getWeapon())
                playerAs << tmp;
            if(asked == "jink" &&
                    (tmp->getArmor() || tmp->getOffensiveHorse() || tmp->getDefensiveHorse()))
                playerBs << tmp;
        }
        if((asked == "slash" && playerAs.isEmpty()) || (asked == "jink" && playerBs.isEmpty()))
            return false;
        if(room->askForSkillInvoke(player, objectName(), data)){
            ServerPlayer *target = asked == "slash" ?
                                   room->askForPlayerChosen(player, playerAs, objectName()) :
                                   room->askForPlayerChosen(player, playerBs, objectName());
            const Card *card = NULL;
            if(asked == "slash")
                card = target->getWeapon();
            else if(asked == "jink"){
                if(target->getEquips().length() == 1 && !target->getWeapon())
                    card = target->getEquips().first();
                else if(target->getWeapon() && target->getEquips().length() == 2)
                    card = target->getEquips().at(1);
                else
                    card = Sanguosha->getCard(room->askForCardChosen(player, target, "e", objectName()));
            }
            if(asked == "jink" && target->getWeapon() && target->getWeapon()->getId() == card->getId())
                return false;
            if(asked == "slash"){
                Slash *feizhen_card = new Slash(card->getSuit(), card->getNumber());
                feizhen_card->setSkillName(objectName());
                feizhen_card->addSubcard(card);
                room->provide(feizhen_card);
            }
            else{
                Jink *feizhen_card = new Jink(card->getSuit(), card->getNumber());
                feizhen_card->setSkillName(objectName());
                feizhen_card->addSubcard(card);
                room->provide(feizhen_card);
            }
        }
        return false;
    }
};

class Konghe: public TriggerSkill{
public:
    Konghe():TriggerSkill("konghe"){
        events << PhaseChange << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        QList<ServerPlayer *> uglys = room->findPlayersBySkillName(objectName());
        if(event == PhaseChange){
            if(uglys.contains(player) || player->getPhase() != Player::RoundStart)
                return false;
            foreach(ServerPlayer *ugly, uglys){
                if(ugly->faceUp())
                    continue;
                if(ugly->askForSkillInvoke(objectName())){
                    ugly->turnOver();
                    QString choice = room->askForChoice(ugly, objectName(), "k1+k2+k3");
                    player->skip(Player::Judge);
                    if(choice == "k1"){
                        room->playSkillEffect(objectName(), qrand() % 2 + 1);
                        player->skip(Player::Draw);
                    }
                    else if(choice == "k2"){
                        room->playSkillEffect(objectName(), qrand() % 2 + 3);
                        player->skip(Player::Play);
                    }
                    else{
                        room->playSkillEffect(objectName(), qrand() % 2 + 5);
                        player->skip(Player::Discard);
                    }
                    break;
                }
            }
        }
        else{
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.to->isDead() || !uglys.contains(damage.to))
                return false;
            if(damage.to->faceUp() && damage.to->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName(), qrand() % 2 + 7);
                damage.to->turnOver();
            }
        }
        return false;
    }
};

JiejiuCard::JiejiuCard(){
    target_fixed = true;
    mute = true;
}

void JiejiuCard::use(Room *, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    const Card *card = Sanguosha->getCard(getSubcards().first());
    PlayerStar target = source->tag["JiejiuSource"].value<PlayerStar>();
    target->playSkillEffect(skill_name, 1);
    source->pindian(target, skill_name, card);
}

class JiejiuViewAsSkill: public OneCardViewAsSkill{
public:
    JiejiuViewAsSkill():OneCardViewAsSkill("jiejiu"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@jiejiu";
    }

    virtual bool viewFilter(const CardItem *i) const{
        return !i->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        JiejiuCard *card = new JiejiuCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Jiejiu: public TriggerSkill{
public:
    Jiejiu():TriggerSkill("jiejiu"){
        events << DamageProceed;
        view_as_skill = new JiejiuViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *, QVariant &data) const{
        QList<ServerPlayer *> huoyansuanni = room->findPlayersBySkillName(objectName());
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.from || damage.from->isKongcheng())
            return false;
        if(damage.nature != DamageStruct::Normal || damage.to->isDead())
            return false;
        foreach(ServerPlayer *suanni, huoyansuanni){
            if(suanni == damage.from)
                continue;
            if(!suanni->hasMark("@block") && !suanni->isKongcheng()){
                suanni->tag["JiejiuSource"] = QVariant::fromValue((PlayerStar)damage.from);
                room->askForUseCard(suanni, "@@jiejiu", QString("@jiejiu:%1:%2").arg(damage.from->objectName()).arg(damage.to->objectName()), true);
                suanni->tag.remove("JiejiuSource");
                if(suanni->getMark("jiejiu") == 4){
                    suanni->loseAllMarks("jiejiu");
                    LogMessage log;
                    log.type = "#Jiejiu";
                    log.from = suanni;
                    log.arg = objectName();
                    log.to << damage.to;
                    room->sendLog(log);
                    room->setEmotion(damage.to, "avoid");
                    return true;
                }
            }
        }
        return false;
    }
};

class JiejiuPindian: public TriggerSkill{
public:
    JiejiuPindian():TriggerSkill("#jiejiu_pindian"){
        events << Pindian << PhaseChange;
    }

    virtual int getPriority(TriggerEvent event) const{
        return event == Pindian ? -1 : 1;
    }

    virtual bool trigger(TriggerEvent event, Room*, ServerPlayer *player, QVariant &data) const{
        if(event == PhaseChange){
            if(player->getPhase() == Player::RoundStart)
                player->loseAllMarks("@block");
            return false;
        }
        PindianStar pindian = data.value<PindianStar>();
        if(pindian->reason == "jiejiu"){
            if(pindian->isSuccess()){
                player->setMark("jiejiu", 4);
                player->obtainCard(pindian->to_card);
                player->playSkillEffect("jiejiu", 2);
            }
            else{
                player->gainMark("@block");
                player->playSkillEffect("jiejiu", 3);
            }
        }
        return false;
    }
};

XiangmaCard::XiangmaCard(){
}

bool XiangmaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    if(to_select->hasFlag("xmtarget"))
        return false;
    return to_select->getOffensiveHorse() || to_select->getDefensiveHorse();
}

void XiangmaCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "e", skill_name);
    const Card *horse = Sanguosha->getCard(card_id);
    if(horse->isKindOf("Horse"))
        room->throwCard(card_id, effect.to, effect.from);
    else{
        if(effect.to->getOffensiveHorse())
            room->throwCard(effect.to->getOffensiveHorse(), effect.to, effect.from);
        else
            room->throwCard(effect.to->getDefensiveHorse(), effect.to, effect.from);
    }
}

class XiangmaViewAsSkill: public ZeroCardViewAsSkill{
public:
    XiangmaViewAsSkill():ZeroCardViewAsSkill("xiangma"){
    }

    virtual const Card *viewAs() const{
        return new XiangmaCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@xiangma";
    }
};

class Xiangma: public TriggerSkill{
public:
    Xiangma():TriggerSkill("xiangma"){
        events << Dying;
        view_as_skill = new XiangmaViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        //DyingStruct dying = data.value<DyingStruct>();
        QList<ServerPlayer *> huangfus = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *duan, huangfus){
            if(player->getHp() > 0)
                break;
            room->setPlayerFlag(player, "xmtarget");
            if(room->askForUseCard(duan, "@@xiangma", "@xiangma:" + player->objectName(), true)){
                Peach *peach = new Peach(Card::NoSuit, 0);
                peach->setSkillName(objectName());
                CardUseStruct use;
                use.card = peach;
                use.from = duan;
                use.to << player;
                room->useCard(use);
            }
            room->setPlayerFlag(player, "-xmtarget");
        }
        return false;
    }
};

class Yima: public TriggerSkill{
public:
    Yima():TriggerSkill("yima"){
        events << CardLost << FinishJudge;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual int getPriority(TriggerEvent) const{
        return -1;
    }

    static void doYima(ServerPlayer *player, const QList<ServerPlayer *> huangfus, const Card *horse){
        Room *room = player->getRoom();
        foreach(ServerPlayer *huangfu, huangfus){
            if(huangfu != player && horse->isKindOf("Horse")
                && huangfu->askForSkillInvoke("yima")){
                room->playSkillEffect("yima");
                huangfu->obtainCard(horse);
                if(player && player->isWounded()){
                    RecoverStruct ruc;
                    ruc.who = huangfu;
                    room->recover(player, ruc);
                }
                break;
            }
        }
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        QList<ServerPlayer *> huangfus = room->findPlayersBySkillName(objectName());
        if(huangfus.isEmpty())
            return false;
        if(event == CardLost){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->to_place == Player::DiscardedPile)
                doYima(player, huangfus, Sanguosha->getCard(move->card_id));
        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(room->getCardPlace(judge->card->getEffectiveId()) == Player::DiscardedPile)
                doYima(player, huangfus, judge->card);
        }
        return false;
    }
};

SouguaCard::SouguaCard(){
}

bool SouguaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.length() < 4 && !to_select->isKongcheng() && to_select != Self;
}

bool SouguaCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() <= 4 && !targets.isEmpty();
}

void SouguaCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    source->turnOver();
    SkillCard::use(room, source, targets);
}

void SouguaCard::onEffect(const CardEffectStruct &effect) const{
    if(!effect.to->isKongcheng())
        effect.from->getRoom()->obtainCard(effect.from, effect.to->getRandomHandCardId(), false);
}

class SouguaViewAsSkill: public ZeroCardViewAsSkill{
public:
    SouguaViewAsSkill():ZeroCardViewAsSkill("sougua"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@sougua";
    }

    virtual const Card *viewAs() const{
        return new SouguaCard;
    }
};

class Sougua: public PhaseChangeSkill{
public:
    Sougua():PhaseChangeSkill("sougua"){
        view_as_skill = new SouguaViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *gouguan) const{
        Room *room = gouguan->getRoom();
        return gouguan->getPhase() == Player::Draw && room->askForUseCard(gouguan, "@@sougua", "@sougua", true);
    }
};

class Liushou: public TriggerSkill{
public:
    Liushou():TriggerSkill("liushou"){
        events << SlashEffected << CardLost;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == SlashEffected){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(!player->faceUp()){
                room->playSkillEffect(objectName(), qrand() % 2 + 1);
                LogMessage log;
                log.from = effect.from;
                log.to << player;
                log.type = "#ComskillNullify";
                log.arg2 = objectName();
                log.arg = effect.slash->objectName();
                room->sendLog(log);

                return true;
            }
        }else if(event == CardLost && player->getPhase() == Player::NotActive){
            CardMoveStar move = data.value<CardMoveStar>();
            if(player->isDead())
                return false;
            if(move->from_place == Player::Equip && !player->faceUp()){
                room->playSkillEffect(objectName(), qrand() % 2 + 3);
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);

                player->turnOver();
            }
        }
        return false;
    }
};

ZhaoanCard::ZhaoanCard(){
    mute = true;
}

bool ZhaoanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && to_select->getKingdom() != Self->getKingdom();
}

void ZhaoanCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->playSkillEffect(skill_name, 1);
    const Card *resp = room->askForCard(effect.to, "Slash,Weapon", "@zhaoan:" + effect.from->objectName(), QVariant::fromValue(effect), NonTrigger);
    if(resp){
        room->playSkillEffect(skill_name, 2);
        effect.to->drawCards(2);
    }
    else{
        LogMessage log;
        log.type = "#Zhaoan";
        log.from = effect.to;
        log.arg = skill_name;
        room->sendLog(log);
        room->playSkillEffect(skill_name, 3);
        room->setPlayerFlag(effect.to, "%zhaoan");
    }
}

class Zhaoan: public ZeroCardViewAsSkill{
public:
    Zhaoan():ZeroCardViewAsSkill("zhaoan"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ZhaoanCard");
    }

    virtual const Card *viewAs() const{
        return new ZhaoanCard;
    }
};

class Fuxu:public MasochismSkill{
public:
    Fuxu():MasochismSkill("fuxu"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        if(!damage.card || !damage.card->isKindOf("Slash"))
            return;

        QVariant data = QVariant::fromValue(damage);
        QList<ServerPlayer *> xius = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *xiu, xius){
            if(player->isDead())
                break;
            if(xiu->askForSkillInvoke(objectName(), data)){
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(.*):(.*)");
                judge.reason = objectName();
                judge.who = player;

                room->judge(judge);
                if(judge.card->isBlack()){
                    room->playSkillEffect(objectName(), qrand() % 2 + 1);
                    player->obtainCard(judge.card);
                    xiu->drawCards(1);
                }
                else if(judge.card->isRed()){
                    room->playSkillEffect(objectName(), qrand() % 2 + 3);
                    if(player->isWounded() &&
                       room->askForCard(xiu, ".|.|.|hand|red", "@fuxu", data, CardDiscarded)){
                        RecoverStruct t;
                        t.who = xiu;
                        room->recover(player, t);
                    }
                }
            }
        }
    }
};

SnakePackage::SnakePackage()
    :GeneralPackage("snake")
{
    General *muhong = new General(this, "muhong", "jiang");
    muhong->addSkill(new Wuzu);
    skills << new WuzuSlash << new WuzuDistance;

    General *xuanzan = new General(this, "xuanzan", "guan");
    xuanzan->addSkill(new Konghe);

    General *oupeng = new General(this, "oupeng", "jiang", 5);
    oupeng->addSkill("#hp-1");
    oupeng->addSkill(new Zhanchi);
    oupeng->addSkill(new MarkAssignSkill("@wings", 1));
    related_skills.insertMulti("zhanchi", "#@wings-1");
    oupeng->addRelateSkill("tengfei");
    skills << new Tengfei << new TengfeiSlash;

    General *lingzhen = new General(this, "lingzhen", "jiang");
    lingzhen->addSkill(new Paohong);
    skills << new PaohongSlash;

    General *baoxu = new General(this, "baoxu", "kou");
    baoxu->addSkill(new Sinue);

    General *fanrui = new General(this, "fanrui", "kou");
    fanrui->addSkill(new Liuxing);
    fanrui->addSkill(new Hunyuan);
    fanrui->addSkill(new Wudao);
    fanrui->addRelateSkill("kongmen");
    related_skills.insertMulti("hunyuan", "qimen");
    related_skills.insertMulti("wudao", "butian");
    //fanrui->addRelateSkill("butian");
    //fanrui->addRelateSkill("qimen");
    skills << new Kongmen;

    General *jindajian = new General(this, "jindajian", "min", 3);
    jindajian->addSkill(new Fangzao);
    jindajian->addSkill(new Jiangxin);

    General *houjian = new General(this, "houjian", "min", 2);
    houjian->addSkill(new Feizhen);

    General *dengfei = new General(this, "dengfei", "kou");
    dengfei->addSkill(new Jiejiu);
    dengfei->addSkill(new JiejiuPindian);
    related_skills.insertMulti("jiejiu", "#jiejiu_pindian");

    General *huangfuduan = new General(this, "huangfuduan", "min", 3);
    huangfuduan->addSkill(new Xiangma);
    huangfuduan->addSkill(new Yima);

    General *liangshijie = new General(this, "liangshijie", "guan", 3);
    liangshijie->addSkill(new Sougua);
    liangshijie->addSkill(new Liushou);

    General *suyuanjing = new General(this, "suyuanjing", "guan", 3);
    suyuanjing->addSkill(new Zhaoan);
    suyuanjing->addSkill(new Fuxu);

    addMetaObject<SinueCard>();
    addMetaObject<FangzaoCard>();
    addMetaObject<FeizhenCard>();
    addMetaObject<JiejiuCard>();
    addMetaObject<XiangmaCard>();
    addMetaObject<SouguaCard>();
    addMetaObject<ZhaoanCard>();
}

ADD_PACKAGE(Snake)
