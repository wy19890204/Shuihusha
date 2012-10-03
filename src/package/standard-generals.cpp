#include "general.h"
#include "standard.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "serverplayer.h"
#include "room.h"
#include "standard-generals.h"
#include "ai.h"

JianaiCard::JianaiCard(){
    will_throw = false;
}

void JianaiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = NULL;
    if(targets.isEmpty()){
        foreach(ServerPlayer *player, room->getAlivePlayers()){
            if(player != source){
                target = player;
                break;
            }
        }
    }else
        target = targets.first();

    room->obtainCard(target, this, false);

    int old_value = source->getMark("jianai");
    int new_value = old_value + subcards.length();
    room->setPlayerMark(source, "jianai", new_value);

    if(old_value < 2 && new_value >= 2){
        RecoverStruct recover;
        recover.card = this;
        recover.who = source;
        room->recover(source, recover);
    }
}

class JianaiViewAsSkill:public ViewAsSkill{
public:
    JianaiViewAsSkill():ViewAsSkill("jianai"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(ServerInfo.GameMode == "04_1v3"
           && selected.length() + Self->getMark("jianai") >= 2)
           return false;
        else
            return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        JianaiCard *jianai_card = new JianaiCard;
        jianai_card->addSubcards(cards);
        return jianai_card;
    }
};

class Jianai: public PhaseChangeSkill{
public:
    Jianai():PhaseChangeSkill("jianai"){
        view_as_skill = new JianaiViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::NotActive
                && target->hasUsed("JianaiCard");
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        target->getRoom()->setPlayerMark(target, "jianai", 0);

        return false;
    }
};

FeigongCard::FeigongCard(){

}

bool FeigongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->canSlash(to_select);
}

void FeigongCard::use(Room *room, ServerPlayer *jidan, const QList<ServerPlayer *> &targets) const{
    QList<ServerPlayer *> lieges = room->getLieges("shu", jidan);
    const Card *slash = NULL;

    QVariant tohelp = QVariant::fromValue((PlayerStar)jidan);
    foreach(ServerPlayer *liege, lieges){
        slash = room->askForCard(liege, "slash", "@feigong-slash:" + jidan->objectName(), tohelp);
        if(slash){
            CardUseStruct card_use;
            card_use.card = slash;
            card_use.from = jidan;
            card_use.to << targets.first();

            room->useCard(card_use);
            return;
        }
    }
}

class FeigongViewAsSkill:public ZeroCardViewAsSkill{
public:
    FeigongViewAsSkill():ZeroCardViewAsSkill("feigong$"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->hasLordSkill("feigong") && Slash::IsAvailable(player);
    }

    virtual const Card *viewAs() const{
        return new FeigongCard;
    }
};

class Feigong: public TriggerSkill{
public:
    Feigong():TriggerSkill("feigong$"){
        events << CardAsked;
        default_choice = "ignore";

        view_as_skill = new FeigongViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasLordSkill("feigong");
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *jidan, QVariant &data) const{
        if (jidan == NULL) return false;
        QString pattern = data.toString();
        if(pattern != "slash")
            return false;

        QList<ServerPlayer *> lieges = room->getLieges("shu", jidan);
        if(lieges.isEmpty())
            return false;

        if(!room->askForSkillInvoke(jidan, objectName()))
            return false;

        room->playSkillEffect(objectName(), getEffectIndex(jidan, NULL));

        QVariant tohelp = QVariant::fromValue((PlayerStar)jidan);
        foreach(ServerPlayer *liege, lieges){
            const Card *slash = room->askForCard(liege, "slash", "@feigong-slash:" + jidan->objectName(), tohelp);
            if(slash){
                room->provide(slash);
                return true;
            }
        }

        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *player, const Card *) const{
        int r = 1 + qrand() % 2;
        if(player->getGeneralName() == "jingtianming" || player->getGeneral2Name() == "jingtianming")
            r += 2;

        return r;
    }
};

class Moshou: public TriggerSkill{
public:
    Moshou():TriggerSkill("moshou"){
        events << SlashEffected << TargetConfirming;

        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *jingtianming, QVariant &data) const{

        if(event == TargetConfirming){

            CardUseStruct use = data.value<CardUseStruct>();
            if(use.card && use.card->inherits("Slash")){

                room->playSkillEffect(objectName());

                LogMessage log;
                log.type = "#Moshou";
                log.from = use.from;
                log.to << jingtianming;
                log.arg = objectName();
                room->sendLog(log);
                QVariant dataforai = QVariant::fromValue(jingtianming);
                if(!room->askForCard(use.from, ".Basic", "@moshou-discard", dataforai, CardDiscarded))
                    jingtianming->addMark("moshou");
            }
        }
        else {
            if(jingtianming->getMark("moshou") > 0){
                jingtianming->setMark("moshou", jingtianming->getMark("moshou") - 1);
                return true;
            }
        }

        return false;
    }
};

class Shangxian: public PhaseChangeSkill{
public:
    Shangxian():PhaseChangeSkill("shangxian"){

    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *jingtianming) const{
        Room *room = jingtianming->getRoom();

        switch(jingtianming->getPhase()){
        case Player::Play: {
            bool invoked = jingtianming->askForSkillInvoke(objectName());
            if(invoked)
                jingtianming->setFlags("shangxian");

            return invoked;
        }
        case Player::NotActive: {
            if(jingtianming->hasFlag("shangxian")){
                if(jingtianming->isKongcheng() || !room->askForDiscard(jingtianming, "shangxian", 1, 1, true))
                    return false;

                ServerPlayer *player = room->askForPlayerChosen(jingtianming, room->getOtherPlayers(jingtianming), objectName());

                QString name = player->getGeneralName();
                if(name == "zhugeliang" || name == "shenzhugeliang" || name == "jiru")
                    room->playSkillEffect("shangxian", 1);
                else
                    room->playSkillEffect("shangxian", 2);

                LogMessage log;
                log.type = "#Shangxian";
                log.from = jingtianming;
                log.to << player;
                room->sendLog(log);

                PlayerStar p = player;
                room->setTag("ShangxianTarget", QVariant::fromValue(p));
            }
            break;
        }
        default:
            break;
        }
        return false;
    }
};

class ShangxianGive: public PhaseChangeSkill{
public:
    ShangxianGive():PhaseChangeSkill("#shangxian-give"){

    }

    virtual int getPriority() const{
        return -4;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->getPhase() == Player::NotActive;
    }

    virtual bool onPhaseChange(ServerPlayer *jingtianming) const{
        Room *room = jingtianming->getRoom();
        if(!room->getTag("ShangxianTarget").isNull())
        {
            PlayerStar target = room->getTag("ShangxianTarget").value<PlayerStar>();
            room->removeTag("ShangxianTarget");
            if(target->isAlive())
                target->gainAnExtraTurn();
        }
        return false;
    }
};

class Tianzhi: public PhaseChangeSkill{
public:
    Tianzhi():PhaseChangeSkill("tianzhi$"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->getPhase() == Player::Start
                && target->hasLordSkill("tianzhi")
                && target->isAlive()
                && target->getMark("tianzhi") == 0;
    }

    virtual bool onPhaseChange(ServerPlayer *jingtianming) const{
        Room *room = jingtianming->getRoom();

        bool can_invoke = true;
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(jingtianming->getHp() > p->getHp()){
                can_invoke = false;
                break;
            }
        }

        if(can_invoke){
            room->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#TianzhiWake";
            log.from = jingtianming;
            log.arg = QString::number(jingtianming->getHp());
            log.arg2 = objectName();
            room->sendLog(log);

            room->setPlayerMark(jingtianming, "tianzhi", 1);
            jingtianming->gainMark("@waked");
            room->setPlayerProperty(jingtianming, "maxhp", jingtianming->getMaxHp() + 1);

            RecoverStruct recover;
            recover.who = jingtianming;
            room->recover(jingtianming, recover);

            room->acquireSkill(jingtianming, "feigong");
        }

        return false;
    }
};

ShengxueCard::ShengxueCard(){
    target_fixed = true;
    once = true;
}

void ShengxueCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    QList<int> cards = room->getNCards(3), left;
    left = cards;

    QList<int> hearts;
    foreach(int card_id, cards){
        const Card *card = Sanguosha->getCard(card_id);
        if(card->getSuit() == Card::Heart)
            hearts << card_id;
    }

    if(!hearts.isEmpty()){
        room->fillAG(cards, source);

        while(!hearts.isEmpty()){
            int card_id = room->askForAG(source, hearts, true, "shengxue");
            if(card_id == -1)
                break;

            if(!hearts.contains(card_id))
                continue;

            hearts.removeOne(card_id);
            left.removeOne(card_id);

            source->obtainCard(Sanguosha->getCard(card_id));
            room->showCard(source, card_id);
        }

        source->invoke("clearAG");
    }

    if(!left.isEmpty())
        room->askForGuanxing(source, left, true);
 }

class Shengxue: public ZeroCardViewAsSkill{
public:
    Shengxue():ZeroCardViewAsSkill("shengxue"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ShengxueCard") && player->getHandcardNum() > player->getMaxHp();
    }

    virtual const Card *viewAs() const{
        return new ShengxueCard;
    }
};

class Feiyue: public TriggerSkill{
public:
    Feiyue():TriggerSkill("feiyue"){
        frequency = Compulsory;
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && TriggerSkill::triggerable(target) && !target->getArmor()
                && !target->hasFlag("wuqian") && target->getMark("qinggang") == 0;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *jiru, QVariant &data) const{
        QString pattern = data.toString();

        if(pattern != "jink")
            return false;

        if(jiru->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = jiru;

            room->judge(judge);

            if(judge.isGood()){
                room->setEmotion(jiru, "armor/eight_diagram");
                Jink *jink = new Jink(Card::NoSuit, 0);
                jink->setSkillName(objectName());
                room->provide(jink);
                //room->setEmotion(jiru, "good");
                return true;
            }else
                room->setEmotion(jiru, "bad");
        }

        return false;
    }
};

class Baihe: public TriggerSkill{
public:
    Baihe():TriggerSkill("baihe"){
        events << TargetConfirmed << CardEffected;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == TargetConfirmed){
            CardUseStruct use = data.value<CardUseStruct>();
            if(use.to.length() <= 1 || !use.to.contains(player) ||
               !use.card->inherits("TrickCard") ||
               !room->askForSkillInvoke(player, objectName(), data))
                    return false;

            player->tag["Baihe"] = use.card->getEffectiveId();
            room->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#BaiheAvoid";
            log.from = player;
            log.arg = use.card->objectName();
            log.arg2 = objectName();

            room->sendLog(log);
            player->drawCards(1);
        }
        else{
            if(!player->isAlive() || !player->hasSkill(objectName()))
                return false;

            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(player->tag["Baihe"].isNull() || player->tag["Baihe"].toInt() != effect.card->getEffectiveId())
                return false;

            player->tag["Baihe"] = QVariant(QString());
            return true;
        }

        return false;
    }
};

class Jiansheng:public OneCardViewAsSkill{
public:
    Jiansheng():OneCardViewAsSkill("jiansheng"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "slash";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        if(!card->isRed())
            return false;

        if(card == Self->getWeapon() && card->objectName() == "crossbow")
            return Self->canSlashWithoutCrossbow();
        else
            return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Card *slash = new Slash(card->getSuit(), card->getNumber());
        slash->addSubcard(card->getId());
        slash->setSkillName(objectName());
        return slash;
    }
};

class Yixian: public PhaseChangeSkill{
public:
    Yixian():PhaseChangeSkill("yixian"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->isWounded();
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::RoundStart)
            return false;
        Room *room = player->getRoom();
        QList<ServerPlayer *> ltys = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *lingtianyi, ltys){
            if(!player->isWounded())
                break;
            if(lingtianyi->isKongcheng())
                continue;
            QVariant data = QVariant::fromValue((PlayerStar)player);
            QString prompt = "@yixian:" + player->objectName();
            const Card *card = room->askForCard(lingtianyi, "BasicCard,TrickCard", prompt, true, data, CardDiscarded);
            if(card){
                RecoverStruct lty;
                lty.card = card;
                lty.who = lingtianyi;

                room->playSkillEffect(objectName());
                LogMessage log;
                log.from = lingtianyi;
                log.to << player;
                log.type = "#UseSkill";
                log.arg = objectName();
                room->sendLog(log);

                room->recover(player, lty, true);
            }
        }
        return false;
    }
};

class Feiming: public TriggerSkill{
public:
    Feiming():TriggerSkill("feiming"){
        events << Dying;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *ren, QVariant &data) const{
        DyingStruct dying = data.value<DyingStruct>();
        if(dying.who == ren && ren->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = ren;

            room->judge(judge);
            if(judge.isGood()){
                RecoverStruct rev;
                rev.card = judge.card;
                rev.recover = ren->getLostHp(false) - ren->getMaxHp() + 1;
                rev.who = ren;
                room->recover(ren, rev);
                if(ren->getHp() != 1)
                    room->setPlayerProperty(ren, "hp", 1);
            }
        }
        return false;
    }
};

class Daowang: public OneCardViewAsSkill{
public:
    Daowang():OneCardViewAsSkill("daowang"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Snatch *snatch = new Snatch(first->getSuit(), first->getNumber());
        snatch->addSubcard(first->getId());
        snatch->setSkillName(objectName());
        return snatch;
    }
};

class Shenxing: public ClientSkill{
public:
    Shenxing():ClientSkill("shenxing"){
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(to->hasSkill(objectName()))
            return +1;
        else if(from->hasSkill(objectName()))
            return -1;
        else
            return 0;
    }
};

class Leishen:public TriggerSkill{
public:
    Leishen():TriggerSkill("leishen"){
        events << TargetConfirmed << SlashProceed << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event , Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == TargetConfirmed){
            CardUseStruct use = data.value<CardUseStruct>();
            bool caninvoke = false;
            if(use.card->inherits("Slash") && use.from->hasSkill(objectName())
                && use.to.contains(player) && use.from->getPhase() == Player::Play){
                   caninvoke = true;
            }
            int handcardnum = player->getHandcardNum();
            if(caninvoke && (handcardnum >= use.from->getHp() || handcardnum <= use.from->getAttackRange()) &&
               use.from->askForSkillInvoke("leishen", QVariant::fromValue(player))){
                    room->playSkillEffect(objectName());
                room->setPlayerFlag(player, "LeishenTarget");
            }
        }
        else if(event == SlashProceed){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(effect.from->hasSkill(objectName()) && effect.to->hasFlag("LeishenTarget")){
                room->slashResult(effect, NULL);
                return true;
            }
        }else if(event == CardFinished){
            CardUseStruct use = data.value<CardUseStruct>();
            foreach(ServerPlayer *to, use.to){
                if(to->hasFlag("LeishenTarget"))
                    room->setPlayerFlag(to, "-LeishenTarget");
            }

        }
        return false;
    }
};

ShangtongCard::ShangtongCard(){
    will_throw = false;
}

bool ShangtongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty() || to_select == Self)
        return false;

    const Card *card = Sanguosha->getCard(subcards.first());
    const EquipCard *equip = qobject_cast<const EquipCard *>(card);
    int equip_index = static_cast<int>(equip->location());
    return to_select->getEquip(equip_index) == NULL;
}

void ShangtongCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *bandashi = effect.from;
    bandashi->getRoom()->moveCardTo(this, effect.to, Player::Equip);
    bandashi->drawCards(1);
}

class Shangtong: public OneCardViewAsSkill{
public:
    Shangtong():OneCardViewAsSkill("shangtong"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getTypeId() == Card::Equip;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ShangtongCard *shangtong_card = new ShangtongCard();
        shangtong_card->addSubcard(card_item->getFilteredCard());
        return shangtong_card;
    }
};

class Jieyong: public TriggerSkill{
public:
    Jieyong():TriggerSkill("jieyong"){
        events << CardLost;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *bandashi = room->findPlayerBySkillName(objectName());
        ServerPlayer *current = room->getCurrent();

        if(bandashi == NULL)
            return false;
        if(bandashi == current)
            return false;
        if(current->getPhase() == Player::Discard){
            QVariantList jieyong = bandashi->tag["Jieyong"].toList();

            CardMoveStar move = data.value<CardMoveStar>();
                jieyong << move->card_id;

            bandashi->tag["Jieyong"] = jieyong;
        }

        return false;
    }
};

class JieyongGet: public PhaseChangeSkill{
public:
    JieyongGet():PhaseChangeSkill("#jieyong-get"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && !target->hasSkill("jieyong");
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->isDead())
            return false;

        Room *room = player->getRoom();
        ServerPlayer *bandashi = room->findPlayerBySkillName(objectName());
        if(bandashi == NULL)
            return false;

        QVariantList jieyong_cards = bandashi->tag["Jieyong"].toList();
        bandashi->tag.remove("Jieyong");

        QList<int> cards;
        foreach(QVariant card_data, jieyong_cards){
            int card_id = card_data.toInt();
            if(room->getCardPlace(card_id) == Player::DiscardedPile)
                cards << card_id;
        }

        if(cards.isEmpty())
            return false;

        if(bandashi->askForSkillInvoke("jieyong", cards.length())){
            room->fillAG(cards, bandashi);

            int to_back = room->askForAG(bandashi, cards, false, objectName());
            player->obtainCard(Sanguosha->getCard(to_back));

            cards.removeOne(to_back);

            bandashi->invoke("clearAG");

            foreach(int card_id, cards)
                bandashi->obtainCard(Sanguosha->getCard(card_id));
        }

        return false;
    }
};

///////////////////////////////
class Jianxiong:public MasochismSkill{
public:
    Jianxiong():MasochismSkill("jianxiong"){
    }

    virtual void onDamaged(ServerPlayer *caocao, const DamageStruct &damage) const{
        Room *room = caocao->getRoom();
        const Card *card = damage.card;
        if(!room->obtainable(card, caocao))
            return;

            QVariant data = QVariant::fromValue(card);
            if(room->askForSkillInvoke(caocao, "jianxiong", data)){
            room->playSkillEffect(objectName());
            caocao->obtainCard(card);
        }
    }
};

class Hujia:public TriggerSkill{
public:
    Hujia():TriggerSkill("hujia$"){
        events << CardAsked;
        default_choice = "ignore";
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasLordSkill("hujia");
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *caocao, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "jink")
            return false;

        QList<ServerPlayer *> lieges = room->getLieges("wei", caocao);
        if(lieges.isEmpty())
            return false;

        if(!room->askForSkillInvoke(caocao, objectName()))
            return false;

        room->playSkillEffect(objectName());
        QVariant tohelp = QVariant::fromValue((PlayerStar)caocao);
        foreach(ServerPlayer *liege, lieges){
            const Card *jink = room->askForCard(liege, "jink", "@hujia-jink:" + caocao->objectName(), tohelp);
            if(jink){
                room->provide(jink);
                return true;
            }
        }

        return false;
    }
};

class TuxiViewAsSkill: public ZeroCardViewAsSkill{
public:
    TuxiViewAsSkill():ZeroCardViewAsSkill("tuxi"){
    }

    virtual const Card *viewAs() const{
        return new TuxiCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@tuxi";
    }
};

class Tuxi:public PhaseChangeSkill{
public:
    Tuxi():PhaseChangeSkill("tuxi"){
        view_as_skill = new TuxiViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *zhangliao) const{
        if(zhangliao->getPhase() == Player::Draw){
            Room *room = zhangliao->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(zhangliao);
            foreach(ServerPlayer *player, other_players){
                if(!player->isKongcheng()){
                    can_invoke = true;
                    break;
                }
            }

            if(can_invoke && room->askForUseCard(zhangliao, "@@tuxi", "@tuxi-card"))
                return true;
        }

        return false;
    }
};

class Tiandu:public TriggerSkill{
public:
    Tiandu():TriggerSkill("tiandu"){
        frequency = Frequent;
        default_choice = "no";
        events << FinishJudge;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *guojia, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();
        CardStar card = judge->card;

        QVariant data_card = QVariant::fromValue(card);
        if(guojia->askForSkillInvoke(objectName(), data_card)){
            if(card->objectName() == "shit"){
                QString result = room->askForChoice(guojia, objectName(), "yes+no");
                if(result == "no")
                    return false;
            }

            guojia->obtainCard(judge->card);
            room->playSkillEffect(objectName());

            return true;
        }

        return false;
    }
};

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
            guojia->drawCards(2, true, objectName());
            QList<int> yiji_cards;
            foreach(const Card *card, guojia->getHandcards()){
                if(card->hasFlag(objectName())){
                    room->setCardFlag(card, "-" + objectName());
                    yiji_cards << card->getEffectiveId();
                }
            }

            if(yiji_cards.isEmpty())
                continue;

            while(room->askForYiji(guojia, yiji_cards))
                ; // empty loop
        }

    }
};

class Ganglie:public MasochismSkill{
public:
    Ganglie():MasochismSkill("ganglie"){

    }

    virtual void onDamaged(ServerPlayer *xiahou, const DamageStruct &damage) const{
        ServerPlayer *from = damage.from;
        Room *room = xiahou->getRoom();
        QVariant source = QVariant::fromValue(from);

        if(from && from->isAlive() && room->askForSkillInvoke(xiahou, "ganglie", source)){
            room->playSkillEffect(objectName());

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart):(.*)");
            judge.good = false;
            judge.reason = objectName();
            judge.who = xiahou;

            room->judge(judge);
            if(judge.isGood()){
                if(!room->askForDiscard(from, objectName(), 2, 2, true)){
                    DamageStruct damage;
                    damage.from = xiahou;
                    damage.to = from;

                    room->setEmotion(xiahou, "good");
                    room->damage(damage);
                }
            }else
                room->setEmotion(xiahou, "bad");
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
            room->obtainCard(simayi, card_id, room->getCardPlace(card_id) != Player::Hand);
            room->playSkillEffect(objectName());
        }
    }
};

class GuicaiViewAsSkill:public OneCardViewAsSkill{
public:
    GuicaiViewAsSkill():OneCardViewAsSkill(""){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@guicai";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new GuicaiCard;
        card->setSuit(card_item->getFilteredCard()->getSuit());
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Guicai: public TriggerSkill{
public:
    Guicai():TriggerSkill("guicai"){
        view_as_skill = new GuicaiViewAsSkill;

        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target) && !target->isKongcheng();
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@guicai-card" << judge->who->objectName()
                << objectName() << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");
        room->setPlayerFlag(player, "retrial");
        const Card *card = room->askForCard(player, "@guicai", prompt, data);

        if(card){
            // the only difference for Guicai & Guidao
            room->throwCard(judge->card, judge->who);

            judge->card = Sanguosha->getCard(card->getEffectiveId());
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = player;
            log.to << judge->who;
            log.card_str = card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);
        }
        else
            room->setPlayerFlag(player, "-retrial");

        return false;
    }
};

class LuoyiBuff: public TriggerSkill{
public:
    LuoyiBuff():TriggerSkill("#luoyi"){
        events << Predamage;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasFlag("luoyi") && target->isAlive();
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *xuchu, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        const Card *reason = damage.card;
        if(reason == NULL)
            return false;

        if(reason->inherits("Slash") || reason->inherits("Duel")){
            LogMessage log;
            log.type = "#LuoyiBuff";
            log.from = damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            room->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

class Luoyi: public DrawCardsSkill{
public:
    Luoyi():DrawCardsSkill("luoyi"){

    }

    virtual int getDrawNum(ServerPlayer *xuchu, int n) const{
        Room *room = xuchu->getRoom();
        if(room->askForSkillInvoke(xuchu, objectName())){
            room->playSkillEffect(objectName());

            xuchu->setFlags(objectName());
            return n - 1;
        }else
            return n;
    }
};

class Luoshen:public TriggerSkill{
public:
    Luoshen():TriggerSkill("luoshen"){
        events << PhaseChange << FinishJudge;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *zhenji, QVariant &data) const{
        if(event == PhaseChange && zhenji->getPhase() == Player::Start){
            while(zhenji->askForSkillInvoke("luoshen")){
                room->playSkillEffect(objectName());

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(spade|club):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = zhenji;
                judge.time_consuming = true;

                room->judge(judge);
                if(judge.isBad())
                    break;
            }

        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(judge->reason == objectName()){
                if(judge->card->isBlack()){
                    zhenji->obtainCard(judge->card);
                    return true;
                }
            }
        }

        return false;
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

class Longdan:public OneCardViewAsSkill{
public:
    Longdan():OneCardViewAsSkill("longdan"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                // jink as slash
                return card->inherits("Jink");
            }

        case Client::Responsing:{
                QString pattern = ClientInstance->getPattern();
                if(pattern == "slash")
                    return card->inherits("Jink");
                else if(pattern == "jink")
                    return card->inherits("Slash");
            }

        default:
            return false;
        }
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "jink" || pattern == "slash";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        if(card->inherits("Slash")){
            Jink *jink = new Jink(card->getSuit(), card->getNumber());
            jink->addSubcard(card);
            jink->setSkillName(objectName());
            return jink;
        }else if(card->inherits("Jink")){
            Slash *slash = new Slash(card->getSuit(), card->getNumber());
            slash->addSubcard(card);
            slash->setSkillName(objectName());
            return slash;
        }else
            return NULL;
    }
};

class Tieji:public TriggerSkill{
public:
    Tieji():TriggerSkill("tieji"){
        events << TargetConfirmed << SlashProceed << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{

        if(event == TargetConfirmed){
            CardUseStruct use = data.value<CardUseStruct>();
            bool caninvoke = false;
            if(use.card->inherits("Slash") && use.from->hasSkill(objectName())
                && use.to.contains(player)){
                   caninvoke = true;
            }
            if(caninvoke && use.from->askForSkillInvoke("tieji", QVariant::fromValue(player))){
                room->playSkillEffect(objectName());

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = use.from;

                room->judge(judge);
                if(judge.isGood()){
                     room->setPlayerFlag(player, "TiejiTarget");
                }
            }
        }
        else if(event == SlashProceed){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(effect.from->hasSkill(objectName()) && effect.to->hasFlag("TiejiTarget")){
                room->slashResult(effect, NULL);
                return true;
            }
        }else if(event == CardFinished){
            CardUseStruct use = data.value<CardUseStruct>();
            foreach(ServerPlayer *to, use.to){
                if(to->hasFlag("TiejiTarget"))
                    room->setPlayerFlag(to, "-TiejiTarget");
            }

        }
        return false;
    }
};

class Guanxing:public PhaseChangeSkill{
public:
    Guanxing():PhaseChangeSkill("guanxing"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *zhuge) const{
        if(zhuge->getPhase() == Player::Start &&
           zhuge->askForSkillInvoke(objectName()))
        {
            Room *room = zhuge->getRoom();
            room->playSkillEffect(objectName());

            int n = qMin(5, room->alivePlayerCount());
            room->askForGuanxing(zhuge, room->getNCards(n, false), false);
        }

        return false;
    }
};

class Kongcheng: public ProhibitSkill{
public:
    Kongcheng():ProhibitSkill("kongcheng"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        if(card->inherits("Slash") || card->inherits("Duel"))
            return to->isKongcheng();
        else
            return false;
    }
};

class KongchengEffect: public TriggerSkill{
public:
    KongchengEffect():TriggerSkill("#kongcheng-effect"){
        frequency = Compulsory;

        events << CardLost;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->isKongcheng()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Hand)
                room->playSkillEffect("kongcheng");
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

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *yueying, QVariant &data) const{
        if (yueying == NULL) return false;
        CardStar card = NULL;
        if(event == CardUsed){
            CardUseStruct use = data.value<CardUseStruct>();
            card = use.card;
        }else if(event == CardResponsed)
            card = data.value<CardStar>();

        if(card->isNDTrick()){            
            if(room->askForSkillInvoke(yueying, objectName())){
                room->playSkillEffect(objectName());
                yueying->drawCards(1);
            }
        }

        return false;
    }
};

class Zhiheng:public ViewAsSkill{
public:
    Zhiheng():ViewAsSkill("zhiheng"){

    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;

        ZhihengCard *zhiheng_card = new ZhihengCard;
        zhiheng_card->addSubcards(cards);

        return zhiheng_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ZhihengCard");
    }
};

class Jiuyuan: public TriggerSkill{
public:
    Jiuyuan():TriggerSkill("jiuyuan$"){
        events << Dying << AskForPeachesDone;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL && target->hasLordSkill("jiuyuan");
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *sunquan, QVariant &data) const{
        switch(event){
        case Dying: {
                foreach(ServerPlayer *wu, room->getOtherPlayers(sunquan)){
                    if(wu->getKingdom() == "wu"){
                        room->playSkillEffect("jiuyuan", 1);
                        room->setPlayerFlag(sunquan, "jiuyuan");
                        break;
                    }
                }

                break;
            }

        case AskForPeachesDone:{
                if(sunquan->hasFlag("jiuyuan")){
                    room->setPlayerFlag(sunquan, "-jiuyuan");
                    if(sunquan->getHp() > 0)
                        room->playSkillEffect("jiuyuan", 2);
                }

                break;
            }

        default:
            break;
        }

        return false;
    }
};

class Yingzi:public DrawCardsSkill{
public:
    Yingzi():DrawCardsSkill("yingzi"){
        frequency = Frequent;
    }

    virtual int getDrawNum(ServerPlayer *zhouyu, int n) const{
        Room *room = zhouyu->getRoom();
        if(room->askForSkillInvoke(zhouyu, objectName())){
            room->playSkillEffect(objectName());
            return n + 1;
        }else
            return n;
    }
};

class Fanjian:public ZeroCardViewAsSkill{
public:
    Fanjian():ZeroCardViewAsSkill("fanjian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng() && ! player->hasUsed("FanjianCard");
    }

    virtual const Card *viewAs() const{
        return new FanjianCard;
    }
};

class Keji: public TriggerSkill{
public:
    Keji():TriggerSkill("keji"){
        events << CardResponsed;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *lvmeng, QVariant &data) const{
        CardStar card_star = data.value<CardStar>();
        if(card_star->inherits("Slash"))
            lvmeng->setFlags("keji_use_slash");

        return false;
    }
};

class KejiSkip: public PhaseChangeSkill{
public:
    KejiSkip():PhaseChangeSkill("#keji-skip"){
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *lvmeng) const{
        if(lvmeng->getPhase() == Player::Start){
            lvmeng->setFlags("-keji_use_slash");
        }else if(lvmeng->getPhase() == Player::Discard){
            if(!lvmeng->hasFlag("keji_use_slash") &&
               lvmeng->getSlashCount() == 0 &&
               lvmeng->askForSkillInvoke("keji"))
            {
                lvmeng->getRoom()->playSkillEffect("keji");

                return true;
            }
        }

        return false;
    }
};

class Lianying: public TriggerSkill{
public:
    Lianying():TriggerSkill("lianying"){
        events << CardLost;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , Room* room, ServerPlayer *luxun, QVariant &data) const{
        if (luxun == NULL) return false;
            CardMoveStar move = data.value<CardMoveStar>();
        if(luxun->isKongcheng() && move->from_place == Player::Hand){
            if(room->askForSkillInvoke(luxun, objectName(), data)){
                    room->playSkillEffect(objectName());
                luxun->drawCards(1);
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

class Kurou: public ZeroCardViewAsSkill{
public:
    Kurou():ZeroCardViewAsSkill("kurou"){

    }

    virtual const Card *viewAs() const{
        return new KurouCard;
    }
};

class Guose: public OneCardViewAsSkill{
public:
    Guose():OneCardViewAsSkill("guose"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getSuit() == Card::Diamond;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Indulgence *indulgence = new Indulgence(first->getSuit(), first->getNumber());
        indulgence->addSubcard(first->getId());
        indulgence->setSkillName(objectName());
        return indulgence;
    }
};

class LiuliViewAsSkill: public OneCardViewAsSkill{
public:
    LiuliViewAsSkill():OneCardViewAsSkill("liuli"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@liuli";
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LiuliCard *liuli_card = new LiuliCard;
        liuli_card->addSubcard(card_item->getFilteredCard());

        return liuli_card;
    }
};

class Liuli: public TriggerSkill{
public:
    Liuli():TriggerSkill("liuli"){
        view_as_skill = new LiuliViewAsSkill;

        events << TargetConfirming;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *daqiao, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();

		if(use.card && use.card->inherits("Slash") && use.to.contains(daqiao) && !daqiao->isNude() && room->alivePlayerCount() > 2){
            QList<ServerPlayer *> players = room->getOtherPlayers(daqiao);
            players.removeOne(use.from);

            bool can_invoke = false;
            foreach(ServerPlayer *p, players){
                if(daqiao->inMyAttackRange(p)){
                    can_invoke = true;
                    break;
                }
            }

            if(can_invoke){
                QString prompt = "@liuli:" + use.from->objectName();
                room->setPlayerFlag(use.from, "slash_source");
                if(room->askForUseCard(daqiao, "@@liuli", prompt)){
                    foreach(ServerPlayer *p, players){
                        if(p->hasFlag("liuli_target")){
                            use.to.insert(use.to.indexOf(daqiao), p);
                            use.to.removeOne(daqiao);

                            data = QVariant::fromValue(use);

                            room->setPlayerFlag(use.from, "-slash_source");
                            room->setPlayerFlag(p, "-liuli_target");
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }
};

class Jieyin: public ViewAsSkill{
public:
    Jieyin():ViewAsSkill("jieyin"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("JieyinCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() > 2)
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        JieyinCard *jieyin_card = new JieyinCard();
        jieyin_card->addSubcards(cards);

        return jieyin_card;
    }
};

class Xiaoji: public TriggerSkill{
public:
    Xiaoji():TriggerSkill("xiaoji"){
        events << CardLost;

        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *sunshangxiang, QVariant &data) const{
        if (sunshangxiang == NULL) return false;
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

class Wushuang:public TriggerSkill{
public:
    Wushuang():TriggerSkill("wushuang"){
        frequency = Compulsory;
        events << TargetConfirmed << SlashProceed << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target != NULL;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == TargetConfirmed){
            CardUseStruct use = data.value<CardUseStruct>();
            bool caninvoke = false;
            if(use.card->inherits("Slash") && use.from->hasSkill(objectName())
                && use.to.contains(player)){
                   caninvoke = true;
            }
            else if(use.card->inherits("Duel") && (
                    (use.from->hasSkill(objectName()) && use.from->objectName() == player->objectName())
                    || (player->hasSkill(objectName()) && use.to.contains(player)))){
                       caninvoke = true;
            }
            if(caninvoke){
                room->playSkillEffect(objectName());
                room->setPlayerFlag(player, "WushuangTarget");
            }
        }
        else if(event == SlashProceed){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            if(!effect.to->hasFlag("WushuangTarget"))
                return false;
            room->playSkillEffect(objectName());

            QString slasher = player->objectName();

            const Card *first_jink = NULL, *second_jink = NULL;
            first_jink = room->askForCard(effect.to, "jink", "@wushuang-jink-1:" + slasher, QVariant(), CardUsed);
            if(first_jink)
                second_jink = room->askForCard(effect.to, "jink", "@wushuang-jink-2:" + slasher, QVariant(), CardUsed);

            Card *jink = NULL;
            if(first_jink && second_jink){
                jink = new DummyCard;
                jink->addSubcard(first_jink);
                jink->addSubcard(second_jink);
            }

            room->slashResult(effect, jink);

            return true;

        }else if(event == CardFinished){
            foreach(ServerPlayer *to, room->getAllPlayers()){
                if(to->hasFlag("WushuangTarget"))
                    room->setPlayerFlag(to, "-WushuangTarget");
            }

        }
        return false;
    }
};

class Lijian: public OneCardViewAsSkill{
public:
    Lijian():OneCardViewAsSkill("lijian"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("LijianCard");
    }

    virtual bool viewFilter(const CardItem *) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LijianCard *lijian_card = new LijianCard;
        lijian_card->addSubcard(card_item->getCard()->getId());

        return lijian_card;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 0;
    }
};

class Biyue: public PhaseChangeSkill{
public:
    Biyue():PhaseChangeSkill("biyue"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *diaochan) const{
        if(diaochan->getPhase() == Player::Finish){
            Room *room = diaochan->getRoom();
            if(room->askForSkillInvoke(diaochan, objectName())){
                room->playSkillEffect(objectName());
                diaochan->drawCards(1);
            }
        }

        return false;
    }
};

class Qingnang: public OneCardViewAsSkill{
public:
    Qingnang():OneCardViewAsSkill("qingnang"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QingnangCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        QingnangCard *qingnang_card = new QingnangCard;
        qingnang_card->addSubcard(card_item->getCard()->getId());

        return qingnang_card;
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

class Qianxun: public ProhibitSkill{
public:
    Qianxun():ProhibitSkill("qianxun"){

    }

    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->inherits("Snatch") || card->inherits("Indulgence");
    }
};

class Mashu: public DistanceSkill{
public:
    Mashu():DistanceSkill("mashu")
    {
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        if(from->hasSkill(objectName()))
            return -1;
        else
            return 0;
    }
};

void StandardPackage::addGenerals(){

    General *jidan = new General(this, "jidan$", "shu");
    jidan->addSkill(new Jianai);
    jidan->addSkill(new Feigong);

    General *jingtianming = new General(this, "jingtianming$", "shu", 3);
    jingtianming->addSkill(new Moshou);
    jingtianming->addSkill(new Shangxian);
    jingtianming->addSkill(new ShangxianGive);
    jingtianming->addSkill(new Tianzhi);
    related_skills.insertMulti("shangxian", "#shangxian-give");

    General *jiru = new General(this, "jiru", "shu", 3);
    jiru->addSkill(new Shengxue); //@todo
    jiru->addSkill(new Feiyue);
    //jiru->addSkill(new Kanpo);

    General *genie = new General(this, "genie", "shu");
    genie->addSkill(new Baihe);
    genie->addSkill(new Jiansheng);

    General *duanmurong = new General(this, "duanmurong", "shu", 3);
    duanmurong->addSkill(new Yixian);
    duanmurong->addSkill(new Feiming); //@todo

    General *daozhi = new General(this, "daozhi", "shu", 3);
    daozhi->addSkill(new Daowang);
    daozhi->addSkill(new Shenxing); //@todo

    General *datiechui = new General(this, "datiechui", "shu");
    datiechui->addSkill(new Leishen);

    General *bandashi = new General(this, "bandashi", "wu", 3);
    bandashi->addSkill(new Shangtong); //@todo
    bandashi->addSkill(new Jieyong);
    bandashi->addSkill(new JieyongGet);
    related_skills.insertMulti("jieyong", "#jieyong-get");

    caocao = new General(this, "caocao$", "wei");
    caocao->addSkill(new Jianxiong);
    caocao->addSkill(new Hujia);

    simayi = new General(this, "simayi", "wei", 3);
    simayi->addSkill(new Fankui);
    simayi->addSkill(new Guicai);

    xiahoudun = new General(this, "xiahoudun", "wei");
    xiahoudun->addSkill(new Ganglie);

    zhangliao = new General(this, "zhangliao", "wei");
    zhangliao->addSkill(new Tuxi);

    xuchu = new General(this, "xuchu", "wei");
    xuchu->addSkill(new Luoyi);
    xuchu->addSkill(new LuoyiBuff);
    related_skills.insertMulti("luoyi", "#luoyi");

    guojia = new General(this, "guojia", "wei", 3);
    guojia->addSkill(new Tiandu);
    guojia->addSkill(new Yiji);

    zhenji = new General(this, "zhenji", "wei", 3, false);
    zhenji->addSkill(new Luoshen);
    zhenji->addSkill(new Qingguo);

    zhangfei = new General(this, "zhangfei", "shu");
    zhangfei->addSkill(new Skill("paoxiao"));

    zhugeliang = new General(this, "zhugeliang", "shu", 3);
    zhugeliang->addSkill(new Guanxing);
    zhugeliang->addSkill(new Kongcheng);
    zhugeliang->addSkill(new KongchengEffect);
    related_skills.insertMulti("kongcheng", "#kongcheng-effect");

    zhaoyun = new General(this, "zhaoyun", "shu");
    zhaoyun->addSkill(new Longdan);

    machao = new General(this, "machao", "shu");
    machao->addSkill(new Tieji);
    machao->addSkill(new Mashu);
    machao->addSkill(new SPConvertSkill("fanqun", "machao", "sp_machao"));

    huangyueying = new General(this, "huangyueying", "shu", 3, false);
    huangyueying->addSkill(new Jizhi);
    huangyueying->addSkill(new Skill("qicai", Skill::Compulsory));

    General *sunquan, *zhouyu, *lvmeng, *luxun, *ganning, *huanggai, *daqiao, *sunshangxiang;
    sunquan = new General(this, "sunquan$", "wu");
    sunquan->addSkill(new Zhiheng);
    sunquan->addSkill(new Jiuyuan);

    ganning = new General(this, "ganning", "wu");
    ganning->addSkill(new Qixi);

    lvmeng = new General(this, "lvmeng", "wu");
    lvmeng->addSkill(new Keji);
    lvmeng->addSkill(new KejiSkip);
    related_skills.insertMulti("keji", "#keji-skip");

    huanggai = new General(this, "huanggai", "wu");
    huanggai->addSkill(new Kurou);

    zhouyu = new General(this, "zhouyu", "wu", 3);
    zhouyu->addSkill(new Yingzi);
    zhouyu->addSkill(new Fanjian);

    daqiao = new General(this, "daqiao", "wu", 3, false);
    daqiao->addSkill(new Guose);
    daqiao->addSkill(new Liuli);

    luxun = new General(this, "luxun", "wu", 3);
    luxun->addSkill(new Qianxun);
    luxun->addSkill(new Lianying);

    sunshangxiang = new General(this, "sunshangxiang", "wu", 3, false);
    sunshangxiang->addSkill(new SPConvertSkill("chujia", "sunshangxiang", "sp_sunshangxiang"));
    sunshangxiang->addSkill(new Jieyin);
    sunshangxiang->addSkill(new Xiaoji);

    General *lvbu, *huatuo, *diaochan;

    huatuo = new General(this, "huatuo", "qun", 3);
    huatuo->addSkill(new Qingnang);
    huatuo->addSkill(new Jijiu);

    lvbu = new General(this, "lvbu", "qun");
    lvbu->addSkill(new Wushuang);

    diaochan = new General(this, "diaochan", "qun", 3, false);
    diaochan->addSkill(new Lijian);
    diaochan->addSkill(new Biyue);
    diaochan->addSkill(new SPConvertSkill("tuoqiao", "diaochan", "sp_diaochan"));

    // for skill cards
    addMetaObject<ZhihengCard>();
    addMetaObject<JianaiCard>();
    addMetaObject<TuxiCard>();
    addMetaObject<JieyinCard>();
    addMetaObject<KurouCard>();
    addMetaObject<LijianCard>();
    addMetaObject<FanjianCard>();
    addMetaObject<GuicaiCard>();
    addMetaObject<QingnangCard>();
    addMetaObject<LiuliCard>();
    addMetaObject<FeigongCard>();
}

TestPackage::TestPackage()
    :Package("test")
{
    // for test only
    new General(this, "sujiang", "god", 5, true, true);
    new General(this, "sujiangf", "god", 5, false, true);

    //@todo: Add images of no nationality's general, it cannot use god to express that.
    new General(this, "anjiang", "god", 4, false, true, true, false);
}

ADD_PACKAGE(Test)
