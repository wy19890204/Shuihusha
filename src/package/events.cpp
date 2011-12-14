#include "events.h"
#include "general.h"
#include "skill.h"
#include "room.h"
#include "carditem.h"
#include "tocheck.h"
#include "clientplayer.h"
#include "client.h"
#include "engine.h"

FangzhuCard::FangzhuCard(){
    mute = true;
}

void FangzhuCard::onEffect(const CardEffectStruct &effect) const{
    int x = effect.from->getLostHp();

    effect.to->drawCards(x);

    Room *room = effect.to->getRoom();

    int index;
    if(effect.to->faceUp())
        index = effect.to->getGeneralName() == "caozhi" ? 3 : 1;
    else
        index = 2;
    room->playSkillEffect("fangzhu", index);

    effect.to->turnOver();
}

class FangzhuViewAsSkill: public ZeroCardViewAsSkill{
public:
    FangzhuViewAsSkill():ZeroCardViewAsSkill("fangzhu"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@fangzhu";
    }

    virtual const Card *viewAs() const{
        return new FangzhuCard;
    }
};

class Fangzhu: public MasochismSkill{
public:
    Fangzhu():MasochismSkill("fangzhu"){
        view_as_skill = new FangzhuViewAsSkill;
    }

    virtual void onDamaged(ServerPlayer *caopi, const DamageStruct &damage) const{
        Room *room = caopi->getRoom();
        room->askForUseCard(caopi, "@@fangzhu", "@fangzhu");
    }
};

class Huoshou: public TriggerSkill{
public:
    Huoshou():TriggerSkill("huoshou"){
        events << Predamage << CardEffected;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("SavageAssault")){
            Room *room = player->getRoom();
            ServerPlayer *menghuo = room->findPlayerBySkillName(objectName());
            if(menghuo){
                LogMessage log;
                log.type = "#HuoshouTransfer";
                log.from = menghuo;
                log.to << damage.to;
                log.arg = player->getGeneralName();
                room->sendLog(log);

                room->playSkillEffect(objectName());

                damage.from = menghuo;
                room->damage(damage);
                return true;
            }
        }

        return false;
    }
};

class Lieren: public TriggerSkill{
public:
    Lieren():TriggerSkill("lieren"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *zhurong, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.card && damage.card->inherits("Slash") && damage.to->isAlive()
            && !zhurong->isKongcheng() && !damage.to->isKongcheng() && damage.to != zhurong){
            Room *room = zhurong->getRoom();
            if(room->askForSkillInvoke(zhurong, objectName(), data)){
                room->playSkillEffect(objectName(), 1);

                bool success = zhurong->pindian(damage.to, "lieren", NULL);
                if(success)
                    room->playSkillEffect(objectName(), 2);
                else{
                    room->playSkillEffect(objectName(), 3);
                    return false;
                }

                if(!damage.to->isNude()){
                    int card_id = room->askForCardChosen(zhurong, damage.to, "he", objectName());
                    if(room->getCardPlace(card_id) == Player::Hand)
                        room->moveCardTo(Sanguosha->getCard(card_id), zhurong, Player::Hand, false);
                    else
                        room->obtainCard(zhurong, card_id);
                }
            }
        }

        return false;
    }
};

class Zaiqi: public PhaseChangeSkill{
public:
    Zaiqi():PhaseChangeSkill("zaiqi"){

    }

    virtual bool onPhaseChange(ServerPlayer *menghuo) const{
        if(menghuo->getPhase() == Player::Draw && menghuo->isWounded()){
            Room *room = menghuo->getRoom();
            if(room->askForSkillInvoke(menghuo, objectName())){
                int x = menghuo->getLostHp(), i;

                room->playSkillEffect(objectName(), 1);
                bool has_heart = false;

                if(has_heart)
                    room->playSkillEffect(objectName(), 2);
                else
                    room->playSkillEffect(objectName(), 3);

                return true;
            }
        }

        return false;
    }
};

class Juxiang: public TriggerSkill{
public:
    Juxiang():TriggerSkill("juxiang"){
        events << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("SavageAssault")){
            Room *room = player->getRoom();
            if(room->getCardPlace(use.card->getEffectiveId()) == Player::DiscardedPile){
                // finding zhurong;
                QList<ServerPlayer *> players = room->getAllPlayers();
                foreach(ServerPlayer *p, players){
                    if(p->hasSkill(objectName())){
                        p->obtainCard(use.card);
                        room->playSkillEffect(objectName());
                        break;
                    }
                }
            }
        }

        return false;
    }
};

class Luanwu: public ZeroCardViewAsSkill{
public:
    Luanwu():ZeroCardViewAsSkill("luanwu"){
        frequency = Limited;
    }

    virtual const Card *viewAs() const{
        return new LuanwuCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@chaos") >= 1;
    }
};

LuanwuCard::LuanwuCard(){
    target_fixed = true;
}

void LuanwuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    source->loseMark("@chaos");
    room->broadcastInvoke("animate", "lightbox:$luanwu");

    QList<ServerPlayer *> players = room->getOtherPlayers(source);
    foreach(ServerPlayer *player, players){
        if(player->isAlive())
            room->cardEffect(this, source, player);
    }
}

void LuanwuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    QList<ServerPlayer *> players = room->getOtherPlayers(effect.to);
    QList<int> distance_list;
    int nearest = 1000;
    foreach(ServerPlayer *player, players){
        int distance = effect.to->distanceTo(player);
        distance_list << distance;

        nearest = qMin(nearest, distance);
    }

    QList<ServerPlayer *> luanwu_targets;
    int i;
    for(i=0; i<distance_list.length(); i++){
        if(distance_list.at(i) == nearest && effect.to->canSlash(players.at(i))){
            luanwu_targets << players.at(i);
        }
    }

    const Card *slash = NULL;
    if(!luanwu_targets.isEmpty() && (slash = room->askForCard(effect.to, "slash", "@luanwu-slash"))){
        ServerPlayer *to_slash;
        if(luanwu_targets.length() == 1)
            to_slash = luanwu_targets.first();
        else
            to_slash = room->askForPlayerChosen(effect.to, luanwu_targets, "luanwu");
        room->cardEffect(slash, effect.to, to_slash);
    }else
        room->loseHp(effect.to);
}

class Weimu: public ProhibitSkill{
public:
    Weimu():ProhibitSkill("weimu"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        return card->inherits("TrickCard") && card->isBlack() && !card->inherits("Collateral");
    }
};

class Jiuchi: public OneCardViewAsSkill{
public:
    Jiuchi():OneCardViewAsSkill("jiuchi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Analeptic::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern.contains("analeptic");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Analeptic *analeptic = new Analeptic(card->getSuit(), card->getNumber());
        analeptic->setSkillName(objectName());
        analeptic->addSubcard(card->getId());

        return analeptic;
    }
};

//events card
QString EventsCard::getType() const{
    return "events";
}

QString EventsCard::getSubtype() const{
    return "events";
}

Card::CardType EventsCard::getTypeId() const{
    return Events;
}

Jiefachang::Jiefachang(Suit suit, int number):EventsCard(suit, number){
    setObjectName("jiefachang");
}

bool Jiefachang::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    if(Self->getPhase() == Player::Play){
        return !to_select->getJudgingArea().isEmpty();
    }
    return !to_select->faceUp();
}

void Jiefachang::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    ServerPlayer *target = targets.first();
    if(source->getPhase() == Player::Play){
        if(target->getJudgingArea().length() > 1)
            room->throwCard(room->askForCardChosen(source, target, "j", "jiefachang"));
        else
            room->throwCard(target->getJudgingArea().last());
    }
    else
        target->turnOver();
}

Daojia::Daojia(Suit suit, int number):EventsCard(suit, number){
    setObjectName("daojia");
}

bool Daojia::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!Self->hasFlag("Daojia")){
        return !targets.isEmpty();
    }
    return targets.isEmpty() && to_select != Self && to_select->getArmor();
}

bool Daojia::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(Self->hasFlag("Daojia"))
        return targets.length() == 1;
    else
        return targets.length() == 0;
}

void Daojia::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    if(targets.isEmpty()){
        source->drawCards(1);
        room->moveCardTo(this, NULL, Player::DrawPile, true);
    }
    else{
        ServerPlayer *target = targets.first();
        source->obtainCard(target->getArmor());
    }
}

Tifanshi::Tifanshi(Suit suit, int number):EventsCard(suit, number){
    setObjectName("tifanshi");
}

bool Tifanshi::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(Self->hasFlag("Tifanshi"))
        return false;
    else
        return targets.isEmpty() && to_select != Self;
}

bool Tifanshi::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(Self->hasFlag("Tifanshi"))
        return true;
    else
        return !targets.isEmpty();
}

void Tifanshi::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    if(targets.isEmpty()){
        int reco = 0;
        foreach(ServerPlayer *tmp, room->getAlivePlayers())
            if(tmp->getRole() == "rebel")
                reco ++;
        if(reco > 0)
            source->drawCards(reco);
    }
    else{
        ServerPlayer *target = targets.first();
        room->askForDiscard(target, "tifanshi", 1, false, true);
    }
}

NinedayGirl::NinedayGirl(Suit suit, int number):EventsCard(suit, number){
    setObjectName("ninedaygirl");
}

bool NinedayGirl::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(Self->hasFlag("NineGirl"))
        return false;
    else
        return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

bool NinedayGirl::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(Self->hasFlag("NineGirl"))
        return true;
    else
        return !targets.isEmpty();
}

void NinedayGirl::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    if(!targets.isEmpty()){
        ServerPlayer *target = targets.first();
        int card_id = room->askForCardChosen(source, target, "h", "ninedaygirl");
        room->showCard(target, card_id);
        room->obtainCard(source, card_id);
    }
}

EventsPackage::EventsPackage()
    :Package("events_package")
{/*
    General *caopi, *xuhuang,]ou);
    menghuo->addSkill(new Zaiqi);

    zhurong = new General(this, "zhurong", "jiang", 4, false);
    zhurong->addSkill(new Juxiang);
    zhurong->addSkill(new Lieren);

    related_skills.insertMulti("juxiang", "#sa_avoid_juxiang");

    related_skills.insertMulti("haoshi", "#haoshi");
    related_skills.insertMulti("haoshi", "#haoshi-give");

    jiaxu = new General(this, "jiaxu", "kou", 3);
    jiaxu->addSkill(new Skill("wansha", Skill::Compulsory));
    jiaxu->addSkill(new Weimu);
    jiaxu->addSkill(new Luanwu);
    addMetaObject<YinghunCard>();
    addMetaObject<FangzhuCard>();
    addMetaObject<HaoshiCard>();
*/
    QList<Card *> cards;
    cards
            << new Jiefachang(Card::Diamond, 4)
            << new Tifanshi(Card::Spade, 7)
            << new Daojia(Card::Club, 12)
            << new NinedayGirl(Card::Heart, 9);

    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(Events)
