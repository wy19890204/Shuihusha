#include "standard.h"
#include "skill.h"
#include "guben.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"

class Jielue: public TriggerSkill{
public:
    Jielue():TriggerSkill("jielue"){
        events << SlashEffect << Pindian;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == Pindian){
            PindianStar pindian = data.value<PindianStar>();
            if(pindian->reason == objectName() && pindian->isSuccess())
                pindian->from->obtainCard(pindian->to_card);
            return false;
        }
        if(player->getPhase() != Player::Play)
            return false;
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.slash->getNumber() == 0)
            return false;
        if(effect.slash && !effect.to->isKongcheng() && effect.from->askForSkillInvoke(objectName(), data))
            effect.from->pindian(effect.to, objectName(), effect.slash);
        return false;
    }
};

class Pishan: public SlashBuffSkill{
public:
    Pishan():SlashBuffSkill("pishan"){
        frequency = Compulsory;
    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *gao = effect.from;
        Room *room = gao->getRoom();
        room->playSkillEffect(objectName());
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = gao;
        log.arg = objectName();
        room->sendLog(log);

        room->slashResult(effect, NULL);
        return true;
    }
};

class Shixie: public PhaseChangeSkill{
public:
    Shixie():PhaseChangeSkill("shixie"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *han) const{
        if(han->getPhase() == Player::Finish){
            Room *room = han->getRoom();
            if(han->getSlashCount() == 0){
                room->playSkillEffect(objectName());
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = han;
                log.arg = objectName();
                room->sendLog(log);

                room->loseHp(han);
            }
        }
        return false;
    }
};
// pass hero
class HuangtianPass : public TriggerSkill{
public:
    HuangtianPass():TriggerSkill("huangtian_p"){
        events << CardFinished << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhangjiao, QVariant &data) const{
        if(zhangjiao->getPhase() != Player::NotActive)
            return false;
        Room *room = zhangjiao->getRoom() ;
        CardStar card = NULL;
        if(event == CardFinished){
            CardUseStruct card_use = data.value<CardUseStruct>();
            card = card_use.card;
        }else if(event == CardResponsed){
            card = data.value<CardStar>();
        }
        if(card && (card->isBlack() || card->inherits("GuidaoCard")) && room->askForSkillInvoke(zhangjiao, objectName(), data)){
            ServerPlayer *target = room->askForPlayerChosen(zhangjiao,room->getAlivePlayers(),objectName());
            if(target){
                bool chained = ! target->isChained();
                target->setChained(chained);
                room->broadcastProperty(target, "chained");
            }
        }
        return false;
    }
};

class LeijiPass: public TriggerSkill{
public:
    LeijiPass():TriggerSkill("leiji_p"){
        frequency = Frequent;
        events << CardFinished;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zhangjiao, QVariant &data) const{
        if(zhangjiao->getPhase() == Player::NotActive)
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        Room *room = zhangjiao->getRoom();
        if(use.card && use.card->isBlack() && (use.card->inherits("TrickCard") || use.card->inherits("EquipCard")) && room->askForSkillInvoke(zhangjiao, objectName(), data)){
            ServerPlayer *target = room->askForPlayerChosen(zhangjiao,room->getAlivePlayers(),objectName());
            if(target){
                room->playSkillEffect(objectName());
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(spade):(.*)");
                judge.good = false;
                judge.reason = objectName();
                judge.who = target;
                room->judge(judge);

                DamageStruct damage;
                damage.card = NULL;
                damage.from = zhangjiao;
                damage.nature = DamageStruct::Thunder;
                if(judge.isBad()){
                    damage.to = target;
                    room->damage(damage);
                }else if(judge.card->getSuit() == Card::Club){
                    damage.to = zhangjiao;
                    room->damage(damage);
                }
            }
        }
        return false;
    }
};

class DajiPass : public TriggerSkill{
public:
    DajiPass():TriggerSkill("daji_p"){
        frequency = Frequent;
        events << Damage << Damaged;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zhangjiao, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == damage.Thunder){
            zhangjiao->drawCards(damage.damage);
        }
        return false;
    }
};

// pass boss
TuxiPassCard::TuxiPassCard(){
}

bool TuxiPassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;

    if(to_select == Self)
        return false;

    return !to_select->isNude();
}

void TuxiPassCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    int card_id = room->askForCardChosen(effect.from, effect.to, "he", "tuxi_p");
    const Card *card = Sanguosha->getCard(card_id);
    room->moveCardTo(card, effect.from, Player::Hand, false);
}

void TuxiPassCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->playSkillEffect("tuxi_p");
    foreach(ServerPlayer *target, targets){
        CardEffectStruct effect;
        effect.card = this;
        effect.from = source;
        effect.to = target;
        effect.multiple = true;

        room->cardEffect(effect);
    }
    if(targets.length() < 2)
        source->drawCards(2-targets.length());
}

class TuxiPassViewAsSkill: public ZeroCardViewAsSkill{
public:
    TuxiPassViewAsSkill():ZeroCardViewAsSkill("tuxi_p"){
    }

    virtual const Card *viewAs() const{
        return new TuxiPassCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@tuxi_p";
    }
};

class TuxiPass:public PhaseChangeSkill{
public:
    TuxiPass():PhaseChangeSkill("tuxi_p"){
        view_as_skill = new TuxiPassViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *zhangliao) const{
        if(zhangliao->getPhase() == Player::Draw){
            Room *room = zhangliao->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(zhangliao);
            foreach(ServerPlayer *player, other_players){
                if(!player->isNude()){
                    can_invoke = true;
                    break;
                }
            }

            if(can_invoke && room->askForUseCard(zhangliao, "@@tuxi_p", "@tuxi_p-card"))
                return true;
        }

        return false;
    }
};

LuoyiPassCard::LuoyiPassCard(){
    target_fixed = true;
    once = true;
}

void LuoyiPassCard::use(Room *room, ServerPlayer *xuchu, const QList<ServerPlayer *> &) const{
    room->throwCard(this);
    xuchu->setFlags("luoyi");
    room->playSkillEffect("luoyi_p");
}

class LuoyiPass:public ViewAsSkill{
public:
    LuoyiPass():ViewAsSkill("luoyi_p"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 2){
            return false ;
        }else if(selected.length() == 1){
            return ! (selected.first()->isEquipped() || to_select->isEquipped()) ;
        }else{
            return true ;
        }
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        if(cards.length()== 1 && !cards.first()->getCard()->inherits("EquipCard"))
            return NULL;
        LuoyiPassCard *luoyi_pass_card = new LuoyiPassCard;
        luoyi_pass_card->addSubcards(cards);

        return luoyi_pass_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("LuoyiPassCard");
    }
};

TuodaoPassCard::TuodaoPassCard(){
    once = true;
}

void TuodaoPassCard::use(Room *room, ServerPlayer *guanyu, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    ServerPlayer *from = guanyu;
    ServerPlayer *to = targets.at(0);

    Duel *duel = new Duel(Card::NoSuit, 0);
    duel->setSkillName("tuodao_p");
    duel->setCancelable(false);

    CardUseStruct use;
    use.from = from;
    use.to << to;
    use.card = duel;
    room->useCard(use);
}

class TuodaoPass: public OneCardViewAsSkill{
public:
    TuodaoPass():OneCardViewAsSkill("tuodao_p"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("TuodaoPassCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return (to_select->getCard()->isBlack() && to_select->getFilteredCard()->inherits("Weapon")) ||
                (to_select->getCard()->isRed() && to_select->getFilteredCard()->inherits("Horse"));
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        TuodaoPassCard *tuodao_pass_card = new TuodaoPassCard;
        tuodao_pass_card->addSubcard(card_item->getCard()->getId());

        return tuodao_pass_card;
    }
};

class GuantongPass: public TriggerSkill{
public:
    GuantongPass():TriggerSkill("guantong_p"){
        events << DamageComplete;
        frequency = Frequent;
        default_choice = "draw" ;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }
    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.from == NULL)
            return false;

        if(!damage.from->hasSkill(objectName()))
            return false;

        if(!damage.card || (!damage.card->inherits("Slash") && !damage.card->inherits("Duel")))
            return false;

        ServerPlayer *xuchu = damage.from;
        Room *room = xuchu->getRoom();
        if(xuchu->askForSkillInvoke(objectName(), data)){
            QString choice = "draw" ;
            if(!xuchu->isKongcheng()){
                choice =  room->askForChoice(xuchu,objectName(),"draw+damage");
            }
            if(choice == "damage"){
                if(room->askForDiscard(xuchu, objectName(), 1 , true)){
                    room->playSkillEffect(objectName());
                    damage.to = player->getNextAlive();
                    damage.card = NULL;
                    damage.damage = 1;
                    damage.nature = DamageStruct::Normal ;
                    room->damage(damage);
                }
            }else{
                xuchu->drawCards(1);
            }
        }
        return false;
    }
};

class YijiPass:public TriggerSkill{
public:
    YijiPass():TriggerSkill("yiji_p"){
        frequency = Frequent;
        events << Predamaged << Damaged << AskForPeachesDone;
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *guojia, QVariant &data) const{
        static const QString flag = "yiji-p_dying" ;
        if(event == Predamaged){
            if(guojia->hasFlag(flag))
                guojia->setFlags("-"+flag);
        }else if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(guojia->isAlive()){
                Room *room = guojia->getRoom();

                if(!room->askForSkillInvoke(guojia, objectName()))
                    return false;

                room->playSkillEffect(objectName());
                int n = damage.damage * (guojia->hasFlag(flag) ? 3 : 2);
                guojia->drawCards(n);
                guojia->setFlags("-"+flag);
            }
        }else if(event == AskForPeachesDone){
            if(guojia->getHp() > 0)
                guojia->setFlags(flag);
        }
        return false;
    }
};

class TianduPass:public TriggerSkill{
public:
    TianduPass():TriggerSkill("tiandu_p"){
        events << FinishJudge;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true ;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(p->hasSkill(objectName()) && judge->card->isRed() && judge->card->getNumber() > p->getHp() + p->getHandcardNum() && p->askForSkillInvoke(objectName(), data)){
                room->playSkillEffect(objectName());
                p->drawCards(1);
                LogMessage log;
                log.type = "#TriggerDrawSkill";
                log.from = p;
                log.arg = objectName();
                log.arg2 = QString::number(1);
                room->sendLog(log);
            }
        }
        return false;
    }
};

LiegongPassCard::LiegongPassCard(){
}

bool LiegongPassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;
    if(to_select == Self)
        return false;
    return true;
}

void LiegongPassCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->playSkillEffect("liegong_p");
    int card_id = getSubcards().first();
    const Card *card = Sanguosha->getCard(card_id);
    Slash *slash = new Slash(card->getSuit(), card->getNumber());
    slash->addSubcard(this);
    slash->setSkillName("liegong_p");
    CardUseStruct use;
    use.card = slash;
    use.from = source;
    use.to = targets;
    room->useCard(use,false);
}

class LiegongPass: public OneCardViewAsSkill{
public:
    LiegongPass():OneCardViewAsSkill("liegong_p"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        int card_id = Self->getMark("liegong_p_mark");
        const Card *card = Sanguosha->getCard(card_id);
        return to_select->getFilteredCard()->getSuit() == card->getSuit();
        return !to_select->isEquipped() ;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Self->getMark("liegong_p_mark") > 0 && ! player->hasUsed("LiegongPassCard") ;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LiegongPassCard *card = new LiegongPassCard;
        card->addSubcard(card_item->getCard());
        return card;
    }
};

class LiegongPassMark: public TriggerSkill{
public:
    LiegongPassMark():TriggerSkill("#liegong_p_mark"){
        events << SlashEffect << PhaseChange;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *huangzhong, QVariant &data) const{
        Room *room = huangzhong->getRoom() ;
        if(event == SlashEffect){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            room->setPlayerMark(huangzhong,"liegong_p_mark",effect.slash->getEffectiveId());
            if(effect.slash->getSkillName() == "liegong_p"){
                room->playSkillEffect("liegong_p");
                foreach(const Card *card, effect.to->getEquips()){
                    if(card->getSuit() == effect.slash->getSuit())
                        room->throwCard(card->getEffectiveId());
                }
                room->slashResult(effect, NULL);
                return true;
            }
        }else if(event == PhaseChange && huangzhong->getPhase() == Player::Finish){
            room->setPlayerMark(huangzhong,"liegong_p_mark",0);
        }
        return false;
    }
};

class GongshenPass: public TriggerSkill{
public:
    GongshenPass():TriggerSkill("gongshen_p"){
        events << SlashHit;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *huangzhong, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        int n = 1 ;
        if(effect.slash->isRed())
            n++;
        huangzhong->gainMark("@gongshen_p",n);
        return false;
    }
};

JianhunPassCard::JianhunPassCard(){
}

bool JianhunPassCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int n = 2 ;
    if(Self->getHp() == 1 || Self->getAttackRange() >= 4 || Self->getMark("@gongshen_p") >= 8)
        n ++ ;
    if(targets.length() >= n)
        return false;
    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;
    if(to_select == Self)
        return false;
    return true;
}

void JianhunPassCard::use(Room *room, ServerPlayer *huangzhong, const QList<ServerPlayer *> &targets) const{
    int n = 2 ;
    if(huangzhong->getHp() == 1 || huangzhong->getAttackRange() >= 4 || Self->getMark("@gongshen_p") >= 8)
        n ++ ;
    CardUseStruct use;
    use.from = huangzhong;
    use.card = Sanguosha->getCard(this->getSubcards().first());
    use.to = targets;
    LogMessage log;
    log.from = huangzhong;
    log.to = targets ;
    log.arg = QString::number(n);
    if(targets.length() == 1){
        log.type = "#JianhunPass1";
        room->sendLog(log);
        for(int i=0;i<n;i++){
            if(use.to.first()->isDead())
                break;
            room->useCard(use,false) ;
        }
    }else{
        log.type = "#JianhunPass2";
        room->sendLog(log);
        room->useCard(use,false) ;
    }
    huangzhong->loseAllMarks("@gongshen_p");
}

class JianhunPass:public ViewAsSkill{
public:
    JianhunPass():ViewAsSkill("jianhun_p"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@gongshen_p") >= 5 ;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->inherits("Slash");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 1){
            JianhunPassCard *jpc = new JianhunPassCard ;
            jpc->addSubcard(cards.first()->getCard());
            return jpc ;
        }
        return NULL;
    }
};

class TiejiPass:public TriggerSkill{
public:
    TiejiPass():TriggerSkill("tieji_p"){
        events << SlashProceed << Predamage;
    }
    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == SlashProceed){
            if(data.canConvert<SlashEffectStruct>()){
                SlashEffectStruct effect = data.value<SlashEffectStruct>();

                if(player->isAlive()){
                    ServerPlayer *machao = effect.from;

                    Room *room = machao->getRoom();
                    if(effect.from->askForSkillInvoke(objectName(), QVariant::fromValue(effect))){
                        room->playSkillEffect("tieji_p");

                        JudgeStruct judge;
                        judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                        judge.good = true;
                        judge.reason = objectName();
                        judge.who = machao;

                        room->judge(judge);
                        if(judge.isGood()){
                            if(judge.card->getSuit() == Card::Diamond){
                                effect.to->setFlags("tieji_p_damage");
                            }else if(judge.card->getSuit() == Card::Heart){
                                machao->obtainCard(judge.card);
                            }
                            room->slashResult(effect, NULL);
                            return true;
                        }
                    }
                    return false;
                }
            }
        }else if(event == Predamage){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.to->hasFlag("tieji_p_damage")){
                damage.damage ++ ;
                data = QVariant::fromValue(damage);
                damage.to->setFlags("-tieji_p_damage");
            }
        }
        return false;
    }
};

class MashuPass: public DistanceSkill{
public:
    MashuPass():DistanceSkill("mashu_p"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        if(from->hasSkill(objectName())){
            correct -- ;
        }
        if(to->hasSkill(objectName())){
            if(to->getOffensiveHorse() != NULL)
                correct ++ ;
            if(to->getDefensiveHorse() != NULL)
                correct ++ ;
        }
        return correct;
    }
};

class TongjiPass: public TriggerSkill{
public:
    TongjiPass():TriggerSkill("tongji_p"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *ganning, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = damage.to->getRoom();
        if(damage.card && damage.card->inherits("Slash") && damage.to->isKongcheng() && !ganning->isKongcheng() && ganning->askForSkillInvoke(objectName(), QVariant::fromValue(damage.to)) ){
            if(room->askForDiscard(ganning, objectName(), 1 , true)){
                LogMessage log;
                log.type = "#TriggerDamageUpSkill";
                log.from = ganning;
                log.to << damage.to;
                log.arg = objectName() ;
                log.arg2 = QString::number(damage.damage + 1);
                room->sendLog(log);

                damage.damage ++;
                data = QVariant::fromValue(damage);
            }
        }

        return false;
    }
};

class JieluePass: public TriggerSkill{
public:
    JieluePass():TriggerSkill("jielue_p"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *ganning, QVariant &data) const{
        const Card *card = NULL;
        CardUseStruct use = data.value<CardUseStruct>();
        card = use.card;
        if(card && card->inherits("Dismantlement") && card->getSuit() == Card::Spade){
            ServerPlayer *to = use.to.first();
            Room *room = ganning->getRoom();
            if(!to->getEquips().empty() && ganning->askForSkillInvoke(objectName(), QVariant::fromValue(to))){
                int card_id = room->askForCardChosen(ganning, to, "e", objectName());
                const Card *card = Sanguosha->getCard(card_id);
                room->moveCardTo(card, ganning, Player::Hand, false);
            }
        }
        return false;
    }
};

FanjianPassCard::FanjianPassCard(){
    will_throw = false ;
}

void FanjianPassCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhouyu = effect.from;
    ServerPlayer *target = effect.to;
    Room *room = zhouyu->getRoom();
    room->playSkillEffect("fanjian_p");

    int card_id = getSubcards().first();
    const Card *card = Sanguosha->getCard(card_id);
    Card::Suit suit = room->askForSuit(target, "fanjian");

    LogMessage log;
    log.type = "#ChooseSuit";
    log.from = target;
    log.arg = Card::Suit2String(suit);
    room->sendLog(log);

    room->showCard(zhouyu, card_id);
    room->getThread()->delay();

    if(card->getSuit() != suit){
        DamageStruct damage;
        damage.card = NULL;
        damage.from = zhouyu;
        damage.to = target;

        room->damage(damage);

        if(target->isAlive()){
            target->obtainCard(card);
        }else{
            zhouyu->obtainCard(card);
        }
    }else{
        zhouyu->obtainCard(card);
    }
}

class FanjianPass: public OneCardViewAsSkill{
public:
    FanjianPass():OneCardViewAsSkill("fanjian_p"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("FanjianPassCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        FanjianPassCard *fanjian_pass_card = new FanjianPassCard;
        fanjian_pass_card->addSubcard(card_item->getCard()->getId());

        return fanjian_pass_card;
    }
};

class QuwuPassViewAsSkill: public ViewAsSkill{
public:
    QuwuPassViewAsSkill():ViewAsSkill("quwu_p"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@quwu_p";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 1)
            return false;

        if(!to_select->getCard()->isEquipped())
            return false;

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 1)
            return NULL;

        DummyCard *card = new DummyCard;
        card->setSkillName(objectName());
        card->addSubcards(cards);
        return card;
    }
};

class QuwuPass: public TriggerSkill{
public:
    QuwuPass():TriggerSkill("quwu_p"){
        view_as_skill = new QuwuPassViewAsSkill;
        events << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && ! target->getEquips().empty() ;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        const Card *card = room->askForCard(player, "@@quwu_p", "@quwu_p");
        if(card){
            room->throwCard(card->getEffectiveId());
            LogMessage log;
            log.type = "#QuwuPass";
            log.from = player;
            room->sendLog(log);

            damage.damage--;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class SheliePass:public PhaseChangeSkill{
public:
    SheliePass():PhaseChangeSkill("shelie_p"){
        frequency = Frequent;
        view_as_skill = new TuxiPassViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *shenlumeng) const{
        if(shenlumeng->getPhase() == Player::Draw){
            Room *room = shenlumeng->getRoom();

            if(shenlumeng->askForSkillInvoke(objectName())){
                shenlumeng->drawCards(2);
                room->playSkillEffect(objectName());
                int n = 0 ;
                while(true){
                    int card_id = room->drawCard();
                    room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special, true);
                    room->getThread()->delay();
                    const Card *card = Sanguosha->getCard(card_id);
                    if(card->getNumber() > n){
                        room->obtainCard(shenlumeng, card_id);
                        n = card->getNumber();
                    }else{
                        room->throwCard(card_id);
                        break;
                    }
                }
                return true;
            }
        }
        return false;
    }
};

GubenPackage::GubenPackage()
    :Package("guben")
{
    // hero
    General *wujiao = new General(this, "wujiao", "kou", 3);
    wujiao->addSkill(new HuangtianPass);
    wujiao->addSkill(new LeijiPass);
    wujiao->addSkill(new DajiPass);

    // boss
    General *yinchun = new General(this, "yinchun", "kou");
    yinchun->addSkill(new TuxiPass);

    General *huanqi = new General(this, "huanqi", "jiang");
    huanqi->addSkill(new LuoyiPass);

    General *fatong = new General(this, "fatong", "kou");
    fatong->addSkill(new TuodaoPass);

    General *jinbigui = new General(this, "jinbigui", "jiang");
    jinbigui->addSkill(new GuantongPass);

    General *zhangkui = new General(this, "zhangkui", "kou");
    zhangkui->addSkill(new Jielue);

    General *moqide = new General(this, "moqide", "kou", 3);
    moqide->addSkill(new TianduPass);
    moqide->addSkill(new YijiPass);

    General *gaochonghan = new General(this, "gaochonghan", "jiang", 7);
    gaochonghan->addSkill(new Pishan);
    gaochonghan->addSkill(new Shixie);

    General *cuimeng = new General(this, "cuimeng", "guan");
    cuimeng->addSkill(new LiegongPass);
    cuimeng->addSkill(new GongshenPass);
    cuimeng->addSkill(new JianhunPass);

    General *yaogang = new General(this, "yaogang", "kou");
    yaogang->addSkill(new TiejiPass);
    yaogang->addSkill(new MashuPass);

    General *chenfei = new General(this, "chenfei", "guan");
    chenfei->addSkill(new TongjiPass);
    chenfei->addSkill(new JieluePass);

    General *gaotong = new General(this, "gaotong", "guan", 3);
    gaotong->addSkill(new FanjianPass);
    gaotong->addSkill(new QuwuPass);

    General *wanglin = new General(this, "wanglin", "jiang");
    wanglin->addSkill(new SheliePass);

    addMetaObject<LiegongPassCard>();
    addMetaObject<JianhunPassCard>();
    addMetaObject<TuodaoPassCard>();
    addMetaObject<LuoyiPassCard>();
    addMetaObject<TuxiPassCard>();
    addMetaObject<FanjianPassCard>();
}

ADD_PACKAGE(Guben);
