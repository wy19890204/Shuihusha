#include "sheep.h"
#include "standard.h"
#include "skill.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "plough.h"

class Citan: public PhaseChangeSkill{
public:
    Citan():PhaseChangeSkill("citan"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        ServerPlayer *yanglin = room->findPlayerBySkillName(objectName());
        if(!yanglin)
            return false;
        if(player->getPhase() == Player::Discard)
            player->setMark("Cit", player->getHandcardNum());
        else if(player->getPhase() == Player::Finish){
            int old = player->getMark("Cit");
            if(old - player->getHandcardNum() >= 2 &&
               yanglin->askForSkillInvoke(objectName(), QVariant::fromValue((PlayerStar)player))){
                room->playSkillEffect(objectName());
                QList<int> card_ids = player->handCards();
                room->fillAG(card_ids, yanglin);
                int to_move = room->askForAG(yanglin, card_ids, true, objectName());
                if(to_move > -1){
                    ServerPlayer *target = room->askForPlayerChosen(yanglin, room->getOtherPlayers(player), objectName());
                    room->obtainCard(target, to_move, false);
                    card_ids.removeOne(to_move);
                }
                yanglin->invoke("clearAG");
            }
        }
        return false;
    }
};

NushaCard::NushaCard(){
    once = true;
}

bool NushaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int i = 0;
    foreach(const Player *player, Self->getSiblings()){
        if(player->getHandcardNum() >= i){
            i = player->getHandcardNum();
        }
    }
    return targets.isEmpty() && to_select->getHandcardNum() == i && to_select != Self;
}

void NushaCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this, source);
    DamageStruct damage;
    damage.from = source;
    damage.to = targets.first();
    damage.card = this;
    room->damage(damage);
}

class Nusha: public OneCardViewAsSkill{
public:
    Nusha():OneCardViewAsSkill("nusha"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("NushaCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new NushaCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Wanku:public PhaseChangeSkill{
public:
    Wanku():PhaseChangeSkill("wanku"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *zhugeliang) const{
        if(zhugeliang->getPhase() == Player::Finish &&
           zhugeliang->getHp() > zhugeliang->getHandcardNum() &&
           zhugeliang->askForSkillInvoke(objectName())){
            zhugeliang->playSkillEffect(objectName());
            zhugeliang->drawCards(zhugeliang->getHp() - zhugeliang->getHandcardNum());
        }
        return false;
    }
};

class Xuandao: public TriggerSkill{
public:
    Xuandao():TriggerSkill("xuandao"){
        events << SlashMissed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct slash_effect = data.value<SlashEffectStruct>();
/*
        if(target->hasFlag("triggered")){
            target->setFlags("-triggered");
            return true;
        }
*/
        PlayerStar next = slash_effect.to->getNextAlive();
        room->playSkillEffect(objectName());
        LogMessage log;
        log.type = "#Xuandao";
        log.from = player;
        log.to << next;
        log.arg = objectName();
        room->sendLog(log);

        slash_effect.to = next;
        slash_effect.to->setFlags("triggered");
        room->setEmotion(next, "victim");
        room->slashEffect(slash_effect);
        return true;
    }
};

class Cuihuo: public TriggerSkill{
public:
    Cuihuo():TriggerSkill("cuihuo"){
        events << CardLost;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *sunshangxiang, QVariant &data) const{
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->from_place == Player::Equip){
            if(room->askForSkillInvoke(sunshangxiang, objectName())){
                room->playSkillEffect(objectName());
                sunshangxiang->drawCards(2);
            }
        }
        return false;
    }
};

class Goldsoup: public MasochismSkill{
public:
    Goldsoup():MasochismSkill("goldsoup"){
        frequency = Compulsory;
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        if(!damage.from || !damage.from->getWeapon())
            return;
        int percent = 30 + player->getEquips().length() * 15;
        if(qrand() % 100 < percent){
            LogMessage log;
            log.from = player;
            log.type = "#TriggerSkill";
            log.arg = objectName();
            room->sendLog(log);
            DamageStruct damage2 = damage;
            damage2.from = player;
            damage2.to = damage.from;
            room->damage(damage2);
        }
    }
};

class Longjiao:public TriggerSkill{
public:
    Longjiao():TriggerSkill("longjiao"){
        events << CardUsed;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *, QVariant &data) const{
        ServerPlayer *zou = room->findPlayerBySkillName(objectName());
        if(!zou)
            return false;
        CardUseStruct effect = data.value<CardUseStruct>();
        bool caninvoke = false;
        if(effect.card->isNDTrick()){
            if(effect.to.contains(zou))
                caninvoke = true; //指定自己为目标
            //if(effect.card->inherits("GlobalEffect"))
            //    caninvoke = true; //指定所有人为目标
            //if(effect.card->inherits("AOE") && effect.from != zou)
            //    caninvoke = true; //其他人使用的AOE
            if(effect.from == zou && effect.to.isEmpty() && effect.card->inherits("ExNihilo"))
                caninvoke = true; //自己使用的无中生有
        }
        if(caninvoke && room->askForSkillInvoke(zou, objectName(), data)){
            zou->drawCards(2);
            room->playSkillEffect(objectName());
            QList<int> card_ids = zou->handCards().mid(zou->getHandcardNum() - 2);
            room->fillAG(card_ids, zou);
            int card_id = room->askForAG(zou, card_ids, false, objectName());
            room->broadcastInvoke("clearAG");
            room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::DrawPile);
        }
        return false;
    }
};

class Juesi: public TriggerSkill{
public:
    Juesi():TriggerSkill("juesi"){
        events << DamageProceed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *caifu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.to->isAlive() && damage.to->getHp() <= 1){
            LogMessage log;
            log.type = "#JuesiBuff";
            log.from = caifu;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            room->sendLog(log);

            damage.damage ++;
            room->playSkillEffect(objectName());
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

CihuCard::CihuCard(){
}

bool CihuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getGeneral()->isFemale() && to_select->isWounded();
}

bool CihuCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() < 2;
}

void CihuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this, source);
    ServerPlayer *ogami = source->tag["CihuOgami"].value<PlayerStar>();
    DamageStruct damage;
    damage.from = source;
    damage.to = ogami;
    room->damage(damage);
    PlayerStar target = !targets.isEmpty() ? targets.first() :
                        (source->getGeneral()->isFemale() && source->isWounded()) ?
                        source : NULL;
    if(target){
        RecoverStruct recover;
        recover.who = source;
        room->recover(target, recover, true);
    }
}

class CihuViewAsSkill: public ViewAsSkill{
public:
    CihuViewAsSkill(): ViewAsSkill("Cihu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        int n = Self->getMark("CihuNum");
        if(selected.length() >= n)
            return false;
        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        int n = Self->getMark("CihuNum");
        if(cards.length() != n)
            return NULL;

        CihuCard *card = new CihuCard;
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@cihu";
    }
};

class Cihu: public MasochismSkill{
public:
    Cihu():MasochismSkill("cihu"){
        view_as_skill = new CihuViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getGeneral()->isFemale();
    }

    virtual void onDamaged(ServerPlayer *akaziki, const DamageStruct &damage) const{
        Room *room = akaziki->getRoom();
        ServerPlayer *tiger = room->findPlayerBySkillName(objectName());
        if(!tiger || !damage.card || !damage.card->inherits("Slash"))
            return;
        PlayerStar ogami = damage.from;
        if(!ogami || !ogami->getGeneral()->isMale())
            return;
        if(tiger->getCardCount(true) >= akaziki->getHp()){
            room->setPlayerMark(tiger, "CihuNum", akaziki->getHp());
            tiger->tag["CihuOgami"] = QVariant::fromValue(ogami);
            QString prompt = QString("@cihu:%1::%2").arg(ogami->getGeneralName()).arg(akaziki->getGeneralName());
            room->askForUseCard(tiger, "@@cihu", prompt, true);
            tiger->tag.remove("CihuOgami");
            room->setPlayerMark(tiger, "CihuNum", 0);
        }
    }
};

class Fangsheng:public PhaseChangeSkill{
public:
    Fangsheng():PhaseChangeSkill("fangsheng"){
    }

    virtual bool onPhaseChange(ServerPlayer *taiwei) const{
        if(taiwei->getPhase() == Player::Start && taiwei->getHandcardNum() > taiwei->getHp() &&
           taiwei->askForSkillInvoke(objectName())){
            Room *room = taiwei->getRoom();
            taiwei->drawCards(2, false);
            room->playSkillEffect(objectName());

            PlayerStar target = room->askForPlayerChosen(taiwei, room->getOtherPlayers(taiwei), objectName());
            target->gainAnExtraTurn(taiwei);
            taiwei->skip();
        }
        return false;
    }
};

class Fengxing: public TriggerSkill{
public:
    Fengxing():TriggerSkill("fengxing"){
        events << PhaseChange << CardLost;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent v, Room* room, ServerPlayer *player, QVariant &data) const{
        if(v == CardLost){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Hand && player->isAlive()){
                if(player->getHandcardNum() < player->getMaxHP()){
                    LogMessage log;
                    log.type = "#TriggerSkill";
                    log.from = player;
                    log.arg = objectName();
                    room->sendLog(log);
                    player->drawCards(1);
                }
            }
        }
        else{
            if(player->getPhase() == Player::Judge ||
               player->getPhase() == Player::Draw ||
               player->getPhase() == Player::Discard)
                return true;
        }
        return false;
    }
};

LingdiCard::LingdiCard(){
    once = true;
}

bool LingdiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;
    if(targets.length() == 1){
        bool faceup = targets.first()->faceUp();
        return to_select->faceUp() != faceup;
    }
    return true;
}

bool LingdiCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void LingdiCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->turnOver();
}

class Lingdi: public OneCardViewAsSkill{
public:
    Lingdi():OneCardViewAsSkill("lingdi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("LingdiCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LingdiCard *card = new LingdiCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }
};

class Qiaodou: public MasochismSkill{
public:
    Qiaodou():MasochismSkill("qiaodou"){
    }

    virtual void onDamaged(ServerPlayer *malin, const DamageStruct &damage) const{
        if(damage.from && malin->askForSkillInvoke(objectName(), QVariant::fromValue((PlayerStar)damage.from))){
            malin->playSkillEffect(objectName());
            damage.from->turnOver();
        }
    }
};

FeiqiangCard::FeiqiangCard(){
    once = true;
}

bool FeiqiangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void FeiqiangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if(!room->askForCard(effect.to, "Jink", "@feiqiang:" + effect.from->objectName(), QVariant::fromValue(effect), CardDiscarded)){
        QString choice = effect.to->getCards("e").isEmpty() ?
                    "gong" : room->askForChoice(effect.from, "feiqiang", "gong+wang", QVariant::fromValue(effect));
        if(choice == "gong")
            room->loseHp(effect.to);
        else
            effect.to->throwAllEquips();
    }
}

class Feiqiang:public OneCardViewAsSkill{
public:
    Feiqiang():OneCardViewAsSkill("feiqiang"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("FeiqiangCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Weapon");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new FeiqiangCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

SheepPackage::SheepPackage()
    :GeneralPackage("sheep")
{
    General *yanglin = new General(this, "yanglin", "kou");
    yanglin->addSkill(new Citan);

    General *kongliang = new General(this, "kongliang", "kou", 3);
    kongliang->addSkill(new Nusha);
    kongliang->addSkill(new Wanku);

    General *xiangchong = new General(this, "xiangchong", "jiang");
    xiangchong->addSkill(new Xuandao);

    General *tanglong = new General(this, "tanglong", "jiang", 3);
    tanglong->addSkill(new Cuihuo);
    tanglong->addSkill(new Goldsoup);

    General *zourun = new General(this, "zourun", "min");
    zourun->addSkill(new Longjiao);

    General *caifu = new General(this, "caifu", "jiang");
    caifu->addSkill(new Juesi);

    General *gudasao = new General(this, "gudasao", "min", 4, false);
    gudasao->addSkill(new Cihu);

    General *hongxin = new General(this, "hongxin", "guan");
    hongxin->addSkill(new Fangsheng);

    General *maling = new General(this, "maling", "jiang", 3);
    maling->addSkill(new Fengxing);
/*
    General *malin = new General(this, "malin", "kou", 3);
    malin->addSkill(new Lingdi);
    malin->addSkill(new Qiaodou);

    General *gongwang = new General(this, "gongwang", "jiang");
    gongwang->addSkill(new Feiqiang);
*/
    addMetaObject<NushaCard>();
    addMetaObject<CihuCard>();
    //addMetaObject<LingdiCard>();
    //addMetaObject<FeiqiangCard>();
}

ADD_PACKAGE(Sheep)
