#include "fcdc.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"

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

FCDCPackage::FCDCPackage()
    :Package("FCDC")
{
    General *xiezhen = new General(this, "xiezhen", "min");
    xiezhen->addSkill(new Xunlie);

    General *pangwanchun = new General(this, "pangwanchun", "jiang");
    pangwanchun->addSkill(new Shenjian);
    pangwanchun->addSkill(new Lianzhu);

    addMetaObject<XunlieCard>();
    addMetaObject<LianzhuCard>();
}

ADD_PACKAGE(FCDC);
