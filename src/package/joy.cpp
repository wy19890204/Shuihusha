#include "joy.h"
#include "engine.h"

Shit::Shit(Suit suit, int number):BasicCard(suit, number){
    setObjectName("shit");

    target_fixed = true;
}

QString Shit::getSubtype() const{
    return "disgusting_card";
}

void Shit::onMove(const CardMoveStruct &move) const{
    ServerPlayer *from = move.from;
    if(from && move.from_place == Player::Hand &&
       from->getRoom()->getCurrent() == move.from
       && (move.to_place == Player::DiscardedPile || move.to_place == Player::Special)
       && move.to == NULL
       && from->isAlive()){

        LogMessage log;
        log.card_str = getEffectIdString();
        log.from = from;

        Room *room = from->getRoom();

        if(getSuit() == Spade){
            log.type = "$ShitLostHp";
            room->sendLog(log);

            room->loseHp(from);

            return;
        }

        DamageStruct damage;
        damage.from = damage.to = from;
        damage.card = this;

        switch(getSuit()){
        case Club: damage.nature = DamageStruct::Thunder; break;
        case Heart: damage.nature = DamageStruct::Fire; break;
        default:
            damage.nature = DamageStruct::Normal;
        }

        log.type = "$ShitDamage";
        room->sendLog(log);

        room->damage(damage);
    }
}

bool Shit::HasShit(const Card *card){
    if(card->isVirtualCard()){
        QList<int> card_ids = card->getSubcards();
        foreach(int card_id, card_ids){
            const Card *c = Sanguosha->getCard(card_id);
            if(c->objectName() == "shit")
                return true;
        }

        return false;
    }else
        return card->objectName() == "shit";
}

Stink::Stink(Suit suit, int number):BasicCard(suit, number){
    setObjectName("stink");
    target_fixed = true;
}

QString Stink::getSubtype() const{
    return "disgusting_card";
}

QString Stink::getEffectPath(bool is_male) const{
    return "audio/card/common/stink.ogg";
}

void Stink::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    ServerPlayer *nextfriend = targets.isEmpty() ? source->getNextAlive() : targets.first();
    room->setEmotion(nextfriend, "bad");
    const Card *pipi = room->askForCard(nextfriend, "Jink,Assassinate", "@haochou:" + source->objectName(), QVariant::fromValue((PlayerStar)source));
    LogMessage log;
    log.from = nextfriend;

    if(!pipi){
        log.type = "#StinkSuccess";
        log.to << nextfriend->getNextAlive();
        room->sendLog(log);
        room->swapSeat(nextfriend, nextfriend->getNextAlive());
    }
    else if(!pipi->inherits("Jink")){
        DamageStruct damage;
        damage.from = nextfriend;
        damage.to = source;
        damage.card = pipi;
        room->setEmotion(nextfriend, "good");
        log.type = "#StinkHit";
        log.to << source;
        room->sendLog(log);
        room->damage(damage);
    }
    else{
        log.type = "#StinkJink";
        log.to << source;
        room->sendLog(log);
        room->setEmotion(nextfriend, "good");
    }
}


KusoPackage::KusoPackage()
    :Package("kuso"){
    QList<Card *> cards;

    cards << new Shit(Card::Club, 1)
            << new Shit(Card::Heart, 8)
            << new Shit(Card::Diamond, 13)
            << new Shit(Card::Spade, 10)
            << new Stink(Card::Diamond, 1);

    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

class GrabPeach: public TriggerSkill{
public:
    GrabPeach():TriggerSkill("grab_peach"){
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Peach")){
            QList<ServerPlayer *> players = room->getOtherPlayers(player);

            foreach(ServerPlayer *p, players){
                if(p->getOffensiveHorse() == parent() &&
                   p->askForSkillInvoke("grab_peach", data))
                {
                    room->throwCard(p->getOffensiveHorse());
                    p->playCardEffect(objectName());
                    p->obtainCard(use.card);

                    return true;
                }
            }
        }

        return false;
    }
};

Saru::Saru(Card::Suit suit, int number)
    :OffensiveHorse(suit, number)
{
    setObjectName("saru");

    grab_peach = new GrabPeach;
    grab_peach->setParent(this);
}

void Saru::onInstall(ServerPlayer *player) const{
    player->getRoom()->getThread()->addTriggerSkill(grab_peach);
}

void Saru::onUninstall(ServerPlayer *player) const{

}

QString Saru::getEffectPath(bool ) const{
    return "audio/card/common/Saru.ogg";
}

class GaleShellSkill: public ArmorSkill{
public:
    GaleShellSkill():ArmorSkill("gale-shell"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Fire){
            LogMessage log;
            log.type = "#GaleShellDamage";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            room->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

GaleShell::GaleShell(Suit suit, int number) :Armor(suit, number){
    setObjectName("gale-shell");
    skill = new GaleShellSkill;

    target_fixed = false;
}

bool GaleShell::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->distanceTo(to_select) <= 1;
}

void GaleShell::onUse(Room *room, const CardUseStruct &card_use) const{
    Card::onUse(room, card_use);
}

Poison::Poison(Suit suit, int number)
    : BasicCard(suit, number){
    setObjectName("poison");
}

QString Poison::getSubtype() const{
    return "attack_card";
}

QString Poison::getEffectPath(bool is_male) const{
    return "audio/card/common/poison.ogg";
}

bool Poison::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->distanceTo(to_select) <= 1;
}

void Poison::onEffect(const CardEffectStruct &card_effect) const{
    Room *room = card_effect.from->getRoom();

    LogMessage log;
    log.from = card_effect.to;
    if(!card_effect.to->hasMark("poison")){
        room->setPlayerMark(card_effect.to, "poison", 1);
        room->setEmotion(card_effect.to, "bad");

        log.type = "#Poison_in";
        room->sendLog(log);
    }
    else{
        room->setPlayerMark(card_effect.to, "poison", 0);
        room->setEmotion(card_effect.to, "good");

        log.type = "#Poison_out";
        room->sendLog(log);
    }
}

JoyPackage::JoyPackage()
    :Package("joy")
{
    QList<Card *> cards;
    cards
                << new Saru(Card::Diamond, 5)
                << new GaleShell(Card::Heart, 1)
                << new Poison(Card::Heart, 7)
                << new Poison(Card::Club, 9)
                << new Poison(Card::Diamond, 11)
                << new Poison(Card::Spade, 13);

    foreach(Card *card, cards)
            card->setParent(this);

    type = CardPack;
}

class Timer: public PhaseChangeSkill{
public:
    Timer():PhaseChangeSkill("timer"){
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Start){
            Room *room = player->getRoom();
            room->loseMaxHp(player);
        }
        return false;
    }
};

class Lingyu: public TriggerSkill{
public:
    Lingyu():TriggerSkill("lingyu"){
        events << GameStart << CardLostDone << CardDrawnDone << CardGotDone;
    }

    static int getMaqueCardNum(ServerPlayer *me){
        int hand = me->getHandcardNum();
        int fu = 0;
        for(int i = 1; i <= 4; i ++)
            fu = fu + me->getPile("fu" + QString::number(i)).length();
        return hand + fu;
    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &) const{
        if(event == GameStart){
            int num = player->getHandcardNum();
            player->drawCards(13 - num);
        }
        else{
            int num = Lingyu::getMaqueCardNum(player);
            if(num > 13){
                if(!player->getPile("fu4").isEmpty() && player->getHandcardNum() == 2){
                    if(player->getHandcards().first()->getNumber() == player->getHandcards().last()->getNumber())
                        room->gameOver(player->getRole());
                }
                if(!player->getPhase() == Player::Draw)
                    room->askForDiscard(player, objectName(), num - 13);
            }
            else if(num < 13)
                room->drawCards(player, 13 - num);
        }


        return false;
    }
};

ZhuangcheCard::ZhuangcheCard(){
    target_fixed = true;
    will_throw = false;
}

void ZhuangcheCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(getSubcards().length() != 3)
        return;

    for(int i = 1; i <= 4; i ++){
        if(source->getPile("fu" + QString::number(i)).isEmpty()){
            foreach(int card_id, getSubcards())
                source->addToPile("fu" + QString::number(i), card_id);
            break;
        }
    }

    if(!source->getPile("fu4").isEmpty() && source->getHandcardNum() == 2){
        if(source->getHandcards().first()->getNumber() == source->getHandcards().last()->getNumber())
            room->gameOver(source->getRole());
    }
}

#include "clientplayer.h"
#include "carditem.h"
class ZhuangcheViewAsSkill: public ViewAsSkill{
public:
    ZhuangcheViewAsSkill():ViewAsSkill("zhuangche"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        int card_id = Self->property("peng").toInt();
        const Card *card = Sanguosha->getCard(card_id);
        if(selected.length() >= 3)
            return false;
        if(to_select->isEquipped())
            return false;
        return to_select->getCard()->getNumber() == card->getNumber();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@zhuangche";
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;
        ZhuangcheCard *card = new ZhuangcheCard;
        card->addSubcards(cards);
        card->addSubcard(Self->property("peng").toInt());
        return card;
    }
};

class Zhuangche: public TriggerSkill{
public:
    Zhuangche():TriggerSkill("zhuangche"){
        events << CardLost;
        view_as_skill = new ZhuangcheViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() == Player::Discard)
            return false;
        QList<ServerPlayer *> birds = room->findPlayersBySkillName(objectName());
        CardMoveStar move = data.value<CardMoveStar>();
        foreach(ServerPlayer *bird, birds){
            if(move->from && move->to_place == Player::DiscardedPile){
                const Card *card = Sanguosha->getCard(move->card_id);
                if(!bird->getPile("fu4").isEmpty() && bird->getHandcardNum() == 1
                   && card->getNumber() == bird->getHandcards().first()->getNumber()){
                    room->gameOver(bird->getRole());
                }
                room->setPlayerProperty(bird, "peng", move->card_id);
                int number = card->getNumber();
                int i = 0;
                foreach(const Card *tmp, bird->getHandcards())
                    if(tmp->getNumber() == number)
                        i ++;
                if(i == 2){
                    QString prompt = QString("@zhuangche:%1:%2:%3").arg(move->from->objectName()).arg(card->getNumberString()).arg(card->objectName());
                    if(room->askForUseCard(bird, "@@zhuangche", prompt, true))
                        break;
                }
            }
        }
        return false;
    }
};

class ZoumaViewAsSkill: public ViewAsSkill{
public:
    ZoumaViewAsSkill():ViewAsSkill("zouma"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 4)
            return false;
        if(to_select->isEquipped())
            return false;
        if(selected.length() == 1){
            int num1 = selected.last()->getCard()->getNumber();
            int num2 = to_select->getCard()->getNumber();
            return (num2 == num1 + 1) || (num1 == num2);
        }
        if(selected.length() == 2){
            int num1 = selected.first()->getCard()->getNumber();
            int num2 = selected.last()->getCard()->getNumber();
            int num3 = to_select->getCard()->getNumber();
            return (num2 == num1 + 1 && num3 == num2 + 1) ||
                    (num1 == num2 && num2 == num3);
        }
        return true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@zouma";
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 3)
            return NULL;
        ZhuangcheCard *card = new ZhuangcheCard;
        card->addSubcards(cards);
        return card;
    }
};

class Zouma: public PhaseChangeSkill{
public:
    Zouma():PhaseChangeSkill("zouma"){
        view_as_skill = new ZoumaViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Start){
            player->skip(Player::Judge);
            player->skip(Player::Play);
            player->skip(Player::Discard);
        }
        else if(player->getPhase() == Player::Draw){
            Room *room = player->getRoom();
            player->drawCards(1);
            while(room->askForUseCard(player, "@@zouma", "@zouma", true));

            if(!player->getPile("fu4").isEmpty() && player->getHandcardNum() == 2){
                if(player->getHandcards().first()->getNumber() == player->getHandcards().last()->getNumber())
                    room->gameOver(player->getRole());
            }
            if(Lingyu::getMaqueCardNum(player) > 13)
                room->askForDiscard(player, "lingyu", Lingyu::getMaqueCardNum(player) - 13);
            return true;
        }
        return false;
    }
};

ChuiniuCard::ChuiniuCard(){
    once = true;
}

bool ChuiniuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getHandcardNum() < Self->getHandcardNum();
}

void ChuiniuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    LogMessage log;
    ServerPlayer *target = targets.first();
    foreach(int hd, source->handCards())
        source->addToPile("niu", hd, false);
    foreach(int hd, target->handCards())
        target->addToPile("niu", hd, false);

    while(source->getHandcardNum() < 3){
        source->drawCards(1, false, false);
        if(source->getHandcards().last()->getNumber() > 6)
            room->throwCard(source->getHandcards().last());
    }
    while(target->getHandcardNum() < 3){
        target->drawCards(1, false, false);
        if(target->getHandcards().last()->getNumber() > 6)
            room->throwCard(target->getHandcards().last());
    }

    ServerPlayer *first = source;
    ServerPlayer *second = target;
    static QMap<ServerPlayer*, int> cmap;
    static QMap<ServerPlayer*, int> nmap;
    forever{
        QString qs = room->askForChoice(first, "chuiniu_count", "2+3+4+5+6+pass");
        if(qs == "pass")
            break;
        else
            cmap[first] = qs.toInt();

        qs = room->askForChoice(first, "chuiniu_num", "2+3+4+5+6");
        nmap[first] = qs.toInt();

        if(cmap[first] < cmap[second])
            continue;
        if(cmap[first] == cmap[second] && nmap[first] <= nmap[second])
            continue;

        log.type = "#Chuiniuing";
        log.from = first;
        log.arg = QString::number(cmap.value(first));
        log.arg2 = QString::number(nmap.value(first));
        room->sendLog(log);

        qSwap(first, second);
    }

    QList<const Card *> allhands = source->getHandcards();
    allhands.append(target->getHandcards());
    int count = 0;
    foreach(const Card *card, allhands){
        if(card->getNumber() == nmap.value(second) || card->getNumber() == 1)
            count ++;
    }
    log.type = "#ChuiniuEnd";
    log.from = second;
    log.arg = QString::number(count);
    log.arg2 = QString::number(nmap.value(second));
    room->sendLog(log);

    bool win = (second == source && count >= cmap.value(second)) ||
               (second != source && count < cmap.value(second));

    room->getThread()->delay();
    QList<int> uvnn = source->getPile("niu");
    uvnn.append(target->getPile("niu"));
    log.type = "#ChuiniuWin";
    if(win){
        log.from = source;
        room->setEmotion(source, "good");
        room->sendLog(log);
        foreach(int x, uvnn)
            room->throwCard(x);
        DummyCard *cards = target->wholeHandCards();
        room->obtainCard(source, cards, false);
        room->setEmotion(target, "bad");
    }
    else{
        log.from = target;
        room->setEmotion(target, "good");
        room->sendLog(log);
        target->throwAllHandCards();
        foreach(int x, uvnn)
            room->obtainCard(target, x, false);
        room->setEmotion(source, "bad");
        room->setPlayerFlag(source, "drank");
    }
}

class Chuiniu: public ZeroCardViewAsSkill{
public:
    Chuiniu():ZeroCardViewAsSkill("chuiniu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ChuiniuCard");
    }

    virtual const Card *viewAs() const{
        return new ChuiniuCard;
    }
};

JoyGeneralPackage::JoyGeneralPackage()
    :Package("joyer")
{
    General *maque = new General(this, "maque", "god", 12);
    maque->addSkill(new Timer);
    maque->addSkill(new Lingyu);
    maque->addSkill(new Zhuangche);
    maque->addSkill(new Zouma);
    maque->addSkill(new Skill("jizha"));

    General *chuiniu = new General(this, "chuiniu", "god", 5);
    chuiniu->addSkill(new Chuiniu);

    addMetaObject<ZhuangcheCard>();
    addMetaObject<ChuiniuCard>();

    type = GeneralPack;
}

ADD_PACKAGE(Kuso)
ADD_PACKAGE(Joy)
ADD_PACKAGE(JoyGeneral)
