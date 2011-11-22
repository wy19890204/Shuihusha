#include "plough.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "god.h"
#include "standard.h"
#include "maneuvering.h"

Ecstasy::Ecstasy(Suit suit, int number): BasicCard(suit, number)
{
    setObjectName("ecstasy");
}

bool Ecstasy::isAvailable(const Player *player) const{
    return !player->hasUsed("Ecstasy");
}

QString Ecstasy::getSubtype() const{
    return "attack_card";
}

bool Ecstasy::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return !targets.isEmpty();
}

bool Ecstasy::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return to_select != Self && Self->inMyAttackRange(to_select);
}

void Ecstasy::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->setPlayerFlag(effect.to, "Ecstasy");
}

Drivolt::Drivolt(Suit suit, int number)
    :SingleTargetTrick(suit, number, true) {
    setObjectName("drivolt");
}

bool Drivolt::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    if(to_select->getKingdom() == Self->getKingdom())
        return false;

    return to_select->getCardCount(true) >= 2;
}

void Drivolt::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    room->loseHp(effect.to);
    room->askForDiscard(effect.to, "Drivolt", 2, false, true);
    effect.to->drawCards(3);
}

Wiretap::Wiretap(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("wiretap");
}

bool Wiretap::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;
    return true;
}

void Wiretap::onEffect(const CardEffectStruct &effect) const{
    effect.to->getRoom()->showAllCards(effect.to, effect.from);
}

Assassinate::Assassinate(Suit suit, int number)
    :SingleTargetTrick(suit, number, false) {
    setObjectName("assassinate");
}

bool Assassinate::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(!targets.isEmpty())
        return false;
    return true;
}

void Assassinate::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    const Card *card1 = room->askForCard(effect.to, "jink", "Assassinate");
    const Card *card2;
    if(card1)
        card2 = room->askForCard(effect.to, "jink", "Assassinate");
    if(card1 && card2)
        effect.from->turnOver();
    else{
        DamageStruct dmae;
        dmae.card = this;
        dmae.from = effect.from;
        dmae.to = effect.to;
        room->damage(dmae);
    }
}

Counterplot::Counterplot(Suit suit, int number)
    :Nullification(suit, number)
{
    setObjectName("counterplot");
}

Provistore::Provistore(Suit suit, int number)
    :DelayedTrick(suit, number)
{
    setObjectName("provistore");
    target_fixed = false;

    judge.pattern = QRegExp("(.*):(diamond):(.*)");
    judge.good = true;
    judge.reason = objectName();
}

bool Provistore::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if(!targets.isEmpty())
        return false;
    if(to_select->containsTrick(objectName()))
        return false;
    return true;
}

void Provistore::takeEffect(ServerPlayer *target) const{
    target->skip(Player::Discard);
}

Treasury::Treasury(Suit suit, int number):Disaster(suit, number){
    setObjectName("treasury");

    judge.pattern = QRegExp("(.*):(heart|diamond):([JQKA])");
    judge.good = false;
    judge.reason = objectName();
}

void Treasury::takeEffect(ServerPlayer *target) const{
    target->drawCards(5);
}

Tsunami::Tsunami(Suit suit, int number):Disaster(suit, number){
    setObjectName("tsunami");

    judge.pattern = QRegExp("(.*):(club|spade):([JQKA])");
    judge.good = false;
    judge.reason = objectName();
}

void Tsunami::takeEffect(ServerPlayer *target) const{
    target->throwAllCards();
}

class DoubleWhipSkill : public WeaponSkill{
public:
    DoubleWhipSkill():WeaponSkill("double_whip"){
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardUseStruct effect = data.value<CardUseStruct>();
        Room *room = player->getRoom();
        if(effect.card->inherits("Ecstasy") && player->askForSkillInvoke("double_whip")){
            foreach(ServerPlayer *effecto, effect.to){
                bool chained = ! effecto->isChained();
                effecto->setChained(chained);
                room->broadcastProperty(effecto, "chained");
            }
        }
        return false;
    }
};

DoubleWhip::DoubleWhip(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("double_whip");
    skill = new DoubleWhipSkill;
}
/*
class LianliClear: public TriggerSkill{
public:
    LianliClear():TriggerSkill("#lianli-clear"){
        events << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> players = room->getAllPlayers();
        foreach(ServerPlayer *player, players){
            if(player->getMark("@tied") > 0)
                player->loseMark("@tied");
        }

        return false;
    }
};

// -------- end of Lianli related skills

QiaocaiCard::QiaocaiCard(){
    once = true;
}

void QiaocaiCard::onEffect(const CardEffectStruct &effect) const{
    QList<const Card *> cards = effect.to->getJudgingArea();
    foreach(const Card *card, cards){
        effect.from->obtainCard(card);
    }
 }

class Qiaocai: public ZeroCardViewAsSkill{
public:
    Qiaocai():ZeroCardViewAsSkill("qiaocai"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@tied") == 0 && ! player->hasUsed("QiaocaiCard");
    }

    virtual const Card *viewAs() const{
        return new QiaocaiCard;
    }
};

class Jinshen: public ProhibitSkill{
public:
    Jinshen():ProhibitSkill("jinshen"){

    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        return card->inherits("Indulgence") || card->inherits("SupplyShortage");
    }
};

class WulingExEffect: public TriggerSkill{
public:
    WulingExEffect():TriggerSkill("#wuling-ex-effect"){
        events << CardEffected << Predamaged;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *xuandi = room->findPlayerBySkillName(objectName());
        if(xuandi == NULL)
            return false;

        QString wuling = xuandi->tag.value("wuling").toString();
        if(event == CardEffected && wuling == "water"){
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.card && effect.card->inherits("Peach")){
                RecoverStruct recover;
                recover.card = effect.card;
                recover.who = effect.from;
                room->recover(player, recover);

                LogMessage log;
                log.type = "#WulingWater";
                log.from = player;
                room->sendLog(log);
            }
        }else if(event == Predamaged && wuling == "earth"){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature != DamageStruct::Normal && damage.damage > 1){
                damage.damage = 1;
                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#WulingEarth";
                log.from = player;
                room->sendLog(log);
            }
        }

        return false;
    }
};

class WulingEffect: public TriggerSkill{
public:
    WulingEffect():TriggerSkill("#wuling-effect"){
        events << Predamaged;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *xuandi = room->findPlayerBySkillName(objectName());
        if(xuandi == NULL)
            return false;

        QString wuling = xuandi->tag.value("wuling").toString();
        DamageStruct damage = data.value<DamageStruct>();

        if(wuling == "wind"){
            if(damage.nature == DamageStruct::Fire && !damage.chain){
                damage.damage ++;
                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#WulingWind";
                log.from = damage.to;
                log.arg = QString::number(damage.damage - 1);
                log.arg2 = QString::number(damage.damage);
                room->sendLog(log);
            }
        }else if(wuling == "thunder"){
            if(damage.nature == DamageStruct::Thunder && !damage.chain){
                damage.damage ++;
                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#WulingThunder";
                log.from = damage.to;
                log.arg = QString::number(damage.damage - 1);
                log.arg2 = QString::number(damage.damage);
                room->sendLog(log);
            }
        }else if(wuling == "fire"){
            if(damage.nature != DamageStruct::Fire){
                damage.nature = DamageStruct::Fire;
                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#WulingFire";
                log.from = damage.to;
                room->sendLog(log);
            }
        }

        return false;
    }
};

class Wuling: public PhaseChangeSkill{
public:
    Wuling():PhaseChangeSkill("wuling"){
        default_choice = "wind";
    }

    virtual bool onPhaseChange(ServerPlayer *xuandi) const{
        static QStringList effects;
        if(effects.isEmpty()){
            effects << "wind" << "thunder" << "water" << "fire" << "earth";
        }

        if(xuandi->getPhase() == Player::Start){
            QString current = xuandi->tag.value("wuling").toString();
            QStringList choices;
            foreach(QString effect, effects){
                if(effect != current)
                    choices << effect;
            }

            Room *room = xuandi->getRoom();
            QString choice = room->askForChoice(xuandi, objectName(), choices.join("+"));
            if(!current.isEmpty())
                xuandi->loseMark("@" + current);

            xuandi->gainMark("@" + choice);
            xuandi->tag["wuling"] = choice;

            room->playSkillEffect(objectName(), effects.indexOf(choice) + 1);
        }

        return false;
    }
};

GuihanCard::GuihanCard(){
    once = true;
}

void GuihanCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *caizhaoji = effect.from;
    caizhaoji->getRoom()->swapSeat(caizhaoji, effect.to);
}

class Guihan: public ViewAsSkill{
public:
    Guihan():ViewAsSkill("guihan"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("GuihanCard");
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(to_select->isEquipped())
            return false;

        if(selected.isEmpty())
            return to_select->getFilteredCard()->isRed();
        else if(selected.length() == 1){
            Card::Suit suit = selected.first()->getFilteredCard()->getSuit();
            return to_select->getFilteredCard()->getSuit() == suit;
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        GuihanCard *card = new GuihanCard;
        card->addSubcards(cards);
        return card;
    }
};

class CaizhaojiHujia: public TriggerSkill{
public:
    CaizhaojiHujia():TriggerSkill("caizhaoji_hujia"){
        events << PhaseChange << FinishJudge;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *caizhaoji, QVariant &data) const{
        if(event == PhaseChange && caizhaoji->getPhase() == Player::Finish){
            int times = 0;
            Room *room = caizhaoji->getRoom();
            while(caizhaoji->askForSkillInvoke(objectName())){
                caizhaoji->setFlags("caizhaoji_hujia");

                room->playSkillEffect(objectName());

                times ++;
                if(times == 3){
                    caizhaoji->turnOver();
                }

                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = caizhaoji;

                room->judge(judge);

                if(judge.isBad())
                    break;
            }

            caizhaoji->setFlags("-caizhaoji_hujia");
        }else if(event == FinishJudge){
            if(caizhaoji->hasFlag("caizhaoji_hujia")){
                JudgeStar judge = data.value<JudgeStar>();
                if(judge->card->isRed()){
                    caizhaoji->obtainCard(judge->card);
                    return true;
                }
            }
        }

        return false;
    }
};

class Shenjun: public TriggerSkill{
public:
    Shenjun():TriggerSkill("shenjun"){
        events << GameStart << PhaseChange << Predamaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(event == GameStart){
            QString gender = room->askForChoice(player, objectName(), "male+female");
            bool is_male = player->getGeneral()->isMale();
            if(gender == "female"){
                if(is_male)
                    room->transfigure(player, "luboyanf", false, false);
            }else if(gender == "male"){
                if(!is_male)
                    room->transfigure(player, "luboyan", false, false);
            }

            LogMessage log;
            log.type = "#ShenjunChoose";
            log.from = player;
            log.arg = gender;
            room->sendLog(log);

        }else if(event == PhaseChange){
            if(player->getPhase() == Player::Start){
                LogMessage log;
                log.from = player;
                log.type = "#ShenjunFlip";
                room->sendLog(log);

                QString new_general = "luboyan";
                if(player->getGeneral()->isMale())
                    new_general.append("f");
                room->transfigure(player, new_general, false, false);
            }
        }else if(event == Predamaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature != DamageStruct::Thunder && damage.from &&
               damage.from->getGeneral()->isMale() != player->getGeneral()->isMale()){

                LogMessage log;
                log.type = "#ShenjunProtect";
                log.to << player;
                log.from = damage.from;
                room->sendLog(log);

                return true;
            }
        }

        return false;
    }
};

class Zonghuo: public TriggerSkill{
public:
    Zonghuo():TriggerSkill("zonghuo"){
        frequency = Compulsory;
        events << EcstasyEffect;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        EcstasyEffectStruct effect = data.value<EcstasyEffectStruct>();
        if(effect.nature != DamageStruct::Fire){
            effect.nature = DamageStruct::Fire;

            data = QVariant::fromValue(effect);

            LogMessage log;
            log.type = "#Zonghuo";
            log.from = player;
            player->getRoom()->sendLog(log);
        }

        return false;
    }
};

class Shaoying: public TriggerSkill{
public:
    Shaoying():TriggerSkill("shaoying"){
        events << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(damage.from == NULL)
            return false;

        if(!damage.from->hasSkill(objectName()))
            return false;

        ServerPlayer *luboyan = damage.from;
        if(!damage.to->isChained() && damage.nature == DamageStruct::Fire
           && luboyan->askForSkillInvoke(objectName(), data))
        {
            Room *room = luboyan->getRoom();

            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(heart|diamond):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = luboyan;

            room->judge(judge);

            if(judge.isGood()){
                DamageStruct shaoying_damage;
                shaoying_damage.nature = DamageStruct::Fire;
                shaoying_damage.from = luboyan;
                shaoying_damage.to = player->getNextAlive();

                room->damage(shaoying_damage);
            }
        }

        return false;
    }
};

class Gongmou: public PhaseChangeSkill{
public:
    Gongmou():PhaseChangeSkill("gongmou"){

    }

    virtual bool onPhaseChange(ServerPlayer *zhongshiji) const{
        switch(zhongshiji->getPhase()){
        case Player::Finish:{
                if(zhongshiji->askForSkillInvoke(objectName())){
                    Room *room = zhongshiji->getRoom();
                    QList<ServerPlayer *> players = room->getOtherPlayers(zhongshiji);
                    ServerPlayer *target = room->askForPlayerChosen(zhongshiji, players, "gongmou");
                    target->gainMark("@conspiracy");
                }
                break;
            }

        case Player::Start:{
                Room *room = zhongshiji->getRoom();
                QList<ServerPlayer *> players = room->getOtherPlayers(zhongshiji);
                foreach(ServerPlayer *player, players){
                    if(player->getMark("@conspiracy") > 0)
                        player->loseMark("@conspiracy");
                }
            }

        default:
            break;
        }


        return false;
    }
};

class GongmouExchange:public PhaseChangeSkill{
public:
    GongmouExchange():PhaseChangeSkill("#gongmou-exchange"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getMark("@conspiracy") > 0;
    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Draw)
            return false;

        player->loseMark("@conspiracy");

        Room *room = player->getRoom();
        ServerPlayer *zhongshiji = room->findPlayerBySkillName("gongmou");
        if(zhongshiji){
            int x = qMin(zhongshiji->getHandcardNum(), player->getHandcardNum());
            if(x == 0)
                return false;

            const Card *to_exchange = NULL;
            if(player->getHandcardNum() == x)
                to_exchange = player->wholeHandCards();
            else
                to_exchange = room->askForExchange(player, "gongmou", x);

            room->moveCardTo(to_exchange, zhongshiji, Player::Hand, false);

            delete to_exchange;

            to_exchange = room->askForExchange(zhongshiji, "gongmou", x);
            room->moveCardTo(to_exchange, player, Player::Hand, false);

            delete to_exchange;

            LogMessage log;
            log.type = "#GongmouExchange";
            log.from = zhongshiji;
            log.to << player;
            log.arg = QString::number(x);
            room->sendLog(log);
        }

        return false;
    }
};

LexueCard::LexueCard(){
    once = true;
    mute = true;
}

bool LexueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self && !to_select->isKongcheng();
}

void LexueCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();

    room->playSkillEffect("lexue", 1);
    const Card *card = room->askForCardShow(effect.to, effect.from, "Lexue");
    int card_id = card->getEffectiveId();
    room->showCard(effect.to, card_id);

    if(card->getTypeId() == Card::Basic || card->isNDTrick()){
        room->setPlayerMark(effect.from, "Lexue", card_id);
        room->setPlayerFlag(effect.from, "Lexue");
    }else{
        effect.from->obtainCard(card);
        room->setPlayerFlag(effect.from, "-Lexue");
    }
}

class Lexue: public ViewAsSkill{
public:
    Lexue():ViewAsSkill("lexue"){

    }

    virtual int getEffectIndex(ServerPlayer *, const Card *card) const{
        if(card->getTypeId() == Card::Basic)
            return 2;
        else
            return 3;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->hasUsed("LexueCard") && player->hasFlag("Lexue")){
            int card_id = player->getMark("Lexue");
            const Card *card = Sanguosha->getCard(card_id);
            return card->isAvailable(player);
        }else
            return true;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        if(player->getPhase() == Player::NotActive)
            return false;

        if(!player->hasFlag("Lexue"))
            return false;

        if(player->hasUsed("LexueCard")){
            int card_id = player->getMark("Lexue");
            const Card *card = Sanguosha->getCard(card_id);
            return  pattern.contains(card->objectName());
        }else
            return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(Self->hasUsed("LexueCard") && selected.isEmpty() && Self->hasFlag("Lexue")){
            int card_id = Self->getMark("Lexue");
            const Card *card = Sanguosha->getCard(card_id);
            return to_select->getFilteredCard()->getSuit() == card->getSuit();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(Self->hasUsed("LexueCard")){
            if(!Self->hasFlag("Lexue"))
                return false;

            if(cards.length() != 1)
                return NULL;

            int card_id = Self->getMark("Lexue");
            const Card *card = Sanguosha->getCard(card_id);
            const Card *first = cards.first()->getFilteredCard();

            Card *new_card = Sanguosha->cloneCard(card->objectName(), first->getSuit(), first->getNumber());
            new_card->addSubcards(cards);
            new_card->setSkillName(objectName());
            return new_card;
        }else{
            return new LexueCard;
        }
    }
};

XunzhiCard::XunzhiCard(){
    target_fixed = true;
}

void XunzhiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    source->drawCards(3);

    QList<ServerPlayer *> players = room->getAlivePlayers();
    QSet<QString> general_names;
    foreach(ServerPlayer *player, players)
        general_names << player->getGeneralName();

    QStringList all_generals = Sanguosha->getLimitedGeneralNames();
    QStringList shu_generals;
    foreach(QString name, all_generals){
        const General *general = Sanguosha->getGeneral(name);
        if(general->getKingdom() == "jiang" && !general_names.contains(name))
            shu_generals << name;
    }

    QString general = room->askForGeneral(source, shu_generals);

    room->transfigure(source, general, false);
    room->acquireSkill(source, "xunzhi", false);
    source->setFlags("xunzhi");
}

class XunzhiViewAsSkill: public ZeroCardViewAsSkill{
public:
    XunzhiViewAsSkill():ZeroCardViewAsSkill("#xunzhi"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getGeneral()->hasSkill("xunzhi");
    }

    virtual const Card *viewAs() const{
        return new XunzhiCard;
    }
};

class Xunzhi: public PhaseChangeSkill{
public:
    Xunzhi():PhaseChangeSkill("xunzhi"){
        view_as_skill = new XunzhiViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::NotActive &&
           target->hasFlag("xunzhi"))
        {
            Room *room = target->getRoom();
            room->transfigure(target, parent()->objectName(), false);
            room->killPlayer(target);
        }

        return false;
    }
};

class Dongcha: public PhaseChangeSkill{
public:
    Dongcha():PhaseChangeSkill("dongcha"){

    }

    virtual bool onPhaseChange(ServerPlayer *jiawenhe) const{
        switch(jiawenhe->getPhase()){
        case Player::Start:{
                if(jiawenhe->askForSkillInvoke(objectName())){
                    Room *room = jiawenhe->getRoom();
                    QList<ServerPlayer *> players = room->getOtherPlayers(jiawenhe);
                    ServerPlayer *dongchaee = room->askForPlayerChosen(jiawenhe, players, objectName());
                    room->setPlayerFlag(dongchaee, "dongchaee");
                    room->setTag("Dongchaee", dongchaee->objectName());
                    room->setTag("Dongchaer", jiawenhe->objectName());

                    room->showAllCards(dongchaee, jiawenhe);
                }

                break;
            }

        case Player::Finish:{
                Room *room = jiawenhe->getRoom();
                QString dongchaee_name = room->getTag("Dongchaee").toString();
                if(!dongchaee_name.isEmpty()){
                    ServerPlayer *dongchaee = room->findChild<ServerPlayer *>(dongchaee_name);
                    room->setPlayerFlag(dongchaee, "-dongchaee");

                    room->setTag("Dongchaee", QVariant());
                    room->setTag("Dongchaer", QVariant());
                }

                break;
            }

        default:
            break;
        }

        return false;
    }
};

class Dushi: public TriggerSkill{
public:
    Dushi():TriggerSkill("dushi"){
        events << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStar damage = data.value<DamageStar>();
        ServerPlayer *killer = damage ? damage->from : NULL;

        if(killer){
            Room *room = player->getRoom();
            if(killer != player && !killer->hasSkill("benghuai")){
                killer->gainMark("@collapse");
                room->acquireSkill(killer, "benghuai");
            }
        }

        return false;
    }
};

class Sizhan: public TriggerSkill{
public:
    Sizhan():TriggerSkill("sizhan"){
        events << Predamaged << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *elai, QVariant &data) const{
        if(event == Predamaged){
            DamageStruct damage = data.value<DamageStruct>();

            LogMessage log;
            log.type = "#SizhanPrevent";
            log.from = elai;
            log.arg = QString::number(damage.damage);
            elai->getRoom()->sendLog(log);

            elai->gainMark("@struggle", damage.damage);

            return true;
        }else if(event == PhaseChange && elai->getPhase() == Player::Finish){
            int x = elai->getMark("@struggle");
            if(x > 0){
                elai->loseMark("@struggle", x);

                LogMessage log;
                log.type = "#SizhanLoseHP";
                log.from = elai;
                log.arg = QString::number(x);

                Room *room = elai->getRoom();
                room->sendLog(log);
                room->loseHp(elai, x);
            }

            elai->setFlags("-shenli");
        }

        return false;
    }
};

class Shenli: public TriggerSkill{
public:
    Shenli():TriggerSkill("shenli"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *elai, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.card && damage.card->inherits("Ecstasy") &&
           elai->getPhase() == Player::Play && !elai->hasFlag("shenli"))
        {
            elai->setFlags("shenli");

            int x = elai->getMark("@struggle");
            if(x > 0){
                x = qMin(3, x);
                damage.damage += x;
                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#ShenliBuff";
                log.from = elai;
                log.arg = QString::number(x);
                log.arg2 = QString::number(damage.damage);
                elai->getRoom()->sendLog(log);
            }
        }

        return false;
    }
};

class Zhenggong: public TriggerSkill{
public:
    Zhenggong():TriggerSkill("zhenggong"){
        events << TurnStart;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return ! target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &) const{
        Room *room = player->getRoom();
        ServerPlayer *dengshizai = room->findPlayerBySkillName(objectName());

        if(dengshizai && dengshizai->faceUp() && dengshizai->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());

            dengshizai->turnOver();

            PlayerStar zhenggong = room->getTag("Zhenggong").value<PlayerStar>();
            if(zhenggong == NULL){
                PlayerStar p = player;
                room->setTag("Zhenggong", QVariant::fromValue(p));
                player->gainMark("@zhenggong");
            }

            room->setCurrent(dengshizai);
            dengshizai->play();

            return true;

        }else{
            PlayerStar p = room->getTag("Zhenggong").value<PlayerStar>();
            if(p){
                p->loseMark("@zhenggong");
                room->setCurrent(p);
                room->setTag("Zhenggong", QVariant());
            }
        }

        return false;
    }
};

class Toudu: public MasochismSkill{
public:
    Toudu():MasochismSkill("toudu"){

    }

    virtual void onDamaged(ServerPlayer *dengshizai, const DamageStruct &) const{
        if(dengshizai->faceUp())
            return;

        if(dengshizai->isKongcheng())
            return;

        if(!dengshizai->askForSkillInvoke("toudu"))
            return;

        Room *room = dengshizai->getRoom();

        if(!room->askForDiscard(dengshizai, "toudu", 1, false, false))
            return;

        dengshizai->turnOver();

        QList<ServerPlayer *> players = room->getOtherPlayers(dengshizai), targets;
        foreach(ServerPlayer *player, players){
            if(dengshizai->canEcstasy(player, false)){
                targets << player;
            }
        }

        if(!targets.isEmpty()){
            ServerPlayer *target = room->askForPlayerChosen(dengshizai, targets, "toudu");

            Ecstasy *Ecstasy = new Ecstasy(Card::NoSuit, 0);
            Ecstasy->setSkillName("toudu");

            CardUseStruct use;
            use.card = Ecstasy;
            use.from = dengshizai;
            use.to << target;
            room->useCard(use);
        }
    }
};

YisheCard::YisheCard(){
    target_fixed = true;
    will_throw = false;
}

void YisheCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    const QList<int> rice = source->getPile("rice");

    if(subcards.isEmpty()){
        foreach(int card_id, rice){
            room->obtainCard(source, card_id);
        }
    }else{
        foreach(int card_id, subcards){
            source->addToPile("rice", card_id);
        }
    }
}

class YisheViewAsSkill: public ViewAsSkill{
public:
    YisheViewAsSkill():ViewAsSkill(""){
        card = new YisheCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->getPile("rice").isEmpty())
            return !player->isKongcheng();
        else
            return true;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        int n = Self->getPile("rice").length();
        if(selected.length() + n >= 5)
            return false;

        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(Self->getPile("rice").isEmpty() && cards.isEmpty())
            return NULL;

        card->clearSubcards();
        card->addSubcards(cards);
        return card;
    }

private:
    YisheCard *card;
};

YisheAskCard::YisheAskCard(){
    target_fixed = true;
}

void YisheAskCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    ServerPlayer *zhanglu = room->findPlayerBySkillName("yishe");
    if(zhanglu == NULL)
        return;

    const QList<int> &yishe = zhanglu->getPile("rice");
    if(yishe.isEmpty())
        return;

    int card_id;
    if(yishe.length() == 1)
        card_id = yishe.first();
    else{
        room->fillAG(yishe, source);
        card_id = room->askForAG(source, yishe, false, "yisheask");
        source->invoke("clearAG");
    }

    source->obtainCard(Sanguosha->getCard(card_id));
    room->showCard(source, card_id);

    if(room->askForChoice(zhanglu, "yisheask", "allow+disallow") == "disallow"){
        zhanglu->addToPile("rice", card_id);
    }
}

class YisheAsk: public ZeroCardViewAsSkill{
public:
    YisheAsk():ZeroCardViewAsSkill("yisheask"){
        default_choice = "disallow";
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->hasSkill("yishe"))
            return false;

        if(player->usedTimes("YisheAskCard") >= 2)
            return false;

        Player *zhanglu = NULL;
        foreach(Player *p, player->parent()->findChildren<Player *>()){
            if(p->isAlive() && p->hasSkill("yishe")){
                zhanglu = p;
                break;
            }
        }

        return zhanglu && !zhanglu->getPile("rice").isEmpty();
    }

    virtual const Card *viewAs() const{
        return new YisheAskCard;
    }
};

class Yishe: public GameStartSkill{
public:
    Yishe():GameStartSkill("yishe"){
        view_as_skill = new YisheViewAsSkill;
    }

    virtual void onGameStart(ServerPlayer *player) const{
        Room *room = player->getRoom();
        foreach(ServerPlayer *p, room->getOtherPlayers(player))
            room->attachSkillToPlayer(p, "yisheask");
    }
};

class Xiliang: public TriggerSkill{
public:
    Xiliang():TriggerSkill("xiliang"){
        events << CardDiscarded;

        default_choice = "obtain";

        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(player->getPhase() != Player::Discard)
            return false;

        ServerPlayer *zhanglu = room->findPlayerBySkillName(objectName());

        if(zhanglu == NULL)
            return false;

        CardStar card = data.value<CardStar>();
        QList<const Card *> red_cards;
        foreach(int card_id, card->getSubcards()){
            const Card *c = Sanguosha->getCard(card_id);
            if(c->isRed())
                red_cards << c;
        }

        if(red_cards.isEmpty())
            return false;

        if(!zhanglu->askForSkillInvoke(objectName(), data))
            return false;

        bool can_put = 5 - zhanglu->getPile("rice").length() >= red_cards.length();
        if(can_put && room->askForChoice(zhanglu, objectName(), "put+obtain") == "put"){
            foreach(const Card *card, red_cards){
                zhanglu->addToPile("rice", card->getEffectiveId());
            }
        }else{
            foreach(const Card *card, red_cards){
                zhanglu->obtainCard(card);
            }
        }

        return false;
    }
};

class Zhenwei: public TriggerSkill{
public:
    Zhenwei():TriggerSkill("zhenwei"){
        events << EcstasyMissed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        EcstasyEffectStruct effect = data.value<EcstasyEffectStruct>();
        if(player->getRoom()->obtainable(effect.jink, player) && player->askForSkillInvoke(objectName(), data))
            player->obtainCard(effect.jink);

        return false;
    }
};

class Yitian: public TriggerSkill{
public:
    Yitian():TriggerSkill("yitian"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to->isCaoCao() && player->askForSkillInvoke(objectName(), data)){
            LogMessage log;
            log.type = "#YitianSolace";
            log.from = player;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage - 1);
            player->getRoom()->sendLog(log);

            damage.damage --;
            data.setValue(damage);
        }

        return false;
    }
};

TaichenCard::TaichenCard(){
}

bool TaichenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty() || to_select->isAllNude())
        return false;

    if(!subcards.isEmpty() && Sanguosha->getCard(subcards.first()) == Self->getWeapon() && !Self->hasSkill("zhengfeng"))
        return Self->distanceTo(to_select) == 1;
    else
        return Self->inMyAttackRange(to_select);
}

void TaichenCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    if(subcards.isEmpty())
        room->loseHp(effect.from);
    else
        room->throwCard(this);

    int i;
    for(i=0; i<2; i++){
        if(!effect.to->isAllNude())
            room->throwCard(room->askForCardChosen(effect.from, effect.to, "hej", "taichen"));
    }
}

class Taichen: public ViewAsSkill{
public:
    Taichen():ViewAsSkill("taichen"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.isEmpty() && to_select->getFilteredCard()->inherits("Weapon");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() <= 1){
            TaichenCard *taichen_card = new TaichenCard;
            taichen_card->addSubcards(cards);
            return taichen_card;
        }else
            return NULL;
    }
};
*/

PloughPackage::PloughPackage()
    :Package("plough")
{
    type = CardPack;
    QList<Card *> cards;

    // spade
    cards
            << new Assassinate(Card::Spade, 1)
            << new SilverLion(Card::Spade, 2)
            //<< new meteor_sword(Card::Spade, 3)
            << new ThunderSlash(Card::Spade, 5)
            << new ThunderSlash(Card::Spade, 6)
            //<< new ThunderEcstasy(Card::Spade, 7)
            << new IronChain(Card::Spade, 8)
            << new Ecstasy(Card::Spade, 9)
            //<< new gold_armor(Card::Spade,10)
            << new Wiretap(Card::Spade, 11)
            << new IronChain(Card::Spade, 12)
            << new Counterplot(Card::Spade, 13)

    // club
            << new Tsunami(Card::Club, 1)
            << new Ecstasy(Card::Club, 2)
            << new Ecstasy(Card::Club, 3)
            << new Analeptic(Card::Club, 4)
            << new Ecstasy(Card::Club, 5)
            << new Provistore(Card::Club, 6)
            << new DoubleWhip(Card::Club, 7)
            << new IronChain(Card::Club, 8)
            << new ThunderSlash(Card::Club, 9)
            //<< new IronChain(Card::Club, 10)
            << new IronChain(Card::Club, 11)
            << new Drivolt(Card::Club, 12)
            << new ArcheryAttack(Card::Club, 13)

    // heart
            << new Provistore(Card::Heart, 1)
            << new Jink(Card::Heart, 2)
            << new Analeptic(Card::Heart, 3)
            << new FireSlash(Card::Heart, 4)
            << new Peach(Card::Heart, 5)
            << new Jink(Card::Heart, 6)
            << new Wiretap(Card::Heart, 7)
            << new Ecstasy(Card::Heart, 8)
            << new Ecstasy(Card::Heart, 9)
            << new Peach(Card::Heart, 10)
            << new Counterplot(Card::Heart, 11)
            << new Drivolt(Card::Heart, 13)

    // diamond
            << new Tsunami(Card::Diamond, 1)
            << new Peach(Card::Diamond, 2)
            << new Peach(Card::Diamond, 3)
            << new FireSlash(Card::Diamond, 4)
            << new Jink(Card::Diamond, 5)
            << new Tsunami(Card::Diamond, 6)
            << new Wiretap(Card::Diamond, 7)
            << new Treasury(Card::Diamond, 8)
            << new Analeptic(Card::Diamond, 9)
            << new Jink(Card::Diamond, 10)
            //<< new gong(Card::Diamond, 11)
            << new Assassinate(Card::Diamond, 12)
            << new Counterplot(Card::Diamond, 13);

    DefensiveHorse *white = new DefensiveHorse(Card::Heart, 12);
    white->setObjectName("white");
    OffensiveHorse *brown = new OffensiveHorse(Card::Spade, 4);
    brown->setObjectName("brown");

    cards << white << brown;
    foreach(Card *card, cards)
        card->setParent(this);
}

ADD_PACKAGE(Plough)
