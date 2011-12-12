#include "general.h"
#include "standard.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "serverplayer.h"
#include "room.h"
#include "standard-skillcards.h"
#include "ai.h"

class Yiji:public MasochismSkill{
public:
    Yiji():MasochismSkill("yiji"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *guojia, const DamageStruct &damage) const{
        Room *room = guojia->getRoom();

        if(!room->askForSkillInvoke(guojia, objectName()))
            return;

        room->playSkillEffect(objectName());

        int x = damage.damage, i;
        for(i=0; i<x; i++){
            guojia->drawCards(2);
            QList<int> yiji_cards = guojia->handCards().mid(guojia->getHandcardNum() - 2);

            while(room->askForYiji(guojia, yiji_cards))
                ; // empty loop
        }

    }
};

class Fankui:public MasochismSkill{
public:
    Fankui():MasochismSkill("fankui"){

    }

    virtual void onDamaged(ServerPlayer *simayi, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = simayi->getRoom();
        QVariant data = QVariant::fromValue(from);
        if(from && !from->isNude() && room->askForSkillInvoke(simayi, "fankui", data)){
            int card_id = room->askForCardChosen(simayi, from, "he", "fankui");
            if(room->getCardPlace(card_id) == Player::Hand)
                room->moveCardTo(Sanguosha->getCard(card_id), simayi, Player::Hand, false);
            else
                room->obtainCard(simayi, card_id);
            room->playSkillEffect(objectName());
        }
    }
};

class Qingguo:public OneCardViewAsSkill{
public:
    Qingguo():OneCardViewAsSkill("qingguo"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isBlack() && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Jink *jink = new Jink(card->getSuit(), card->getNumber());
        jink->setSkillName(objectName());
        jink->addSubcard(card->getId());
        return jink;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "jink";
    }
};

class Tieji:public SlashBuffSkill{
public:
    Tieji():SlashBuffSkill("tieji"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *machao = effect.from;

        Room *room = machao->getRoom();
        if(effect.from->askForSkillInvoke("tieji", QVariant::fromValue(effect))){
            room->playSkillEffect(objectName());

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = machao;

            room->judge(judge);
            if(judge.isGood()){
                room->slashResult(effect, NULL);
                return true;
            }
        }

        return false;
    }
};

class Jizhi:public TriggerSkill{
public:
    Jizhi():TriggerSkill("jizhi"){
        frequency = Frequent;
        events << CardUsed << CardResponsed;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yueying, QVariant &data) const{
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->isNDTrick()){
            Room *room = yueying->getRoom();
            if(room->askForSkillInvoke(yueying, objectName())){
                room->playSkillEffect(objectName());
                yueying->drawCards(1);
            }
        }

        return false;
    }
};

class Qixi: public OneCardViewAsSkill{
public:
    Qixi():OneCardViewAsSkill("qixi"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isBlack();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Dismantlement *dismantlement = new Dismantlement(first->getSuit(), first->getNumber());
        dismantlement->addSubcard(first->getId());
        dismantlement->setSkillName(objectName());
        return dismantlement;
    }
};

class Xiaoji: public TriggerSkill{
public:
    Xiaoji():TriggerSkill("xiaoji"){
        events << CardLost;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *sunshangxiang, QVariant &data) const{
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->from_place == Player::Equip){
            Room *room = sunshangxiang->getRoom();
            if(room->askForSkillInvoke(sunshangxiang, objectName())){
                room->playSkillEffect(objectName());
                sunshangxiang->drawCards(2);
            }
        }

        return false;
    }
};

class Jijiu: public OneCardViewAsSkill{
public:
    Jijiu():OneCardViewAsSkill("jijiu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern.contains("peach") && player->getPhase() == Player::NotActive;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isRed();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Peach *peach = new Peach(first->getSuit(), first->getNumber());
        peach->addSubcard(first->getId());
        peach->setSkillName(objectName());
        return peach;
    }
};

void StandardPackage::addGenerals(){
    General *guojia, *simayi, *zhenji;

    simayi = new General(this, "simayi", "guan", 3);
    simayi->addSkill(new Fankui);

    guojia = new General(this, "guojia", "guan", 3);
    guojia->addSkill(new Yiji);

    zhenji = new General(this, "zhenji", "guan", 3, false);
    zhenji->addSkill(new Qingguo);

    General *zhangfei, *machao, *huangyueying;

    zhangfei = new General(this, "zhangfei", "jiang");
    zhangfei->addSkill(new Skill("paoxiao"));

    machao = new General(this, "machao", "jiang");
    machao->addSkill(new Tieji);

    huangyueying = new General(this, "huangyueying", "jiang", 3, false);
    huangyueying->addSkill(new Jizhi);

    General *ganning, *sunshangxiang;

    ganning = new General(this, "ganning", "min");
    ganning->addSkill(new Qixi);

    sunshangxiang = new General(this, "sunshangxiang", "min", 3, false);
    sunshangxiang->addSkill(new Xiaoji);

    General *huatuo = new General(this, "huatuo", "kou", 3);
    huatuo->addSkill(new Jijiu);

    // for skill cards
    addMetaObject<CheatCard>();
}

class Xiuluo: public PhaseChangeSkill{
public:
    Xiuluo():PhaseChangeSkill("xiuluo"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && !target->isKongcheng()
                && !target->getJudgingArea().isEmpty();
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(!target->askForSkillInvoke(objectName()))
            return false;

        Room *room = target->getRoom();
        int card_id = room->askForCardChosen(target, target, "j", objectName());
        const Card *card = Sanguosha->getCard(card_id);

        QString suit_str = card->getSuitString();
        QString pattern = QString(".%1").arg(suit_str.at(0).toUpper());
        QString prompt = QString("@xiuluo:::%1").arg(suit_str);
        if(room->askForCard(target, pattern, prompt)){
            room->throwCard(card);
        }

        return false;
    }
};

class Shenwei: public DrawCardsSkill{
public:
    Shenwei():DrawCardsSkill("shenwei"){
        frequency = Compulsory;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        return n + 2;
    }
};

class Ubuna:public ZeroCardViewAsSkill{
public:
    Ubuna():ZeroCardViewAsSkill("ubuna"){
    }

    virtual const Card *viewAs() const{
        return new UbunaCard;
    }
};

class Ubunb:public ZeroCardViewAsSkill{
public:
    Ubunb():ZeroCardViewAsSkill("ubunb"){
    }

    virtual const Card *viewAs() const{
        return Sanguosha->cloneSkillCard("BuzhenCard");
    }
};

class Ubunc:public ZeroCardViewAsSkill{
public:
    Ubunc():ZeroCardViewAsSkill("ubunc"){
    }

    virtual const Card *viewAs() const{
        return new UbuncCard;
    }
};

class Ubund:public ZeroCardViewAsSkill{
public:
    Ubund():ZeroCardViewAsSkill("ubund"){
    }

    virtual const Card *viewAs() const{
        return new UbundCard;
    }
};

class Ubune: public TriggerSkill{
public:
    Ubune():TriggerSkill("ubune"){
        events << Dying;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        DyingStruct dying = data.value<DyingStruct>();
        if(dying.who == player && player->askForSkillInvoke(objectName())){
            RecoverStruct rev;
            rev.who = player;
            rev.recover = player->getMaxHP();
            player->getRoom()->recover(player, rev);
            player->getRoom()->setPlayerProperty(player, "hp", player->getMaxHP());
        }
        return false;
    }
};

TestPackage::TestPackage()
    :Package("test")
{    
    General *shenlvbu1 = new General(this, "shenlvbu1", "god", 8, true, true);
    shenlvbu1->addSkill("cuju");
    shenlvbu1->addSkill("wubang");
    shenlvbu1->addSkill("huanshu");

    General *shenlvbu2 = new General(this, "shenlvbu2", "god", 4, true, true);
    shenlvbu2->addSkill("cuju");
    shenlvbu2->addSkill("huanshu");
    shenlvbu2->addSkill(new Xiuluo);
    shenlvbu2->addSkill(new Shenwei);
    shenlvbu2->addSkill(new Skill("shenji"));

    General *ubuntenkei = new General(this, "ubuntenkei", "god", 4, false, true);
    ubuntenkei->addSkill(new Ubuna);
    addMetaObject<UbunaCard>();
    ubuntenkei->addSkill(new Ubunb);
    ubuntenkei->addSkill(new Ubunc);
    addMetaObject<UbuncCard>();
    ubuntenkei->addSkill(new Ubund);
    addMetaObject<UbundCard>();
    ubuntenkei->addSkill(new Ubune);

    new General(this, "sujiang", "god", 5, true, true);
    new General(this, "sujiangf", "god", 5, false, true);
}

ADD_PACKAGE(Test)
