#include "fcdc.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"

XunlieCard::XunlieCard(){
}

bool XunlieCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int i = 0;
    foreach(const Player *player, Self->getSiblings()){
        if(player->getHandcardNum() >= i){
            i = player->getHandcardNum();
        }
    }
    return targets.isEmpty() && !to_select->isKongcheng() && to_select->getHandcardNum() == i && to_select != Self;
}

void XunlieCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    int i = 0;
    const Card *card = effect.to->getRandomHandCard();
    effect.from->obtainCard(card, false);
    i ++;
    if(!effect.to->isKongcheng() && room->askForChoice(effect.from, "xuelie", "get+cancel") == "get"){
        card = effect.to->getRandomHandCard();
        effect.from->obtainCard(card, false);
        i ++;
    }
    if(i == 1)
        effect.from->drawCards(1);
    room->setEmotion(effect.to, "bad");
    room->setEmotion(effect.from, "good");
}

class XunlieViewAsSkill: public OneCardViewAsSkill{
public:
    XunlieViewAsSkill():OneCardViewAsSkill("xunlie"){
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        XunlieCard *card = new XunlieCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }

    virtual bool viewFilter(const CardItem *to_selec) const{
        return to_selec->getCard()->inherits("EquipCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@xunlie";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
};

class Xunlie:public PhaseChangeSkill{
public:
    Xunlie():PhaseChangeSkill("xunlie"){
        view_as_skill = new XunlieViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *xiezhen) const{
        if(xiezhen->getPhase() == Player::Draw){
            Room *room = xiezhen->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(xiezhen);
            foreach(ServerPlayer *player, other_players){
                if(!player->isKongcheng()){
                    can_invoke = true;
                    break;
                }
            }
            if(!can_invoke)
                return false;
            QList<const Card *> cards = xiezhen->getCards("he");
            foreach(const Card *cd, cards){
                if(cd->inherits("EquipCard")){
                    if(room->askForUseCard(xiezhen, "@@xunlie", "@xunlie"))
                        return true;
                    break;
                }
            }
        }
        return false;
    }
};

class Qinxin: public TriggerSkill{
public:
    Qinxin():TriggerSkill("qinxin"){
        events << PhaseChange;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        if(player->getPhase() != Player::Start || !player->askForSkillInvoke(objectName()))
            return false;
        Card::Suit suit = room->askForSuit(player, objectName());
        JudgeStruct judge;
        judge.pattern = QRegExp("(.*):(.*):(.*)");
        judge.reason = objectName();
        judge.who = player;
        room->judge(judge);

        if(judge.card->getSuit() == suit){
            RecoverStruct rec;
            rec.who = player;
            room->recover(player, rec, true);
        }
        else
            player->obtainCard(judge.card);

        return false;
    }
};

YinjianCard::YinjianCard(){

}

bool YinjianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const{
    if(!to_select->getGeneral()->isMale() || targets.length() > 1)
        return false;
    if(targets.isEmpty())
        return true;
    else{
        QString kingdom = targets.first()->getKingdom();
        return to_select->getKingdom() != kingdom;
    }
    return false;
}

bool YinjianCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const{
    return targets.length() == 2;
}

void YinjianCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> players = targets;
    QStringList to_select;
    to_select << targets.first()->getGeneralName() << targets.last()->getGeneralName();
    QString select = room->askForChoice(source, "yinjian", to_select.join("+"));

    ServerPlayer *target = room->findPlayer(select);
    players.removeOne(target);
    ServerPlayer *other = players.first();

    QList<const Card *> handcards = source->getHandcards();
    if(source->getHandcardNum() <= 2){
        foreach(const Card *cd, handcards)
            room->moveCardTo(cd, target, Player::Hand, false);
    }
    else{
        int i;
        for(i=0; i<2; i++){
            int id = room->askForCardChosen(source, source, "h", "yinjian");
            room->moveCardTo(Sanguosha->getCard(id), target, Player::Hand, false);
        }
    }

    int id = room->askForCardChosen(target, target, "h", "yinjian");
    room->moveCardTo(Sanguosha->getCard(id), other, Player::Hand, false);

}

class Yinjian: public ZeroCardViewAsSkill{
public:
    Yinjian():ZeroCardViewAsSkill("yinjian"){
    }

    virtual const Card *viewAs() const{
        return new YinjianCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YinjianCard");
    }
};

FCDCPackage::FCDCPackage()
    :Package("FCDC")
{
    General *xiezhen = new General(this, "xiezhen", "min");
    xiezhen->addSkill(new Xunlie);

    addMetaObject<XunlieCard>();
}

ADD_PACKAGE(FCDC);
