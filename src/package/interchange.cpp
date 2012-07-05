#include "standard.h"
#include "skill.h"
#include "interchange.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"
#include "plough.h"

class Xianxi: public TriggerSkill{
public:
    Xianxi():TriggerSkill("xianxi"){
        events << SlashMissed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        int jink = effect.jink->getEffectiveId();
        if(!Sanguosha->getCard(jink)->inherits("Jink"))
            return false;
        LogMessage log;
        log.from = player;
        log.type = "#Xianxi";
        log.arg = objectName();
        room->sendLog(log);
        if(player->getCardCount(true) >= 2){
            if(!room->askForDiscard(player, objectName(), 2, true, true))
                room->loseHp(player);
        }
        else
            room->loseHp(player);
        return false;
    }
};

class Lianzang: public TriggerSkill{
public:
    Lianzang():TriggerSkill("lianzang"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *tenkei = room->findPlayerBySkillName(objectName());
        if(!tenkei)
            return false;
        QVariantList eatdeath_skills = tenkei->tag["EatDeath"].toList();
        if(room->askForSkillInvoke(tenkei, objectName(), data)){
            QStringList eatdeaths;
            foreach(QVariant tmp, eatdeath_skills)
                eatdeaths << tmp.toString();
            if(!eatdeaths.isEmpty()){
                QString choice = room->askForChoice(tenkei, objectName(), eatdeaths.join("+"));
                room->detachSkillFromPlayer(tenkei, choice);
                eatdeath_skills.removeOne(choice);
            }
            room->loseMaxHp(tenkei);
            QList<const Skill *> skills = player->getVisibleSkillList();
            foreach(const Skill *skill, skills){
                if(skill->parent() &&
                   skill->getFrequency() != Skill::Limited &&
                   skill->getFrequency() != Skill::Wake &&
                   !skill->isLordSkill()){
                    QString sk = skill->objectName();
                    room->acquireSkill(tenkei, sk);
                    eatdeath_skills << sk;
                }
            }
            tenkei->tag["EatDeath"] = eatdeath_skills;
        }

        return false;
    }
};

class Touxi: public TriggerSkill{
public:
    Touxi():TriggerSkill("touxi"){
        events << CardResponsed;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardStar card_star = data.value<CardStar>();
        if(!card_star->inherits("Jink"))
            return false;
        Room *room = player->getRoom();
        ServerPlayer *duwei = room->findPlayerBySkillName(objectName());
        if(!duwei)
            return false;
        bool caninvoke = false;
        foreach(const Card *tp, duwei->getCards("he")){
            if(tp->inherits("Weapon") || tp->inherits("Armor")){
                caninvoke = true;
                break;
            }
        }
        const Card *card = caninvoke ? room->askForCard(duwei, "Weapon,Armor", "@touxi:" + player->objectName(), data): NULL;
        if(card){
            Assassinate *ass = new Assassinate(card->getSuit(), card->getNumber());
            ass->setSkillName(objectName());
            ass->addSubcard(card);
            CardUseStruct use;
            use.card = ass;
            use.from = duwei;
            use.to << player;
            room->useCard(use);
        }
        return false;
    }
};

class Guanxing:public PhaseChangeSkill{
public:
    Guanxing():PhaseChangeSkill("guanxing"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *zhuge) const{
        if(zhuge->getPhase() == Player::Start &&
           zhuge->askForSkillInvoke(objectName()))
        {
            Room *room = zhuge->getRoom();
            room->playSkillEffect(objectName());

            int n = qMin(5, room->alivePlayerCount());
            room->doGuanxing(zhuge, room->getNCards(n, false), false);
        }

        return false;
    }
};

class Fanzhan:public PhaseChangeSkill{
public:
    Fanzhan():PhaseChangeSkill("fanzhan"){
    }

    virtual bool onPhaseChange(ServerPlayer *fuji) const{
        Room *room = fuji->getRoom();
        if(fuji->getPhase() == Player::Start){
            room->setTag("Fanzhan", false);
            return false;
        }
        if(fuji->getPhase() != Player::NotActive)
            return false;
        if(fuji->isWounded() || room->getAlivePlayers().length() <= 2)
            return false;
        if(fuji->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());
            room->setTag("Fanzhan", true);
        }
        return false;
    }
};

class FanzhanClear: public TriggerSkill{
public:
    FanzhanClear():TriggerSkill("#fanzhan-clear"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        player->getRoom()->removeTag("Fanzhan");
        return false;
    }
};

class Fangsheng:public PhaseChangeSkill{
public:
    Fangsheng():PhaseChangeSkill("fangsheng"){
    }

    virtual bool onPhaseChange(ServerPlayer *taiwei) const{
        if(taiwei->getPhase() == Player::Start && taiwei->getHandcardNum() > taiwei->getHp() &&
           taiwei->askForSkillInvoke(objectName())){
            Room *room = taiwei->getRoom();
            taiwei->drawCards(2, false);
            room->playSkillEffect(objectName());

            PlayerStar target = room->askForPlayerChosen(taiwei, room->getOtherPlayers(taiwei), objectName());
            target->gainAnExtraTurn(taiwei);
            taiwei->skip();
        }
        return false;
    }
};

class Tancai:public PhaseChangeSkill{
public:
    Tancai():PhaseChangeSkill("tancai"){
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool onPhaseChange(ServerPlayer *dujian) const{
        if(dujian->getPhase() == Player::Discard &&
           dujian->askForSkillInvoke(objectName())){
            dujian->getRoom()->playSkillEffect(objectName());
            dujian->drawCards(dujian->getLostHp() + 1, false);
        }
        return false;
    }
};

JingtianCard::JingtianCard(){
}

bool JingtianCard::targetFilter(const QList<const Player *> &targets, const Player *t, const Player *Self) const{
    return targets.isEmpty() && t != Self;
}

void JingtianCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    LogMessage log;
    log.type = "#Jingtian";
    log.from = effect.from;
    log.to << effect.to;
    log.arg = QString::number(effect.to->getHp());
    log.arg2 = QString::number(effect.from->getHp());

    room->sendLog(log);
    room->setPlayerProperty(effect.to, "hp", effect.from->getHp());
}

class Jingtian:public ViewAsSkill{
public:
    Jingtian():ViewAsSkill("jingtian"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty())
            return !to_select->isEquipped();
        else if(selected.length() == 1){
            const Card *card = selected.first()->getFilteredCard();
            return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == card->getSuit();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;
        JingtianCard *card = new JingtianCard;
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("JingtianCard");
    }
};

class Wodao: public FilterSkill{
public:
    Wodao():FilterSkill("wodao"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getCard();
        return (card->getSuit() == Card::Spade && card->inherits("TrickCard")) ||
                card->inherits("EventsCard") ||
                card->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *c = card_item->getCard();
        if(c->getSuit() == Card::Heart && !c->inherits("EventsCard")){
            Slash *slash = new Slash(c->getSuit(), c->getNumber());
            slash->setSkillName(objectName());
            slash->addSubcard(card_item->getCard());
            return slash;
        }
        else{
            Duel *duel = new Duel(c->getSuit(), c->getNumber());
            duel->setSkillName(objectName());
            duel->addSubcard(card_item->getCard());
            return duel;
        }
    }
};

class Tongmou: public SlashBuffSkill{
public:
    Tongmou():SlashBuffSkill("tongmou"){
    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *fuan = effect.from;
        Room *room = fuan->getRoom();
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *tmp, room->getOtherPlayers(fuan)){
            if(tmp->inMyAttackRange(effect.to))
                targets << tmp;
        }
        if(targets.isEmpty() || !fuan->askForSkillInvoke(objectName()))
            return false;
        ServerPlayer *target = room->askForPlayerChosen(fuan, targets, objectName());
        if(room->askForCard(target, "slash", "@tongmou:" + fuan->objectName())){
            room->playSkillEffect(objectName());
            LogMessage log;
            log.type = "#Tongmou";
            log.from = effect.to;
            log.to << fuan << target;
            log.arg = objectName();
            room->sendLog(log);

            room->slashResult(effect, NULL);
            return true;
        }
        return false;
    }
};

XianhaiCard::XianhaiCard(){
}

bool XianhaiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void XianhaiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    const Card *c = Sanguosha->getCard(getSubcards().first());
    Drivolt *drive = new Drivolt(c->getSuit(), c->getNumber());
    drive->setSkillName("xianhai");
    drive->addSubcard(c);
    CardUseStruct use;
    use.card = drive;
    use.from = card_use.from;
    use.to = card_use.to;
    room->useCard(use);
}

class XianhaiViewAsSkill: public OneCardViewAsSkill{
public:
    XianhaiViewAsSkill():OneCardViewAsSkill("xianhai"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@xianhai";
    }

    virtual bool viewFilter(const CardItem *n) const{
        return !n->isEquipped() && n->getCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        XianhaiCard *card = new XianhaiCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

class Xianhai: public MasochismSkill{
public:
    Xianhai():MasochismSkill("xianhai"){
        view_as_skill = new XianhaiViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *an, const DamageStruct &damage) const{
        Room *room = an->getRoom();
        room->askForUseCard(an, "@@xianhai", "@xianhai");
    }
};

class Shenyong:public TriggerSkill{
public:
    Shenyong():TriggerSkill("shenyong"){
        events << CardAsked;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *shibao, QVariant &data) const{
        QString asked = data.toString();
        if(asked == "jink" && shibao->askForSkillInvoke(objectName())){
            Room *room = shibao->getRoom();
            if(room->askForUseCard(shibao, "slash", "@askforslash")){
                Jink *jink = new Jink(Card::NoSuit, 0);
                jink->setSkillName(objectName());
                room->provide(jink);
                room->setEmotion(shibao, "good");
                return true;
            }
        }
        return false;
    }
};

class Xumou: public OneCardViewAsSkill{
public:
    Xumou():OneCardViewAsSkill("xumou"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return !to_select->isEquipped() && card->isBlack()
                && card->getNumber() >= 6 && card->getNumber() <= 10;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Collateral *oll = new Collateral(first->getSuit(), first->getNumber());
        oll->addSubcard(first->getId());
        oll->setSkillName(objectName());
        return oll;
    }
};

class Yuli: public MasochismSkill{
public:
    Yuli():MasochismSkill("yuli"){
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual void onDamaged(ServerPlayer *other, const DamageStruct &damage) const{
        Room *room = other->getRoom();
        ServerPlayer *murong = room->getCurrent();
        if(murong->hasSkill(objectName()) && murong->askForSkillInvoke(objectName()))
            murong->drawCards(damage.damage);
    }
};

class Youxia: public TriggerSkill{
public:
    Youxia():TriggerSkill("youxia"){
        events << CardLost;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *jinge = room->findPlayerBySkillName(objectName());
        if(player->isKongcheng() && jinge && !jinge->isKongcheng() && jinge->isWounded()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(player->isAlive() && move->from_place == Player::Hand && jinge->askForSkillInvoke(objectName(), data)){
                const Card *card = room->askForCardShow(jinge, player, "youxia");
                player->obtainCard(card, false);
                RecoverStruct o;
                o.card = card;
                room->recover(jinge, o);
            }
        }
        return false;
    }
};

InterChangePackage::InterChangePackage()
    :Package("interchange")
{
    General *qinming = new General(this, "qinming", "guan");
    qinming->addSkill(new Xianxi);

    General *caiqing = new General(this, "caiqing", "jiang", 5);
    caiqing->addSkill(new Lianzang);

    General *duwei = new General(this, "duwei", "jiang");
    duwei->addSkill(new Touxi);

    General *puwenying = new General(this, "puwenying", "guan", 3);
    puwenying->addSkill(new Guanxing);
    puwenying->addSkill(new Fanzhan);
    puwenying->addSkill(new FanzhanClear);
    related_skills.insertMulti("fanzhan", "#fanzhan-clear");

    General *hongxin = new General(this, "hongxin", "guan");
    hongxin->addSkill(new Fangsheng);

    General *zhangmengfang = new General(this, "zhangmengfang", "guan");
    zhangmengfang->addSkill(new Tancai);

    General *litianrun = new General(this, "litianrun", "jiang");
    litianrun->addSkill(new Jingtian);

    General *duxue = new General(this, "duxue", "kou");
    duxue->addSkill(new Wodao);

    General *fuan = new General(this, "fuan", "guan", 3);
    fuan->addSkill(new Tongmou);
    fuan->addSkill(new Xianhai);

    General *shibao = new General(this, "shibao", "jiang");
    shibao->addSkill(new Shenyong);

    General *murongyanda = new General(this, "murongyanda", "guan", 4);
    murongyanda->addSkill(new Xumou);
    murongyanda->addSkill(new Yuli);
    murongyanda->addSkill("#losthp");

    General *ximenjinge = new General(this, "ximenjinge", "jiang");
    ximenjinge->addSkill(new Youxia);

    addMetaObject<JingtianCard>();
    addMetaObject<XianhaiCard>();
}

ADD_PACKAGE(InterChange);
