#include "standard.h"
#include "standard-skillcards.h"
#include "room.h"
#include "clientplayer.h"
#include "engine.h"
#include "client.h"
#include "settings.h"
#include "carditem.h"

CheatCard::CheatCard(){
    target_fixed = true;
    will_throw = false;
}

void CheatCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(Config.FreeChoose)
        room->obtainCard(source, subcards.first());
}

UbunaCard::UbunaCard(){
    target_fixed = true;
}

void UbunaCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    int num = source->getMaxHP();
    QString numstring = room->askForChoice(source, "ubuna", "3+4+5+back+next");
    if(numstring == "3")
        num = 3;
    else if(numstring == "4")
        num = 4;
    else if(numstring == "5")
        num = 5;
    else if(numstring == "back")
        num --;
    else
        num ++;
    room->setPlayerProperty(source, "maxhp", num);
}

UbuncCard::UbuncCard(){
}

bool UbuncCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void UbuncCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QString kingdom = room->askForChoice(effect.from, "ubunc", "guan+jiang+min+kou+god");
    room->setPlayerProperty(effect.to, "kingdom", kingdom);
}

UbundCard::UbundCard(){
}

bool UbundCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return to_select != Self;
}

bool UbundCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() <= 1;
}

void UbundCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(targets.isEmpty()){
        QStringList skills = source->getVisSkist("ubun");
        if(skills.isEmpty())
            return;
        QString ski = room->askForChoice(source, "ubund", skills.join("+"));
        room->detachSkillFromPlayer(source, ski);
    }
    else{
        ServerPlayer *target = targets.first();
        QStringList skills = target->getVisSkist("ubun");
        if(!skills.isEmpty()){
            QString ski = room->askForChoice(source, "ubund", skills.join("+"));
            room->acquireSkill(source, ski);
        }
    }
}
