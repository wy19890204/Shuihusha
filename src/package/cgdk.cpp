#include "cgdk.h"
#include "standard.h"
#include "skill.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"

class Liehuo: public TriggerSkill{
public:
    Liehuo():TriggerSkill("liehuo"){
        events << SlashMissed << Damage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *bao, QVariant &data) const{
        Room *room = bao->getRoom();
        ServerPlayer *target;
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
            bao->obtainCard(target->getRandomHandCard());
        }
        return false;
    }
};

class Jueming: public ProhibitSkill{
public:
    Jueming():ProhibitSkill("jueming"){
    }

    virtual bool isProhibited(const Player *, const Player *to, const Card *card) const{
        if(to->getPhase() == Player::NotActive && to->getHp() == 1)
            return card->inherits("Slash") || card->inherits("Duel") || card->inherits("Assassinate");
        else
            return false;
    }
};

class Jiuhan:public TriggerSkill{
public:
    Jiuhan():TriggerSkill("jiuhan"){
        events << HpRecover;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *nana, QVariant &data) const{
        Room *room = nana->getRoom();
        RecoverStruct rec = data.value<RecoverStruct>();
        if(rec.who == nana && rec.card->inherits("Analeptic") &&
           nana->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            LogMessage log;
            log.type = "#Jiuhan";
            log.from = nana;
            log.arg = objectName();
            log.arg2 = QString::number(1);
            room->sendLog(log);
            rec.recover ++;

            data = QVariant::fromValue(rec);
        }
        return false;
    }
};

YunchouCard::YunchouCard(){
    target_fixed = true;
}

void YunchouCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *nouse = card_use.from;
    QStringList ndtricks;
    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach(const Card *card, cards){
        if(card->isNDTrick() && !card->inherits("Nullification") &&
           !ndtricks.contains(card->objectName()))
            ndtricks << card->objectName();
    }
    QString name = room->askForChoice(nouse, "yunchou", ndtricks.join("+"));
    room->setPlayerProperty(nouse, "yunchoustore", name);
}

class YunchouSelect: public ZeroCardViewAsSkill{
public:
    YunchouSelect():ZeroCardViewAsSkill("yunchou-select"){
    }

    virtual const Card *viewAs() const{
        return new YunchouCard;
    }
};

class Yunchou:public OneCardViewAsSkill{
public:
    Yunchou():OneCardViewAsSkill("yunchou"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("TrickCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        QString name = Self->property("yunchoustore").toString();
        Card *new_card = Sanguosha->cloneCard(name, card->getSuit(), card->getNumber());
        new_card->addSubcard(card);
        new_card->setSkillName("yunchou");
        Self->setFlags("Yunchou_used");
        return new_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->property("yunchoustore").isNull() && !player->hasFlag("Yunchou_used");
    }
};

class ZhiquN: public OneCardViewAsSkill{
public:
    ZhiquN():OneCardViewAsSkill("zhiqu-n"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("EquipCard");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "nullification" || pattern == "nulliplot";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getFilteredCard();
        Card *ncard = new Nullification(first->getSuit(), first->getNumber());
        ncard->addSubcard(first);
        ncard->setSkillName("zhiqu");

        return ncard;
    }
};

#include "plough.h"
class ZhiquC: public OneCardViewAsSkill{
public:
    ZhiquC():OneCardViewAsSkill("zhiqu-c"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("EquipCard");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "nulliplot";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getFilteredCard();
        Card *ncard = new Counterplot(first->getSuit(), first->getNumber());
        ncard->addSubcard(first);
        ncard->setSkillName("zhiqu");

        return ncard;
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
               yanglin->askForSkillInvoke(objectName())){
                //room->showAllCards(player, yanglin);
                QList<int> card_ids;
                foreach(const Card *a, player->getHandcards())
                    card_ids << a->getId();
                room->fillAG(card_ids, yanglin);
                int to_move = room->askForAG(yanglin, card_ids, true, objectName());
                if(to_move > -1){
                    ServerPlayer *target = room->askForPlayerChosen(yanglin, room->getOtherPlayers(player), objectName());
                    target->obtainCard(Sanguosha->getCard(to_move));
                    card_ids.removeOne(to_move);
                }
                yanglin->invoke("clearAG");
            }
        }
        return false;
    }
};

BingjiCard::BingjiCard(){
}

bool BingjiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int x = qMax(1, Self->getLostHp());
    return targets.length() < x;
}

bool BingjiCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    int x = qMax(1, Self->getLostHp());
    return targets.length() <= x && !targets.isEmpty();
}

void BingjiCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    SlashEffectStruct eff;
    eff.from = effect.from;
    //eff.slash = this;
    eff.to = effect.to;
    eff.drank = effect.from->hasFlag("drank");
    room->slashEffect(eff);
}

class Bingji: public ViewAsSkill{
public:
    Bingji():ViewAsSkill("bingji"){
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
        ServerPlayer *fanrui = target->getRoom()->findPlayerBySkillName(objectName());
        return target->getPhase() == Player::Start
                && fanrui && fanrui->getMark("wudao") == 0
                && fanrui->isKongcheng();
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        ServerPlayer *fanrui = room->findPlayerBySkillName(objectName());
        if(!fanrui)
            return false;

        LogMessage log;
        log.type = "#WakeUp";
        log.from = fanrui;
        log.arg = objectName();
        room->sendLog(log);
        room->playSkillEffect(objectName());
        room->broadcastInvoke("animate", "lightbox:$wudao:5000");
        room->getThread()->delay(2500);

        room->drawCards(fanrui, 2);
        room->setPlayerMark(fanrui, objectName(), 1);
        room->loseMaxHp(fanrui);
        room->acquireSkill(fanrui, "butian");
        room->acquireSkill(fanrui, "qimen");

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
        if(damage.from && malin->askForSkillInvoke(objectName()))
            damage.from->turnOver();
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

class LinmoSelect: public ZeroCardViewAsSkill{
public:
    LinmoSelect():ZeroCardViewAsSkill("linmo-select"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->getPile("zi").isEmpty();
    }

    virtual const Card *viewAs() const{
        return new LinmoCard;
    }
};

class LinmoViewAsSkill:public OneCardViewAsSkill{
public:
    LinmoViewAsSkill():OneCardViewAsSkill("linmo"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        QString name = Self->property("linmostore").toString();
        Card *new_card = Sanguosha->cloneCard(name, card->getSuit(), card->getNumber());
        new_card->addSubcard(card);
        new_card->setSkillName("linmo");
        Self->setFlags("Linmo_used");
        return new_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->property("linmostore").isNull() && !player->hasFlag("Linmo_used");
    }
};

class Linmo: public TriggerSkill{
public:
    Linmo():TriggerSkill("linmo"){
        view_as_skill = new LinmoViewAsSkill;
        events << CardFinished;
        //frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *writer = room->findPlayerBySkillName(objectName());
        if(!writer)
            return false;
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.to.contains(writer) && (use.card->inherits("BasicCard") || use.card->isNDTrick())
            && room->getCardPlace(use.card->getEffectiveId() == Player::DiscardedPile)){
            bool hassamezi = false;
            foreach(int x, writer->getPile("zi")){
                if(Sanguosha->getCard(x)->objectName() == use.card->objectName()){
                    hassamezi = true;
                    break;
                }
            }
            if(!hassamezi && writer->askForSkillInvoke(objectName()))
                writer->addToPile("zi", use.card->getEffectiveId());
        }
        return false;
    }
};

class LinmoClear: public PhaseChangeSkill{
public:
    LinmoClear():PhaseChangeSkill("#linmo-clear"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::NotActive){
            player->property("linmostore") = "";
            foreach(int a, player->getPile("zi"))
                player->getRoom()->throwCard(a);
        }
        return false;
    }
};

ZhaixingCard::ZhaixingCard(){
    target_fixed = true;
    will_throw = false;
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
        PlayerStar from = room->askForPlayerChosen(player, targets, objectName());
        if(from->getJudgingArea().isEmpty())
            return false;
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
                ServerPlayer *to = room->askForPlayerChosen(player, tos, objectName());
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
            log.type = "#Foyuan";
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

CGDKPackage::CGDKPackage()
    :Package("CGDK")
{
    General *wuyong = new General(this, "wuyong", "kou", 3);
    wuyong->addSkill(new YunchouSelect);
    wuyong->addSkill(new Yunchou);
    wuyong->addSkill(new ZhiquN);
    wuyong->addSkill(new ZhiquC);

    General *ruanxiaoqi = new General(this, "ruanxiaoqi", "min");
    ruanxiaoqi->addSkill(new Jueming);
    ruanxiaoqi->addSkill(new Jiuhan);

    General *xiebao = new General(this, "xiebao", "min");
    xiebao->addSkill(new Liehuo);

    General *xiaorang = new General(this, "xiaorang", "min", 3);
    xiaorang->addSkill(new LinmoSelect);
    xiaorang->addSkill(new Linmo);
    xiaorang->addSkill(new LinmoClear);
    related_skills.insertMulti("linmo", "#linmo-clear");
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

    addMetaObject<BingjiCard>();
    addMetaObject<YunchouCard>();
    addMetaObject<LingdiCard>();
    addMetaObject<LinmoCard>();
    addMetaObject<ZhaixingCard>();
}

ADD_PACKAGE(CGDK)
