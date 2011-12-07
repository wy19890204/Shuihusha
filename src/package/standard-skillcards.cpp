#include "standard.h"
#include "standard-skillcards.h"
#include "room.h"
#include "clientplayer.h"
#include "engine.h"
#include "client.h"
#include "settings.h"

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

GuicaiCard::GuicaiCard(){
    target_fixed = true;
    will_throw = false;
}

void GuicaiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{

}

CheatCard::CheatCard(){
    target_fixed = true;
    will_throw = false;
}

void CheatCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(Config.FreeChoose)
        room->obtainCard(source, subcards.first());
}
