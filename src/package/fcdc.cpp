#include "fcdc.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"

LianmaCard::LianmaCard(){
    target_fixed = true;
    once = true;
}

void LianmaCard::use(Room *room, ServerPlayer *huyanzhuo, const QList<ServerPlayer *> &) const{
    QList<ServerPlayer *> players = room->getAlivePlayers();

    //room->broadcastSkillInvoke(objectName());
    QString choice = room->askForChoice(huyanzhuo, "lianma", "lian+ma");
    if(choice == "lian"){
        foreach(ServerPlayer *player, players){
            if(player->hasEquip("Horse", true)){
                if(!player->isChained()){
                    player->setChained(true);
                    room->broadcastProperty(player, "chained");
                    room->setEmotion(player, "chain");
                }
            }
        }
    }else{
        foreach(ServerPlayer *player, players){
            if(!player->hasEquip("Horse", true)){
                if(player->isChained())
                    room->setPlayerProperty(player, "chained", false);
            }
        }
    }
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

class Zhongjia: public MaxCardsSkill{
public:
    Zhongjia():MaxCardsSkill("zhongjia"){
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

class Shuangzhan: public TriggerSkill{
public:
    Shuangzhan():TriggerSkill("shuangzhan"){
        events << SlashProceed;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *dongping, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        int x = 0;
        foreach(ServerPlayer *tmp, room->getAlivePlayers()){
            if(dongping->inMyAttackRange(tmp)){
                if(tmp == dongping)
                    continue;
                c ++;
            }
        }
        if(x <= 2){
            Room *room = dongping->getRoom();
            room->playSkillEffect(objectName(), qrand() % 2 + 3);
            const Card *first_jink = NULL, *second_jink = NULL;
            first_jink = room->askForCard(effect.to, "jink", "@shuangzhan-jink-1:" + dongping->objectName(), false, QVariant(), JinkUsed);
            if(first_jink)
                second_jink = room->askForCard(effect.to, "jink", "@shuangzhan-jink-2:" + dongping->objectName(), false, QVariant(), JinkUsed);

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

BomingCard::BomingCard(){
    once = true;
}

bool BomingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select->hasEquip() && Self->inMyAttackRange(to_select) && Self != to_select;
}

void BomingCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    DummyCard *dummy = new DummyCard;
    int dmgnum = effect.to->getEquips().length();
    foreach(const Card *equip, effect.to->getEquips())
        dummy->addSubcard(equip);
    DamageStruct damage;
    damage.from = effect.from;
    damage.to = effect.to;
    damage.card = dummy;
    damage.damage = dmgnum;
    room->damage(damage);
    room->loseHp(effect.from, dmgnum);
}

class Boming: public ZeroCardViewAsSkill{
public:
    Boming():ZeroCardViewAsSkill("boming"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("BomingCard");
    }

    virtual const Card *viewAs() const{
        return new BomingCard;
    }
};

XunlieCard::XunlieCard(){
}

bool XunlieCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int i = 0;
    foreach(const Player *player, Self->getSiblings()){
        if(player->getHandcardNum() >= i){
            i = player->getHandcardNum();
        }
    }
    return targets.isEmpty() && !to_select->isKongcheng() && to_select->getHandcardNum() == i && to_select != Self;
}

void XunlieCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    int i = 0;
    const Card *card = effect.to->getRandomHandCard();
    effect.from->obtainCard(card, false);
    i ++;
    if(!effect.to->isKongcheng() && room->askForChoice(effect.from, "xuelie", "get+cancel") == "get"){
        card = effect.to->getRandomHandCard();
        effect.from->obtainCard(card, false);
        i ++;
    }
    if(i == 1)
        effect.from->drawCards(1);
    room->setEmotion(effect.to, "bad");
    room->setEmotion(effect.from, "good");
}

class XunlieViewAsSkill: public OneCardViewAsSkill{
public:
    XunlieViewAsSkill():OneCardViewAsSkill("xunlie"){
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        XunlieCard *card = new XunlieCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }

    virtual bool viewFilter(const CardItem *to_selec) const{
        return to_selec->getCard()->inherits("EquipCard");
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
        if(xiezhen->getPhase() == Player::Draw){
            Room *room = xiezhen->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(xiezhen);
            foreach(ServerPlayer *player, other_players){
                if(!player->isKongcheng()){
                    can_invoke = true;
                    break;
                }
            }
            if(!can_invoke)
                return false;
            QList<const Card *> cards = xiezhen->getCards("he");
            foreach(const Card *cd, cards){
                if(cd->inherits("EquipCard")){
                    if(room->askForUseCard(xiezhen, "@@xunlie", "@xunlie"))
                        return true;
                    break;
                }
            }
        }
        return false;
    }
};

class Shenjian: public TriggerSkill{
public:
    Shenjian():TriggerSkill("shenjian"){
        events << CardEffected;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->inherits("ArcheryAttack")){
            LogMessage log;
            log.type = "#SkillNullify";
            log.from = player;
            log.arg = objectName();
            log.arg2 = "archery_attack";
            player->getRoom()->sendLog(log);

            return true;
        }else
            return false;
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

class Tiansuan: public TriggerSkill{
public:
    Tiansuan():TriggerSkill("tiansuan"){
        events << Pindian;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *pianzi = room->findPlayerBySkillName(objectName());
        if(!pianzi || !pianzi->askForSkillInvoke(objectName()))
            return false;
        PindianStar pindian = data.value<PindianStar>();
        QStringList choices;
        QString from_card = Sanguosha->translate(QString("@tiansuan:%1:%2:%3:%4").
                            arg(pindian->from->getGeneralName()).
                            arg(pindian->from_card->getSuitString()).
                            arg(pindian->from_card->getNumberString()).
                            arg(pindian->from_card->objectName()));
        QString to_card = Sanguosha->translate(QString("@tiansuan:%1:%2:%3:%4").
                          arg(pindian->to->getGeneralName()).
                          arg(pindian->to_card->getSuitString()).
                          arg(pindian->to_card->getNumberString()).
                          arg(pindian->to_card->objectName()));
        choices << from_card
                << to_card;
/*
        LogMessage log;
        log.type = "$Tiansuan_from";
        log.from = pindian->from;
        log.to << pindian->to;
        log.card_str = pindian->from_card->getEffectIdString();
        room->sendLog(log);
        log.type = "$Tiansuan_to";
        log.from = pianzi;
        log.card_str = pindian->to_card->getEffectIdString();
        room->sendLog(log);
*/
        QString choice = room->askForChoice(pianzi, objectName(), choices.join("+"));
        if(choice == from_card){
            int omiga = room->drawCard();
            //room->moveCardTo(Sanguosha->getCard(omiga), pindian->from, Player::Hand, false);
            room->moveCardTo(pindian->from_card, NULL, Player::DrawPile, true);
            pindian->from_card = Sanguosha->getCard(omiga);
        }
        else{
            int omiga = room->drawCard();
            //room->moveCardTo(Sanguosha->getCard(omiga), pindian->to, Player::Hand, false);
            room->moveCardTo(pindian->to_card, NULL, Player::DrawPile, true);
            pindian->to_card = Sanguosha->getCard(omiga);
        }
        data = QVariant::fromValue(pindian);
        return false;
    }
};

HuazhuCard::HuazhuCard(){
    once = true;
    will_throw = false;
}

bool HuazhuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void HuazhuCard::use(Room *, ServerPlayer *iszjj, const QList<ServerPlayer *> &targets) const{
    iszjj->pindian(targets.first(), "huazhu", this);
}

class HuazhuViewAsSkill: public OneCardViewAsSkill{
public:
    HuazhuViewAsSkill():OneCardViewAsSkill("huazhu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("HuazhuCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new HuazhuCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Huazhu: public TriggerSkill{
public:
    Huazhu():TriggerSkill("huazhu"){
        events << Pindian;
        view_as_skill = new HuazhuViewAsSkill;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        PindianStar pindian = data.value<PindianStar>();
        if(pindian->reason != "huazhu" || !pindian->from->hasSkill(objectName()))
            return false;
        if(pindian->isSuccess()){
            int x = (pindian->from_card->getNumber() + 1) / 2;
            ServerPlayer *target = room->askForPlayerChosen(pindian->from, room->getAllPlayers(), objectName());
            if(target->getHandcardNum() > x)
                room->askForDiscard(target, objectName(), target->getHandcardNum() - x);
            else if(target->getHandcardNum() < x)
                target->drawCards(x - target->getHandcardNum());
        }
        return false;
    }
};

class Shuilao: public OneCardViewAsSkill{
public:
    Shuilao():OneCardViewAsSkill("shuilao"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->isNDTrick();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Indulgence *indulgence = new Indulgence(first->getSuit(), first->getNumber());
        indulgence->addSubcard(first->getId());
        indulgence->setSkillName(objectName());
        return indulgence;
    }
};

class Fengxing: public TriggerSkill{
public:
    Fengxing():TriggerSkill("fengxing"){
        events << PhaseChange << CardLost;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent v, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(v == CardLost){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Hand && player->isAlive()){
                if(player->getHandcardNum() < player->getMaxHP()){
                    LogMessage log;
                    log.type = "#TriggerSkill";
                    log.from = player;
                    log.arg = objectName();
                    room->sendLog(log);
                    player->drawCards(1);
                }
            }
        }
        else{
            if(player->getPhase() == Player::Judge ||
               player->getPhase() == Player::Draw ||
               player->getPhase() == Player::Discard)
                return true;
        }
        return false;
    }
};

#include "plough.h"
class Mitan: public OneCardViewAsSkill{
public:
    Mitan():OneCardViewAsSkill("mitan"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("TrickCard") ||
                to_select->getCard()->inherits("EventsCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *c = card_item->getCard();
        Wiretap *wp = new Wiretap(c->getSuit(), c->getNumber());
        wp->setSkillName(objectName());
        wp->addSubcard(card_item->getCard());

        return wp;
    }
};

class Jibao: public PhaseChangeSkill{
public:
    Jibao():PhaseChangeSkill("jibao"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::RoundStart)
            room->setPlayerMark(player, "jibao", player->getHandcardNum());
        else if(player->getPhase() == Player::NotActive){
            if(player->getMark("jibao") == player->getHandcardNum() &&
               !player->isKongcheng() &&
               player->askForSkillInvoke(objectName())){
                room->askForDiscard(player, objectName(), 1);
                player->gainAnExtraTurn(player);
            }
        }
        return false;
    }
};

class Duoquan: public TriggerSkill{
public:
    Duoquan():TriggerSkill("duoquan"){
        events << Death;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;
        Room *room = player->getRoom();
        ServerPlayer *caijing = room->findPlayerBySkillName(objectName());
        if(caijing && caijing != killer){
            QVariant shiti = QVariant::fromValue((PlayerStar)player);
            if(!room->askForSkillInvoke(caijing, objectName(), shiti))
                return false;
            QStringList skills;
            foreach(const Skill *skill, player->getVisibleSkillList()){
                if(skill->parent() &&
                   skill->getFrequency() != Skill::Limited &&
                   skill->getFrequency() != Skill::Wake &&
                   !skill->isLordSkill()){
                    QString sk = skill->objectName();
                    skills << sk;
                }
            }
            if(!skills.isEmpty()){
                QString skill = room->askForChoice(caijing, objectName(), skills.join("+"));
                room->acquireSkill(caijing, skill);
            }
            DummyCard *all_cards = player->wholeHandCards();
            if(all_cards){
                room->obtainCard(caijing, all_cards, false);
                delete all_cards;
            }
        }
        return false;
    }
};

class Qinxin: public TriggerSkill{
public:
    Qinxin():TriggerSkill("qinxin"){
        events << PhaseChange;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        if(player->getPhase() != Player::RoundStart || !player->askForSkillInvoke(objectName()))
            return false;
        Card::Suit suit = room->askForSuit(player, objectName());
        LogMessage log;
        log.type = "#DeclareSuit";
        log.from = player;
        log.arg = Card::Suit2String(suit);
        room->sendLog(log);

        JudgeStruct judge;
        judge.pattern = QRegExp("(.*):(" + Card::Suit2String(suit) + "):(.*)");
        judge.reason = objectName();
        judge.good = player->isWounded() ? true : false;
        judge.who = player;
        room->judge(judge);

        if(judge.card->getSuit() == suit){
            RecoverStruct rec;
            rec.who = player;
            room->recover(player, rec, true);
        }
        else
            player->obtainCard(judge.card);

        room->playSkillEffect(objectName());
        return false;
    }
};

YinjianCard::YinjianCard(){
    once = true;
    will_throw = false;
}

bool YinjianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    if(targets.length() >= 2 || !to_select->getGeneral()->isMale())
        return false;
    if(targets.length() == 1){
        QString kingdom = targets.first()->getKingdom();
        return to_select->getKingdom() != kingdom;
    }
    return true;
}

bool YinjianCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    return targets.length() == 2;
}

void YinjianCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *from = targets.first();
    ServerPlayer *to = targets.last();

    from->obtainCard(this, false);

    room->obtainCard(to, room->askForCardShow(from, source, "yinjian"), false);
}

class Yinjian: public ViewAsSkill{
public:
    Yinjian():ViewAsSkill("yinjian"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 2)
            return false;
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 2){
            YinjianCard *card = new YinjianCard();
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YinjianCard");
    }
};

class Qingdong: public TriggerSkill{
public:
    Qingdong():TriggerSkill("qingdong"){
        events << CardLost;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName()) &&
                target->getGeneral()->isMale() &&
                target->getPhase() == Player::NotActive;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *conan, QVariant &data) const{
        Room *room = conan->getRoom();
        ServerPlayer *yulan = room->findPlayerBySkillName(objectName());
        CardMoveStar move = data.value<CardMoveStar>();
        if(conan->isDead() || !yulan)
            return false;
        if(move->from_place == Player::Hand && yulan->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(diamond):(.*)");
            judge.good = false;
            judge.reason = objectName();
            judge.who = conan;

            room->judge(judge);
            if(judge.isGood()){
                int cd = conan->getMaxHP() - conan->getHandcardNum();
                conan->drawCards(qMin(4, qMax(0, cd)));
            }
        }
        return false;
    }
};

class Qingshang: public TriggerSkill{
public:
    Qingshang():TriggerSkill("qingshang"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;

        if(killer){
            if(killer != player)
                killer->gainMark("@shang");
        }
        return false;
    }
};

FCDCPackage::FCDCPackage()
    :Package("FCDC")
{
    General *huyanzhuo = new General(this, "huyanzhuo", "guan");
    huyanzhuo->addSkill(new Lianma);
    huyanzhuo->addSkill(new Zhongjia);

    General *dongping = new General(this, "dongping", "guan");
    dongping->addSkill(new Shuangzhan);

    General *shixiu = new General(this, "shixiu", "kou", 4);
    shixiu->addSkill(new Boming);

    General *xiezhen = new General(this, "xiezhen", "min");
    xiezhen->addSkill(new Xunlie);

    General *pangwanchun = new General(this, "pangwanchun", "jiang");
    pangwanchun->addSkill(new Shenjian);
    pangwanchun->addSkill(new Lianzhu);

    General *jiangjing = new General(this, "jiangjing", "jiang");
    jiangjing->addSkill(new Tiansuan);
    jiangjing->addSkill(new Huazhu);

    General *tongmeng = new General(this, "tongmeng", "min", 3);
    tongmeng->addSkill(new Shuilao);
    tongmeng->addSkill(new Skill("shuizhan", Skill::Compulsory));

    General *maling = new General(this, "maling", "jiang", 3);
    maling->addSkill(new Fengxing);

    General *daizong = new General(this, "daizong", "jiang", 3);
    daizong->addSkill(new Mitan);
    daizong->addSkill(new Jibao);

    General *caijing = new General(this, "caijing", "guan");
    caijing->addSkill(new Duoquan);

    General *lishishi = new General(this, "lishishi", "min", 3, false);
    lishishi->addSkill(new Qinxin);
    lishishi->addSkill(new Yinjian);

    General *yulan = new General(this, "yulan", "guan", 3, false);
    yulan->addSkill(new Qingdong);
    yulan->addSkill(new Qingshang);

    addMetaObject<LianmaCard>();
    addMetaObject<BomingCard>();
    addMetaObject<XunlieCard>();
    addMetaObject<LianzhuCard>();
    addMetaObject<HuazhuCard>();
    addMetaObject<YinjianCard>();
}

ADD_PACKAGE(FCDC);
