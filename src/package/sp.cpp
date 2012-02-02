#include "standard.h"
#include "skill.h"
#include "sp.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"

class Shemi: public TriggerSkill{
public:
    Shemi():TriggerSkill("shemi"){
        events << PhaseChange << TurnedOver;
    }

    virtual bool trigger(TriggerEvent e, ServerPlayer *emperor, QVariant &data) const{
        //Room *room = emperor->getRoom();
        if(e == PhaseChange){
            if(emperor->getPhase() == Player::Discard &&
               emperor->askForSkillInvoke(objectName(), data)){
                emperor->turnOver();
                return true;
            }
        }
        else{
            if(!emperor->hasFlag("NongQ")){
                int index = emperor->faceUp() ? 2: 1;
                emperor->getRoom()->playSkillEffect(objectName(), index);
            }
            int x = emperor->getLostHp();
            x = qMax(qMin(x,2),1);
            emperor->drawCards(x);
        }
        return false;
    }
};

class Lizheng: public DistanceSkill{
public:
    Lizheng():DistanceSkill("lizheng"){
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(to->hasSkill(objectName()) && !to->faceUp())
            return +1;
        else
            return 0;
    }
};

class Nongquan:public PhaseChangeSkill{
public:
    Nongquan():PhaseChangeSkill("nongquan$"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getKingdom() == "guan" && !target->hasLordSkill(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *otherguan) const{
        Room *room = otherguan->getRoom();
        if(otherguan->getPhase() != Player::Draw)
            return false;
        ServerPlayer *head = room->getLord();
        if(head->hasLordSkill(objectName()) && otherguan->getKingdom() == "guan"
           && otherguan->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());
            room->setPlayerFlag(head, "NongQ");
            head->turnOver();
            room->setPlayerFlag(head, "-NongQ");
            return true;
        }
        return false;
    }
};

JiebaoCard::JiebaoCard(){
}

bool JiebaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;

    if(to_select == Self)
        return false;

    return !to_select->isNude();
}

void JiebaoCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    int card_id = room->askForCardChosen(effect.from, effect.to, "he", "jiebao");
    const Card *card = Sanguosha->getCard(card_id);
    room->moveCardTo(card, effect.from, Player::Hand, false);

    room->setEmotion(effect.to, "bad");
    room->setEmotion(effect.from, "good");
}

class JiebaoViewAsSkill: public ZeroCardViewAsSkill{
public:
    JiebaoViewAsSkill():ZeroCardViewAsSkill("jiebao"){
    }

    virtual const Card *viewAs() const{
        return new JiebaoCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@jiebao";
    }
};

class Jiebao: public TriggerSkill{
public:
    Jiebao():TriggerSkill("jiebao"){
        events << Death;
        view_as_skill = new JiebaoViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *tianwang = room->findPlayerBySkillName(objectName());
        if(!tianwang)
            return false;
        bool can_invoke = false;
        QList<ServerPlayer *> other_players = room->getOtherPlayers(tianwang);
        foreach(ServerPlayer *player, other_players){
            if(!player->isNude()){
                can_invoke = true;
                break;
            }
        }
        if(can_invoke)
            room->askForUseCard(tianwang, "@@jiebao", "@jiebao:" + player->objectName());
        return false;
    }
};

class Dushi: public TriggerSkill{
public:
    Dushi():TriggerSkill("dushi"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill("dushi");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStar damage = data.value<DamageStar>();
        if(damage->to == player && damage->from && damage->from != player){
            if(!player->askForSkillInvoke(objectName()))
                return false;
            QList<ServerPlayer *> targets = room->getOtherPlayers(damage->from);
            targets.removeOne(player);
            if(targets.isEmpty())
                return false;
            ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
            room->showAllCards(target);
            int kuro = 0;
            foreach(const Card *tcard, target->getHandcards()){
                if(tcard->isBlack())
                    kuro ++;
            }
            if(kuro > 0){
                DamageStruct uct;
                uct.from = target;
                uct.to = damage->from;
                uct.damage = kuro;
                room->damage(uct);
            }
        }
        return false;
    }
};

class Shuntian: public TriggerSkill{
public:
    Shuntian():TriggerSkill("shuntian"){
        events << GameStart;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room *rom = player->getRoom();
        QString kim;
        if(!player->isLord())
            kim = rom->getLord()->getKingdom();
        rom->setPlayerProperty(player, "kingdom", kim);
        return false;
    }
};

YuzhongCard::YuzhongCard(){

}

bool YuzhongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int num = Self->getMark("YuZy");
    return targets.length() < num;
}

bool YuzhongCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    int num = Self->getMark("YuZy");
    return targets.length() <= num;
}

void YuzhongCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->drawCards(1);
}

class YuzhongViewAsSkill: public ZeroCardViewAsSkill{
public:
    YuzhongViewAsSkill():ZeroCardViewAsSkill("Yuzhong"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "@@yuzhong";
    }

    virtual const Card *viewAs() const{
        return new YuzhongCard;
    }
};

class Yuzhong: public TriggerSkill{
public:
    Yuzhong():TriggerSkill("yuzhong"){
        events << Death;
        view_as_skill = new YuzhongViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    static int getKingdoms(Room *room){
        QSet<QString> kingdom_set;
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            kingdom_set << p->getKingdom();
        }
        return kingdom_set.size();
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        int num = getKingdoms(room);
        DamageStar damage = data.value<DamageStar>();
        if(damage->from != damage->to && damage->from->hasSkill(objectName())){
            ServerPlayer *source = damage->from;
            QString choice = room->askForChoice(source, objectName(), "hp+card+cancel");
            if(choice != "cancel"){
                LogMessage log;
                log.type = "#InvokeSkill";
                log.from = source;
                log.arg = "yuzhong";
                room->sendLog(log);
            }
            if(choice == "hp"){
                RecoverStruct rev;
                rev.who = source;
                rev.recover = num;
                room->recover(room->getLord(), rev);
            }
            else if(choice == "card"){
                room->getLord()->drawCards(num);
            }
        }
        return false;
    }
};

class Yuzhong2: public TriggerSkill{
public:
    Yuzhong2():TriggerSkill("#yuzh0ng"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        room->setPlayerMark(player, "YuZy", Yuzhong::getKingdoms(room));
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.to->isLord())
            return false;
        QString choice = room->askForChoice(player, "yuzhong", "all+me+cancel");
        if(choice == "cancel")
            return false;
        if(choice == "all")
            if(!room->askForUseCard(player, "@@yuzhong", "@yuzhong"))
                choice = "me";
        if(choice == "me"){
            LogMessage log;
            log.type = "#InvokeSkill";
            log.from = player;
            log.arg = "yuzhong";
            room->sendLog(log);
            player->drawCards(2);
        }

        room->setPlayerMark(player, "YuZy", 0);
        return false;
    }
};

class Chengfu: public TriggerSkill{
public:
    Chengfu():TriggerSkill("chengfu"){
        events << CardLost;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName()) && target->getPhase() == Player::NotActive;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *conan, QVariant &data) const{
        Room *room = conan->getRoom();
        CardMoveStar move = data.value<CardMoveStar>();
        if(conan->isDead())
            return false;
        if(move->from_place == Player::Hand){
            room->playSkillEffect(objectName(), qrand() % 2 + 1);
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = conan;
            log.arg = objectName();
            room->sendLog(log);

            conan->drawCards(1);
        }
        return false;
    }
};

#include "qjwm.h"
class Xiaduo: public TriggerSkill{
public:
    Xiaduo():TriggerSkill("xiaduo"){
        events << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.from || !damage.from->hasSkill(objectName()))
            return false;
        if(damage.from->distanceTo(damage.to) != 1)
            return false;
        ServerPlayer *wanglun = damage.from;
        Room *room = wanglun->getRoom();
        QList<ServerPlayer *> ones;
        foreach(ServerPlayer *tmp, room->getOtherPlayers(damage.to))
            if(wanglun->distanceTo(tmp) == 1)
                ones << tmp;
        if(!ones.isEmpty() && room->askForCard(wanglun, ".equip", "@xiaduo", data)){
            room->playSkillEffect(objectName());
            ServerPlayer *target = room->askForPlayerChosen(wanglun, ones, objectName());
            LogMessage log;
            log.type = "#UseSkill";
            log.from = wanglun;
            log.to << target;
            log.arg = objectName();
            room->sendLog(log);

            DamageStruct dama;
            dama.from = wanglun;
            dama.to = target;
            room->damage(dama);
        }
        return false;
    }
};

class Youxia: public TriggerSkill{
public:
    Youxia():TriggerSkill("youxia"){
        events << CardLost;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *jinge = room->findPlayerBySkillName(objectName());
        if(player->isKongcheng() && jinge && !jinge->isKongcheng() && jinge->isWounded()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Hand && jinge->askForSkillInvoke(objectName(), data)){
                const Card *card = room->askForCardShow(jinge, player, "youxia");
                player->obtainCard(card);
                RecoverStruct o;
                o.card = card;
                room->recover(jinge, o);
            }
        }
        return false;
    }
};

SPPackage::SPPackage()
    :Package("sp")
{
    General *zhaoji = new General(this, "zhaoji$", "guan", 3);
    zhaoji->addSkill(new Shemi);
    zhaoji->addSkill(new Lizheng);
    zhaoji->addSkill(new Nongquan);

    General *chaogai = new General(this, "chaogai", "kou");
    chaogai->addSkill(new Jiebao);
    chaogai->addSkill(new Dushi);
    chaogai->addSkill(new Skill("shaxue$", Skill::Compulsory));

    General *jiangsong = new General(this, "jiangsong", "guan");
    jiangsong->addSkill(new Yuzhong);
    jiangsong->addSkill(new Yuzhong2);
    related_skills.insertMulti("yuzhong", "#yuzh0ng");
    jiangsong->addSkill(new Shuntian);

    General *wanglun = new General(this, "wanglun", "kou", 3);
    wanglun->addSkill(new Chengfu);
    wanglun->addSkill(new Xiaduo);

    General *ximenjinge = new General(this, "ximenjinge", "jiang");
    ximenjinge->addSkill(new Youxia);

    addMetaObject<YuzhongCard>();
    addMetaObject<JiebaoCard>();
}

ADD_PACKAGE(SP);
