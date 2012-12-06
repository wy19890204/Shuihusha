#include "standard.h"
#include "common-skillcards.h"
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
    effect.to->getRoom()->recover(effect.to, recover, true);
}

SacrificeCard::SacrificeCard(){
    will_throw = false;
    target_fixed = true;
}

void SacrificeCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(!Config.EnableReincarnation)
        return;
    QStringList deathnote = room->getTag("DeadPerson").toString().split("+");
    if(deathnote.isEmpty())
        return;
    QString choice = deathnote.length() == 1 ? deathnote.first() :
                     room->askForChoice(source, "sacrifice", deathnote.join("+"));
    ServerPlayer *target = room->findPlayer(choice, true);
    const Card *card = room->askForCardShow(source, target, "sacrifice");
    target->obtainCard(card, false);
}

UbunbCard::UbunbCard(){
}

bool UbunbCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void UbunbCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QStringList all_generals = Sanguosha->getLimitedGeneralNames();
    qShuffle(all_generals);
    QStringList choices = all_generals.mid(0, 4);
    QString name = room->askForGeneral(effect.from, choices, "guansheng");
    room->transfigure(effect.to, name, false, true, effect.to->getGeneralName());
}

UbuncCard::UbuncCard(){
}

bool UbuncCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.length() < 2;
}

void UbuncCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(targets.length() == 1){
        QString kingdom = room->askForKingdom(source);
        //QString kingdom = room->askForChoice(source, "ubunc", "guan+jiang+min+kou+god");
        room->setPlayerProperty(targets.first(), "kingdom", kingdom);
    }
    else{
        ServerPlayer *first = targets.first();
        room->swapSeat(first, targets.last());
    }
}

UbundCard::UbundCard(){
}

bool UbundCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return true;
}

bool UbundCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() <= 1;
}

void UbundCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(targets.isEmpty()){
        QStringList genlist = Sanguosha->getLimitedGeneralNames();
        qShuffle(genlist);
        genlist = genlist.mid(0, 4);
        QString general = room->askForGeneral(source, genlist);
        QStringList choices;
        foreach(const SkillClass *skill, Sanguosha->getGeneral(general)->getVisibleSkillList())
            choices << skill->objectName();
        if(!choices.isEmpty()){
            QString ski = choices.count() == 1 ? choices.first() :
                          room->askForChoice(source, "ubund", choices.join("+"));
            room->acquireSkill(source, ski);
        }
    }
    else{
        ServerPlayer *target = targets.first();
        if(target == source){
            QStringList skills = source->getVisibleSkillList("ubun");
            if(!skills.isEmpty()){
                QString ski = room->askForChoice(source, "ubund", skills.join("+"));
                room->detachSkillFromPlayer(source, ski);
            }
        }
        else{
            QStringList skills = target->getVisibleSkillList("ubun");
            if(!skills.isEmpty()){
                QString ski = room->askForChoice(source, "ubund", skills.join("+"));
                room->acquireSkill(source, ski);
            }
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
