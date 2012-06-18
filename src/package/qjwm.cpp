#include "qjwm.h"
#include "skill.h"
#include "standard.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"

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
                    const Card *card = room->askForCard(jiuwenlong, "EquipCard", "@xiagu", data, CardDiscarded);
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

TaolueCard::TaolueCard(){
    once = true;
    mute = true;
    will_throw = false;
}

bool TaolueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void TaolueCard::use(Room *room, ServerPlayer *player, const QList<ServerPlayer *> &targets) const{
    bool success = player->pindian(targets.first(), "Taolue", this);
    if(!success){
        room->playSkillEffect("taolue", 2);
        if(!player->isNude())
            room->askForDiscard(player, "taolue", 1, false, true);
        return;
    }
    room->playSkillEffect("taolue", 1);
    PlayerStar from = targets.first();
    if(from->getCards("ej").isEmpty())
        return;

    int card_id = room->askForCardChosen(player, from , "ej", "taolue");
    const Card *card = Sanguosha->getCard(card_id);
    Player::Place place = room->getCardPlace(card_id);

    int equip_index = -1;
    const DelayedTrick *trick = NULL;
    if(place == Player::Equip){
        const EquipCard *equip = qobject_cast<const EquipCard *>(card);
        equip_index = static_cast<int>(equip->location());
    }else{
        trick = DelayedTrick::CastFrom(card);
    }

    QList<ServerPlayer *> tos;
    foreach(ServerPlayer *p, room->getAlivePlayers()){
        if(equip_index != -1){
            if(p->getEquip(equip_index) == NULL)
                tos << p;
        }else{
            if(!player->isProhibited(p, trick) && !p->containsTrick(trick->objectName()))
                tos << p;
        }
    }
    if(trick && trick->isVirtualCard())
        delete trick;

    room->setTag("TaolueTarget", QVariant::fromValue(from));
    ServerPlayer *to = room->askForPlayerChosen(player, tos, "qiaobian");
    if(to)
        room->moveCardTo(card, to, place);
    room->removeTag("TaolueTarget");
}

class Taolue: public OneCardViewAsSkill{
public:
    Taolue():OneCardViewAsSkill("taolue"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("TaolueCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new TaolueCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Changsheng: public TriggerSkill{
public:
    Changsheng():TriggerSkill("changsheng"){
        events << Pindian;
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *, QVariant &data) const{
        QList<ServerPlayer *> aokoo = room->findPlayersBySkillName(objectName());
        if(aokoo.isEmpty())
            return false;
        PindianStar pindian = data.value<PindianStar>();
        if(!aokoo.contains(pindian->from) && !aokoo.contains(pindian->to))
            return false;
        Card *pdcd;
        foreach(ServerPlayer *aoko, aokoo){
            bool invoke = false;
            if(pindian->from != aoko && pindian->to != aoko)
                continue;
            if(pindian->from != aoko && pindian->to_card->getSuit() == Card::Spade){
                pdcd = Sanguosha->cloneCard(pindian->to_card->objectName(), pindian->to_card->getSuit(), 13);
                pdcd->addSubcard(pindian->to_card);
                pdcd->setSkillName(objectName());
                pindian->to_card = pdcd;
                invoke = true;
                room->playSkillEffect(objectName(), 2);
            }
            else if(pindian->to != aoko && pindian->from_card->getSuit() == Card::Spade){
                pdcd = Sanguosha->cloneCard(pindian->from_card->objectName(), pindian->from_card->getSuit(), 13);
                pdcd->addSubcard(pindian->from_card);
                pdcd->setSkillName(objectName());
                pindian->from_card = pdcd;
                invoke = true;
                room->playSkillEffect(objectName(), 1);
            }

            if(invoke){
                LogMessage log;
                log.type = "#Changsheng";
                log.from = aoko;
                log.arg = objectName();
                room->sendLog(log);
            }

            data = QVariant::fromValue(pindian);
        }
        return false;
    }
};

class Losthp: public TriggerSkill{
public:
    Losthp():TriggerSkill("#losthp"){
        events << GameStart;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        room->setPlayerProperty(player, "hp", player->getHp() - 1);
        return false;
    }
};

class Zhanchi:public PhaseChangeSkill{
public:
    Zhanchi():PhaseChangeSkill("zhanchi"){
        frequency = Limited;
    }

    virtual bool onPhaseChange(ServerPlayer *opt) const{
        if(opt->getMark("@vfui") > 0 && opt->getPhase() == Player::Judge){
            Room *room = opt->getRoom();
            if(opt->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName(), 1);
                room->broadcastInvoke("animate", "lightbox:$zhanchi");
                while(!opt->getJudgingArea().isEmpty())
                    room->throwCard(opt->getJudgingArea().first()->getId());
                room->acquireSkill(opt, "tengfei");
                opt->loseMark("@vfui");
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

class TengfeiRange: public ClientSkill{
public:
    TengfeiRange():ClientSkill("#tengfei-range"){
    }

    virtual int getAtkrg(const Player *from) const{
        return from->getHp();
    }
};

class Longluo:public TriggerSkill{
public:
    Longluo():TriggerSkill("longluo"){
        events << CardEffected;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *shien, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!effect.card->inherits("Slash"))
            return false;

        if(room->askForSkillInvoke(shien, objectName(), data)){
            room->playSkillEffect(objectName());
            for(int i = 0; i < 2; i++){
                int card_id = room->drawCard();
                room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special, true);
                room->getThread()->delay();

                CardStar card = Sanguosha->getCard(card_id);
                LogMessage lolo;
                lolo.from = shien;
                lolo.card_str = card->getEffectIdString();
                if(!card->inherits("BasicCard")){
                    lolo.type = "$Longluo1";
                    room->throwCard(card_id);
                    room->sendLog(lolo);
                }else{
                    lolo.type = "$Longluo2";
                    shien->tag["LongluoCard"] = QVariant::fromValue(card);
                    room->sendLog(lolo);
                    ServerPlayer *target = room->askForPlayerChosen(shien, room->getAllPlayers(), objectName());
                    if(!target)
                        target = shien;
                    room->obtainCard(target, card_id);
                    shien->tag.remove("LongluoCard");
                }
            }
        }
        return false;
    }
};

XiaozaiCard::XiaozaiCard(){
    will_throw = false;
}

bool XiaozaiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    return !to_select->hasFlag("Xiaozai");
}

void XiaozaiCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->obtainCard(effect.to, this, false);
    PlayerStar target = effect.to;
    effect.from->tag["Xiaozai"] = QVariant::fromValue(target);
}

class XiaozaiViewAsSkill: public ViewAsSkill{
public:
    XiaozaiViewAsSkill():ViewAsSkill("xiaozai"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped() && selected.length() < 2;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@xiaozai";
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;
        XiaozaiCard *card = new XiaozaiCard;
        card->addSubcards(cards);
        return card;
    }
};

class Xiaozai: public TriggerSkill{
public:
    Xiaozai():TriggerSkill("xiaozai"){
        view_as_skill = new XiaozaiViewAsSkill;
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.from)
            room->setPlayerFlag(damage.from, "Xiaozai");
        if(player->getHandcardNum() > 1 && room->askForUseCard(player, "@@xiaozai", "@xiaozai")){
            ServerPlayer *cup = player->tag["Xiaozai"].value<PlayerStar>();
            if(cup){
                damage.to = cup;
                room->damage(damage);
                return true;
            }
        }
        if(damage.from){
            room->setPlayerFlag(damage.from, "-Xiaozai");
            player->tag.remove("Xiaozai");
        }
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
        const Card *card = room->askForCard(player, "@butian", prompt, data, CardDiscarded);

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
                rev.who = ren;
                room->recover(ren, rev);
            }
        }
        return false;
    }
};

QJWMPackage::QJWMPackage()
    :Package("QJWM"){

    General *shijin = new General(this, "shijin", "kou");
    shijin->addSkill(new Wubang);
    shijin->addSkill(new Xiagu);

    General *hantao = new General(this, "hantao", "guan");
    hantao->addSkill(new Taolue);
    hantao->addSkill(new Changsheng);

    General *oupeng = new General(this, "oupeng", "jiang", 5);
    oupeng->addSkill(new Losthp);
    oupeng->addSkill(new Zhanchi);
    oupeng->addSkill(new MarkAssignSkill("@vfui", 1));
    related_skills.insertMulti("zhanchi", "#@vfui-1");
    oupeng->addRelateSkill("tengfei");
    skills << new Tengfei << new TengfeiRange;
    related_skills.insertMulti("tengfei", "#tengfei-range");

    General *shien = new General(this, "shien", "min", 3);
    shien->addSkill(new Longluo);
    shien->addSkill(new Xiaozai);

    General *luozhenren = new General(this, "luozhenren", "kou", 3);
    luozhenren->addSkill(new Butian);
    luozhenren->addSkill(new Huaxian);

    addMetaObject<TaolueCard>();
    addMetaObject<XiaozaiCard>();
    addMetaObject<ButianCard>();
}

ADD_PACKAGE(QJWM)
