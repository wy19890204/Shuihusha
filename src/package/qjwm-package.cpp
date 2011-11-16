#include "qjwm-package.h"
#include "skill.h"
#include "standard.h"
#include "maneuvering.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"

class Jingzhun: public SlashBuffSkill{
public:
    Jingzhun():SlashBuffSkill("jingzhun"){
        frequency = Compulsory;
    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *huarong = effect.from;
        Room *room = huarong->getRoom();
        if(huarong->getPhase() != Player::Play)
            return false;

        if(huarong->distanceTo(effect.to) == huarong->getAttackRange()){
            room->playSkillEffect(objectName());
            LogMessage log;
            log.type = "#Jingzhun";
            log.from = huarong;
            log.to << effect.to;
            log.arg = objectName();
            room->sendLog(log);

            room->slashResult(effect, NULL);
            return true;
        }
        return false;
    }
};

class KaixianPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return !player->hasEquip(card) &&
                card->getNumber() <= 5;
    }

    virtual bool willThrow() const{
        return false;
    }
};

class Kaixian: public PhaseChangeSkill{
public:
    Kaixian():PhaseChangeSkill("kaixian"){

    }

    virtual bool onPhaseChange(ServerPlayer *huarong) const{
        Room *room = huarong->getRoom();
        if(huarong->getPhase() == Player::Start){
            if(!huarong->isKongcheng() && room->askForSkillInvoke(huarong, objectName())){
                const Card *card = room->askForCard(huarong, ".kaixian!", "@kaixian");
                room->setPlayerMark(huarong, "kaixian", card->getNumber());
                LogMessage log;
                log.type = "$Kaixian";
                log.from = huarong;
                log.card_str = card->getEffectIdString();
                room->sendLog(log);

                room->playSkillEffect(objectName());
            }
        }
        else if(huarong->getPhase() == Player::NotActive)
            room->setPlayerMark(huarong, "kaixian", 0);

        return false;
    }
};

class Kongliang: public TriggerSkill{
public:
    Kongliang():TriggerSkill("kongliang"){
        events << DrawNCards << PhaseChange;
    }

    static bool CompareBySuit(int card1, int card2){
        const Card *c1 = Sanguosha->getCard(card1);
        const Card *c2 = Sanguosha->getCard(card2);

        int a = static_cast<int>(c1->getSuit());
        int b = static_cast<int>(c2->getSuit());

        return a < b;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *liying, QVariant &data) const{
        Room *room = liying->getRoom();
        if(event == DrawNCards){
            if(room->askForSkillInvoke(liying, objectName())){
                room->playSkillEffect(objectName());
                data = liying->getMaxHP() + liying->getLostHp();
                liying->setFlags("kongliang");
            }
        }
        else if(event == PhaseChange && liying->hasFlag("kongliang") && liying->getPhase() == Player::Play){
            QList<int> card_ids;
            foreach(const Card *tmp, liying->getHandcards()){
                card_ids << tmp->getId();
            }
            qSort(card_ids.begin(), card_ids.end(), CompareBySuit);
            room->fillAG(card_ids);
            int count = 0;
            while(!card_ids.isEmpty() && count < 2){
                int card_id = room->askForAG(liying, card_ids, false, objectName());
                card_ids.removeOne(card_id);
                room->throwCard(card_id);
                room->takeAG(NULL, card_id);

                // throw the rest cards that matches the same suit
                const Card *card = Sanguosha->getCard(card_id);
                Card::Suit suit = card->getSuit();
                QMutableListIterator<int> itor(card_ids);
                while(itor.hasNext()){
                    const Card *c = Sanguosha->getCard(itor.next());
                    if(c->getSuit() == suit){
                        itor.remove();
                        room->throwCard(card_id);
                        room->takeAG(NULL, c->getId());
                    }
                }
                count ++;
            }
            room->broadcastInvoke("clearAG");
        }
        return false;
    }
};

class Liba: public TriggerSkill{
public:
    Liba():TriggerSkill("liba"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *luda, QVariant &data) const{
        Room *room = luda->getRoom();
        if(luda->getPhase() != Player::Play)
            return false;
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.card && damage.card->inherits("Slash") && damage.to->isAlive()
            && !damage.to->isKongcheng()){
            if(room->askForSkillInvoke(luda, objectName(), data)){
                room->playSkillEffect(objectName());
                const Card *card = room->askForCardShow(damage.to, luda, objectName());
                if(!card->inherits("BasicCard")){
                    room->throwCard(card->getId());
                    LogMessage log;
                    log.type = "$ForceDiscardCard";
                    log.from = luda;
                    log.to << damage.to;
                    log.card_str = card->getEffectIdString();
                    room->sendLog(log);

                    damage.damage ++;
                }
                data = QVariant::fromValue(damage);
            }
        }
        return false;
    }
};

class Fuhu: public TriggerSkill{
public:
    Fuhu():TriggerSkill("fuhu"){
        events << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *wusong = room->findPlayerBySkillName(objectName());
        if(!wusong)
            return false;
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.card && damage.card->inherits("Slash") && damage.to->isAlive()
            && !wusong->isKongcheng() && damage.from != wusong
                    && room->askForSkillInvoke(wusong, objectName(), data)){
            room->playSkillEffect(objectName());
            const Card *card = room->askForCard(wusong, ".basic", "@fuhu", data);
            if(!card)
                return false;
            Slash *slash = new Slash(card->getSuit(), card->getNumber());
            slash->setSkillName(objectName());
            CardUseStruct use;
            use.card = slash;
            use.from = wusong;
            use.to << damage.to;

            if(card->inherits("Analeptic")){
                LogMessage log;
                log.type = "$Fuhu";
                log.from = wusong;
                log.card_str = card->getEffectIdString();
                room->sendLog(log);

                room->setPlayerFlag(wusong, "drank");
            }
            room->useCard(use);
        }
        return false;
    }
};

class Wubang: public TriggerSkill{
public:
    Wubang():TriggerSkill("wubang"){
        events << CardLost;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *jiuwenlong = room->findPlayerBySkillName(objectName());
        if(!jiuwenlong || player == jiuwenlong)
            return false;
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->to_place == Player::DiscardedPile){
            const Card *weapon = Sanguosha->getCard(move->card_id);
            if(weapon->inherits("Weapon") &&
               jiuwenlong->askForSkillInvoke(objectName()))
                jiuwenlong->obtainCard(weapon);
        }
        return false;
    }
};

class EquipPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return card->getTypeId() == Card::Equip;
    }
};

class Xiagu: public TriggerSkill{
public:
    Xiagu():TriggerSkill("xiagu"){
        events << Predamage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *jiuwenlong = room->findPlayerBySkillName(objectName());
        if(!jiuwenlong || jiuwenlong->isKongcheng())
            return false;
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.nature == DamageStruct::Normal && damage.to->isAlive() && damage.damage > 0){
            if(room->askForSkillInvoke(jiuwenlong, objectName(), data)){
                room->playSkillEffect(objectName());
                const Card *card = room->askForCard(jiuwenlong, ".equip", "@xiagu", data);
                if(card){
                    LogMessage log;
                    log.type = "$Xiagu";
                    log.from = jiuwenlong;
                    log.to << damage.to;
                    log.card_str = card->getEffectIdString();
                    room->sendLog(log);

                    damage.damage --;
                }
                data = QVariant::fromValue(damage);
            }
        }
        return false;
    }
};

DaleiCard::DaleiCard(){
    once = true;
    will_throw = false;
}

bool DaleiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getGeneral()->isMale() &&
            !to_select->isKongcheng() && to_select != Self;
}

void DaleiCard::use(Room *room, ServerPlayer *xiaoyi, const QList<ServerPlayer *> &targets) const{
    bool success = xiaoyi->pindian(targets.first(), "dalei", this);
    if(success){
        room->setPlayerFlag(xiaoyi, "dalei_success");
        room->setPlayerFlag(targets.first(), "dalei_target");
    }else{
        DamageStruct damage;
        damage.from = targets.first();
        damage.to = xiaoyi;
        room->damage(damage);
    }
}

class DaleiViewAsSkill: public OneCardViewAsSkill{
public:
    DaleiViewAsSkill():OneCardViewAsSkill("dalei"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("DaleiCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new DaleiCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Dalei: public TriggerSkill{
public:
    Dalei():TriggerSkill("dalei"){
        view_as_skill = new DaleiViewAsSkill;
        events << Damage << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(!player->hasFlag("dalei_success"))
            return false;
        if(event == PhaseChange){
            if(player->getPhase() == Player::NotActive){
                room->setPlayerFlag(player, "-dalei_success");
                foreach(ServerPlayer *tmp, room->getAllPlayers()){
                    if(tmp->hasFlag("dalei_target"))
                        room->setPlayerFlag(tmp, "-dalei_target");
                }
            }
            return false;
        }
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to->hasFlag("dalei_target") && player->askForSkillInvoke(objectName())){
            ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(damage.to), objectName());
            RecoverStruct rev;
            rev.who = player;
            room->recover(target, rev);
        }
        return false;
    }
};

class Fuqin: public MasochismSkill{
public:
    Fuqin():MasochismSkill("fuqin"){
    }

    virtual void onDamaged(ServerPlayer *yan, const DamageStruct &damage) const{
        Room *room = yan->getRoom();
        int lstn = yan->getLostHp();
        QString choice = damage.from ?
                         room->askForChoice(yan, objectName(), "yan+qing+nil"):
                         room->askForChoice(yan, objectName(), "qing+nil");
        if(choice == "nil")
            return;
        if(choice == "yan"){
            for(int i = 0; i < lstn; i++){
                room->throwCard(room->askForCardChosen(damage.from, yan, "he", objectName()));
                if(damage.from->isNude())
                    break;
            }
        }
        else{
            ServerPlayer *target = room->askForPlayerChosen(yan, room->getAllPlayers(), objectName());
            target->drawCards(lstn);
        }
    }
};

QJWMPackage::QJWMPackage():Package("QJWM"){

    General *huarong = new General(this, "huarong", "wei", 4); //guan == wei
    huarong->addSkill(new Jingzhun);
    huarong->addSkill(new Kaixian);
    patterns.insert(".kaixian!", new KaixianPattern);

    General *liying = new General(this, "liying", "wei");
    liying->addSkill(new Kongliang);

    General *luzhishen = new General(this, "luzhishen", "qun"); //kou == qun
    luzhishen->addSkill(new Liba);
    luzhishen->addSkill(new Skill("zuohua", Skill::Compulsory));

    General *wusong = new General(this, "wusong", "qun");
    wusong->addSkill(new Fuhu);

    General *shijin = new General(this, "shijin", "qun");
    shijin->addSkill(new Wubang);
    shijin->addSkill(new Xiagu);
    patterns[".equip"] = new EquipPattern;

    General *yanqing = new General(this, "yanqing", "wu"); //min == wu
    yanqing->addSkill(new Dalei);
    yanqing->addSkill(new Fuqin);

    General *zhuwu = new General(this, "zhuwu", "qun");
    General *hantao = new General(this, "hantao", "wei");
    General *oupeng = new General(this, "oupeng", "wu"); //jiang == shu
    General *shien = new General(this, "shien", "wu");
    General *luozhenren = new General(this, "luozhenren", "qun");
    General *wangqing = new General(this, "wangqing", "wu");
    /*
    related_skills.insertMulti("jiushi", "#jiushi-flip");

    addMetaObject<JujianCard>();
    addMetaObject<MingceCard>();
    addMetaObject<GanluCard>();
    addMetaObject<XianzhenCard>();
    addMetaObject<XianzhenSlashCard>();
    addMetaObject<XuanhuoCard>();
    */
    addMetaObject<DaleiCard>();
}

ADD_PACKAGE(QJWM)
