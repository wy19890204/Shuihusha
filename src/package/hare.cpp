#include "hare.h"
#include "skill.h"
#include "carditem.h"
#include "maneuvering.h"
#include "plough.h"
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
                    if(!player->isProhibited(p, trick) && !p->containsTrick(trick->objectName(), false))
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

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.from && !effect.from->getWeapon() &&
            (effect.card->inherits("Assassinate") || effect.card->inherits("Slash"))){
            LogMessage log;
            log.type = "#ComskillNullify";
            log.from = effect.from;
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

SheyanCard::SheyanCard(){
    target_fixed = true;
}

void SheyanCard::onUse(Room *room, const CardUseStruct &card_use) const{
    room->throwCard(this);
    int card_id = getSubcards().first();
    Card::Suit suit = Sanguosha->getCard(card_id)->getSuit();
    int num = Sanguosha->getCard(card_id)->getNumber();

    CardUseStruct use;
    use.from = card_use.from;
    AmazingGrace *amazingGrace = new AmazingGrace(suit, num);
    amazingGrace->addSubcard(card_id);
    amazingGrace->setSkillName("sheyan");
    use.card = amazingGrace;
    room->useCard(use);
}

class Sheyan: public OneCardViewAsSkill{
public:
    Sheyan():OneCardViewAsSkill("sheyan"){

    }
    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("SheyanCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        SheyanCard *card = new SheyanCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Jiayao:public TriggerSkill{
public:
    Jiayao():TriggerSkill("jiayao"){
        events << CardEffect;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!effect.card->inherits("AmazingGrace"))
            return false;
        ServerPlayer *sq = room->findPlayerBySkillName(objectName());
        if(!sq || room->getTag("Jiayao").toInt() == 1 || !sq->askForSkillInvoke(objectName()))
            return false;
        room->setTag("Jiayao", 1);
        sq->drawCards(1);
        room->playSkillEffect(objectName());
        QVariantList ag_list = room->getTag("AmazingGrace").toList();
        int a = 0;
        foreach(QVariant card_id, ag_list){
            const Card *card = Sanguosha->getCard(card_id.toInt());
            if(card->inherits("Peach") || card->inherits("Analeptic"))
                a ++;
        }
        RecoverStruct rev;
        rev.recover = a;
        room->recover(sq, rev, true);
        return false;
    }
};

class Fushang: public MasochismSkill{
public:
    Fushang():MasochismSkill("fushang"){
        frequency = Compulsory;
    }

    virtual void onDamaged(ServerPlayer *hedgehog, const DamageStruct &damage) const{
        if(hedgehog->getMaxHP() > 3){
            LogMessage log;
            Room *room = hedgehog->getRoom();
            log.type = "#TriggerSkill";
            log.from = hedgehog;
            log.arg = objectName();
            room->sendLog(log);
            room->playSkillEffect(objectName());
            room->loseMaxHp(hedgehog);
            hedgehog->drawCards(3);
        }
    }
};

YijieCard::YijieCard(){
    will_throw = false;
    target_fixed = false;
}

bool YijieCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return to_select != Self && targets.size() == 0;
}

void YijieCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const{
    room->loseHp(source);
    targets.first()->drawCards(2);
}

class YijieViewAsSkill:public ZeroCardViewAsSkill{
public:
    YijieViewAsSkill():ZeroCardViewAsSkill("yijie"){
    }

    virtual const Card *viewAs() const{
        return new YijieCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YijieCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return false;
    }
};

class Yijie: public TriggerSkill{
public:
    Yijie():TriggerSkill("yijie"){
        events << Death;
        view_as_skill = new YijieViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        QList<ServerPlayer *> targets = room->getAlivePlayers();

        if(targets.isEmpty() || !player->askForSkillInvoke(objectName(), data))
            return false;

        ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());

        room->broadcastSkillInvoke(objectName());
        target->obtainCard(player->wholeHandCards(), false);
        if(target->isWounded()){
            RecoverStruct recover;
            recover.who = target;
            recover.recover = 1;
            room->recover(target, recover, true);
        }
        return false;
    }
};

class Qiangqu:public TriggerSkill{
public:
    Qiangqu():TriggerSkill("qiangqu"){
        events << DamageProceed;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") && damage.to->getGeneral()->isFemale()
            && damage.to->isWounded() && !damage.to->isNude() && player->askForSkillInvoke(objectName(), data)){
            int card_id = room->askForCardChosen(damage.from, damage.to, "he", objectName());
            RecoverStruct re;
            re.card = Sanguosha->getCard(card_id);
            re.who = player;
            room->obtainCard(player, card_id, false);

            LogMessage log;
            log.from = player;
            log.type = "#Qiangqu";
            log.to << damage.to;
            room->sendLog(log);
            room->recover(damage.to, re, true);
            room->playSkillEffect(objectName());
            room->recover(damage.from, re, true);
            return true;
        }
        return false;
    }
};

class Huatian:public TriggerSkill{
public:
    Huatian():TriggerSkill("huatian"){
        events << Damaged << HpRecovered;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == Damaged){
            DamageStruct damage = data.value<DamageStruct>();
            for(int i = 0; i < damage.damage; i++){
                QList<ServerPlayer *> wounders;
                foreach(ServerPlayer *tmp, room->getOtherPlayers(damage.to)){
                    if(tmp->isWounded())
                        wounders << tmp;
                }
                if(!wounders.isEmpty()){
                    room->setPlayerMark(player, "HBTJ", 1);
                    if(!damage.to->askForSkillInvoke(objectName())){
                        room->setPlayerMark(player, "HBTJ", 0);
                        break;
                    }
                    ServerPlayer *target = room->askForPlayerChosen(player, wounders, objectName());
                    room->setPlayerMark(player, "HBTJ", 0);
                    RecoverStruct recovvv;
                    recovvv.who = player;
                    room->playSkillEffect(objectName(), qrand() % 2 + 1);
                    room->recover(target, recovvv, true);
                }
            }
            return false;
        }
        RecoverStruct rec = data.value<RecoverStruct>();
        for(int i = rec.recover; i > 0; i--){
            if(!player->askForSkillInvoke(objectName()))
                break;
            room->setPlayerMark(player, "HBTJ", 2);
            ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
            room->setPlayerMark(player, "HBTJ", 0);

            room->playSkillEffect(objectName(), qrand() % 2 + 3);
            DamageStruct damage;
            damage.from = player;
            damage.to = target;
            room->damage(damage);
        }
        return false;
    }
};

class Shihao: public DrawCardsSkill{
public:
    Shihao():DrawCardsSkill("shihao"){
    }

    virtual int getDrawNum(ServerPlayer *x, int n) const{
        Room *room = x->getRoom();
        if(room->askForSkillInvoke(x, objectName())){
            room->playSkillEffect(objectName());

            x->setFlags(objectName());
            return n - 1;
        }else
            return n;
    }
};

class Shihaodo:public PhaseChangeSkill{
public:
    Shihaodo():PhaseChangeSkill("#shihao-do"){
    }

    virtual bool onPhaseChange(ServerPlayer *zhugui) const{
        if(!zhugui->hasFlag("shihao"))
            return false;
        Room *room = zhugui->getRoom();
        if(zhugui->getPhase() == Player::Play){
            Wiretap *wp = new Wiretap(Card::NoSuit, 0);
            wp->setSkillName("shihao");
            QList<ServerPlayer *> targets;
            foreach(ServerPlayer *target, room->getOtherPlayers(zhugui)){
                if(!target->isProhibited(target, wp))
                    targets << target;
            }
            if(targets.isEmpty())
                return false;
            PlayerStar target = room->askForPlayerChosen(zhugui, targets, "shihao");
            CardUseStruct use;
            use.card = wp;
            use.from = zhugui;
            use.to << target;
            room->useCard(use);
        }
        else if(zhugui->getPhase() == Player::Finish){
            zhugui->drawCards(1);
            QList<int> yiji_cards = zhugui->handCards().mid(zhugui->getHandcardNum() - 1);
            room->askForYiji(zhugui, yiji_cards);
        }

        return false;
    }
};

class Laolian: public TriggerSkill{
public:
    Laolian():TriggerSkill("laolian"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *guigui, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Peach") || use.card->inherits("Analeptic")){
            room->throwCard(use.card);
            if(guigui->askForSkillInvoke(objectName())){
                PlayerStar target = room->askForPlayerChosen(guigui, room->getOtherPlayers(guigui), objectName());
                Ecstasy *ey = new Ecstasy(Card::NoSuit, 0);
                ey->setSkillName(objectName());
                CardUseStruct use;
                use.card = ey;
                use.from = guigui;
                use.to << target;
                room->useCard(use, false);
            }
        }
        return false;
    }
};

class Shemi: public TriggerSkill{
public:
    Shemi():TriggerSkill("shemi"){
        events << PhaseChange << TurnedOver;
    }

    virtual bool trigger(TriggerEvent e, Room* room, ServerPlayer *emperor, QVariant &data) const{
        if(e == PhaseChange){
            if(emperor->getPhase() == Player::Discard &&
               emperor->askForSkillInvoke(objectName(), data)){
                emperor->turnOver();
                return true;
            }
        }
        else{
            if(!emperor->hasFlag("NongQ")){
                int index = emperor->faceUp() ? 2: 1;
                room->playSkillEffect(objectName(), index);
            }
            int x = emperor->getLostHp();
            x = qMax(qMin(x,2),1);
            emperor->drawCards(x);
        }
        return false;
    }
};

class Lizheng: public ClientSkill{
public:
    Lizheng():ClientSkill("lizheng"){
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(to->hasSkill(objectName()) && !to->faceUp())
            return +1;
        else
            return 0;
    }
};

class Nongquan:public PhaseChangeSkill{
public:
    Nongquan():PhaseChangeSkill("nongquan$"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getKingdom() == "guan" && !target->hasLordSkill(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *otherguan) const{
        Room *room = otherguan->getRoom();
        if(otherguan->getPhase() != Player::Draw)
            return false;
        ServerPlayer *head = room->getLord();
        if(head->hasLordSkill(objectName()) && otherguan->getKingdom() == "guan"
           && otherguan->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());
            room->setPlayerFlag(head, "NongQ");
            head->turnOver();
            room->setPlayerFlag(head, "-NongQ");
            return true;
        }
        return false;
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

    General *tongwei = new General(this, "tongwei", "min", 3);
    tongwei->addSkill(new Dalang);
    tongwei->addSkill(new Qianshui);

    General *songqing = new General(this, "songqing", "min", 3);
    songqing->addSkill(new Sheyan);
    songqing->addSkill(new Jiayao);

    General *dingdesun = new General(this, "dingdesun", "jiang", 6);
    dingdesun->addSkill(new Skill("beizhan"));
    dingdesun->addSkill(new Fushang);

    General *songwan = new General(this, "songwan", "kou");
    songwan->addSkill(new Yijie);

    General *zhoutong = new General(this, "zhoutong", "kou", 3);
    zhoutong->addSkill(new Qiangqu);
    zhoutong->addSkill(new Huatian);

    General *zhugui = new General(this, "zhugui", "kou");
    zhugui->addSkill("#losthp_1");
    zhugui->addSkill(new Shihao);
    zhugui->addSkill(new Shihaodo);
    related_skills.insertMulti("shihao", "#shihao-do");
    zhugui->addSkill(new Laolian);

    General *zhaoji = new General(this, "zhaoji$", "guan", 3);
    zhaoji->addSkill(new Shemi);
    zhaoji->addSkill(new Lizheng);
    zhaoji->addSkill(new Nongquan);

    addMetaObject<SixiangCard>();
    addMetaObject<LinmoCard>();
    addMetaObject<ZhaixingCard>();
    addMetaObject<BinggongCard>();
    addMetaObject<SheyanCard>();
    addMetaObject<YijieCard>();
}

ADD_PACKAGE(Hare)
