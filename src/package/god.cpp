#include "god.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "settings.h"
#include "tocheck.h"

class Jinqiang:public TriggerSkill{
public:
    Jinqiang():TriggerSkill("jinqiang"){
        events << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *xuning, QVariant &data) const{
        Room *room = xuning->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        if(xuning != damage.to && damage.chain && damage.nature == DamageStruct::Normal){
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = xuning;
            log.arg = objectName();
            room->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
            return false;
        }
        return false;
    }
};

class Qinyin: public TriggerSkill{
public:
    Qinyin():TriggerSkill("qinyin"){
        events << CardLost << PhaseChange;
        default_choice = "down";
    }

    void perform(ServerPlayer *shenzhouyu) const{
        Room *room = shenzhouyu->getRoom();
        QString result = room->askForChoice(shenzhouyu, objectName(), "up+down");
        QList<ServerPlayer *> all_players = room->getAllPlayers();
        if(result == "up"){
            room->playSkillEffect(objectName(), 2);
            foreach(ServerPlayer *player, all_players){
                RecoverStruct recover;
                recover.who = shenzhouyu;
                room->recover(player, recover);
            }
        }else if(result == "down"){
            foreach(ServerPlayer *player, all_players){
                room->loseHp(player);
            }

            int index = 1;
            if(room->findPlayer("caocao+shencaocao+shencc"))
                index = 3;

            room->playSkillEffect(objectName(), index);
        }
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *shenzhouyu, QVariant &data) const{
        if(shenzhouyu->getPhase() != Player::Discard)
            return false;

        if(event == CardLost){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->to_place == Player::DiscardedPile){
                shenzhouyu->addMark("qinyin");
                if(shenzhouyu->getMark("qinyin") == 2){
                    if(shenzhouyu->askForSkillInvoke(objectName()))
                        perform(shenzhouyu);
                }
            }
        }else if(event == PhaseChange){
            shenzhouyu->setMark("qinyin", 0);
        }

        return false;
    }
};

class Wumou:public TriggerSkill{
public:
    Wumou():TriggerSkill("wumou"){
        frequency = Compulsory;
        events << CardUsed << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->inherits("TrickCard") && !card->inherits("DelayedTrick")){
            Room *room = player->getRoom();
            room->playSkillEffect(objectName());

            int num = player->getMark("@wrath");
            if(num >= 1 && room->askForChoice(player, objectName(), "discard+losehp") == "discard"){
                player->loseMark("@wrath");
            }else
                room->loseHp(player);
        }

        return false;
    }
};

class Shenfen: public TriggerSkill{
public:
    Shenfen():TriggerSkill("shenfen"){
        events << CardEffected;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.from && effect.to == player && !player->getEquips().isEmpty()){
            QStringList suits;
            foreach(const Card *rmp, player->getEquips()){
                if(!suits.contains(rmp->getSuitString()))
                    suits << rmp->getSuitString();
            }
            if(!effect.card->inherits("Slash") && !effect.card->inherits("Duel") && !effect.card->inherits("Ecstasy"))
                return false;
            QString suit = effect.card->getSuitString();
            if(!suits.contains(suit))
                return false;

            LogMessage log;
            log.type = "#Caiquan";
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

WuqianCard::WuqianCard(){
    target_fixed = true;
}

void WuqianCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    QList<int> card_ids;
    QList<const Card *> cards = source->getCards("he");
    foreach(const Card *tmp, cards)
        card_ids << tmp->getId();
    room->fillAG(card_ids, source);
    int card_id = room->askForAG(source, card_ids, false, objectName());
    card_ids.removeOne(card_id);
    room->takeAG(source, card_id);
    room->broadcastInvoke("clearAG");
    //room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::DrawPile);
    //source->drawCards(1);
    //room->moveCardTo(Sanguosha->getCard(card_id), room->askForPlayerChosen(source, room->getAllPlayers(), "dd"), Player::Equip);
    //room->throwCard(card_id);
}

class Wuqian: public ZeroCardViewAsSkill{
public:
    Wuqian():ZeroCardViewAsSkill("wuqian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isNude();
    }

    virtual const Card *viewAs() const{
        return new WuqianCard;
    }
};

WushenSlash::WushenSlash(Card::Suit suit, int number)
    :Slash(suit, number)
{

}

class Wushen: public FilterSkill{
public:
    Wushen():FilterSkill("wushen"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        WushenSlash *slash = new WushenSlash(card->getSuit(), card->getNumber());
        slash->addSubcard(card_item->getCard()->getId());
        slash->setSkillName(objectName());

        return slash;
    }
};

class Qixing: public PhaseChangeSkill{
public:
    Qixing():PhaseChangeSkill("qixing"){
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getMark("@star") > 0;
    }

    static void Exchange(ServerPlayer *shenzhuge){
        QList<int> stars = shenzhuge->getPile("stars");
        if(stars.isEmpty())
            return;

        Room *room = shenzhuge->getRoom();
        room->fillAG(stars, shenzhuge);

        int ai_delay = Config.AIDelay;
        Config.AIDelay = 0;

        int n = 0;
        while(!stars.isEmpty()){
            int card_id = room->askForAG(shenzhuge, stars, true, "qixing");
            if(card_id == -1)
                break;

            stars.removeOne(card_id);
            ++ n;

            room->moveCardTo(Sanguosha->getCard(card_id), shenzhuge, Player::Hand, false);
        }

        Config.AIDelay = ai_delay;

        shenzhuge->invoke("clearAG");

        if(n == 0)
            return;

        const Card *exchange_card = room->askForExchange(shenzhuge, "qixing", n);

        foreach(int card_id, exchange_card->getSubcards())
            shenzhuge->addToPile("stars", card_id, false);

        LogMessage log;
        log.type = "#QixingExchange";
        log.from = shenzhuge;
        log.arg = QString::number(n);
        room->sendLog(log);

        delete exchange_card;
    }

    static void DiscardStar(ServerPlayer *shenzhuge, int n){
        Room *room = shenzhuge->getRoom();
        const QList<int> stars = shenzhuge->getPile("stars");

        room->fillAG(stars, shenzhuge);

        int i;
        for(i=0; i<n; i++){
            int card_id = room->askForAG(shenzhuge, stars, false, "qixing-discard");
            room->throwCard(card_id);
        }

        shenzhuge->invoke("clearAG");
    }

    virtual bool onPhaseChange(ServerPlayer *shenzhuge) const{
        if(shenzhuge->getPhase() == Player::Draw){
            Exchange(shenzhuge);
        }

        return false;
    }
};

class QixingStart: public GameStartSkill{
public:
    QixingStart():GameStartSkill("#qixing"){

    }

    virtual int getPriority() const{
        return -1;
    }

    virtual void onGameStart(ServerPlayer *shenzhuge) const{
        shenzhuge->gainMark("@star", 7);
        shenzhuge->drawCards(7);

        QList<int> stars = shenzhuge->handCards().mid(0, 7);

        foreach(int card_id, stars)
            shenzhuge->addToPile("stars", card_id, false);

        Qixing::Exchange(shenzhuge);
    }
};

KuangfengCard::KuangfengCard(){

}

bool KuangfengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void KuangfengCard::onEffect(const CardEffectStruct &effect) const{
    Qixing::DiscardStar(effect.from, 1);

    effect.from->loseMark("@star");
    effect.to->gainMark("@gale");
}

class KuangfengViewAsSkill: public ZeroCardViewAsSkill{
public:
    KuangfengViewAsSkill():ZeroCardViewAsSkill("kuangfeng"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@kuangfeng";
    }

    virtual const Card *viewAs() const{
        return new KuangfengCard;
    }
};

class Kuangfeng: public TriggerSkill{
public:
    Kuangfeng():TriggerSkill("kuangfeng"){
        view_as_skill = new KuangfengViewAsSkill;

        events << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@gale") > 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Fire){
            LogMessage log;
            log.type = "#GalePower";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            player->getRoom()->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

class QixingAsk: public PhaseChangeSkill{
public:
    QixingAsk():PhaseChangeSkill("#qixing-ask"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        if(target->getPhase() == Player::Finish){
            if(target->getMark("@star") > 0 && target->hasSkill("kuangfeng"))
                room->askForUseCard(target, "@kuangfeng", "@@kuangfeng-card");

            if(target->getMark("@star") > 0 && target->hasSkill("dawu"))
                room->askForUseCard(target, "@dawu", "@@dawu-card");
        }else if(target->getPhase() == Player::Start){
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer *player, players){
                if(player->getMark("@gale") > 0)
                    player->loseMark("@gale");
                if(player->getMark("@fog") > 0)
                    player->loseMark("@fog");
            }
        }

        return false;
    }
};

class QixingClear: public TriggerSkill{
public:
    QixingClear():TriggerSkill("#qixing-clear"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> players = room->getAllPlayers();
        foreach(ServerPlayer *player, players){
            player->loseAllMarks("@gale");
            player->loseAllMarks("@fog");
        }

        return false;
    }
};

DawuCard::DawuCard(){

}

bool DawuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.length() < Self->getMark("@star");
}

void DawuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    int n = targets.length();
    Qixing::DiscardStar(source, n);

    source->loseMark("@star", n);
    foreach(ServerPlayer *target, targets){
        target->gainMark("@fog");
    }
}

class DawuViewAsSkill: public ZeroCardViewAsSkill{
public:
    DawuViewAsSkill():ZeroCardViewAsSkill("dawu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@dawu";
    }

    virtual const Card *viewAs() const{
        return new DawuCard;
    }
};

class Dawu: public TriggerSkill{
public:
    Dawu():TriggerSkill("dawu"){
        view_as_skill = new DawuViewAsSkill;

        events << Predamaged;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@fog") > 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature != DamageStruct::Thunder){
            Room *room = player->getRoom();

            LogMessage log;
            log.type = "#FogProtect";
            log.from = player;
            log.arg = QString::number(damage.damage);
            if(damage.nature == DamageStruct::Normal)
                log.arg2 = "normal_nature";
            else if(damage.nature == DamageStruct::Fire)
                log.arg2 = "fire_nature";
            room->sendLog(log);

            return true;
        }else
            return false;
    }
};

class Renjie: public TriggerSkill{
public:
    Renjie():TriggerSkill("renjie"){
        events << DamageDone << CardDiscarded;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == CardDiscarded){
            if(player->getPhase() == Player::Discard){
                CardStar card = data.value<CardStar>();
                int n = card->subcardsLength();
                if(n > 0)
                    player->gainMark("@bear", n);
            }
        }else if(event == DamageDone){
            DamageStruct damage = data.value<DamageStruct>();
            player->gainMark("@bear", damage.damage);
        }

        return false;
    }
};

class LianpoCount: public TriggerSkill{
public:
    LianpoCount():TriggerSkill("#lianpo-count"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;

        if(killer && killer->hasSkill("lianpo")){
            killer->addMark("lianpo");

            LogMessage log;
            log.type = "#LianpoRecord";
            log.from = killer;
            log.to << player;

            Room *room = player->getRoom();
            log.arg = room->getCurrent()->getGeneralName();
            room->sendLog(log);
        }

        return false;
    }
};

class Baiyin: public PhaseChangeSkill{
public:
    Baiyin():PhaseChangeSkill("baiyin"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("baiyin") == 0
                && target->getMark("@bear") >= 4;
    }

    virtual bool onPhaseChange(ServerPlayer *shensimayi) const{
        Room *room = shensimayi->getRoom();

        LogMessage log;
        log.type = "#BaiyinWake";
        log.from = shensimayi;
        log.arg = QString::number(shensimayi->getMark("@bear"));
        room->sendLog(log);

        room->loseMaxHp(shensimayi);
        room->acquireSkill(shensimayi, "jilve");

        shensimayi->setMark("baiyin", 1);

        return false;
    }
};

JilveCard::JilveCard(){
    target_fixed = true;
}

void JilveCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *shensimayi = card_use.from;

    QStringList choices;
    if(!shensimayi->hasUsed("ZhihengCard"))
        choices << "zhiheng";

    if(!shensimayi->tag.value("JilveWansha", false).toBool())
        choices << "wansha";

    if(choices.isEmpty())
        return;

    QString choice;
    if(choices.length() == 1)
        choice = choices.first();
    else
        choice = room->askForChoice(shensimayi, "jilve", "zhiheng+wansha");

    shensimayi->loseMark("@bear");

    if(choice == "wansha"){
        room->acquireSkill(shensimayi, "wansha");
        shensimayi->tag["JilveWansha"] = true;
    }else
        room->askForUseCard(shensimayi, "@zhiheng", "@jilve-zhiheng");
}

// wansha & zhiheng
class JilveViewAsSkill: public ZeroCardViewAsSkill{
public:
    JilveViewAsSkill():ZeroCardViewAsSkill(""){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->usedTimes("JilveCard") < 2 && player->getMark("@bear") > 0;
    }

    virtual const Card *viewAs() const{
        return new JilveCard;
    }
};

class Jilve: public TriggerSkill{
public:
    Jilve():TriggerSkill("jilve"){
        events << CardUsed << CardResponsed // jizhi
                << AskForRetrial // guicai
                << Damaged; // fangzhu

        view_as_skill = new JilveViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target)
                && target->getMark("@bear") > 0;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == CardUsed || event == CardResponsed){
            CardStar card = NULL;
            if(event == CardUsed)
                card = data.value<CardUseStruct>().card;
            else
                card = data.value<CardStar>();

            if(card->isNDTrick() && player->askForSkillInvoke("jilve", data)){
                player->loseMark("@bear");
                player->drawCards(1);
            }
        }else if(event == AskForRetrial){
            const TriggerSkill *guicai = Sanguosha->getTriggerSkill("guicai");
            if(guicai && !player->isKongcheng() && player->askForSkillInvoke("jilve", data)){
                player->loseMark("@bear");
                guicai->trigger(event, player, data);
            }
        }else if(event == Damaged){
            const TriggerSkill *fangzhu = Sanguosha->getTriggerSkill("fangzhu");
            if(fangzhu && player->askForSkillInvoke("jilve", data)){
                player->loseMark("@bear");
                fangzhu->trigger(event, player, data);
            }
        }

        return false;
    }
};

class JilveClear: public PhaseChangeSkill{
public:
    JilveClear():PhaseChangeSkill("#jilve-clear"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::NotActive
                && target->tag.value("JilveWansha").toBool();
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        target->getRoom()->detachSkillFromPlayer(target, "wansha");
        target->tag.remove("JilveWansha");
        return false;
    }
};

class Lianpo: public PhaseChangeSkill{
public:
    Lianpo():PhaseChangeSkill("lianpo"){
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::NotActive;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        ServerPlayer *shensimayi = room->findPlayerBySkillName("lianpo");
        if(shensimayi == NULL || shensimayi->getMark("lianpo") <= 0)
            return false;

        int n = shensimayi->getMark("lianpo");
        shensimayi->setMark("lianpo", 0);

        if(!shensimayi->askForSkillInvoke("lianpo"))
            return false;

        LogMessage log;
        log.type = "#LianpoCanInvoke";
        log.from = shensimayi;
        log.arg = QString::number(n);
        room->sendLog(log);

        room->getThread()->trigger(TurnStart, shensimayi);

        return false;
    }
};

class Juejing: public TriggerSkill{
public:
    Juejing():TriggerSkill("juejing"){
        events << PhaseChange;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        if(player->getPhase() == Player::Draw){
            QVariant draw_num = 2 + player->getLostHp();
            player->getRoom()->getThread()->trigger(DrawNCards, player, draw_num);
            int n = draw_num.toInt();
            if(n > 0)
                player->drawCards(n, false);

            return true;
        }

        return false;
    }
};

class Longhun: public ViewAsSkill{
public:
    Longhun():ViewAsSkill("longhun"){

    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "slash"
                || pattern == "jink"
                || pattern.contains("peach")
                || pattern == "nullification";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->isWounded() || Slash::IsAvailable(player);
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        int n = qMax(1, Self->getHp());

        if(selected.length() >= n)
            return false;

        if(n > 1 && !selected.isEmpty()){
            Card::Suit suit = selected.first()->getFilteredCard()->getSuit();
            return card->getSuit() == suit;
        }

        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                if(Self->isWounded() && card->getSuit() == Card::Heart)
                    return true;
                else if(Slash::IsAvailable(Self) && card->getSuit() == Card::Diamond)
                    return true;
                else
                    return false;
            }

        case Client::Responsing:{
                QString pattern = ClientInstance->getPattern();
                if(pattern == "jink")
                    return card->getSuit() == Card::Club;
                else if(pattern == "nullification")
                    return card->getSuit() == Card::Spade;
                else if(pattern == "peach" || pattern == "peach+analeptic")
                    return card->getSuit() == Card::Heart;
                else if(pattern == "slash")
                    return card->getSuit() == Card::Diamond;
            }

        default:
            break;
        }

        return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        int n = qMax(1, Self->getHp());

        if(cards.length() != n)
            return NULL;

        const Card *card = cards.first()->getFilteredCard();
        Card *new_card = NULL;

        Card::Suit suit = card->getSuit();
        int number = cards.length() > 1 ? 0 : card->getNumber();
        switch(card->getSuit()){
        case Card::Spade:{
                new_card = new Nullification(suit, number);
                break;
            }

        case Card::Heart:{
                new_card = new Peach(suit, number);
                break;
            }

        case Card::Club:{
                new_card = new Jink(suit, number);
                break;
            }

        case Card::Diamond:{
                new_card = new FireSlash(suit, number);
                break;
            }
        default:
            break;
        }

        if(new_card){
            new_card->setSkillName(objectName());
            new_card->addSubcards(cards);
        }

        return new_card;
    }
};

class XianjiClear: public PhaseChangeSkill{
public:
    XianjiClear():PhaseChangeSkill("#xianji-clear"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::NotActive){
            Room *room = target->getRoom();
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer *player, players){
                if(player->hasFlag("jilei")){
                    player->jilei(".");
                    player->invoke("jilei");

                    LogMessage log;
                    log.type = "#XianjiClear";
                    log.from = player;
                    room->sendLog(log);
                }
            }
        }

        return false;
    }
};

class Xianji: public PhaseChangeSkill{
public:
    Xianji():PhaseChangeSkill("xianji"){
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() != Player::Start)
            return false;
        Room *room = target->getRoom();
        ServerPlayer *waste = room->findPlayerBySkillName(objectName());
        const Card *card = NULL;
        if(waste && !waste->isNude())
            card = room->askForCard(waste, "..", "@xianji", QVariant::fromValue(target));
        if(!waste || !card)
           return false;
        QString choice = card->getType();
        room->playSkillEffect(objectName());

        target->jilei(choice);
        target->invoke("jilei", choice);
        target->setFlags("jilei");

        LogMessage log;
        log.type = "#Xianji";
        log.from = waste;
        log.to << target;
        log.arg2 = objectName();
        log.arg = choice;
        room->sendLog(log);

        return false;
    }
};

class Houlue: public TriggerSkill{
public:
    Houlue():TriggerSkill("houlue"){
        events << CardLost;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *lese = room->findPlayerBySkillName(objectName());
        if(!lese)
            return false;
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->to_place == Player::DiscardedPile){
            const Card *card = Sanguosha->getCard(move->card_id);
            if(card->isNDTrick() && lese->askForSkillInvoke(objectName())){
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(.*):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = lese;

                room->judge(judge);
                if(!judge.card->inherits("TrickCard")){
                    lese->obtainCard(card);
                    room->playSkillEffect(objectName(), 1);
                }
                else{
                    lese->obtainCard(judge.card);
                    room->playSkillEffect(objectName(), 2);
                }
            }
        }
        return false;
    }
};

FeihuangCard::FeihuangCard(){
    target_fixed = true;
    will_throw = false;
}

void FeihuangCard::use(Room *, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    foreach(int card_id, this->getSubcards())
        source->addToPile("stone", card_id);
}

class FeihuangViewAsSkill: public ViewAsSkill{
public:
    FeihuangViewAsSkill(): ViewAsSkill("feihuang"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        int n = Self->getHandcardNum() - Self->getHp();
        if(selected.length() >= n)
            return false;

        if(to_select->isEquipped())
            return false;

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != Self->getHandcardNum() - Self->getHp())
            return NULL;

        FeihuangCard *card = new FeihuangCard;
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@feihuang";
    }
};

class Feihuang: public PhaseChangeSkill{
public:
    Feihuang():PhaseChangeSkill("feihuang"){
        view_as_skill = new FeihuangViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *vq) const{
        if(vq->getPhase() != Player::Discard)
            return false;
        Room *room = vq->getRoom();
        if(vq->getHandcardNum() > vq->getHp() && room->askForUseCard(vq, "@@feihuang", "@feihuang"))
            return true;
        return false;
    }
};

MeiyuCard::MeiyuCard(){
}

bool MeiyuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->canSlash(to_select, false);
}

void MeiyuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->throwCard(effect.from->getPile("stone").last());
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("meiyu");
    CardUseStruct use;
    use.card = slash;
    use.from = effect.from;
    use.to << effect.to;
    room->useCard(use, false);
}

class Meiyu: public ZeroCardViewAsSkill{
public:
    Meiyu():ZeroCardViewAsSkill("meiyu"){
    }

    virtual const Card *viewAs() const{
        return new MeiyuCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->getPile("stone").isEmpty();
    }
};

class HuafoSlash: public OneCardViewAsSkill{
public:
    HuafoSlash():OneCardViewAsSkill("hua1fo"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("BasicCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        Slash *s = new Slash(card->getSuit(), card->getNumber());
        s->addSubcard(card);
        s->setSkillName(objectName());
        return s;
    }
};

class HuafoAnale: public OneCardViewAsSkill{
public:
    HuafoAnale():OneCardViewAsSkill("hua2fo"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("BasicCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        Analeptic *a = new Analeptic(card->getSuit(), card->getNumber());
        a->addSubcard(card);
        a->setSkillName(objectName());
        return a;
    }
};

class Dunwu: public TriggerSkill{
public:
    Dunwu():TriggerSkill("dunwu"){
        events << Damage;
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        if(damage.damage > 0){
            LogMessage log;
            log.type = "#Dunwu";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = objectName();
            room->sendLog(log);

            room->playSkillEffect(objectName());
        }
        for(int i = 0; i < damage.damage; i++){
            int card_id = room->drawCard();
            const Card *card = Sanguosha->getCard(card_id);
            room->moveCardTo(card, NULL, Player::Special, true);
            room->getThread()->delay();

            if(card->inherits("Tsunami"))
                room->killPlayer(player);
            else
                player->addToPile("jiyan", card_id);

            if(player->getPile("jiyan").length() >= 4){
                LogMessage log;
                log.type = "#DunwuB";
                log.from = player;
                log.arg = "jiyan";
                log.arg2 = QString::number(4);
                room->sendLog(log);

                foreach(int i, player->getPile("jiyan"))
                    room->throwCard(i);
                room->loseMaxHp(player);
            }
        }
        return false;
    }
};

GodPackage::GodPackage()
    :Package("god")
{
    General *shenwuyong = new General(this, "shenwuyong", "god", 3);
    shenwuyong->addSkill(new Xianji);
    shenwuyong->addSkill(new XianjiClear);
    shenwuyong->addSkill(new Houlue);
    related_skills.insertMulti("xianji", "#xianji-clear");

    General *shenzhangqing = new General(this, "shenzhangqing", "god", 4);
    shenzhangqing->addSkill(new Feihuang);
    shenzhangqing->addSkill(new Meiyu);

    General *shenluzhishen = new General(this, "shenluzhishen", "god", 5);
    shenluzhishen->addSkill(new Dunwu);
    shenluzhishen->addSkill(new HuafoSlash);
    shenluzhishen->addSkill(new HuafoAnale);

    General *shentest = new General(this, "shentest", "god");
    shentest->addSkill(new Shenfen);
    shentest->addSkill(new Wuqian);
    addMetaObject<WuqianCard>();
/*
    General *shenzhugeliang = new General(this, "shenzhugeliang", "god", 3);
    shenzhugeliang->addSkill(new Qixing);
    shenzhugeliang->addSkill(new QixingStart);
    shenzhugeliang->addSkill(new Dawu);

    related_skills.insertMulti("qixing", "#qixing-ask");
    related_skills.insertMulti("qixing", "#qixing-clear");

    General *shenlubu = new General(this, "shenlubu", "god", 5);
    shenlubu->addSkill(new Kuangbao);
    shenlubu->addSkill(new MarkAssignSkill("@wrath", 2));
    shenlubu->addSkill(new Wumou);
    addMetaObject<MediumYeyanCard>();
    addMetaObject<SmallYeyanCard>();
    addMetaObject<WushenSlash>();
    addMetaObject<DawuCard>();*/
    addMetaObject<FeihuangCard>();
    addMetaObject<MeiyuCard>();
}

ADD_PACKAGE(God)
