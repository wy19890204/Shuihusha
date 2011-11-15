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
                log.card_str = QString::number(card->getId());
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

QJWMPackage::QJWMPackage():Package("QJWM"){

    General *huarong = new General(this, "huarong", "wei", 4);
    huarong->addSkill(new Jingzhun);
    huarong->addSkill(new Kaixian);
    patterns.insert(".kaixian!", new KaixianPattern);

    General *liying = new General(this, "liying", "wei");
    liying->addSkill(new Kongliang);

    /*

    related_skills.insertMulti("jiushi", "#jiushi-flip");

    General *xushu = new General(this, "xushu", "shu", 3);
    xushu->addSkill(new Wuyan);
    xushu->addSkill(new Jujian);

    General *masu = new General(this, "masu", "shu", 3);
    masu->addSkill(new Xinzhan);
    masu->addSkill(new Huilei);

    General *fazheng = new General(this, "fazheng", "shu", 3);
    fazheng->addSkill(new Enyuan);
    fazheng->addSkill(new Xuanhuo);

    General *lingtong = new General(this, "lingtong", "wu");
    lingtong->addSkill(new Xuanfeng);

    General *xusheng = new General(this, "xusheng", "wu");
    xusheng->addSkill(new Pojun);

    General *wuguotai = new General(this, "wuguotai", "wu", 3, false);
    wuguotai->addSkill(new Ganlu);
    wuguotai->addSkill(new Buyi);

    General *chengong = new General(this, "chengong", "qun", 3);
    chengong->addSkill(new Zhichi);
    chengong->addSkill(new ZhichiClear);
    chengong->addSkill(new Mingce);

    related_skills.insertMulti("zhichi", "#zhichi-clear");

    General *gaoshun = new General(this, "gaoshun", "qun");
    gaoshun->addSkill(new Xianzhen);
    gaoshun->addSkill(new Jiejiu);

    addMetaObject<JujianCard>();
    addMetaObject<MingceCard>();
    addMetaObject<GanluCard>();
    addMetaObject<XianzhenCard>();
    addMetaObject<XianzhenSlashCard>();
    addMetaObject<XuanhuoCard>();
    addMetaObject<XinzhanCard>();
    */
}

ADD_PACKAGE(QJWM)
