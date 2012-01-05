#include "zcyn.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "tocheck.h"

class Tongwu: public TriggerSkill{
public:
    Tongwu():TriggerSkill("tongwu"){
        events << SlashMissed;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *erge, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(!effect.to->isNude() && effect.jink){
            Room *room = erge->getRoom();
            if(erge->askForSkillInvoke(objectName(), data)){
                room->playSkillEffect(objectName());
                erge->obtainCard(effect.jink);
                ServerPlayer *target = room->askForPlayerChosen(erge, room->getOtherPlayers(effect.to), objectName());
                target->obtainCard(effect.jink);
            }
        }
        return false;
    }
};

#include "plough.h"
class Fuji:public PhaseChangeSkill{
public:
    Fuji():PhaseChangeSkill("fuji"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *p) const{
        if(p->getPhase() != Player::Judge || p->getJudgingArea().isEmpty())
            return false;
        Room *room = p->getRoom();
        ServerPlayer *ruan2 = room->findPlayerBySkillName(objectName());
        if(ruan2 && room->askForCard(ruan2, ".", "@fuji:" + p->objectName(), QVariant::fromValue(p))){
            Assassinate *ass = new Assassinate(Card::NoSuit, 2);
            ass->setSkillName(objectName());
            ass->setCancelable(false);
            CardUseStruct use;
            use.card = ass;
            use.from = ruan2;
            use.to << p;
            room->useCard(use);
        }
        return false;
    }
};

class Guizi: public TriggerSkill{
public:
    Guizi():TriggerSkill("guizi"){
        events << Dying;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *poolguy, QVariant &data) const{
        Room *room = poolguy->getRoom();
        ServerPlayer *bear = room->findPlayerBySkillName(objectName());
        DyingStruct dying = data.value<DyingStruct>();
        if(!bear || !dying.who)
            return false;
        if(dying.who == poolguy && room->askForCard(bear, "..S", "@guizi:" + poolguy->objectName(), data)){
            room->playSkillEffect(objectName());
            DamageStruct damage;
            damage.from = bear;

            LogMessage log;
            log.type = "#Guizi";
            log.from = bear;
            log.to << poolguy;
            log.arg = objectName();
            room->sendLog(log);

            room->getThread()->delay(1500);
            room->killPlayer(poolguy, &damage);
            return true;
        }
        return false;
    }
};

SixiangCard::SixiangCard(){
}

bool SixiangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int x = Self->getMark("Sixh");
    return targets.length() < x;
}

bool SixiangCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    int x = Self->getMark("Sixh");
    return targets.length() <= x && !targets.isEmpty();
}

void SixiangCard::onEffect(const CardEffectStruct &effect) const{
    int handcardnum = effect.to->getHandcardNum();
    int x = effect.from->getMark("Sixh");
    int delta = handcardnum - x;
    Room *room = effect.from->getRoom();
    if(delta > 0)
        room->askForDiscard(effect.to, "sixiang", delta);
    else
        effect.to->drawCards(qAbs(delta));
}

class SixiangViewAsSkill: public OneCardViewAsSkill{
public:
    SixiangViewAsSkill():OneCardViewAsSkill("sixiang"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@sixiang";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        SixiangCard *card = new SixiangCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Sixiang:public PhaseChangeSkill{
public:
    Sixiang():PhaseChangeSkill("sixiang"){
        view_as_skill = new SixiangViewAsSkill;
    }

    int getKingdoms(Room *room) const{
        QSet<QString> kingdom_set;
        foreach(ServerPlayer *tmp, room->getAlivePlayers())
            kingdom_set << tmp->getKingdom();
        return kingdom_set.size();
    }

    virtual bool onPhaseChange(ServerPlayer *jingmuan) const{
        Room *room = jingmuan->getRoom();
        if(jingmuan->getPhase() == Player::Start && !jingmuan->isKongcheng()){
            int x = getKingdoms(room);
            room->setPlayerMark(jingmuan, "Sixh", x);
            if(room->askForUseCard(jingmuan, "@@sixiang", "@sixiang"))
                jingmuan->setFlags("elephant");
        }
        else if(jingmuan->getPhase() == Player::Discard && jingmuan->hasFlag("elephant")){
            int x = getKingdoms(room);
            int total = jingmuan->getEquips().length() + jingmuan->getHandcardNum();

            LogMessage log;
            log.from = jingmuan;
            log.arg2 = objectName();
            if(total <= x){
                jingmuan->throwAllHandCards();
                jingmuan->throwAllEquips();
                log.type = "#SixiangWorst";
                log.arg = QString::number(total);
                room->sendLog(log);
            }else{
                room->askForDiscard(jingmuan, objectName(), x, false, true);
                log.type = "#SixiangBad";
                log.arg = QString::number(x);
                room->sendLog(log);
            }
        }
        return false;
    }
};

class Tianyan: public PhaseChangeSkill{
public:
    Tianyan():PhaseChangeSkill("tianyan"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Judge || player->getHandcardNum() < 3)
            return false;
        Room *room = player->getRoom();
        ServerPlayer *tianqi = room->findPlayerBySkillName(objectName());
        if(tianqi && tianqi->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());

            QList<int> cards = room->getNCards(3);
            if(!cards.isEmpty()){
                room->fillAG(cards, tianqi);
                while(!cards.isEmpty()){
                    int card_id = room->askForAG(tianqi, cards, true, objectName());
                    if(card_id == -1)
                        break;
                    if(!cards.contains(card_id))
                        continue;
                    cards.removeOne(card_id);
                    room->throwCard(card_id);
                    room->takeAG(NULL, card_id);

                    LogMessage log;
                    log.from = tianqi;
                    log.type = "$DiscardCard";
                    log.card_str = QString::number(card_id);
                    room->sendLog(log);
                }
                for(int i = cards.length() - 1; i >= 0; i--){
                    room->throwCard(cards.at(i));
                    const Card *tmp = Sanguosha->getCard(cards.at(i));
                    room->moveCardTo(tmp, NULL, Player::DrawPile);
                }
                tianqi->invoke("clearAG");
            }
        }
        return false;
    }
};

class Dujian: public TriggerSkill{
public:
    Dujian():TriggerSkill("dujian"){
        events << Predamage;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to == damage.from || damage.damage < 1 || !damage.card->inherits("Slash"))
            return false;
        if(!damage.to->isNude() && !damage.to->inMyAttackRange(player)
            && player->askForSkillInvoke(objectName(), data)){
            player->getRoom()->playSkillEffect(objectName());
            damage.to->turnOver();
            return true;
        }
        return false;
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
        bs->addSubcard(card_item->getCard());
        return bs;
    }
};

class Longjiao:public TriggerSkill{
public:
    Longjiao():TriggerSkill("longjiao"){
        events << CardEffected;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zou, QVariant &data) const{
        Room *room = zou->getRoom();
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!effect.card->isNDTrick())
            return false;

        if(room->askForSkillInvoke(zou, objectName(), data)){
            QList<int> card_ids = room->getNCards(2);
            room->obtainCard(zou, card_ids.first());
            room->obtainCard(zou, card_ids.last());
            QStringList c;
            c << Sanguosha->getCard(card_ids.first())->getEffectIdString();
            c << Sanguosha->getCard(card_ids.last())->getEffectIdString();
            if(room->askForChoice(zou, objectName(), c.join("+")) == c.at(0)){

            }
        }
        return false;
    }
};

class Juesi: public TriggerSkill{
public:
    Juesi():TriggerSkill("juesi"){
        events << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *caifu, QVariant &data) const{
        Room *room = caifu->getRoom();
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.to->isAlive() && damage.to->getHp() <= 1){
            LogMessage log;
            log.type = "#JuesiBuff";
            log.from = caifu;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            room->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

ZCYNPackage::ZCYNPackage()
    :Package("ZCYN")
{
    General *guansheng = new General(this, "guansheng", "jiang");
    guansheng->addSkill(new Tongwu);

    General *ruanxiaoer = new General(this, "ruanxiaoer", "min");
    ruanxiaoer->addSkill(new Fuji);

    General *yangxiong = new General(this, "yangxiong", "jiang");
    yangxiong->addSkill(new Guizi);

    General *haosiwen = new General(this, "haosiwen", "guan");
    haosiwen->addSkill(new Sixiang);

    General *pengqi = new General(this, "pengqi", "guan");
    pengqi->addSkill(new Tianyan);

    General *shiwengong = new General(this, "shiwengong", "jiang");
    shiwengong->addSkill(new Dujian);

    General *lingzhen = new General(this, "lingzhen", "jiang");
    lingzhen->addSkill(new Paohong);

    General *zourun = new General(this, "zourun", "min");
    zourun->addSkill(new Longjiao);

    General *caifu = new General(this, "caifu", "jiang");
    caifu->addSkill(new Juesi);

    addMetaObject<SixiangCard>();
}

ADD_PACKAGE(ZCYN);
