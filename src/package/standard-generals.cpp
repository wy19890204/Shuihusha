#include "general.h"
#include "standard.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "serverplayer.h"
#include "standard-generals.h"
#include "room.h"

GanlinCard::GanlinCard(){
    will_throw = false;
}

void GanlinCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();

    room->obtainCard(target, this, false);
    int n = source->getLostHp() - source->getHandcardNum();
    if(n > 0 && source->askForSkillInvoke("ganlin")){
        source->drawCards(n);
        room->setPlayerFlag(source, "Ganlin");
    }
};

class GanlinViewAsSkill:public ViewAsSkill{
public:
    GanlinViewAsSkill():ViewAsSkill("ganlin"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasFlag("Ganlin");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        GanlinCard *ganlin_card = new GanlinCard;
        ganlin_card->addSubcards(cards);
        return ganlin_card;
    }
};

class Ganlin: public PhaseChangeSkill{
public:
    Ganlin():PhaseChangeSkill("ganlin"){
        view_as_skill = new GanlinViewAsSkill;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool onPhaseChange(ServerPlayer *p) const{
        if(p->getPhase() == Player::NotActive){
            Room *room = p->getRoom();
            room->setPlayerFlag(p, "-Ganlin");
        }
        return false;
    }
};

JuyiCard::JuyiCard(){
    once = true;
    target_fixed = true;
}

void JuyiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    ServerPlayer *song = room->getLord();
    if(!song->hasLordSkill("juyi") || song == source)
        return;
    if(song->isKongcheng() && source->isKongcheng())
        return;
    if(room->askForChoice(song, "jui", "agree+deny") == "agree"){
        DummyCard *card1 = source->wholeHandCards();
        DummyCard *card2 = song->wholeHandCards();
        if(card1){
            room->obtainCard(song, card1, false);
            delete card1;
        }
        room->getThread()->delay();

        if(card2){
            room->obtainCard(source, card2, false);
            delete card2;
        }
        LogMessage log;
        log.type = "#Juyi";
        log.from = source;
        log.to << song;
        room->sendLog(log);
    }
}

class JuyiViewAsSkill: public ZeroCardViewAsSkill{
public:
    JuyiViewAsSkill():ZeroCardViewAsSkill("jui"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasLordSkill("juyi")
                && player->getKingdom() == "kou"
                && !player->hasUsed("JuyiCard");
    }

    virtual const Card *viewAs() const{
        return new JuyiCard;
    }
};

class Juyi: public GameStartSkill{
public:
    Juyi():GameStartSkill("juyi$"){

    }

    virtual void onGameStart(ServerPlayer *player) const{
        if(!player->isLord())
            return;
        Room *room = player->getRoom();
        foreach(ServerPlayer *tmp, room->getAlivePlayers()){
            room->attachSkillToPlayer(tmp, "jui");
        }
    }
};

class Baoguo:public TriggerSkill{
public:
    Baoguo():TriggerSkill("baoguo"){
        events << Predamaged << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent evt, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        QList<ServerPlayer *> ducks = room->findPlayersBySkillName(objectName());
        if(ducks.isEmpty())
            return false;
        foreach(ServerPlayer *duck, ducks){
            if(evt == Damaged){
                if(duck == player && duck->isWounded() && duck->askForSkillInvoke(objectName())){
                    if(duck->getMark("baoguo") == 0)
                        room->playSkillEffect(objectName(), 1);
                    duck->drawCards(duck->getLostHp());
                }
                duck->setMark("baoguo", 0);
            }
            else if(duck != player && !duck->isNude() && damage.damage > 0
                && room->askForCard(duck, ".", "@baoguo:" + player->objectName() + ":" + QString::number(damage.damage), data, CardDiscarded)){
                room->playSkillEffect(objectName(), 2);
                LogMessage log;
                log.type = "#Baoguo";
                log.from = duck;
                log.to << damage.to;
                log.arg = objectName();
                log.arg2 = QString::number(damage.damage);
                room->sendLog(log);

                damage.to = duck;
                duck->setMark("baoguo", 1);
                room->damage(damage);
                return true;
            }
        }
        return false;
    }
};

StandardPackage::StandardPackage()
    :Package("standard")
{
    General *songjiang = new General(this, "songjiang$", "kou");
    songjiang->addSkill(new Ganlin);
    songjiang->addSkill(new Juyi);
    skills << new JuyiViewAsSkill;

    General *lujunyi = new General(this, "lujunyi", "guan");
    lujunyi->addSkill(new Baoguo);

    addMetaObject<GanlinCard>();
    addMetaObject<JuyiCard>();
}

ADD_PACKAGE(Standard)
