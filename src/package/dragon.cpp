#include "dragon.h"
#include "general.h"
#include "skill.h"
#include "carditem.h"
#include "engine.h"
#include "standard.h"
#include "client.h"
#include "maneuvering.h"
#include "plough.h"

class Xianxi: public TriggerSkill{
public:
    Xianxi():TriggerSkill("xianxi"){
        events << SlashMissed;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
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
        room->playSkillEffect(skill_name, 2);
        if(!player->isNude())
            room->askForDiscard(player, skill_name, 1, false, true);
        return;
    }
    room->playSkillEffect(skill_name, 1);
    PlayerStar from = targets.first();
    if(from->getCards("ej").isEmpty())
        return;

    int card_id = room->askForCardChosen(player, from , "ej", skill_name);
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
            if(!player->isProhibited(p, trick) && !p->containsTrick(trick->objectName(), false))
                tos << p;
        }
    }
    if(trick && trick->isVirtualCard())
        delete trick;

    room->setTag("TaolueTarget", QVariant::fromValue(from));
    ServerPlayer *to = room->askForPlayerChosen(player, tos, skill_name);
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

class Xiaofang: public TriggerSkill{
public:
    Xiaofang():TriggerSkill("xiaofang"){
        events << Predamaged << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *water = room->findPlayerBySkillName(objectName());
        if(!water || water->isKongcheng())
            return false;
        if(event == Predamaged){
            if(damage.nature == DamageStruct::Fire &&
               water->askForSkillInvoke(objectName()) &&
               room->askForDiscard(water, objectName(), 1)){
                LogMessage log;
                log.type = "#Xiaofan";
                log.from = water;
                log.arg = objectName();
                log.to << damage.to;
                room->sendLog(log);

                return true;
            }
        }
        else{
            if(damage.nature == DamageStruct::Thunder &&
               water->askForSkillInvoke(objectName()) &&
               room->askForDiscard(water, objectName(), 1)){
                ServerPlayer *forbider = damage.to;
                foreach(ServerPlayer *tmp, room->getOtherPlayers(water)){
                    if(tmp == forbider)
                        continue;
                    DamageStruct ailue;
                    ailue.from = water;
                    ailue.to = tmp;
                    ailue.nature = DamageStruct::Thunder;
                    room->damage(ailue);
                }
            }
        }
        return false;
    }
};

ShexinCard::ShexinCard(){
    once = true;
}

bool ShexinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void ShexinCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this, source);

    ServerPlayer *target = targets.value(0, source);
    CardEffectStruct effect;
    effect.card = this;
    effect.from = source;
    effect.to = target;

    room->cardEffect(effect);
}

void ShexinCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QList<int> cardes = effect.to->handCards();
    room->fillAG(cardes, effect.from);
    room->askForAG(effect.from, cardes, true, "shexin");
    QList<const Card *> cards = effect.to->getHandcards();
    foreach(const Card *card, cards){
        if(!card->inherits("BasicCard")){
            room->showCard(effect.to, card->getEffectiveId());
            room->getThread()->delay();
            room->throwCard(card, effect.to, effect.from);
        }else{
            room->showCard(effect.to, card->getEffectiveId());
            room->getThread()->delay();
        }
        effect.from->invoke("clearAG");
    }
}

class Shexin: public OneCardViewAsSkill{
public:
    Shexin():OneCardViewAsSkill("shexin"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ShexinCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("EquipCard") || to_select->getCard()->isNDTrick();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ShexinCard *card = new ShexinCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Wugou:public ViewAsSkill{
public:
    Wugou():ViewAsSkill("wugou"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty())
            return to_select->getCard()->inherits("BasicCard");
        else if(selected.length() == 1){
            const Card *card = selected.first()->getFilteredCard();
            return to_select->getCard()->inherits("BasicCard") && to_select->getFilteredCard()->isRed() == card->isRed();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 2){
            const Card *first = cards.first()->getCard();
            int secondnum = cards.last()->getCard()->getNumber();
            Assassinate *a = new Assassinate(first->getSuit(), qMin(13, first->getNumber() + secondnum));
            a->addSubcards(cards);
            a->setSkillName(objectName());
            return a;
        }else
            return NULL;
    }
};

class Qiaojiang:public OneCardViewAsSkill{
public:
    Qiaojiang():OneCardViewAsSkill("qiaojiang"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                // black trick as slash
                return card->inherits("TrickCard") && card->isBlack();
            }
        case Client::Responsing:{
                QString pattern = ClientInstance->getPattern();
                if(pattern == "slash")
                    return card->inherits("TrickCard") && card->isBlack();
                else if(pattern == "jink")
                    return card->inherits("TrickCard") && card->isRed();
            }
        default:
            return false;
        }
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "jink" || pattern == "slash";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        if(!card->inherits("TrickCard"))
            return NULL;
        if(card->isRed()){
            Jink *jink = new Jink(card->getSuit(), card->getNumber());
            jink->addSubcard(card);
            jink->setSkillName(objectName());
            return jink;
        }else{
            Slash *slash = new Slash(card->getSuit(), card->getNumber());
            slash->addSubcard(card);
            slash->setSkillName(objectName());
            return slash;
        }
    }
};

QianxianCard::QianxianCard(){
    once = true;
}

bool QianxianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select == Self)
        return false;
    if(targets.isEmpty())
        return true;
    if(targets.length() == 1){
        int max1 = targets.first()->getMaxHP();
        return to_select->getMaxHP() != max1;
    }
    return false;
}

bool QianxianCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(targets.length() != 2)
        return false;
    int max1 = targets.first()->getMaxHP();
    int max2 = targets.last()->getMaxHP();
    return max1 != max2;
}

void QianxianCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    const Card *club = room->askForCard(effect.to, ".C", "@qianxian:" + effect.from->objectName(), QVariant::fromValue(effect), NonTrigger);
    if(club){
        effect.from->obtainCard(club);
        if(!effect.to->faceUp())
            effect.to->turnOver();
        room->setPlayerProperty(effect.to, "chained", false);
    }
    else{
        if(effect.to->faceUp())
            effect.to->turnOver();
        room->setPlayerProperty(effect.to, "chained", true);
    }
}

class Qianxian: public OneCardViewAsSkill{
public:
    Qianxian():OneCardViewAsSkill("qianxian"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QianxianCard");
    }

    virtual bool viewFilter(const CardItem *blackfeiyanshitrick) const{
        const Card *card = blackfeiyanshitrick->getCard();
        return card->isBlack() && card->isNDTrick();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        QianxianCard *card = new QianxianCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

class Meicha: public OneCardViewAsSkill{
public:
    Meicha():OneCardViewAsSkill("meicha"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Analeptic::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("analeptic");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Analeptic *analeptic = new Analeptic(card->getSuit(), card->getNumber());
        analeptic->setSkillName(objectName());
        analeptic->addSubcard(card->getId());
        return analeptic;
    }
};

DragonPackage::DragonPackage()
    :GeneralPackage("dragon")
{
    General *qinming = new General(this, "qinming", "guan");
    qinming->addSkill(new Xianxi);

    General *hantao = new General(this, "hantao", "guan");
    hantao->addSkill(new Taolue);
    hantao->addSkill(new Changsheng);

    General *shantinggui = new General(this, "shantinggui", "jiang", 5, true, true);
    shantinggui->addSkill(new Xiaofang);

    General *yangchun = new General(this, "yangchun", "kou");
    yangchun->addSkill(new Shexin);

    General *zhengtianshou = new General(this, "zhengtianshou", "kou", 3);
    zhengtianshou->addSkill(new Wugou);
    zhengtianshou->addSkill(new Qiaojiang);

    General *wangpo = new General(this, "wangpo", "min", 3, false);
    wangpo->addSkill(new Qianxian);
    wangpo->addSkill(new Meicha);

    addMetaObject<TaolueCard>();
    addMetaObject<ShexinCard>();
    addMetaObject<QianxianCard>();
}

//ADD_PACKAGE(Dragon)
