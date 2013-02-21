#include "rat.h"
#include "maneuvering.h"
#include "plough.h"

class Kong1iang: public TriggerSkill{
public:
    Kong1iang():TriggerSkill("kong1iang"){
        events << PhaseChange;
    }

    static bool CompareBySuit(int card1, int card2){
        const Card *c1 = Sanguosha->getCard(card1);
        const Card *c2 = Sanguosha->getCard(card2);

        int a = static_cast<int>(c1->getSuit());
        int b = static_cast<int>(c2->getSuit());

        return a < b;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *liying, QVariant &data) const{
        if(liying->getPhase() != Player::Draw)
            return false;
        if(room->askForSkillInvoke(liying, objectName())){
            room->playSkillEffect(objectName(), qrand() % 2 + 1);
            liying->drawCards(liying->getMaxHP() + liying->getLostHp());

            QList<int> card_ids = liying->handCards();
            qSort(card_ids.begin(), card_ids.end(), CompareBySuit);
            int count = 0;
            while(!card_ids.isEmpty() && count < 2){
                room->fillAG(card_ids);
                int card_id = -1;
                do{
                    card_id = room->askForAG(liying, card_ids, false, objectName());
                }while(card_id < 0);
                room->throwCard(card_id, liying);
                card_ids.removeOne(card_id);

                // throw the rest cards that matches the same suit
                const Card *card = Sanguosha->getCard(card_id);
                Card::Suit suit = card->getSuit();
                /*LogMessage ogg;
                ogg.type = "#Kongrice";
                ogg.from = liying;
                ogg.arg = card->getSuitString();
                room->sendLog(ogg);*/

                DummyCard *dummy_card = new DummyCard;
                for(int i = card_ids.length() - 1; i > -1; i --){
                    const Card *c = Sanguosha->getCard(card_ids.at(i));
                    if(c->getSuit() == suit){
                        card_ids.removeAt(i);
                        dummy_card->addSubcard(c);
                    }
                }
                room->throwCard(dummy_card, liying);

                count ++;
                room->broadcastInvoke("clearAG");
            }
            room->playSkillEffect(objectName(), 3);
            return true;
        }
        return false;
    }
};

class Shuangzhan: public TriggerSkill{
public:
    Shuangzhan():TriggerSkill("shuangzhan"){
        events << SlashProceed;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *dongping, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        int x = dongping->getPlayersInMyAttackRange().count();
        if(x <= 2){
            room->playSkillEffect(objectName(), qrand() % 2 + 3);

            const Card *first_jink = NULL, *second_jink = NULL;
            first_jink = room->askForCard(effect.to, "jink", "@shuangzhan-jink-1:" + dongping->objectName(), QVariant(), JinkUsed);
            if(first_jink)
                second_jink = room->askForCard(effect.to, "jink", "@shuangzhan-jink-2:" + dongping->objectName(), QVariant(), JinkUsed);

            Card *jink = NULL;
            if(first_jink && second_jink){
                jink = new DummyCard;
                jink->addSubcard(first_jink);
                jink->addSubcard(second_jink);
            }
            room->slashResult(effect, jink);
            return true;
        }
        return false;
    }
};

class ShuangzhanSlash: public SlashSkill{
public:
    ShuangzhanSlash():SlashSkill("#shuangzhan-slash"){
    }

    virtual int getSlashExtraGoals(const Player *from, const Player *, const Card *) const{
        if(from->hasSkill("shuangzhan")){
            int x = 0;
            foreach(const Player *tmp, from->getSiblings())
                if(tmp->isAlive() && from->inMyAttackRange(tmp))
                    x++;
            if(x > 2)
                return 1;
        }
        return 0;
    }
};

class Yinyu: public TriggerSkill{
public:
    Yinyu():TriggerSkill("yinyu"){
        events << PhaseChange << SlashProceed;
    }

    static void ClearMarks(ServerPlayer *qing){
        foreach(QString mark, qing->getAllMarkName(1, "@stone"))
            qing->getRoom()->setPlayerMark(qing, mark, 0);
        foreach(ServerPlayer *tmp, qing->getRoom()->getOtherPlayers(qing))
            tmp->removeMark("yinyuc");
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *qing, QVariant &data) const{
        if(event == SlashProceed){
            if(qing->hasMark("@stoned")){
                SlashEffectStruct effect = data.value<SlashEffectStruct>();
                if(effect.slash->getSkillName() != "yuanpei"){
                    int index = effect.from->hasMark("mengshi_wake") ? 13: 7;
                    room->playSkillEffect("yinyu", index);
                }
                room->slashResult(effect, NULL);
                return true;
            }
            return false;
        }
        if(qing->getPhase() == Player::RoundStart){
            ClearMarks(qing);
            if(qing->askForSkillInvoke(objectName())){
                int index = qing->hasMark("mengshi_wake") ? 8: qrand() % 2 + 1;
                room->playSkillEffect(objectName(), index);

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(.*):(.*)");
                judge.good = true;
                judge.who = qing;
                judge.reason = objectName();
                room->judge(judge);

                LogMessage log;
                log.from = qing;
                switch(judge.card->getSuit()){
                case Card::Heart:{
                        room->setPlayerMark(qing, "@stoneh", 1);
                        log.type = "#Yinyu1";
                        break;
                    }
                case Card::Diamond:{
                        room->setPlayerMark(qing, "@stoned", 1);
                        log.type = "#Yinyu2";
                        break;
                    }
                case Card::Spade:{
                        room->setPlayerMark(qing, "@stones", 1);
                        log.type = "#Yinyu4";
                        break;
                    }
                case Card::Club:{
                        foreach(ServerPlayer *tmp, room->getOtherPlayers(qing))
                            tmp->addMark("yinyuc");
                        room->setPlayerMark(qing, "@stonec", 1);
                        log.type = "#Yinyu8";
                        break;
                    }
                default:
                    break;
                }
                room->sendLog(log);
            }
        }
        else if(qing->getPhase() == Player::NotActive)
            ClearMarks(qing);
        return false;
    }
};

class YinyuSlash: public ClientSkill{
public:
    YinyuSlash(): ClientSkill("#yinyu-slash"){
    }

    virtual int getSlashRange(const Player *from, const Player *, const Card *) const{
        if(from->hasMark("@stoneh"))
            return 1234;
        else
            return 0;
    }

    virtual int getSlashResidue(const Player *from) const{
        if(from->hasMark("@stones"))
            return 998;
        else
            return 0;
    }

    virtual bool isSlashPenetrate(const Player *, const Player *to, const Card *) const{
        return to->hasMark("yinyuc");
    }
};

class Fuji:public PhaseChangeSkill{
public:
    Fuji():PhaseChangeSkill("fuji"){
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        PlayerStar p = player;
        if(p->getPhase() != Player::Judge || p->getJudgingArea().isEmpty())
            return false;
        Room *room = p->getRoom();
        const Card *card = Sanguosha->cloneCard("assassinate", Card::NoSuit, 0);
        QList<ServerPlayer *> ruan2z = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *ruan2, ruan2z){
            if(ruan2 == p || ruan2->isProhibited(p, card))
                continue;
            if(room->askForCard(ruan2, ".", "@fuji:" + p->objectName(), true, QVariant::fromValue(p), CardDiscarded)){
                Assassinate *ass = new Assassinate(Card::NoSuit, 0);
                ass->setSkillName(objectName());
                ass->setCancelable(false);
                CardUseStruct use;
                use.card = ass;
                use.from = ruan2;
                use.to << p;
                room->useCard(use);
            }
        }
        return false;
    }
};

class Shunshui: public TriggerSkill{
public:
    Shunshui(): TriggerSkill("shunshui"){
        events << CardLost;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual int getPriority(TriggerEvent) const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        QList<ServerPlayer *> zhangQshun = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *zhangshun, zhangQshun){
            CardMoveStar move = data.value<CardMoveStar>();
            if(player != zhangshun && move->from_place == Player::Judging && move->to_place == Player::DiscardedPile){
                QString suit_str = Sanguosha->getCard(move->card_id)->getSuitString();
                QString prompt = QString("@shunshui:::%1").arg(suit_str);
                const Card *card = room->askForCard(zhangshun, ".|" + suit_str, prompt, true, data, CardDiscarded);
                if(card){
                    QList<ServerPlayer *> targets;
                    foreach(ServerPlayer *tmp, room->getAlivePlayers())
                        if(zhangshun->canSlash(tmp, false))
                            targets << tmp;
                    if(!targets.isEmpty()){
                        ServerPlayer *target = room->askForPlayerChosen(zhangshun, targets, objectName());
                        Slash *slash = new Slash(Card::NoSuit, 0);
                        slash->setSkillName(objectName());
                        CardUseStruct use;
                        use.card = slash;
                        use.from = zhangshun;
                        use.to << target;
                        room->useCard(use, false);
                    }
                }
            }
        }
        return false;
    }
};

class Lihun: public TriggerSkill{
public:
    Lihun():TriggerSkill("lihun"){
        events << Dying;
    }

    static int GetCard(Room *room, ServerPlayer *from, ServerPlayer *to){
        int first = room->askForCardChosen(from, to, "he", "lihun");
        room->obtainCard(from, first, room->getCardPlace(first) != Player::Hand);
        return first;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *shun, QVariant &data) const{
        DyingStruct dying = data.value<DyingStruct>();
        DamageStruct *damage = dying.damage;
        if(!damage)
            return false;
        PlayerStar from = damage->from;
        if(from && !from->isNude() && shun->askForSkillInvoke(objectName(), QVariant::fromValue(from))){
            room->playSkillEffect(objectName());
            DummyCard *dummy = new DummyCard;
            dummy->addSubcard(GetCard(room, shun, from));
            if(!from->isNude() && shun->askForSkillInvoke(objectName(), QVariant::fromValue(from)))
                dummy->addSubcard(GetCard(room, shun, from));
            ServerPlayer *target = room->askForPlayerChosen(shun, room->getOtherPlayers(from), objectName());
            room->obtainCard(target, dummy, false);
            delete dummy;
        }
        return false;
    }
};

class Pozhen: public TriggerSkill{
public:
    Pozhen():TriggerSkill("pozhen"){
        events << CardUsed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->isNDTrick()){
            room->playSkillEffect(objectName());
            LogMessage log;
            log.type = "#Pozhen";
            log.from = use.from;
            log.arg = objectName();
            log.arg2 = use.card->objectName();
            room->sendLog(log);
        }
        return false;
    }
};

BuzhenCard::BuzhenCard(){
    mute = true;
}

bool BuzhenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;
    return true;
}

bool BuzhenCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void BuzhenCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *first = targets.first();
    room->swapSeat(first, targets.last());
}

class BuzhenViewAsSkill: public ZeroCardViewAsSkill{
public:
    BuzhenViewAsSkill():ZeroCardViewAsSkill("buzhen"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@buzhen";
    }

    virtual const Card *viewAs() const{
        return new BuzhenCard;
    }
};

class Buzhen:public PhaseChangeSkill{
public:
    Buzhen():PhaseChangeSkill("buzhen"){
        view_as_skill = new BuzhenViewAsSkill;
        frequency = Limited;
    }

    virtual bool onPhaseChange(ServerPlayer *zhuwu) const{
        if(zhuwu->hasMark("@embattle") && zhuwu->getPhase() == Player::NotActive){
            Room *room = zhuwu->getRoom();
            if(zhuwu->isNude())
                return false;
            int n = 5;
            while(room->askForUseCard(zhuwu, "@@buzhen", "@buzhen", true)){
                if(n <= 1)
                    break;
                n--;
            }
            if(n < 5){
                room->playSkillEffect(objectName());
                zhuwu->loseMark("@embattle");
                zhuwu->throwAllEquips();
                zhuwu->throwAllHandCards();
                room->playLightbox(zhuwu, "buzhen", "5500", 5000);
            }
        }
        return false;
    }
};

class Fangzhen: public TriggerSkill{
public:
    Fangzhen():TriggerSkill("fangzhen"){
        events << DamagedProceed;
    }

    virtual int getPriority(TriggerEvent) const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *zhuwu, QVariant &data) const{
        DamageStruct dmag = data.value<DamageStruct>();
        if(!dmag.card || dmag.card->getSuit() == Card::NoSuit || dmag.damage < 1)
            return false;
        QString suit = dmag.card->getSuitString();
        if(room->askForCard(zhuwu, ".|" + suit, "@fangzhen:::" + suit, true, data, CardDiscarded)){
            room->playSkillEffect(objectName());
            LogMessage log;
            log.type = "#Fangzhen";
            log.from = zhuwu;
            log.arg = objectName();
            log.arg2 = QString::number(dmag.damage);
            room->sendLog(log);
            return true;
        }
        return false;
    }
};

ShougeCard::ShougeCard(){
    will_throw = false;
    target_fixed = true;
    mute = true;
}

void ShougeCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->playSkillEffect(skill_name, qrand() % 2 + 1);
    foreach(int table, getSubcards())
        source->addToPile("vege", table);
}

class ShougeViewAsSkill: public ViewAsSkill{
public:
    ShougeViewAsSkill():ViewAsSkill("shouge"){

    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *to_select) const{
        return to_select->getCard()->inherits("Peach") ||
                to_select->getCard()->inherits("Analeptic");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        ShougeCard *card = new ShougeCard;
        card->addSubcards(cards);
        return card;
    }
};

class Shouge: public TriggerSkill{
public:
    Shouge():TriggerSkill("shouge"){
        view_as_skill = new ShougeViewAsSkill;
        events << CardLost;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *vgqq, QVariant &data) const{
        if(vgqq->getPile("vege").isEmpty() || vgqq->getPhase() != Player::NotActive)
            return false;
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->from_place == Player::Hand && move->to != vgqq
           && vgqq->isAlive() && vgqq->askForSkillInvoke(objectName())){
            room->throwCard(vgqq->getPile("vege").last());
            room->playSkillEffect("shouge", qrand() % 2 + 3);
            vgqq->drawCards(3);
            return vgqq->getPile("vege").isEmpty();
        }
        return false;
    }
};

class Qiongtu: public PhaseChangeSkill{
public:
    Qiongtu():PhaseChangeSkill("qiongtu"){
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual int getPriority(TriggerEvent) const{
        return -1;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        PlayerStar target = player;
        if(target->getPhase() != Player::Finish)
            return false;
        QList<ServerPlayer *> zhangqings = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *zhangqing, zhangqings){
            if(zhangqing != player && target->getHandcardNum() <= 1 && !target->isNude()
                && zhangqing->askForSkillInvoke(objectName(), QVariant::fromValue(target))){
                room->playSkillEffect(objectName());
                int card_id = room->askForCardChosen(zhangqing, target, "he", objectName());
                room->obtainCard(zhangqing, card_id, room->getCardPlace(card_id) != Player::Hand);
            }
        }
        return false;
    }
};

class Xiayao: public OneCardViewAsSkill{
public:
    Xiayao():OneCardViewAsSkill("xiayao"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getPhase() == Player::Play;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Ecstasy *ecstasy = new Ecstasy(card->getSuit(), card->getNumber());
        ecstasy->setSkillName(objectName());
        ecstasy->addSubcard(card_item->getFilteredCard());

        return ecstasy;
    }
};

class ShudanClear: public TriggerSkill{
public:
    ShudanClear():TriggerSkill("#shudan_clear"){
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &) const{
        if(player->getPhase() == Player::NotActive)
            room->setTag("Shudan", QVariant());
        return false;
    }
};

class Shudan: public TriggerSkill{
public:
    Shudan():TriggerSkill("shudan"){
        events << Damaged << CardEffected;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::NotActive)
            return false;
        if(event == Damaged){
            room->setTag("Shudan", player->objectName());
            room->playSkillEffect(objectName(), 1);

            LogMessage log;
            log.type = "#ShudanDamaged";
            log.from = player;
            room->sendLog(log);

        }else if(event == CardEffected){
            if(room->getTag("Shudan").toString() != player->objectName())
                return false;
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.card->inherits("Slash") || effect.card->isNDTrick()){
                room->playSkillEffect(objectName(), 2);
                LogMessage log;
                log.type = "#ShudanAvoid";
                log.arg = objectName();
                log.from = player;
                room->sendLog(log);
                return true;
            }
        }
        return false;
    }
};

class Feiyan: public ClientSkill{
public:
    Feiyan():ClientSkill("feiyan"){
    }

    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->inherits("Snatch") || card->inherits("SupplyShortage");
    }
};

class Shentou: public OneCardViewAsSkill{
public:
    Shentou():OneCardViewAsSkill("shentou"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Snatch *snatch = new Snatch(first->getSuit(), first->getNumber());
        snatch->addSubcard(first->getId());
        snatch->setSkillName(objectName());
        return snatch;
    }
};

class Dujian: public TriggerSkill{
public:
    Dujian():TriggerSkill("dujian"){
        events << DamageProceed;
    }

    virtual int getPriority(TriggerEvent) const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to == damage.from || damage.damage < 1)
            return false;
        if(!damage.to->inMyAttackRange(player)
            && player->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            damage.to->turnOver();
            return true;
        }
        return false;
    }
};

HuanshuCard::HuanshuCard(){
    mute = true;
}

bool HuanshuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select != Self;
}

void HuanshuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    bool ichi, me;

    JudgeStruct judge;
    judge.reason = "huanshu1";
    judge.pattern = QRegExp("(.*):(.*):(.*)");
    judge.who = effect.to;
    room->judge(judge);
    ichi = judge.card->isRed();

    judge.reason = "huanshu2";
    judge.pattern = ichi ? QRegExp("(.*):(heart|diamond):(.*)"): QRegExp("(.*):(club|spade):(.*)");
    judge.good = false;
    room->judge(judge);
    me = judge.card->isRed();

    DamageStruct damage;
    damage.damage = 2;
    damage.from = effect.from;
    damage.to = effect.to;
    if(ichi && me){
        damage.nature = DamageStruct::Fire;
        room->playSkillEffect(skill_name, qrand() % 2 + 1);
        room->damage(damage);
    }
    else if(!ichi && !me){
        room->playSkillEffect(skill_name, qrand() % 2 + 1);
        damage.nature = DamageStruct::Thunder;
        room->damage(damage);
    }
    else
        room->playSkillEffect(skill_name, 3);
}

class HuanshuViewAsSkill: public ZeroCardViewAsSkill{
public:
    HuanshuViewAsSkill():ZeroCardViewAsSkill("huanshu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@huanshu";
    }

    virtual const Card *viewAs() const{
        return new HuanshuCard;
    }
};

class Huanshu: public MasochismSkill{
public:
    Huanshu():MasochismSkill("huanshu"){
        view_as_skill = new HuanshuViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *qiaodaoq, const DamageStruct &damage) const{
        Room *room = qiaodaoq->getRoom();
        int x = damage.damage, i;
        for(i=0; i<x; i++){
            if(!room->askForUseCard(qiaodaoq, "@@huanshu", "@huanshu", true))
                break;
        }
    }
};

class Mozhang: public PhaseChangeSkill{
public:
    Mozhang():PhaseChangeSkill("mozhang"){
        frequency = Compulsory;
    }

    virtual int getPriority(TriggerEvent) const{
        return -1;
    }

    virtual bool onPhaseChange(ServerPlayer *p) const{
        if(p->getPhase() == Player::NotActive){
            Room *room = p->getRoom();
            if(!p->isChained()){
                p->setChained(true);
                room->playSkillEffect(objectName());
                LogMessage log;
                log.type = "#Mozhang";
                log.from = p;
                log.arg = objectName();
                room->sendLog(log);

                room->broadcastProperty(p, "chained");
                room->setEmotion(p, "chain");
            }
        }
        return false;
    }
};

YuanpeiCard::YuanpeiCard(){
    mute = true;
    once = true;
}

bool YuanpeiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select->getGenderString() == "male";
}

void YuanpeiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->playSkillEffect(skill_name, qrand() % 2 + 1);
    const Card *card = room->askForCard(effect.to, "Slash,Weapon$", "@yuanpei:" + effect.from->objectName(), QVariant::fromValue(effect), NonTrigger);
    if(card){
        effect.from->obtainCard(card);
        effect.from->drawCards(1);
        effect.to->drawCards(1);
    }
    else{
        room->setPlayerFlag(effect.from, "yuanpei");
        LogMessage lsp;
        lsp.type = "#Yuanpei";
        lsp.from = effect.from;
        lsp.to << effect.to;
        lsp.arg = skill_name;
        room->sendLog(lsp);
    }
}

class Yuanpei: public ViewAsSkill{
public:
    Yuanpei():ViewAsSkill("yuanpei"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(Self->hasFlag("yuanpei"))
            return selected.isEmpty() && !to_select->isEquipped() && to_select->getCard()->isRed();
        else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(Self->hasFlag("yuanpei")){
            if(cards.length() == 1){
                const Card *card = cards.first()->getCard();
                Card *slash = new Slash(card->getSuit(), card->getNumber());
                slash->addSubcard(card->getId());
                slash->setSkillName("yuanpei");
                return slash;
            }
        }
        else{
            if(cards.isEmpty())
                return new YuanpeiCard;
        }
        return NULL;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(!player->hasUsed("YuanpeiCard"))
            return true;
        else
            return player->hasFlag("yuanpei") && Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return Self->hasFlag("yuanpei") && pattern == "slash";
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return qrand() % 2 + 3;
    }
};

class Mengshi: public PhaseChangeSkill{
public:
    Mengshi():PhaseChangeSkill("mengshi"){
        frequency = Wake;
    }

    virtual int getPriority(TriggerEvent) const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && !target->hasMark("mengshi_wake")
                && target->getPhase() == Player::RoundStart
                && target->getHandcardNum() < target->getAttackRange();
    }

    virtual bool onPhaseChange(ServerPlayer *qyyy) const{
        Room *room = qyyy->getRoom();

        room->awake(qyyy, objectName(), "1500", 1500);
        qyyy->drawCards(3);
        room->acquireSkill(qyyy, "yinyu");
        return false;
    }
};

RatPackage::RatPackage()
    :GeneralPackage("rat")
{
    General *liying = new General(this, "liying", "guan");
    liying->addSkill(new Kong1iang);

    General *dongping = new General(this, "dongping", "guan");
    dongping->addSkill(new Shuangzhan);
    skills << new ShuangzhanSlash;

    General *zhangqing = new General(this, "zhangqing", "guan");
    zhangqing->addSkill(new Yinyu);
    skills << new YinyuSlash;

    General *ruanxiaoer = new General(this, "ruanxiaoer", "min");
    ruanxiaoer->addSkill(new Fuji);

    General *zhangshun = new General(this, "zhangshun", "min", 3);
    zhangshun->addSkill(new Shunshui);
    zhangshun->addSkill(new Lihun);

    General *zhuwu = new General(this, "zhuwu", "kou", 3);
    zhuwu->addSkill(new Pozhen);
    zhuwu->addSkill(new Buzhen);
    zhuwu->addSkill(new MarkAssignSkill("@embattle", 1));
    related_skills.insertMulti("buzhen", "#@embattle-1");
    zhuwu->addSkill(new Fangzhen);

    General *qingzhang = new General(this, "qingzhang", "kou", 3);
    qingzhang->addSkill(new Shouge);
    qingzhang->addSkill(new Qiongtu);

    General *baisheng = new General(this, "baisheng", "min", 3);
    baisheng->addSkill(new Xiayao);
    baisheng->addSkill(new Shudan);
    baisheng->addSkill(new ShudanClear);
    related_skills.insertMulti("shudan", "#shudan_clear");

    General *shiqian = new General(this, "shiqian", "kou", 3);
    shiqian->addSkill(new Feiyan);
    shiqian->addSkill(new Shentou);

    General *shiwengong = new General(this, "shiwengong", "jiang");
    shiwengong->addSkill(new Dujian);

    General *qiaodaoqing = new General(this, "qiaodaoqing", "jiang", 3);
    qiaodaoqing->addSkill(new Huanshu);
    qiaodaoqing->addSkill(new Mozhang);

    General *qiongying = new General(this, "qiongying", "jiang", 3, false);
    qiongying->addSkill(new Yuanpei);
    qiongying->addSkill(new Mengshi);
    related_skills.insertMulti("mengshi", "yinyu");

    addMetaObject<BuzhenCard>();
    addMetaObject<ShougeCard>();
    addMetaObject<HuanshuCard>();
    addMetaObject<YuanpeiCard>();
}

ADD_PACKAGE(Rat)
