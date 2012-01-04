#include "zcyn-package.h"
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

class Lianhuan: public OneCardViewAsSkill{
public:
    Lianhuan():OneCardViewAsSkill("lianhuan"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        IronChain *chain = new IronChain(card->getSuit(), card->getNumber());
        chain->addSubcard(card);
        chain->setSkillName(objectName());
        return chain;
    }
};

class Niepan: public TriggerSkill{
public:
    Niepan():TriggerSkill("niepan"){
        events << AskForPeaches;
        frequency = Limited;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && target->getMark("@nirvana") > 0;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *pangtong, QVariant &data) const{
        DyingStruct dying_data = data.value<DyingStruct>();
        if(dying_data.who != pangtong)
            return false;

        Room *room = pangtong->getRoom();
        if(pangtong->askForSkillInvoke(objectName(), data)){
            room->broadcastInvoke("animate", "lightbox:$niepan");
            room->playSkillEffect(objectName());

            pangtong->loseMark("@nirvana");

            room->setPlayerProperty(pangtong, "hp", qMin(3, pangtong->getMaxHP()));
            pangtong->throwAllCards();
            pangtong->drawCards(3);

            if(pangtong->isChained()){
                if(dying_data.damage == NULL || dying_data.damage->nature == DamageStruct::Normal)
                    room->setPlayerProperty(pangtong, "chained", false);
            }
            if(!pangtong->faceUp())
                pangtong->turnOver();
        }

        return false;
    }
};

class Huoji: public OneCardViewAsSkill{
public:
    Huoji():OneCardViewAsSkill("huoji"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        FireAttack *fire_attack = new FireAttack(card->getSuit(), card->getNumber());
        fire_attack->addSubcard(card->getId());
        fire_attack->setSkillName(objectName());
        return fire_attack;
    }
};

class Bazhen: public TriggerSkill{
public:
    Bazhen():TriggerSkill("bazhen"){
        frequency = Compulsory;
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->getArmor() && target->getMark("qinggang") == 0 && target->getMark("wuqian") == 0;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *wolong, QVariant &data) const{
        QString pattern = data.toString();

        if(pattern != "jink")
            return false;

        Room *room = wolong->getRoom();
        if(wolong->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = wolong;

            room->judge(judge);

            if(judge.isGood()){
                Jink *jink = new Jink(Card::NoSuit, 0);
                jink->setSkillName(objectName());
                room->provide(jink);
                room->setEmotion(wolong, "good");
                return true;
            }else
                room->setEmotion(wolong, "bad");
        }
        return false;
    }
};

ZCYNPackage::ZCYNPackage()
    :Package("ZCYN")
{
    General *guansheng = new General(this, "guansheng", "jiang");
    guansheng->addSkill(new Tongwu);

    General *haosiwen = new General(this, "haosiwen", "guan");
    haosiwen->addSkill(new Sixiang);

    General *pengqi = new General(this, "pengqi", "guan");
    pengqi->addSkill(new Tianyan);
/*
    yuanshao = new General(this, "yuanshao$", "kou");
    yuanshao->addSkill(new Luanji);

    shuangxiong = new General(this, "shuangxiong", "kou");

    pangde = new General(this, "pangde", "kou");
    pangde->addSkill(new Mengjin);
*/
    addMetaObject<SixiangCard>();
}

ADD_PACKAGE(ZCYN);
