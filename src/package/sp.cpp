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
    room->playSkillEffect("baoquan", qrand() % 2 + 5);
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
                        QString choice = !target->isWounded() ? "draw2card" :
                                         room->askForChoice(lusashi, objectName(), "draw2card+recover1hp");
                        if(choice == "draw2card")
                            target->drawCards(2);
                        else{
                            RecoverStruct rev;
                            rev.who = lusashi;
                            room->recover(target, rev);
                        }
                        break;
                    }
                    default:
                        room->askForUseCard(lusashi, "@@baoquan", "@baoquan", true);
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
                log.type = "#Tigerhide";
                room->sendLog(log);
                room->acquireSkill(player, "tigerou");
            }
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

SPPackage::SPPackage()
    :Package("sp")
{
    General *luda = new General(this, "luda", "di");
    luda->addSkill(new Baoquan);

    General *tora = new General(this, "tora", "god", 4, false);
    tora->addSkill(new Strike);
    tora->addSkill(new Lift);
    tora->addSkill(new Exterminate);
    tora->addSkill(new MarkAssignSkill("@kacha", 1));
    related_skills.insertMulti("exterminate", "#@kacha-1");
    skills << new Tigerou;

    addMetaObject<BaoquanCard>();
}

//ADD_PACKAGE(SP)
