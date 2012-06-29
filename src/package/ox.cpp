#include "ox.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"

GuibingCard::GuibingCard(){

}

bool GuibingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->canSlash(to_select);
}

void GuibingCard::use(Room *room, ServerPlayer *gaolian, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *to = targets.first();

    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(club|spade):(.*)");
    judge.good = true;
    judge.reason = objectName();
    judge.who = gaolian;

    room->judge(judge);

    if(judge.isGood()){
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("guibing");

        CardUseStruct use;
        use.from = gaolian;
        use.to << to;
        use.card = slash;
        room->useCard(use);
    }else
        room->setPlayerFlag(gaolian, "Guibing");
}

class GuibingViewAsSkill:public ZeroCardViewAsSkill{
public:
    GuibingViewAsSkill():ZeroCardViewAsSkill("guibing"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasFlag("Guibing") && Slash::IsAvailable(player);
    }

    virtual const Card *viewAs() const{
        return new GuibingCard;
    }
};

class Guibing: public TriggerSkill{
public:
    Guibing():TriggerSkill("guibing"){
        events << CardAsked;
        view_as_skill = new GuibingViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *gaolian, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "slash")
            return false;

        if(gaolian->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club|spade):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = gaolian;

            room->playSkillEffect(objectName());
            room->judge(judge);

            if(judge.isGood()){
                Slash *slash = new Slash(Card::NoSuit, 0);
                slash->setSkillName(objectName());
                room->provide(slash);
                return true;
            }
        }
        return false;
    }
};

HeiwuCard::HeiwuCard(){
    target_fixed = true;
}

void HeiwuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    int num = getSubcards().length();
    room->moveCardTo(this, NULL, Player::DrawPile, true);
    QList<int> fog = room->getNCards(num, false);
    room->askForGuanxing(source, fog, false);
};

class Heiwu:public ViewAsSkill{
public:
    Heiwu():ViewAsSkill("heiwu"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        HeiwuCard *heiwu_card = new HeiwuCard;
        heiwu_card->addSubcards(cards);
        return heiwu_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->isKongcheng();
    }
};

class Aoxiang: public TriggerSkill{
public:
    Aoxiang():TriggerSkill("aoxiang"){
        events << HpChanged;
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->isWounded())
            room->setGerenalGender("tongguan", "F");
        else
            room->setGerenalGender("tongguan", "M");
        /*
        if(player->getGeneralName() != "tongguanf")
            player->tag["AoxiangStore"] = player->getGeneralName();
        if(player->isWounded())
            //p:getGeneral():setGender(sgs.General_Female)
            //player->getGeneral()->setGender(General::Female);
            room->setPlayerProperty(player, "general", "tongguanf");
        else{
            QString gen_name = player->tag.value("AoxiangStore", "tongguan").toString();
            room->setPlayerProperty(player, "general", gen_name);
        }
        */
        return false;
    }
};

class AoxiangChange: public TriggerSkill{
public:
    AoxiangChange():TriggerSkill("#aox_cg"){
        events << GameStart;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &) const{
        if(player->getGeneral2Name() == "tongguan"){
            room->setPlayerProperty(player, "general2", player->getGeneralName());
            room->setPlayerProperty(player, "general", "tongguan");
        }
        return false;
    }
};

ZhengfaCard::ZhengfaCard(){
    once = true;
    will_throw = false;
    mute = true;
}

bool ZhengfaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!Self->hasUsed("ZhengfaCard"))
        return targets.isEmpty() && to_select->getGender() != Self->getGender()
            && !to_select->isWounded() && !to_select->isKongcheng() && to_select != Self;
    else
        return targets.length() < Self->getHp() && Self->canSlash(to_select);
}

void ZhengfaCard::use(Room *room, ServerPlayer *tonguan, const QList<ServerPlayer *> &targets) const{
    if(tonguan->hasFlag("zhengfa-success")){
        foreach(ServerPlayer *tarmp, targets)
            room->cardEffect(this, tonguan, tarmp);
        room->playSkillEffect("zhengfa", tonguan->getGeneral()->isMale()? 2: 4);
    }
    else{
        bool success = tonguan->pindian(targets.first(), "zhengfa", this);
        if(success){
            room->playSkillEffect("zhengfa", tonguan->getGeneral()->isMale()? 1: 3);
            room->setPlayerFlag(tonguan, "zhengfa-success");
            room->askForUseCard(tonguan, "@@zhengfa", "@zhengfa-effect", true);
        }else{
            room->playSkillEffect("zhengfa", tonguan->getGeneral()->isMale()? 5: 6);
            tonguan->turnOver();
        }
    }
}

void ZhengfaCard::onEffect(const CardEffectStruct &effect) const{
    DamageStruct damage;
    damage.from = effect.from;
    damage.to = effect.to;
    damage.card = this;
    effect.from->getRoom()->damage(damage);
}

class Zhengfa: public ViewAsSkill{
public:
    Zhengfa():ViewAsSkill("zhengfa"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ZhengfaCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !Self->hasUsed("ZhengfaCard")? !to_select->isEquipped(): false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@zhengfa";
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        Card *zhengfcard = new ZhengfaCard;
        if(!cards.isEmpty())
            zhengfcard->addSubcard(cards.first()->getCard());
        return zhengfcard;
    }
};

LianmaCard::LianmaCard(){
    target_fixed = true;
    once = true;
}

void LianmaCard::use(Room *room, ServerPlayer *huyanzhuo, const QList<ServerPlayer *> &) const{
    QList<ServerPlayer *> players = room->getAlivePlayers();

    //room->broadcastSkillInvoke(objectName());
    QString choice = room->askForChoice(huyanzhuo, "lianma", "lian+ma");
    if(choice == "lian"){
        foreach(ServerPlayer *player, players){
            if(player->hasEquip("Horse", true)){
                if(!player->isChained()){
                    player->setChained(true);
                    room->broadcastProperty(player, "chained");
                    room->setEmotion(player, "chain");
                }
            }
        }
    }else{
        foreach(ServerPlayer *player, players){
            if(!player->hasEquip("Horse", true)){
                if(player->isChained())
                    room->setPlayerProperty(player, "chained", false);
            }
        }
    }
};

class Lianma: public ZeroCardViewAsSkill{
public:
    Lianma():ZeroCardViewAsSkill("lianma"){
        default_choice = "lian";
    }

    virtual const Card *viewAs() const{
        return new LianmaCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("LianmaCard");
    }
};

class Zhongjia: public ClientSkill{
public:
    Zhongjia():ClientSkill("zhongjia"){
    }

    virtual int getExtra(const Player *target) const{
        if(!target->hasSkill(objectName()))
            return 0;
        else{
            int extra = target->isChained() ? 1 : 0;
            foreach(const Player *player, target->getSiblings()){
                if(player->isAlive() && player->isChained())
                    extra ++;
            }
            return extra;
        }
    }
};

SheruCard::SheruCard(){
    once = true;
}

bool SheruCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    return to_select->isWounded() && to_select != Self;
}

void SheruCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    //room->throwCard(this);
    QString choice = room->askForChoice(effect.from, "sheru", "she+ru");
    int x = effect.to->getLostHp();
    if(choice == "she"){
        effect.to->drawCards(x);
        room->loseHp(effect.to);
    }else{
        if(effect.to->getCardCount(true) <= x){
            effect.to->throwAllHandCards();
            effect.to->throwAllEquips();
        }else{
            int card_id = -1;
            for(int i=1; i<=x; i++){
                card_id = room->askForCardChosen(effect.from, effect.to, "he", "sheru");
                room->throwCard(card_id);
                if(effect.to->isNude())
                    break;
            }
        }
        RecoverStruct recover;
        recover.who = effect.from;
        room->recover(effect.to, recover, true);
    }
    //room->broadcastSkillInvoke("sheru");
}

class Sheru: public OneCardViewAsSkill{
public:
    Sheru():OneCardViewAsSkill("sheru"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("SheruCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->isBlack() && card->inherits("BasicCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        SheruCard *sheru_card = new SheruCard;
        sheru_card->addSubcard(card_item->getFilteredCard());

        return sheru_card;
    }
};

XunlieCard::XunlieCard(){
}

bool XunlieCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int i = 0;
    foreach(const Player *player, Self->getSiblings()){
        if(player->getHandcardNum() >= i){
            i = player->getHandcardNum();
        }
    }
    return targets.isEmpty() && !to_select->isKongcheng() && to_select->getHandcardNum() == i && to_select != Self;
}

void XunlieCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    int i = 0;
    const Card *card = effect.to->getRandomHandCard();
    effect.from->obtainCard(card, false);
    i ++;
    if(!effect.to->isKongcheng() && room->askForChoice(effect.from, "xuelie", "get+cancel") == "get"){
        card = effect.to->getRandomHandCard();
        effect.from->obtainCard(card, false);
        i ++;
    }
    if(i == 1)
        effect.from->drawCards(1);
    room->setEmotion(effect.to, "bad");
    room->setEmotion(effect.from, "good");
}

class XunlieViewAsSkill: public OneCardViewAsSkill{
public:
    XunlieViewAsSkill():OneCardViewAsSkill("xunlie"){
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        XunlieCard *card = new XunlieCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }

    virtual bool viewFilter(const CardItem *to_selec) const{
        return to_selec->getCard()->inherits("EquipCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@xunlie";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }
};

class Xunlie:public PhaseChangeSkill{
public:
    Xunlie():PhaseChangeSkill("xunlie"){
        view_as_skill = new XunlieViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *xiezhen) const{
        if(xiezhen->getPhase() == Player::Draw){
            Room *room = xiezhen->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(xiezhen);
            foreach(ServerPlayer *player, other_players){
                if(!player->isKongcheng()){
                    can_invoke = true;
                    break;
                }
            }
            if(!can_invoke)
                return false;
            QList<const Card *> cards = xiezhen->getCards("he");
            foreach(const Card *cd, cards){
                if(cd->inherits("EquipCard")){
                    if(room->askForUseCard(xiezhen, "@@xunlie", "@xunlie", true))
                        return true;
                    break;
                }
            }
        }
        return false;
    }
};

LianzhuCard::LianzhuCard(){
    once = true;
    target_fixed = true;
}

void LianzhuCard::onUse(Room *room, const CardUseStruct &card_use) const{
    card_use.from->turnOver();
    ArcheryAttack *ar = new ArcheryAttack(Card::NoSuit, 0);
    ar->setSkillName("lianzhu");
    CardUseStruct use;
    use.card = ar;
    use.from = card_use.from;
    use.to = card_use.to;
    room->useCard(use);
}

class Lianzhu: public ZeroCardViewAsSkill{
public:
    Lianzhu():ZeroCardViewAsSkill("lianzhu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("LianzhuCard");
    }

    virtual const Card *viewAs() const{
        return new LianzhuCard;
    }
};

class Tongxia: public PhaseChangeSkill{
public:
    Tongxia():PhaseChangeSkill("tongxia"){

    }

    virtual bool onPhaseChange(ServerPlayer *hx) const{
        Room *room = hx->getRoom();
        if(hx->getPhase() == Player::Draw && hx->askForSkillInvoke(objectName())){
            QList<int> card_ids = room->getNCards(3);
            room->fillAG(card_ids);
            room->playSkillEffect(objectName());

            while(!card_ids.isEmpty()){
                int card_id = room->askForAG(hx, card_ids, false, "tongxia");
                CardStar card = Sanguosha->getCard(card_id);
                hx->tag["TongxiaCard"] = QVariant::fromValue(card);
                ServerPlayer *target = room->askForPlayerChosen(hx, room->getAllPlayers(), objectName());
                if(!target)
                    target = hx;
                //room->takeAG(target, card_id);
                if(card->inherits("EquipCard")){
                    const EquipCard *equipped = qobject_cast<const EquipCard *>(card);
                    QList<ServerPlayer *> targets;
                    targets << target;
                    equipped->use(room, hx, targets);
                }
                else
                    target->obtainCard(card);

                card_ids.removeOne(card_id);
                room->broadcastInvoke("clearAG");
                room->fillAG(card_ids);
            }
            room->broadcastInvoke("clearAG");

            return true;
        }
        hx->tag.remove("TongxiaCard");
        return false;
    }
};

/*
class Tiansuan: public TriggerSkill{
public:
    Tiansuan():TriggerSkill("tiansuan"){
        events << Pindian;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *, QVariant &data) const{
        ServerPlayer *pianzi = room->findPlayerBySkillName(objectName());
        if(!pianzi || !pianzi->askForSkillInvoke(objectName()))
            return false;
        PindianStar pindian = data.value<PindianStar>();
        QStringList choices;
        QString from_card = Sanguosha->translate(QString("@tiansuan:%1:%2:%3:%4").
                            arg(pindian->from->getGeneralName()).
                            arg(pindian->from_card->getSuitString()).
                            arg(pindian->from_card->getNumberString()).
                            arg(pindian->from_card->objectName()));
        QString to_card = Sanguosha->translate(QString("@tiansuan:%1:%2:%3:%4").
                          arg(pindian->to->getGeneralName()).
                          arg(pindian->to_card->getSuitString()).
                          arg(pindian->to_card->getNumberString()).
                          arg(pindian->to_card->objectName()));
        choices << from_card
                << to_card;
        QString choice = room->askForChoice(pianzi, objectName(), choices.join("+"));
        if(choice == from_card){
            int omiga = room->drawCard();
            //room->moveCardTo(Sanguosha->getCard(omiga), pindian->from, Player::Hand, false);
            room->moveCardTo(pindian->from_card, NULL, Player::DrawPile, true);
            pindian->from_card = Sanguosha->getCard(omiga);
        }
        else{
            int omiga = room->drawCard();
            //room->moveCardTo(Sanguosha->getCard(omiga), pindian->to, Player::Hand, false);
            room->moveCardTo(pindian->to_card, NULL, Player::DrawPile, true);
            pindian->to_card = Sanguosha->getCard(omiga);
        }
        data = QVariant::fromValue(pindian);
        return false;
    }
};

HuazhuCard::HuazhuCard(){
    once = true;
    will_throw = false;
}

bool HuazhuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void HuazhuCard::use(Room *, ServerPlayer *iszjj, const QList<ServerPlayer *> &targets) const{
    iszjj->pindian(targets.first(), "huazhu", this);
}

class HuazhuViewAsSkill: public OneCardViewAsSkill{
public:
    HuazhuViewAsSkill():OneCardViewAsSkill("huazhu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("HuazhuCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new HuazhuCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Huazhu: public TriggerSkill{
public:
    Huazhu():TriggerSkill("huazhu"){
        events << Pindian;
        view_as_skill = new HuazhuViewAsSkill;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        if(pindian->reason != "huazhu" || !pindian->from->hasSkill(objectName()))
            return false;
        if(pindian->isSuccess()){
            int x = (pindian->from_card->getNumber() + 1) / 2;
            ServerPlayer *target = room->askForPlayerChosen(pindian->from, room->getAllPlayers(), objectName());
            if(target->getHandcardNum() > x)
                room->askForDiscard(target, objectName(), target->getHandcardNum() - x);
            else if(target->getHandcardNum() < x)
                target->drawCards(x - target->getHandcardNum());
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
*/

OxPackage::OxPackage()
    :Package("ox")
{
    General *gaolian = new General(this, "gaolian", "guan", 3);
    gaolian->addSkill(new Guibing);
    gaolian->addSkill(new Heiwu);

    General *tongguan = new General(this, "tongguan", "guan");
    tongguan->addSkill(new Aoxiang);
    tongguan->addSkill(new AoxiangChange);
    related_skills.insertMulti("aoxiang", "#aox_cg");
    tongguan->addSkill(new Zhengfa);

    tongguan = new General(this, "tongguanf", "yan", 4, false, true);
    tongguan->addSkill("aoxiang");
    tongguan->addSkill("zhengfa");
    tongguan->addSkill("zhengfa");

    General *huyanzhuo = new General(this, "huyanzhuo", "guan");
    huyanzhuo->addSkill(new Lianma);
    huyanzhuo->addSkill(new Zhongjia);

    General *dongchaoxueba = new General(this, "dongchaoxueba", "jiang");
    dongchaoxueba->addSkill(new Sheru);

    General *pangwanchun = new General(this, "pangwanchun", "jiang");
    pangwanchun->addSkill(new Lianzhu);

    General *huangxin = new General(this, "huangxin", "jiang");
    huangxin->addSkill(new Tongxia);

    General *xiezhen = new General(this, "xiezhen", "min");
    xiezhen->addSkill(new Xunlie);
/*
    General *jiangjing = new General(this, "jiangjing", "jiang");
    jiangjing->addSkill(new Tiansuan);
    jiangjing->addSkill(new Huazhu);

    General *maling = new General(this, "maling", "jiang", 3);
    maling->addSkill(new Fengxing);
*/
    addMetaObject<GuibingCard>();
    addMetaObject<HeiwuCard>();
    addMetaObject<ZhengfaCard>();
    addMetaObject<LianmaCard>();
    addMetaObject<SheruCard>();
    addMetaObject<XunlieCard>();
    addMetaObject<LianzhuCard>();
}

ADD_PACKAGE(Ox);
