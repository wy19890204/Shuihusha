#include "cgdk.h"
#include "standard.h"
#include "skill.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "plough.h"

class Liehuo: public TriggerSkill{
public:
    Liehuo():TriggerSkill("liehuo"){
        events << SlashMissed << Damage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *bao, QVariant &data) const{
        Room *room = bao->getRoom();
        PlayerStar target;
        if(event == SlashMissed){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            target = effect.to;
        }
        else{
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.to && damage.card->inherits("Slash"))
                target = damage.to;
            else
                return false;
        }
        if(target && !target->isKongcheng() &&
           target->getHandcardNum() > bao->getHandcardNum() &&
           room->askForSkillInvoke(bao, objectName(), QVariant::fromValue(target))){
            room->playSkillEffect(objectName());
            bao->obtainCard(target->getRandomHandCard(), false);
        }
        return false;
    }
};

class Citan: public PhaseChangeSkill{
public:
    Citan():PhaseChangeSkill("citan"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        ServerPlayer *yanglin = room->findPlayerBySkillName(objectName());
        if(!yanglin)
            return false;
        if(player->getPhase() == Player::Discard)
            player->setMark("Cit", player->getHandcardNum());
        else if(player->getPhase() == Player::Finish){
            int old = player->getMark("Cit");
            if(old - player->getHandcardNum() >= 2 &&
               yanglin->askForSkillInvoke(objectName(), QVariant::fromValue((PlayerStar)player))){
                room->playSkillEffect(objectName());
                QList<int> card_ids = player->handCards();
                room->fillAG(card_ids, yanglin);
                int to_move = room->askForAG(yanglin, card_ids, true, objectName());
                if(to_move > -1){
                    ServerPlayer *target = room->askForPlayerChosen(yanglin, room->getOtherPlayers(player), objectName());
                    room->obtainCard(target, to_move, false);
                    card_ids.removeOne(to_move);
                }
                yanglin->invoke("clearAG");
            }
        }
        return false;
    }
};

BingjiCard::BingjiCard(){
    mute = true;
}

bool BingjiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int x = qMax(1, Self->getLostHp());
    return targets.length() < x;
}

bool BingjiCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    int x = qMax(1, Self->getLostHp());
    return targets.length() <= x && !targets.isEmpty();
}

void BingjiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("bingji");
    foreach(int x, getSubcards())
        slash->addSubcard(Sanguosha->getCard(x));
    CardUseStruct use;
    use.card = slash;
    use.from = card_use.from;
    use.to = card_use.to;
    room->useCard(use);
}

class Bingji: public ViewAsSkill{
public:
    Bingji():ViewAsSkill("bingji"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty())
            return true;
        else if(selected.length() == 1){
            QString type1 = selected.first()->getFilteredCard()->getType();
            return to_select->getFilteredCard()->getType() == type1;
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 2){
            BingjiCard *card = new BingjiCard();
            card->addSubcards(cards);
            return card;
        }else
            return NULL;
    }
};

class Kongmen: public TriggerSkill{
public:
    Kongmen():TriggerSkill("kongmen"){
        events << CardLost;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *mowang, QVariant &data) const{
        if(mowang->isKongcheng()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Hand){
                Room *room = mowang->getRoom();
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = mowang;
                log.arg = objectName();
                room->playSkillEffect(objectName());
                room->sendLog(log);
                RecoverStruct o;
                o.card = Sanguosha->getCard(move->card_id);
                room->recover(mowang, o);
            }
        }
        return false;
    }
};

class Wudao: public PhaseChangeSkill{
public:
    Wudao():PhaseChangeSkill("wudao"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        if(target->getPhase() != Player::Start)
            return false;
        Room *room = target->getRoom();
        QList<ServerPlayer *> fanruis = room->findPlayersBySkillName(objectName());
        if(!fanruis.isEmpty()){
            foreach(ServerPlayer *fanrui, fanruis){
                if(fanrui->getMark("wudao") == 0 && fanrui->isKongcheng())
                    return true;
            }
        }
        return false;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> fanruis = room->findPlayersBySkillName(objectName());
        if(fanruis.isEmpty())
            return false;

        LogMessage log;
        log.type = "#WakeUp";
        log.arg = objectName();
        foreach(ServerPlayer *fanrui, fanruis){
            log.from = fanrui;
            room->sendLog(log);
            room->playSkillEffect(objectName());
            room->broadcastInvoke("animate", "lightbox:$wudao:2500");
            room->getThread()->delay(2500);

            room->drawCards(fanrui, 2);
            room->setPlayerMark(fanrui, objectName(), 1);
            room->loseMaxHp(fanrui);
            room->acquireSkill(fanrui, "butian");
            room->acquireSkill(fanrui, "qimen");
        }
        return false;
    }
};

LingdiCard::LingdiCard(){
    once = true;
}

bool LingdiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;
    if(targets.length() == 1){
        bool faceup = targets.first()->faceUp();
        return to_select->faceUp() != faceup;
    }
    return true;
}

bool LingdiCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void LingdiCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->turnOver();
}

class Lingdi: public OneCardViewAsSkill{
public:
    Lingdi():OneCardViewAsSkill("lingdi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("LingdiCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LingdiCard *card = new LingdiCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }
};

class Qiaodou: public MasochismSkill{
public:
    Qiaodou():MasochismSkill("qiaodou"){
    }

    virtual void onDamaged(ServerPlayer *malin, const DamageStruct &damage) const{
        if(damage.from && malin->askForSkillInvoke(objectName(), QVariant::fromValue((PlayerStar)damage.from))){
            malin->getRoom()->playSkillEffect(objectName());
            damage.from->turnOver();
        }
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

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
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

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();
        if(judge->card->getSuit() != Card::Diamond || player->isNude())
            return false;

        QStringList prompt_list;
        prompt_list << "@zhaixing-card" << judge->who->objectName()
                << "" << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");

        player->tag["Judge"] = data;
        const Card *card = room->askForCard(player, "@zhaixing", prompt, data);

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

class Dalang: public PhaseChangeSkill{
public:
    Dalang():PhaseChangeSkill("dalang"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Judge)
            return false;
        Room *room = player->getRoom();
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *tmp, room->getAllPlayers())
            if(!tmp->getJudgingArea().isEmpty())
                targets << tmp;
        if(targets.isEmpty() || !player->askForSkillInvoke(objectName()))
            return false;
        room->playSkillEffect(objectName());
        PlayerStar from = room->askForPlayerChosen(player, targets, "dalangfrom");
        while(!from->getJudgingArea().isEmpty()){
            QList<int> card_ids;
            foreach(const Card *c, from->getJudgingArea())
                card_ids << c->getId();
            room->fillAG(card_ids, player);
            int card_id = room->askForAG(player, card_ids, true, objectName());
            if(card_id > -1){
                const Card *card = Sanguosha->getCard(card_id);
                const DelayedTrick *trick = DelayedTrick::CastFrom(card);
                QList<ServerPlayer *> tos;
                foreach(ServerPlayer *p, room->getAlivePlayers()){
                    if(!player->isProhibited(p, trick) && !p->containsTrick(trick->objectName()))
                        tos << p;
                }
                if(trick && trick->isVirtualCard())
                    delete trick;
                room->setTag("DalangTarget", QVariant::fromValue(from));
                ServerPlayer *to = room->askForPlayerChosen(player, tos, "dalangtu");
                if(to)
                    room->moveCardTo(card, to, Player::Judging);
                room->removeTag("DalangTarget");

                card_ids.removeOne(card_id);
                player->invoke("clearAG");
                room->fillAG(card_ids, player);
            }
            else
                break;
        }
        player->invoke("clearAG");
        player->skip(Player::Draw);
        return true;
    }
};

class Qianshui: public TriggerSkill{
public:
    Qianshui():TriggerSkill("qianshui"){
        events << CardEffected;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.from && !effect.from->getWeapon() &&
            (effect.card->inherits("Assassinate") || effect.card->inherits("Slash"))){
            LogMessage log;
            log.type = "#ComskillNullify";
            log.from = effect.from;
            Room *room = player->getRoom();
            log.to << effect.to;
            log.arg = effect.card->objectName();
            log.arg2 = objectName();

            room->sendLog(log);
            room->playSkillEffect(objectName());
            return true;
        }
        return false;
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

class Duoming: public TriggerSkill{
public:
    Duoming(): TriggerSkill("duoming"){
        events << Damage;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") && damage.to->isAlive() &&
            damage.to->isKongcheng() && player->askForSkillInvoke(objectName(), data)){
            Room *room = damage.to->getRoom();
            room->playSkillEffect(objectName());
            room->loseMaxHp(damage.to);
        }
        return false;
    }
};

class Moucai: public MasochismSkill{
public:
    Moucai():MasochismSkill("moucai"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> lily = room->findPlayersBySkillName(objectName());
        if(lily.isEmpty())
            return;
        foreach(ServerPlayer *lili, lily){
            if(lili && player->getHandcardNum() > lili->getHp() && lili->askForSkillInvoke(objectName(), QVariant::fromValue((PlayerStar)player))){
                room->playSkillEffect(objectName());
                const Card *wolegequ = player->getRandomHandCard();
                lili->obtainCard(wolegequ, false);
            }
        }
    }
};

class EquiPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return player->hasEquip(card);
    }
    virtual bool willThrow() const{
        return false;
    }
};

class Heidian: public TriggerSkill{
public:
    Heidian():TriggerSkill("heidian"){
        events << Damaged << CardLost;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent v, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> sunny = room->findPlayersBySkillName(objectName());
        if(sunny.isEmpty())
            return false;
        LogMessage log;
        log.type = "#TriggerSkill";
        log.arg = objectName();
        foreach(ServerPlayer *sun, sunny){
            log.from = sun;
            if(v == Damaged){
                DamageStruct damage = data.value<DamageStruct>();
                if(damage.to == sun && damage.from && damage.from != damage.to &&
                   !damage.from->isKongcheng()){
                    room->playSkillEffect(objectName(), 2);
                    room->sendLog(log);
                    if(!room->askForCard(damage.from, ".", "@heidian1:" + sun->objectName(), data, CardDiscarded))
                        room->throwCard(damage.from->getRandomHandCard());
                }
            }
            else if(v == CardLost){
                if(player == sun)
                    continue;
                if(player->isKongcheng()){
                    CardMoveStar move = data.value<CardMoveStar>();
                    if(move->from_place == Player::Hand && player->isAlive()){
                        room->playSkillEffect(objectName(), 1);
                        room->sendLog(log);

                        const Card *card = room->askForCard(player, ".Equi", "@heidian2:" + sun->objectName(), data, NonTrigger);
                        if(card)
                            sun->obtainCard(card);
                        else
                            room->loseHp(player);
                    }
                }
            }
        }
        return false;
    }
};

class Renrou: public TriggerSkill{
public:
    Renrou():TriggerSkill("renrou"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        if(player->isNude())
            return false;
        Room *room = player->getRoom();
        QList<ServerPlayer *> ernian = room->findPlayersBySkillName(objectName());
        if(ernian.isEmpty())
            return false;
        QVariant shiti = QVariant::fromValue((PlayerStar)player);
        foreach(ServerPlayer *erniang, ernian){
            if(erniang->isAlive() && room->askForSkillInvoke(erniang, objectName(), shiti)){
                room->playSkillEffect(objectName(), 1);
                int cardnum = player->getCardCount(true);
                erniang->obtainCard(player->getWeapon());
                erniang->obtainCard(player->getArmor());
                erniang->obtainCard(player->getDefensiveHorse());
                erniang->obtainCard(player->getOffensiveHorse());
                DummyCard *all_cards = player->wholeHandCards();
                if(all_cards){
                    room->moveCardTo(all_cards, erniang, Player::Hand, false);
                    delete all_cards;
                }
                QList<int> yiji_cards = erniang->handCards().mid(erniang->getHandcardNum() - cardnum);
                bool isyiji = false;
                while(room->askForYiji(erniang, yiji_cards))
                    isyiji = true;
                if(isyiji)
                    room->playSkillEffect(objectName(), 2);
            }
        }
        return false;
    }
};

CGDKPackage::CGDKPackage()
    :Package("CGDK")
{
    General *xiebao = new General(this, "xiebao", "min");
    xiebao->addSkill(new Liehuo);

    General *xiaorang = new General(this, "xiaorang", "min", 3);
    xiaorang->addSkill(new Linmo);
    xiaorang->addSkill(new Zhaixing);

    General *yanglin = new General(this, "yanglin", "kou");
    yanglin->addSkill(new Citan);

    General *guosheng = new General(this, "guosheng", "jiang");
    guosheng->addSkill(new Bingji);

    General *fanrui = new General(this, "fanrui", "kou", 3);
    fanrui->addSkill(new Kongmen);
    fanrui->addSkill(new Wudao);

    General *malin = new General(this, "malin", "kou", 3);
    malin->addSkill(new Lingdi);
    malin->addSkill(new Qiaodou);

    General *tongwei = new General(this, "tongwei", "min", 3);
    tongwei->addSkill(new Dalang);
    tongwei->addSkill(new Qianshui);

    General *zhengtianshou = new General(this, "zhengtianshou", "kou", 3);
    zhengtianshou->addSkill(new Wugou);
    zhengtianshou->addSkill(new Qiaojiang);

    General *lili = new General(this, "lili", "kou", 3);
    lili->addSkill(new Duoming);
    lili->addSkill(new Moucai);

    General *sunerniang = new General(this, "sunerniang", "kou", 3, false);
    sunerniang->addSkill(new Heidian);
    sunerniang->addSkill(new Renrou);
    patterns[".Equi"] = new EquiPattern;

    addMetaObject<BingjiCard>();
    addMetaObject<LingdiCard>();
    addMetaObject<LinmoCard>();
    addMetaObject<ZhaixingCard>();
}

ADD_PACKAGE(CGDK)
