#include "standard.h"
#include "standard-skillcards.h"
#include "room.h"
#include "clientplayer.h"
#include "engine.h"
#include "client.h"
#include "settings.h"
#include "carditem.h"

QingnangCard::QingnangCard(){
    once = true;
}

bool QingnangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->isWounded();
}

bool QingnangCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.value(0, Self)->isWounded();
}

void QingnangCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    ServerPlayer *target = targets.value(0, source);

    CardEffectStruct effect;
    effect.card = this;
    effect.from = source;
    effect.to = target;

    room->cardEffect(effect);
}

void QingnangCard::onEffect(const CardEffectStruct &effect) const{
    RecoverStruct recover;
    recover.card = this;
    recover.who = effect.from;
    effect.to->getRoom()->recover(effect.to, recover);
}

CheatCard::CheatCard(){
    target_fixed = true;
    will_throw = false;
}

void CheatCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(Config.FreeChoose){
        room->broadcastInvoke("playAudio", "cheat");
        room->obtainCard(source, subcards.first());
    }
}

ChangeCard::ChangeCard(){
    target_fixed = true;
}

void ChangeCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(Config.FreeChoose){
        QString name = Self->tag["GeneralName"].toString();
        room->transfigure(source, name, false, true);
    }
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
    room->setPlayerProperty(source, "maxhp", qAbs(num));
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

UbuneCard::UbuneCard(){
    will_throw = false;
}

bool UbuneCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    const Card *card = Sanguosha->getCard(this->getSubcards().first());
    if(card->inherits("DelayedTrick"))
        if(to_select->getJudgingArea().contains(Sanguosha->getCard(this->getSubcards().first())))
            return false;
    return targets.isEmpty();
}

void UbuneCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    const Card *card = Sanguosha->getCard(this->getSubcards().first());
    if(card->inherits("DelayedTrick")){
        room->moveCardTo(card, effect.to, Player::Judging);
    }
    else{
        const EquipCard *equipped = qobject_cast<const EquipCard *>(card);
        equipped->use(room, effect.to, QList<ServerPlayer *>());
    }
}
