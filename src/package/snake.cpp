#include "snake.h"
#include "standard.h"
#include "skill.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"
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
                room->broadcastInvoke("animate", "lightbox:$zhanchi");
                while(!opt->getJudgingArea().isEmpty())
                    room->throwCard(opt->getJudgingArea().first()->getId());
                room->acquireSkill(opt, "tengfei");
                opt->loseMark("@wings");
                room->getThread()->delay();
                room->setEmotion(opt, "limited");
                room->broadcastInvoke("playAudio", "limited");
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

    virtual bool onPhaseChange(ServerPlayer *opt) const{
        if(opt->getPhase() == Player::NotActive){
            Room *room = opt->getRoom();
            if(opt->getMaxHP() > 3)
                room->playSkillEffect(objectName(), 1);
            else if(opt->getMaxHP() > 1)
                room->playSkillEffect(objectName(), 2);
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
                room->recover(mowang, o, true);
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
        if(target->getPhase() != Player::Start)
            return false;
        Room *room = target->getRoom();
        QList<ServerPlayer *> fanruis = room->findPlayersBySkillName(objectName());
        if(!fanruis.isEmpty()){
            foreach(ServerPlayer *fanrui, fanruis){
                if(!fanrui->hasMark("wudao_wake") && fanrui->isKongcheng())
                    return true;
            }
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> fanruis = room->findPlayersBySkillName(objectName());
        if(fanruis.isEmpty())
            return false;

        LogMessage log;
        log.type = "#WakeUp";
        log.arg = objectName();
        foreach(ServerPlayer *fanrui, fanruis){
            log.from = fanrui;
            room->sendLog(log);
            room->playSkillEffect(objectName());
            room->broadcastInvoke("animate", "lightbox:$wudao:2500");
            room->getThread()->delay(2500);

            room->drawCards(fanrui, 2);
            room->setPlayerMark(fanrui, "wudao_wake", 1);
            room->loseMaxHp(fanrui);
            room->acquireSkill(fanrui, "butian");
            room->acquireSkill(fanrui, "qimen");
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
            room->setEmotion(player, "good");
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
                    if(choice == "k1")
                        player->skip(Player::Draw);
                    else if(choice == "k2")
                        player->skip(Player::Play);
                    else
                        player->skip(Player::Discard);
                    break;
                }
            }
        }
        else{
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.to->isDead() || !uglys.contains(damage.to))
                return false;
            if(damage.to->faceUp() && damage.to->askForSkillInvoke(objectName()))
                damage.to->turnOver();
        }
        return false;
    }
};

JiejiuCard::JiejiuCard(){
    target_fixed = true;
}

void JiejiuCard::use(Room *, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    const Card *card = Sanguosha->getCard(getSubcards().first());
    PlayerStar target = source->tag["JiejiuSource"].value<PlayerStar>();
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
            if(!suanni->hasMark("@block") && !suanni->isKongcheng()){
                suanni->tag["JiejiuSource"] = QVariant::fromValue((PlayerStar)damage.from);
                room->askForUseCard(suanni, "@@jiejiu", "@jiejiu:" + damage.from->objectName(), true);
                suanni->tag.remove("JiejiuSource");
                if(suanni->getMark("jiejiu") == 4){
                    suanni->loseAllMarks("jiejiu");
                    LogMessage log;
                    log.type = "#Jiejiu";
                    log.from = suanni;
                    log.arg = objectName();
                    log.to << damage.to;
                    room->sendLog(log);
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
            }
            else
                player->gainMark("@block");
        }
        return false;
    }
};

SnakePackage::SnakePackage()
    :GeneralPackage("snake")
{
    General *muhong = new General(this, "muhong", "jiang");
    muhong->addSkill(new Wuzu);
    skills << new WuzuSlash << new WuzuDistance;

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

    General *fanrui = new General(this, "fanrui", "kou", 3);
    fanrui->addSkill(new Kongmen);
    fanrui->addSkill(new Wudao);

    General *jindajian = new General(this, "jindajian", "min", 3);
    jindajian->addSkill(new Fangzao);
    jindajian->addSkill(new Jiangxin);

    General *houjian = new General(this, "houjian", "kou", 2);
    houjian->addSkill(new Feizhen);

    General *xuanzan = new General(this, "xuanzan", "guan");
    xuanzan->addSkill(new Konghe);

    General *dengfei = new General(this, "dengfei", "kou");
    dengfei->addSkill(new Jiejiu);
    dengfei->addSkill(new JiejiuPindian);
    related_skills.insertMulti("jiejiu", "#jiejiu_pindian");

    addMetaObject<SinueCard>();
    addMetaObject<FangzaoCard>();
    addMetaObject<FeizhenCard>();
    addMetaObject<JiejiuCard>();
}

ADD_PACKAGE(Snake)
