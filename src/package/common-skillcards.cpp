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

FreeRegulateCard::FreeRegulateCard(){
    will_throw = false;
    mute = true;
}

bool FreeRegulateCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

bool FreeRegulateCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() <= 1;
}

void FreeRegulateCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->setPlayerStatistics(source, "cheat", 1);
    if(targets.isEmpty()){
        if(getSubcards().isEmpty()){
            if(!room->getDiscardPile().isEmpty()){
                room->playSkillEffect(skill_name, 4);
                int card_id = room->getDiscardPile().first();
                room->obtainCard(source, card_id);
            }
        }
        else{
            room->playSkillEffect(skill_name, 1);
            room->throwCard(this, source);
        }
    }
    else{
        PlayerStar target = targets.first();
        if(getSubcards().isEmpty()){
            if(target != source){
                room->playSkillEffect(skill_name, 3);
                room->setPlayerFlag(source, "loot");
                int card_id = room->askForCardChosen(source, target, "hejp", "free-regulate");
                room->obtainCard(source, card_id);
                room->setPlayerFlag(source, "-loot");
            }
            else
                room->gameOver(source->objectName());
        }
        else{
            if(target == source){
                foreach(int i, getSubcards()){
                    const Card *card = Sanguosha->getCard(i);
                    if(card->isEquipped()){
                        room->playSkillEffect(skill_name, 5);
                        room->obtainCard(source, card);
                    }
                }
                if(getSubcards().length() == 1){
                    const Card *card = Sanguosha->getCard(getSubcards().first());
                    for(int i = 0; ;i++){
                        const Card *c = Sanguosha->getCard(i);
                        if(!c)
                            break;
                        if(c != card){
                            if(c->objectName() == card->objectName() ||
                                    (c->getSubtype() == "offensive_horse" && card->getSubtype() == "offensive_horse") ||
                                    (c->getSubtype() == "defensive_horse" && card->getSubtype() == "defensive_horse")){
                                room->playSkillEffect(skill_name, 6);
                                room->obtainCard(source, c);
                                break;
                            }
                        }
                    }
                }
            }
            else{
                room->playSkillEffect(skill_name, 2);
                room->obtainCard(target, this, false);
            }
        }
    }
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
                     room->askForChoice(source, "sacrifice", deathnote.join("+"), room->getTag("DeadPerson"));
    ServerPlayer *target = room->findPlayer(choice, true);
    const Card *card = room->askForCardShow(source, target, "sacrifice");
    target->obtainCard(card, false);
}

UbunbCard::UbunbCard(){
}

bool UbunbCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.length() < 2;
}

void UbunbCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(targets.length() == 1){
        QStringList all_generals = Sanguosha->getLimitedGeneralNames();
        qShuffle(all_generals);
        QStringList choices = all_generals.mid(0, 4);
        QString name = room->askForGeneral(source, choices, "guansheng");
        room->transfigure(targets.first(), name, false, true);
    }
    else{
        QString role = targets.last()->getRole();
        targets.last()->setRole(targets.first()->getRole());
        targets.first()->setRole(role);
        room->broadcastProperty(targets.first(), "role");
        room->broadcastProperty(targets.last(), "role");
    }
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
