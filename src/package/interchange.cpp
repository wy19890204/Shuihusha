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

ShensuanCard::ShensuanCard(){
}

bool ShensuanCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.length() < subcardsLength() && to_select->isWounded();
}

bool ShensuanCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() <= subcardsLength();
}

void ShensuanCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(targets.isEmpty()){
        QList<ServerPlayer *> to;
        to << source;
        SkillCard::use(room, source, to);
    }else
        SkillCard::use(room, source, targets);
}

void ShensuanCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    if(effect.to->isWounded()){
        RecoverStruct recover;
        recover.card = this;
        recover.who = effect.from;
        room->recover(effect.to, recover);
    }else
        effect.to->drawCards(2);
}

class ShensuanViewAsSkill: public ViewAsSkill{
public:
    ShensuanViewAsSkill():ViewAsSkill("shensuan"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@shensuan";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 3)
            return false;
        int sum = 0;
        foreach(CardItem *item, selected){
            sum += item->getCard()->getNumber();
        }
        sum += to_select->getCard()->getNumber();
        return sum <= Self->getMark("shensuan");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        int sum = 0;
        foreach(CardItem *item, cards){
            sum += item->getCard()->getNumber();
        }

        if(sum == Self->getMark("shensuan")){
            ShensuanCard *card = new ShensuanCard;
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }
};

class Shensuan: public MasochismSkill{
public:
    Shensuan():MasochismSkill("shensuan"){
        view_as_skill = new ShensuanViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *jiangjing, const DamageStruct &damage) const{
        const Card *card = damage.card;
        if(card == NULL)
            return;

        int point = card->getNumber();
        if(point == 0)
            return;

        if(jiangjing->isNude())
            return;

        Room *room = jiangjing->getRoom();
        room->setPlayerMark(jiangjing, objectName(), point);

        QString prompt = QString("@shensuan:::%1").arg(point);
        room->askForUseCard(jiangjing, "@@shensuan", prompt);
    }
};

class Gunzhu:public TriggerSkill{
public:
    Gunzhu():TriggerSkill("gunzhu"){
        frequency = Frequent;
        events << CardUsed << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *jiangjing, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->isNDTrick()){
            int num = card->getNumber();
            Room *room = jiangjing->getRoom();
            if(room->askForSkillInvoke(jiangjing, objectName())){
                room->playSkillEffect(objectName());
                JudgeStruct judge;
                judge.reason = objectName();
                judge.who = jiangjing;

                room->judge(judge);
                if(qAbs(judge.card->getNumber() - num) < 3)
                    jiangjing->drawCards(2);
            }
        }
        return false;
    }
};

class TouxiPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return card->inherits("Weapon") || card->inherits("Armor");
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
        const Card *card = caninvoke ? room->askForCard(duwei, ".Touxi", "@touxi:" + player->objectName(), data): NULL;
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
            target->gainAnExtraTurn();
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
            dujian->drawCards(3, false);
        }
        return false;
    }
};

class Luanji:public ViewAsSkill{
public:
    Luanji():ViewAsSkill("luanji"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty())
            return !to_select->isEquipped() && to_select->getCard()->isRed();
        else if(selected.length() == 1)
            return to_select->getCard()->inherits("TrickCard");
        else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 2){
            const Card *first = cards.first()->getCard();
            ArcheryAttack *aa = new ArcheryAttack(first->getSuit(), 0);
            aa->addSubcards(cards);
            aa->setSkillName(objectName());
            return aa;
        }else
            return NULL;
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
        if(c->getSuit() == Card::Heart && !card->inherits("EventsCard")){
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

InterChangePackage::InterChangePackage()
    :Package("interchange")
{
    General *qinming = new General(this, "qinming", "guan");
    qinming->addSkill(new Xianxi);

    General *caiqing = new General(this, "caiqing", "jiang", 5);
    caiqing->addSkill(new Lianzang);

    General *shenjiangjing = new General(this, "shenjiangjing", "god", 3);
    shenjiangjing->addSkill(new Shensuan);
    shenjiangjing->addSkill(new Gunzhu);

    General *duwei = new General(this, "duwei", "jiang");
    duwei->addSkill(new Touxi);
    patterns[".Touxi"] = new TouxiPattern;

    General *puwenying = new General(this, "puwenying", "guan", 3);
    puwenying->addSkill(new Guanxing);
    puwenying->addSkill(new Fanzhan);

    General *tongmeng = new General(this, "tongmeng", "min", 3);
    tongmeng->addSkill(new Shuilao);
    tongmeng->addSkill(new Skill("shuizhan", Skill::Compulsory));

    General *hongxin = new General(this, "hongxin", "guan");
    hongxin->addSkill(new Fangsheng);

    General *zhangmengfang = new General(this, "zhangmengfang", "guan");
    zhangmengfang->addSkill(new Tancai);

    General *pangwanchun = new General(this, "pangwanchun", "jiang");
    pangwanchun->addSkill(new Luanji);

    General *litianrun = new General(this, "litianrun", "jiang");
    litianrun->addSkill(new Jingtian);

    General *duxue = new General(this, "duxue", "kou");
    duxue->addSkill(new Wodao);

    addMetaObject<ShensuanCard>();
    addMetaObject<JingtianCard>();
}

ADD_PACKAGE(InterChange);
