#include "standard.h"
#include "skill.h"
#include "peasa.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"

class Piaoyong: public PhaseChangeSkill{
public:
    Piaoyong():PhaseChangeSkill("piaoyong"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Start || !player->askForSkillInvoke(objectName()))
            return false;
        Room *room = player->getRoom();
        player->skip(Player::Judge);
        if(room->askForChoice(player, objectName(), "first+second") == "first")
            player->skip(Player::Draw);
        else
            player->skip(Player::Play);
        foreach(const Card *card, player->getJudgingArea())
            room->throwCard(card);
        ServerPlayer *target = room->askForPlayerChosen(player, room->getAllPlayers(), objectName());
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName(objectName());
        CardUseStruct use;
        use.card = slash;
        use.from = player;
        use.to << target;
        room->useCard(use, false);
        return false;
    }
};

class Wuzong: public PhaseChangeSkill{
public:
    Wuzong():PhaseChangeSkill("wuzong"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Start && player->askForSkillInvoke(objectName())){
            player->drawCards(3);
            room->acquireSkill(player, "wusheng");
            room->acquireSkill(player, "paoxiao");
            room->setPlayerFlag(player, "wuzong");
            return false;
        }
        if(player->getPhase() == Player::NotActive && player->hasFlag("wuzong"))
            room->killPlayer(player);
        return false;
    }
};

//fugui
//zizhu

class Qiuhe: public TriggerSkill{
public:
    Qiuhe():TriggerSkill("qiuhe"){
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!effect.from || (!effect.card->inherits("Slash") && !effect.card->isNDTrick()))
            return false;
        if(effect.from->hasFlag("qiuhe") && player->askForSkillInvoke(objectName())){
            effect.from->setFlags("qiuhe");
            effect.from->drawCards(1);
            player->obtainCard(effect.card);
            return true;
        }
        return false;
    }
};

class Duanbing: public TriggerSkill{
public:
    Duanbing():TriggerSkill("duanbing"){
        events << SlashEffect << SlashProceed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(event == SlashEffect){
            if(!player->getWeapon())
                effect.to->addMark("qinggang");
        }
        else{
            if(player->getArmor())
                return false;
            Room *room = player->getRoom();
            QString slasher = player->objectName();

            const Card *first_jink = NULL, *second_jink = NULL;
            first_jink = room->askForCard(effect.to, "jink", "@duanbing-jink-1:" + slasher);
            if(first_jink)
                second_jink = room->askForCard(effect.to, "jink", "@duanbing-jink-2:" + slasher);

            Card *jink = NULL;
            if(first_jink && second_jink){
                jink = new DummyCard;
                jink->addSubcard(first_jink);
                jink->addSubcard(second_jink);
            }
            room->slashResult(effect, jink);
            return true;
        }
        return false;
    }
};

PeasaPackage::PeasaPackage()
    :Package("peasa")
{
    General *guanzhang = new General(this, "guanzhang", "jiang");
    guanzhang->addSkill(new Piaoyong);
    guanzhang->addSkill(new Wuzong);

    General *mizhu = new General(this, "mizhu", "jiang", 3);
    //zhugejin->addSkill(new Fugui);
    //zhugejin->addSkill(new Zizhu);

    General *zhugejin = new General(this, "zhugejin", "min", 3);
    zhugejin->addSkill(new Qiuhe);
    //zhugejin->addSkill(new Kuanhp);

    General *dingfeng = new General(this, "dingfeng", "min");
    dingfeng->addSkill(new Duanbing);
}

ADD_PACKAGE(Peasa);
