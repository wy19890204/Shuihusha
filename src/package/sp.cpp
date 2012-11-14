#include "standard.h"
#include "skill.h"
#include "sp.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"

BaoquanCard::BaoquanCard(){
    mute = true;
}

void BaoquanCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this, source);
    room->playSkillEffect(skill_name, qrand() % 2 + 5);
    int fist = getSubcards().count();

    DamageStruct damage;
    damage.damage = fist;
    damage.from = source;
    damage.to = targets.first();
    room->damage(damage);
}

class BaoquanViewAsSkill: public ViewAsSkill{
public:
    BaoquanViewAsSkill():ViewAsSkill("baoquan"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *to_select) const{
        return to_select->getCard()->inherits("EquipCard");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        BaoquanCard *card = new BaoquanCard;
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@baoquan";
    }
};

class Baoquan: public TriggerSkill{
public:
    Baoquan():TriggerSkill("baoquan"){
        events << PhaseChange << Damage;
        view_as_skill = new BaoquanViewAsSkill;
    }

    virtual QString getDefaultChoice(ServerPlayer *player) const{
        if(player->getLostHp() > 1)
            return "recover1hp";
        else
            return "draw2card";
    }

    virtual bool trigger(TriggerEvent e, Room* room, ServerPlayer *lusashi, QVariant &data) const{
        if(e == PhaseChange){
            if(lusashi->getPhase() == Player::RoundStart)
                room->setPlayerMark(lusashi, "@fist", 0);
            else if(lusashi->getPhase() == Player::Finish){
                int fist = lusashi->getMark("@fist");
                if(fist < 1)
                    return false;
                if(fist == 1 || fist == 2){
                    if(!lusashi->askForSkillInvoke(objectName())){
                        room->setPlayerMark(lusashi, "@fist", 0);
                        return false;
                    }
                }
                switch(fist){
                case 1:{
                        room->playSkillEffect(objectName(), qrand() % 2 + 1);
                        lusashi->drawCards(1);
                        break;
                    }
                case 2:{
                        room->playSkillEffect(objectName(), qrand() % 2 + 3);
                        PlayerStar target = room->askForPlayerChosen(lusashi, room->getAllPlayers(), objectName());
                        QString choice = target->isWounded() ?
                                         room->askForChoice(lusashi, objectName(), "draw2card+recover1hp", QVariant::fromValue(target)) :
                                         "draw2card";
                        if(choice == "draw2card")
                            target->drawCards(2);
                        else{
                            RecoverStruct rev;
                            rev.who = lusashi;
                            room->recover(target, rev);
                        }
                        break;
                    }
                default:{
                        room->askForUseCard(lusashi, "@@baoquan", "@baoquan", true);
                        if(fist >= 5){
                            LogMessage log;
                            log.type = "#RemoveHidden";
                            log.from = lusashi;
                            room->sendLog(log);
                            room->acquireSkill(lusashi, "zuohua");
                        }
                    }
                }
                room->setPlayerMark(lusashi, "@fist", 0);
            }
        }
        else{
            if(lusashi->getPhase() == Player::NotActive)
                return false;
            DamageStruct damage = data.value<DamageStruct>();
            lusashi->gainMark("@fist", damage.damage);
        }
        return false;
    }
};

class StrikeViewAsSkill: public ViewAsSkill{
public:
    StrikeViewAsSkill():ViewAsSkill("strike"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getPhase() == Player::Play && Slash::IsAvailable(player);
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 2 && !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        const Card *first = cards.at(0)->getFilteredCard();
        const Card *second = cards.at(1)->getFilteredCard();

        Card::Suit suit = Card::NoSuit;
        if(first->isBlack() && second->isBlack())
            suit = Card::Spade;
        else if(first->isRed() && second->isRed())
            suit = Card::Heart;

        Slash *slash = new Slash(suit, 0);
        slash->setSkillName(objectName());
        slash->addSubcard(first);
        slash->addSubcard(second);

        return slash;
    }
};

class Strike: public TriggerSkill{
public:
    Strike():TriggerSkill("strike"){
        events << CardUsed;
        view_as_skill = new StrikeViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Slash") && use.card->isVirtualCard() && use.card->getSkillName() == objectName()){
            LogMessage log;
            log.type = "#Strike";
            log.from = use.from;
            log.arg = objectName();
            room->sendLog(log);
        }
        return false;
    }
};

class Lift: public TriggerSkill{
public:
    Lift(): TriggerSkill("lift"){
        events << SlashMissed;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();

        if(player->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            player->turnOver();
            LogMessage log;
            log.type = "#Lift";
            log.from = player;
            log.to << effect.to;
            log.arg = objectName();
            room->sendLog(log);

            if(effect.slash->getSkillName() == "strike")
                player->addMark("tola");
            room->slashResult(effect, NULL);
            if(player->getMark("tola") >= 3){
                log.type = "#RemoveHidden";
                room->sendLog(log);
                room->acquireSkill(player, "tigerou");
            }
            return true;
        }

        return false;
    }
};

class Exterminate: public TriggerSkill{
public:
    Exterminate():TriggerSkill("exterminate"){
        events << DamageConclude;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *hanae, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(hanae == damage.to || !hanae->hasMark("@kacha"))
            return false;
        if(hanae->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            QList<ServerPlayer *> targets;
            foreach(ServerPlayer *tmp, room->getAllPlayers()){
                if(damage.to->distanceTo(tmp) == 1)
                    targets << tmp;
            }
            LogMessage log;
            log.type = "#Exterminate";
            log.from = hanae;
            log.to = targets;
            log.arg = objectName();
            room->sendLog(log);

            room->loseMaxHp(hanae);
            hanae->loseMark("@kacha");
            DamageStruct dama = damage;
            foreach(ServerPlayer *tmp, targets){
                dama.to = tmp;
                room->damage(dama);
            }
        }
        return false;
    }
};

class Tigerou: public TriggerSkill{
public:
    Tigerou():TriggerSkill("tigerou"){
        events << Predamage << CardEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == CardEffected){
            if(!player->hasSkill(objectName()))
                return false;
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.card->inherits("SavageAssault")){
                LogMessage log;
                log.type = "#SkillNullify";
                log.from = player;
                log.arg = objectName();
                log.arg2 = "savage_assault";
                room->sendLog(log);
                return true;
            }else
                return false;
        }
        else{
            if(player->hasSkill(objectName()))
                return false;
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.card && damage.card->inherits("SavageAssault")){
                ServerPlayer *menghuo = room->findPlayerBySkillName(objectName());
                if(menghuo){
                    LogMessage log;
                    log.type = "#TigerTransfer";
                    log.from = menghuo;
                    log.to << damage.to;
                    log.arg = player->getGeneralName();
                    log.arg2 = objectName();
                    room->sendLog(log);

                    room->playSkillEffect(objectName());

                    damage.from = menghuo;
                    room->damage(damage);
                    return true;
                }
            }
        }
        return false;
    }
};

LuanjunCard::LuanjunCard(){
    mute = true;
}

bool LuanjunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isNude() && to_select != Self;
}

void LuanjunCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->playSkillEffect(skill_name, qrand() % 2 + 3);
    int card_id = room->askForCardChosen(effect.from, effect.to, "he", skill_name);
    room->throwCard(card_id, effect.to, effect.from);
    room->setEmotion(effect.to, "bad");
}

class LuanjunViewAsSkill: public ZeroCardViewAsSkill{
public:
    LuanjunViewAsSkill():ZeroCardViewAsSkill("luanjun"){
    }

    virtual const Card *viewAs() const{
        return new LuanjunCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@luanjun";
    }
};

class Luanjun: public TriggerSkill{
public:
    Luanjun():TriggerSkill("luanjun"){
        view_as_skill = new LuanjunViewAsSkill;
        events << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &) const{
        QList<ServerPlayer *> edogawa = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *conan, edogawa){
            if(player->getKingdom() == "jiang")
                room->askForUseCard(conan, "@@luanjun", "@luanjun", true);
            if(conan == player || conan->getKingdom() == player->getKingdom())
                continue;
            if(player->isAlive() && conan->askForSkillInvoke(objectName(), QVariant::fromValue((PlayerStar)player))){
                room->playSkillEffect(objectName(), qrand() % 2 + 1);
                player->drawCards(1);
            }
        }
        return false;
    }
};

class Qingshang:public TriggerSkill{
public:
    Qingshang():TriggerSkill("qingshang"){
        events << HpRecovered;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &) const{
        if(player->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName(), 1);
            ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());

            JudgeStruct judge;
            judge.reason = objectName();
            judge.who = player;
            room->judge(judge);
            Card::Color mycolor = judge.card->getColor();

            room->getThread()->delay();

            judge.who = target;
            judge.pattern = mycolor == Card::Red ? QRegExp("(.*):(heart|diamond):(.*)"): QRegExp("(.*):(club|spade):(.*)");
            judge.good = false;
            room->judge(judge);
            if(judge.card->getColor() == mycolor){
                room->playSkillEffect(objectName(), 2);
                DamageStruct mmm;
                mmm.from = player;
                mmm.to = target;
                room->damage(mmm);
            }
            else
                room->playSkillEffect(objectName(), 3);
        }
        return false;
    }
};

/*
class Shuntian: public TriggerSkill{
public:
    Shuntian():TriggerSkill("shuntian"){
        events << GameStart;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room* rom, ServerPlayer *player, QVariant &data) const{
        QString kim;
        if(!player->isLord())
            kim = rom->getLord()->getKingdom();
        rom->setPlayerProperty(player, "kingdom", kim);
        return false;
    }
};

YuzhongCard::YuzhongCard(){
    mute = true;
}

bool YuzhongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int num = Self->getKingdoms();
    return targets.length() < num;
}

bool YuzhongCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    int num = Self->getKingdoms();
    return targets.length() <= num;
}

void YuzhongCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->drawCards(1);
}

class YuzhongViewAsSkill: public ZeroCardViewAsSkill{
public:
    YuzhongViewAsSkill():ZeroCardViewAsSkill("yuzhong"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@yuzhong";
    }

    virtual const Card *viewAs() const{
        return new YuzhongCard;
    }
};

class Yuzhong: public TriggerSkill{
public:
    Yuzhong():TriggerSkill("yuzhong"){
        events << Death;
        view_as_skill = new YuzhongViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        int num = room->getKingdoms();
        DamageStar damage = data.value<DamageStar>();
        if(damage && damage->from != damage->to && damage->from->hasSkill(objectName())){
            ServerPlayer *source = damage->from;
            QString choice = room->askForChoice(source, objectName(), "hp+card+cancel");
            if(choice == "cancel")
                return false;
            LogMessage log;
            log.type = "#InvokeSkill";
            log.from = source;
            log.arg = objectName();
            room->sendLog(log);
            if(choice == "hp"){
                room->playSkillEffect(objectName(), 1);
                RecoverStruct rev;
                rev.who = source;
                rev.recover = num;
                room->recover(room->getLord(), rev);
            }
            else if(choice == "card"){
                room->playSkillEffect(objectName(), 2);
                room->getLord()->drawCards(num);
            }
        }
        return false;
    }
};

class Yuzhong2: public TriggerSkill{
public:
    Yuzhong2():TriggerSkill("#yuzh0ng"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.to->isLord())
            return false;
        QString choice = room->askForChoice(player, "yuzhong", "all+me+cancel");
        if(choice == "cancel")
            return false;
        LogMessage log;
        log.type = "#InvokeSkill";
        log.from = player;
        log.arg = "yuzhong";
        room->sendLog(log);
        if(choice == "all"){
            if(!room->askForUseCard(player, "@@yuzhong", "@yuzhong"))
                choice = "me";
            else
                room->playSkillEffect(objectName(), 3);
        }
        if(choice == "me"){
            room->playSkillEffect(objectName(), 4);
            LogMessage log;
            log.type = "#InvokeSkill";
            log.from = player;
            log.arg = "yuzhong";
            room->sendLog(log);
            player->drawCards(2);
        }

        return false;
    }
};

*/
SPPackage::SPPackage()
    :GeneralPackage("sp")
{
    General *luda = new General(this, "luda", "guan");
    luda->addSkill(new Baoquan);

    General *tora = new General(this, "tora", "god", 4, false);
    tora->addSkill(new Strike);
    tora->addSkill(new Lift);
    tora->addSkill(new Exterminate);
    tora->addSkill(new MarkAssignSkill("@kacha", 1));
    related_skills.insertMulti("exterminate", "#@kacha-1");
    skills << new Tigerou;

    General *keyin = new General(this, "keyin", "jiang", 3);
    keyin->addSkill(new Luanjun);
    keyin->addSkill(new Qingshang);
/*
    General *jiangsong = new General(this, "jiangsong", "guan");
    jiangsong->addSkill(new Yuzhong);
    jiangsong->addSkill(new Yuzhong2);
    related_skills.insertMulti("yuzhong", "#yuzh0ng");
    jiangsong->addSkill(new Shuntian);

    addMetaObject<YuzhongCard>();
*/
    addMetaObject<BaoquanCard>();
    addMetaObject<LuanjunCard>();
}

ADD_PACKAGE(SP)
