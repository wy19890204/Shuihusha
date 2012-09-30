#include "hare.h"
#include "general.h"
#include "skill.h"
#include "room.h"
#include "carditem.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "engine.h"

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

    virtual bool onPhaseChange(ServerPlayer *jingmuan) const{
        Room *room = jingmuan->getRoom();
        if(jingmuan->getPhase() == Player::Start && !jingmuan->isKongcheng()){
            room->setPlayerMark(jingmuan, "Sixh", room->getKingdoms());
            if(room->askForUseCard(jingmuan, "@@sixiang", "@sixiang", true))
                jingmuan->setFlags("elephant");
        }
        else if(jingmuan->getPhase() == Player::Discard && jingmuan->hasFlag("elephant")){
            int x = room->getKingdoms();
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

class Fenhui: public TriggerSkill{
public:
    Fenhui():TriggerSkill("fenhui"){
        frequency = Compulsory;
        events << Predamage << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        LogMessage log;
        log.from = player;
        log.arg2 = objectName();

        if(event == Predamage){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature != DamageStruct::Fire){
                damage.nature = DamageStruct::Fire;

                log.type = "#FenhuiFire";
                log.arg = QString::number(damage.damage);
                room->sendLog(log);
                room->playSkillEffect(objectName(), qrand() % 2 + 1);

                data = QVariant::fromValue(damage);
                return false;
            }
       }else if(event == Predamaged){
           DamageStruct damage = data.value<DamageStruct>();
           if(damage.nature == DamageStruct::Fire){
               Room *room = player->getRoom();
               log.type = "#FenhuiProtect";
               log.arg = QString::number(damage.damage);
               room->sendLog(log);

               room->playSkillEffect(objectName(), qrand() % 2 + 3);
               return true;
           }else
               return false;
       }
       return false;
    }
};

class ShenhuoViewAsSkill: public OneCardViewAsSkill{
public:
    ShenhuoViewAsSkill():OneCardViewAsSkill("shenhuo"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->isRed() && card->inherits("TrickCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        FireAttack *fire_attack = new FireAttack(card->getSuit(), card->getNumber());
        fire_attack->addSubcard(card->getId());
        fire_attack->setSkillName(objectName());
        return fire_attack;
    }
};

class Shenhuo:public TriggerSkill{
public:
    Shenhuo():TriggerSkill("shenhuo"){
        view_as_skill = new ShenhuoViewAsSkill;
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *weidingguo, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        CardStar card = use.card;
        if(card->inherits("FireAttack")){
            if(room->askForSkillInvoke(weidingguo, objectName())){
                if(card->getSkillName() != "shenhuo")
                    room->playSkillEffect(objectName());
                weidingguo->drawCards(2);
            }
        }
        return false;
    }
};

LinmoCard::LinmoCard(){
    target_fixed = true;
}

void LinmoCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *xiao = card_use.from;
    QList<int> card_ids = xiao->getPile("zi");
    room->fillAG(card_ids, xiao);
    int zid = room->askForAG(xiao, card_ids, false, objectName());
    QString zi = Sanguosha->getCard(zid)->objectName();
    card_ids.removeOne(zid);
    xiao->invoke("clearAG");

    room->setPlayerProperty(xiao, "linmostore", zi);
}

class LinmoViewAsSkill:public ViewAsSkill{
public:
    LinmoViewAsSkill():ViewAsSkill("linmo"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(Self->hasUsed("LinmoCard") && selected.isEmpty() && !Self->hasFlag("linmo")){
            return !to_select->isEquipped();
        }else
            return false;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->getPile("zi").isEmpty())
            return false;
        if(player->hasUsed("LinmoCard") && !player->hasFlag("linmo")){
            QString name = Self->property("linmostore").toString();
            Card *card = Sanguosha->cloneCard(name, Card::NoSuit, 0);
            return card->isAvailable(player);
        }else if(player->hasFlag("linmo"))
            return false;
        else
            return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(Self->hasUsed("LinmoCard")){
            if(Self->hasFlag("linmo"))
                return false;
            if(cards.length() != 1)
                return NULL;
            const Card *card = cards.first()->getCard();
            QString name = Self->property("linmostore").toString();
            Card *new_card = Sanguosha->cloneCard(name, card->getSuit(), card->getNumber());
            new_card->addSubcard(card);
            new_card->setSkillName("linmo");
            Self->setFlags("linmo");
            return new_card;
        }else{
            return new LinmoCard;
        }
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if(player->getPhase() == Player::NotActive)
            return false;
        if(player->hasFlag("linmo"))
            return false;
        if(player->hasUsed("LinmoCard")){
            QString name = Self->property("linmostore").toString();
            Card *card = Sanguosha->cloneCard(name, Card::NoSuit, 0);
            return pattern.contains(card->objectName());
        }else
            return false;
    }
};

class Linmo: public TriggerSkill{
public:
    Linmo():TriggerSkill("linmo"){
        view_as_skill = new LinmoViewAsSkill;
        events << CardFinished << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *writer = room->findPlayerBySkillName(objectName());
        if(!writer)
            return false;
        if(writer == player){
            if(event == PhaseChange){
                if(player->getPhase() != Player::NotActive)
                    return false;
                player->property("linmostore") = "";
                foreach(int a, player->getPile("zi"))
                    room->throwCard(a);
            }
            return false;
        }
        if(event != CardFinished)
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->isVirtualCard())
            return false;
        const Card *word = Sanguosha->getCard(use.card->getEffectiveId());
        if(use.to.contains(writer) && (word->inherits("BasicCard") || word->isNDTrick())
            && room->getCardPlace(use.card->getEffectiveId()) == Player::DiscardedPile){
            bool hassamezi = false;
            foreach(int x, writer->getPile("zi")){
                if(Sanguosha->getCard(x)->objectName() == word->objectName()){
                    hassamezi = true;
                    break;
                }
            }
            if(!hassamezi && writer->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());
                writer->addToPile("zi", use.card->getEffectiveId());
            }
        }
        return false;
    }
};

ZhaixingCard::ZhaixingCard(){
    target_fixed = true;
    will_throw = false;
    can_jilei = true;
}

void ZhaixingCard::use(Room *room, ServerPlayer *zhangjiao, const QList<ServerPlayer *> &targets) const{

}

class ZhaixingViewAsSkill:public OneCardViewAsSkill{
public:
    ZhaixingViewAsSkill():OneCardViewAsSkill(""){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@zhaixing";
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ZhaixingCard *card = new ZhaixingCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Zhaixing: public TriggerSkill{
public:
    Zhaixing():TriggerSkill("zhaixing"){
        view_as_skill = new ZhaixingViewAsSkill;
        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        if(!TriggerSkill::triggerable(target))
            return false;
        return !target->isNude();
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        if(judge->card->getSuit() != Card::Diamond || player->isNude())
            return false;

        QStringList prompt_list;
        prompt_list << "@zhaixing-card" << judge->who->objectName()
                << "" << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");

        player->tag["Judge"] = data;
        const Card *card = room->askForCard(player, "@zhaixing", prompt, true, data);

        if(card){
            player->obtainCard(judge->card);
            player->drawCards(1);
            judge->card = Sanguosha->getCard(card->getEffectiveId());
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = player;
            log.to << judge->who;
            log.card_str = card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);
        }
        return false;
    }
};

class Shenpan: public TriggerSkill{
public:
    Shenpan():TriggerSkill("shenpan"){
        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();

        if(player->askForSkillInvoke(objectName(), data)){
            player->obtainCard(judge->card);
            room->playSkillEffect(objectName());
            int card_id = room->drawCard();
            room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special, true);
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
            return true;
        }
        return false;
    }
};

BinggongCard::BinggongCard(){
    will_throw = false;
}

bool BinggongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select != Self;
}

void BinggongCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    int num = this->getSubcards().length();
    ServerPlayer *target = targets.first();
    target->obtainCard(this, false);
    if(num >= 3){
        RecoverStruct rev;
        rev.who = source;
        room->recover(source, rev, true);
    }
}

class BinggongViewAsSkill: public ViewAsSkill{
public:
    BinggongViewAsSkill():ViewAsSkill("binggong"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        int num = Self->getMark("Bingo");
        return !to_select->isEquipped() && selected.length() < num;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != Self->getMark("Bingo"))
            return NULL;

        BinggongCard *card = new BinggongCard();
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@binggong";
    }
};

class Binggong: public PhaseChangeSkill{
public:
    Binggong():PhaseChangeSkill("binggong"){
        view_as_skill = new BinggongViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *pei) const{
        Room *room = pei->getRoom();
        if(pei->getPhase() == Player::Start && pei->getHandcardNum() > pei->getHp()){
            int num = pei->getHandcardNum() - pei->getHp();
            room->setPlayerMark(pei, "Bingo", num);
            room->askForUseCard(pei, "@@binggong", "@binggong", true);
        }
        room->setPlayerMark(pei, "Bingo", 0);
        return false;
    }
};

class Hengchong: public TriggerSkill{
public:
    Hengchong():TriggerSkill("hengchong"){
        events << SlashMissed;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->getWeapon())
            return false;
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        QString suit_str = effect.slash->getSuitString();
        QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
        QString prompt = QString("@hengchong:%1::%2").arg(effect.to->getGeneralName()).arg(suit_str);
        CardStar card = room->askForCard(player, pattern, prompt, true, data, CardDiscarded);
        if(card){
            room->playSkillEffect(objectName());
            room->slashResult(effect, NULL);
            ServerPlayer *target = room->askForPlayerChosen(player, room->getNextandPrevious(effect.to), objectName());
            DamageStruct damage;
            damage.from = player;
            damage.to = target;
            room->damage(damage);

            LogMessage log;
            log.type = "#Hengchong";
            log.from = player;
            log.to << effect.to << target;
            log.arg = objectName();
            log.arg2 = card->objectName();
            room->sendLog(log);

            return true;
        }
        return false;
    }
};

FeiqiangCard::FeiqiangCard(){
    once = true;
}

bool FeiqiangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void FeiqiangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if(!room->askForCard(effect.to, "Jink", "@feiqiang:" + effect.from->objectName(), false, QVariant::fromValue(effect), CardDiscarded)){
        QString choice = effect.to->getCards("e").isEmpty() ? "gong"
            : room->askForChoice(effect.from, "feiqiang", "gong+wang");
        if(choice == "gong")
            room->loseHp(effect.to);
        else
            effect.to->throwAllEquips();
    }
}

class Feiqiang:public OneCardViewAsSkill{
public:
    Feiqiang():OneCardViewAsSkill("feiqiang"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("FeiqiangCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Weapon");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new FeiqiangCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

HarePackage::HarePackage()
    :Package("hare")
{
    General *haosiwen = new General(this, "haosiwen", "guan");
    haosiwen->addSkill(new Sixiang);

    General *weidingguo = new General(this, "weidingguo", "jiang", 3);
    weidingguo->addSkill(new Fenhui);
    weidingguo->addSkill(new Shenhuo);

    General *xiaorang = new General(this, "xiaorang", "min", 3);
    xiaorang->addSkill(new Linmo);
    xiaorang->addSkill(new Zhaixing);

    General *peixuan = new General(this, "peixuan", "guan", 3);
    peixuan->addSkill(new Shenpan);
    peixuan->addSkill(new Binggong);

    General *ligun = new General(this, "ligun", "jiang");
    ligun->addSkill(new Hengchong);

    General *gongwang = new General(this, "gongwang", "jiang");
    gongwang->addSkill(new Feiqiang);

    addMetaObject<SixiangCard>();
    addMetaObject<LinmoCard>();
    addMetaObject<ZhaixingCard>();
    addMetaObject<BinggongCard>();
    addMetaObject<FeiqiangCard>();
}

ADD_PACKAGE(Hare)
