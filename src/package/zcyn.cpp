#include "zcyn.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "tocheck.h"

class Tongwu: public TriggerSkill{
public:
    Tongwu():TriggerSkill("tongwu"){
        events << SlashMissed;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *erge, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(!effect.to->isNude() && effect.jink){
            Room *room = erge->getRoom();
            if(erge->askForSkillInvoke(objectName(), data)){
                room->playSkillEffect(objectName());
                erge->obtainCard(effect.jink);
                ServerPlayer *target = room->askForPlayerChosen(erge, room->getOtherPlayers(effect.to), objectName());
                target->obtainCard(effect.jink);
            }
        }
        return false;
    }
};

#include "plough.h"
class Fuji:public PhaseChangeSkill{
public:
    Fuji():PhaseChangeSkill("fuji"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        PlayerStar p = player;
        if(p->getPhase() != Player::Judge || p->getJudgingArea().isEmpty())
            return false;
        Room *room = p->getRoom();
        ServerPlayer *ruan2 = room->findPlayerBySkillName(objectName());
        if(ruan2 && room->askForCard(ruan2, ".", "@fuji:" + p->objectName(), QVariant::fromValue(p))){
            Assassinate *ass = new Assassinate(Card::NoSuit, 0);
            ass->setSkillName(objectName());
            ass->setCancelable(false);
            CardUseStruct use;
            use.card = ass;
            use.from = ruan2;
            use.to << p;
            room->useCard(use);
        }
        return false;
    }
};

class Guizi: public TriggerSkill{
public:
    Guizi():TriggerSkill("guizi"){
        events << Dying;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *poolguy, QVariant &data) const{
        Room *room = poolguy->getRoom();
        ServerPlayer *bear = room->findPlayerBySkillName(objectName());
        DyingStruct dying = data.value<DyingStruct>();
        if(!bear || !dying.who)
            return false;
        if(dying.who == poolguy && room->askForCard(bear, "..S", "@guizi:" + poolguy->objectName(), data)){
            room->playSkillEffect(objectName());
            DamageStruct damage;
            damage.from = bear;

            LogMessage log;
            log.type = "#Guizi";
            log.from = bear;
            log.to << poolguy;
            log.arg = objectName();
            room->sendLog(log);

            room->getThread()->delay(1500);
            room->killPlayer(poolguy, &damage);
            return true;
        }
        return false;
    }
};

SixiangCard::SixiangCard(){
}

bool SixiangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int x = Self->getMark("Sixh");
    return targets.length() < x;
}

bool SixiangCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    int x = Self->getMark("Sixh");
    return targets.length() <= x && !targets.isEmpty();
}

void SixiangCard::onEffect(const CardEffectStruct &effect) const{
    int handcardnum = effect.to->getHandcardNum();
    int x = effect.from->getMark("Sixh");
    int delta = handcardnum - x;
    Room *room = effect.from->getRoom();
    if(delta > 0)
        room->askForDiscard(effect.to, "sixiang", delta);
    else
        effect.to->drawCards(qAbs(delta));
}

class SixiangViewAsSkill: public OneCardViewAsSkill{
public:
    SixiangViewAsSkill():OneCardViewAsSkill("sixiang"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@sixiang";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        SixiangCard *card = new SixiangCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Sixiang:public PhaseChangeSkill{
public:
    Sixiang():PhaseChangeSkill("sixiang"){
        view_as_skill = new SixiangViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *jingmuan) const{
        Room *room = jingmuan->getRoom();
        if(jingmuan->getPhase() == Player::Start && !jingmuan->isKongcheng()){
            room->setPlayerMark(jingmuan, "Sixh", room->getKingdoms());
            if(room->askForUseCard(jingmuan, "@@sixiang", "@sixiang"))
                jingmuan->setFlags("elephant");
        }
        else if(jingmuan->getPhase() == Player::Discard && jingmuan->hasFlag("elephant")){
            int x = room->getKingdoms();
            int total = jingmuan->getEquips().length() + jingmuan->getHandcardNum();

            LogMessage log;
            log.from = jingmuan;
            log.arg2 = objectName();
            if(total <= x){
                jingmuan->throwAllHandCards();
                jingmuan->throwAllEquips();
                log.type = "#SixiangWorst";
                log.arg = QString::number(total);
                room->sendLog(log);
            }else{
                room->askForDiscard(jingmuan, objectName(), x, false, true);
                log.type = "#SixiangBad";
                log.arg = QString::number(x);
                room->sendLog(log);
            }
        }
        return false;
    }
};

class Tianyan: public PhaseChangeSkill{
public:
    Tianyan():PhaseChangeSkill("tianyan"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Judge || player->getHandcardNum() < 3)
            return false;
        Room *room = player->getRoom();
        ServerPlayer *tianqi = room->findPlayerBySkillName(objectName());
        if(tianqi && tianqi->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());

            tianqi->tag["TianyanTarget"] = QVariant::fromValue((PlayerStar)player);
            QList<int> cards = room->getNCards(3);
            if(!cards.isEmpty()){
                room->fillAG(cards, tianqi);
                while(!cards.isEmpty()){
                    int card_id = room->askForAG(tianqi, cards, true, objectName());
                    if(card_id == -1)
                        break;
                    if(!cards.contains(card_id))
                        continue;
                    cards.removeOne(card_id);
                    room->throwCard(card_id);
                    room->takeAG(NULL, card_id);

                    LogMessage log;
                    log.from = tianqi;
                    log.type = "$DiscardCard";
                    log.card_str = QString::number(card_id);
                    room->sendLog(log);
                }
                for(int i = cards.length() - 1; i >= 0; i--){
                    room->throwCard(cards.at(i));
                    const Card *tmp = Sanguosha->getCard(cards.at(i));
                    room->moveCardTo(tmp, NULL, Player::DrawPile);
                }
                tianqi->invoke("clearAG");
            }
        }
        tianqi->tag.remove("TianyanTarget");
        return false;
    }
};

class Dujian: public TriggerSkill{
public:
    Dujian():TriggerSkill("dujian"){
        events << Predamage;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to == damage.from || damage.damage < 1 || !damage.card->inherits("Slash"))
            return false;
        if(!damage.to->isNude() && !damage.to->inMyAttackRange(player)
            && player->askForSkillInvoke(objectName(), data)){
            player->getRoom()->playSkillEffect(objectName());
            damage.to->turnOver();
            return true;
        }
        return false;
    }
};

class Paohong: public FilterSkill{
public:
    Paohong():FilterSkill("paohong"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getCard();
        return card->objectName() == "slash" && card->isBlack();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *c = card_item->getCard();
        ThunderSlash *bs = new ThunderSlash(c->getSuit(), c->getNumber());
        bs->setSkillName(objectName());
        bs->addSubcard(card_item->getCard());
        return bs;
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

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
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
        events << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *caifu, QVariant &data) const{
        Room *room = caifu->getRoom();
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

class Hengchong: public TriggerSkill{
public:
    Hengchong():TriggerSkill("hengchong"){
        events << SlashMissed;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        if(player->getWeapon())
            return false;
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = player->getRoom();
        QString suit_str = effect.slash->getSuitString();
        QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
        QString prompt = QString("@hengchong:%1::%2").arg(effect.to->getGeneralName()).arg(suit_str);
        CardStar card = room->askForCard(player, pattern, prompt, data);
        if(card){
            room->playSkillEffect(objectName());
            room->slashResult(effect, NULL);
            ServerPlayer *target = room->askForPlayerChosen(player, room->getNextandPrevious(effect.to), objectName());
            DamageStruct damage;
            damage.from = player;
            damage.to = target;
            room->damage(damage);

            LogMessage log;
            log.type = "#Hengchong";
            log.from = player;
            log.to << effect.to << target;
            log.arg = objectName();
            log.arg2 = card->objectName();
            room->sendLog(log);

            return true;
        }
        return false;
    }
};

class Tuzai: public TriggerSkill{
public:
    Tuzai():TriggerSkill("tuzai"){
        events << Damage;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Slash") &&
           damage.to && !damage.to->isKongcheng()
            && player->askForSkillInvoke(objectName(), data)){
            Room *room = player->getRoom();
            room->playSkillEffect(objectName());
            int dust = damage.to->getRandomHandCardId();
            room->showCard(damage.to, dust);

            if(Sanguosha->getCard(dust)->isRed()){
                room->throwCard(dust);
                player->drawCards(1);
            }
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
    room->throwCard(this);
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
        room->recover(target, recover);
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
            room->askForUseCard(tiger, "@@cihu", prompt);
            tiger->tag.remove("CihuOgami");
            room->setPlayerMark(tiger, "CihuNum", 0);
        }
    }
};

ZCYNPackage::ZCYNPackage()
    :Package("ZCYN")
{
    General *guansheng = new General(this, "guansheng", "jiang");
    guansheng->addSkill(new Tongwu);

    General *ruanxiaoer = new General(this, "ruanxiaoer", "min");
    ruanxiaoer->addSkill(new Fuji);

    General *yangxiong = new General(this, "yangxiong", "jiang");
    yangxiong->addSkill(new Guizi);

    General *haosiwen = new General(this, "haosiwen", "guan");
    haosiwen->addSkill(new Sixiang);

    General *pengqi = new General(this, "pengqi", "guan");
    pengqi->addSkill(new Tianyan);

    General *shiwengong = new General(this, "shiwengong", "jiang");
    shiwengong->addSkill(new Dujian);

    General *lingzhen = new General(this, "lingzhen", "jiang");
    lingzhen->addSkill(new Paohong);

    General *ligun = new General(this, "ligun", "jiang");
    ligun->addSkill(new Hengchong);

    General *caozheng = new General(this, "caozheng", "min");
    caozheng->addSkill(new Tuzai);

    General *zourun = new General(this, "zourun", "min");
    zourun->addSkill(new Longjiao);

    General *caifu = new General(this, "caifu", "jiang");
    caifu->addSkill(new Juesi);

    General *gudasao = new General(this, "gudasao", "min", 4, false);
    gudasao->addSkill(new Cihu);

    addMetaObject<SixiangCard>();
    addMetaObject<CihuCard>();
}

ADD_PACKAGE(ZCYN);
