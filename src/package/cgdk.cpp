#include "cgdk.h"
#include "standard.h"
#include "skill.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"

#include <QCommandLinkButton>

class Liehuo: public TriggerSkill{
public:
    Liehuo():TriggerSkill("liehuo"){
        events << SlashMissed << Damage;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *bao, QVariant &data) const{
        Room *room = bao->getRoom();
        ServerPlayer *target;
        if(event == SlashMissed){
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            target = effect.to;
        }
        else{
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.to && damage.card->inherits("Slash"))
                target = damage.to;
            else
                return false;
        }
        if(target && !target->isKongcheng() &&
           target->getHandcardNum() > bao->getHandcardNum() &&
           room->askForSkillInvoke(bao, objectName(), QVariant::fromValue(target))){
            bao->obtainCard(target->getRandomHandCard());
        }
        return false;
    }
};

class Jueming: public ProhibitSkill{
public:
    Jueming():ProhibitSkill("jueming"){
    }

    virtual bool isProhibited(const Player *, const Player *to, const Card *card) const{
        if(to->getPhase() == Player::NotActive && to->getHp() == 1)
            return card->inherits("Slash") || card->inherits("Duel") || card->inherits("Assassinate");
        else
            return false;
    }
};

class Jiuhan:public TriggerSkill{
public:
    Jiuhan():TriggerSkill("jiuhan"){
        events << HpRecover;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *nana, QVariant &data) const{
        Room *room = nana->getRoom();
        RecoverStruct rec = data.value<RecoverStruct>();
        if(rec.who == nana && rec.card->inherits("Analeptic") &&
           nana->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
            LogMessage log;
            log.type = "#Jiuhan";
            log.from = nana;
            log.arg = objectName();
            log.arg2 = QString::number(1);
            room->sendLog(log);
            rec.recover ++;

            data = QVariant::fromValue(rec);
        }
        return false;
    }
};

class Zaoxian: public PhaseChangeSkill{
public:
    Zaoxian():PhaseChangeSkill("zaoxian"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("zaoxian") == 0
                && target->getPile("field").length() >= 3;
    }

    virtual bool onPhaseChange(ServerPlayer *dengai) const{
        Room *room = dengai->getRoom();

        room->setPlayerMark(dengai, "zaoxian", 1);
        room->loseMaxHp(dengai);

        LogMessage log;
        log.type = "#ZaoxianWake";
        log.from = dengai;
        log.arg = QString::number(dengai->getPile("field").length());
        room->sendLog(log);

        room->acquireSkill(dengai, "Yunchou");

        return false;
    }
};

YunchouCard::YunchouCard(){
    target_fixed = true;
}

void YunchouCard::onUse(Room *room, const CardUseStruct &card_use) const{
    ServerPlayer *nouse = card_use.from;
    QStringList ndtricks;
    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach(const Card *card, cards){
        if(card->isNDTrick() && !card->inherits("Nullification") &&
           !ndtricks.contains(card->objectName()))
            ndtricks << card->objectName();
    }
    QString name = room->askForChoice(nouse, "yunchou", ndtricks.join("+"));
    room->setPlayerProperty(nouse, "yunchoustore", name);
}

class YunchouSelect: public ZeroCardViewAsSkill{
public:
    YunchouSelect():ZeroCardViewAsSkill("yunchou-select"){
    }

    virtual const Card *viewAs() const{
        return new YunchouCard;
    }
};

class Yunchou:public OneCardViewAsSkill{
public:
    Yunchou():OneCardViewAsSkill("yunchou"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("TrickCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        QString name = Self->property("yunchoustore").toString();
        Card *new_card = Sanguosha->cloneCard(name, card->getSuit(), card->getNumber());
        new_card->addSubcard(card);
        new_card->setSkillName("yunchou");
        Self->setFlags("Yunchou_used");
        return new_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->property("yunchoustore").isNull() && !player->hasFlag("Yunchou_used");
    }
};

class ZhiquN: public OneCardViewAsSkill{
public:
    ZhiquN():OneCardViewAsSkill("zhiqu-n"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("EquipCard");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "nullification" || pattern == "nulliplot";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getFilteredCard();
        Card *ncard = new Nullification(first->getSuit(), first->getNumber());
        ncard->addSubcard(first);
        ncard->setSkillName("zhiqu");

        return ncard;
    }
};

#include "plough.h"
class ZhiquC: public OneCardViewAsSkill{
public:
    ZhiquC():OneCardViewAsSkill("zhiqu-c"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("EquipCard");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "nulliplot";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getFilteredCard();
        Card *ncard = new Counterplot(first->getSuit(), first->getNumber());
        ncard->addSubcard(first);
        ncard->setSkillName("zhiqu");

        return ncard;
    }
};

class Hunzi: public PhaseChangeSkill{
public:
    Hunzi():PhaseChangeSkill("hunzi"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("hunzi") == 0
                && target->getPhase() == Player::Start
                && target->getHp() == 1;
    }

    virtual bool onPhaseChange(ServerPlayer *sunce) const{
        Room *room = sunce->getRoom();

        LogMessage log;
        log.type = "#HunziWake";
        log.from = sunce;
        room->sendLog(log);

        room->loseMaxHp(sunce);

        room->acquireSkill(sunce, "yinghun");
        room->acquireSkill(sunce, "yingzi");

        room->setPlayerMark(sunce, "hunzi", 1);

        return false;
    }
};

class SunceZhiba: public TriggerSkill{
public:
    SunceZhiba():TriggerSkill("sunce_zhiba$"){
        events << GameStart << Pindian;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == GameStart){
            if(!player->hasLordSkill(objectName()))
                return false;

            Room *room = player->getRoom();
            foreach(ServerPlayer *p, room->getOtherPlayers(player)){
                if(!p->hasSkill("zhiba_pindian"))
                    room->attachSkillToPlayer(p, "zhiba_pindian");
            }
        }else if(event == Pindian){
            PindianStar pindian = data.value<PindianStar>();
            if(pindian->reason == "zhiba" &&
               pindian->to->hasLordSkill(objectName()) &&
               !pindian->isSuccess())
            {
                pindian->to->obtainCard(pindian->from_card);
                pindian->to->obtainCard(pindian->to_card);
            }
        }

        return false;
    }
};

TiaoxinCard::TiaoxinCard(){
    once = true;
    mute = true;
}

bool TiaoxinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->canSlash(Self);
}

void TiaoxinCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();

    if(effect.from->hasArmorEffect("eight_diagram") || effect.from->hasSkill("bazhen"))
        room->playSkillEffect("tiaoxin", 3);
    else
        room->playSkillEffect("tiaoxin", qrand() % 2 + 1);

    const Card *slash = room->askForCard(effect.to, "slash", "@tiaoxin-slash:" + effect.from->objectName());

    if(slash){
        CardUseStruct use;
        use.card = slash;
        use.to << effect.from;
        use.from = effect.to;
        room->useCard(use);
    }else if(!effect.to->isNude()){
        room->throwCard(room->askForCardChosen(effect.from, effect.to, "he", "tiaoxin"));
    }
}

class Tiaoxin: public ZeroCardViewAsSkill{
public:
    Tiaoxin():ZeroCardViewAsSkill("tiaoxin"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("TiaoxinCard");
    }

    virtual const Card *viewAs() const{
        return new TiaoxinCard;
    }
};

class Zhiji: public PhaseChangeSkill{
public:
    Zhiji():PhaseChangeSkill("zhiji"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getMark("zhiji") == 0
                && target->getPhase() == Player::Start
                && target->isKongcheng();
    }

    virtual bool onPhaseChange(ServerPlayer *jiangwei) const{
        Room *room = jiangwei->getRoom();

        LogMessage log;
        log.type = "#ZhijiWake";
        log.from = jiangwei;
        room->sendLog(log);

        room->playSkillEffect("zhiji");
        room->broadcastInvoke("animate", "lightbox:$zhiji:5000");
        room->getThread()->delay(5000);

        if(room->askForChoice(jiangwei, objectName(), "recover+draw") == "recover"){
            RecoverStruct recover;
            recover.who = jiangwei;
            room->recover(jiangwei, recover);
        }else
            room->drawCards(jiangwei, 2);

        room->setPlayerMark(jiangwei, "zhiji", 1);
        room->acquireSkill(jiangwei, "guanxing");

        room->loseMaxHp(jiangwei);

        return false;
    }
};

ZhijianCard::ZhijianCard(){
    will_throw = false;
}

bool ZhijianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty() || to_select == Self)
        return false;

    const Card *card = Sanguosha->getCard(subcards.first());
    const EquipCard *equip = qobject_cast<const EquipCard *>(card);
    int equip_index = static_cast<int>(equip->location());
    return to_select->getEquip(equip_index) == NULL;
}

void ZhijianCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *erzhang = effect.from;
    erzhang->getRoom()->moveCardTo(this, effect.to, Player::Equip);
    erzhang->drawCards(1);
}

class Zhijian: public OneCardViewAsSkill{
public:
    Zhijian():OneCardViewAsSkill("zhijian"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getTypeId() == Card::Equip;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ZhijianCard *zhijian_card = new ZhijianCard();
        zhijian_card->addSubcard(card_item->getFilteredCard());
        return zhijian_card;
    }
};

class Guzheng: public TriggerSkill{
public:
    Guzheng():TriggerSkill("guzheng"){
        events << CardDiscarded;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill("guzheng");
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *erzhang = room->findPlayerBySkillName(objectName());

        if(erzhang == NULL)
            return false;

        if(player->getPhase() == Player::Discard){
            QVariantList guzheng = erzhang->tag["Guzheng"].toList();

            CardStar card = data.value<CardStar>();
            foreach(int card_id, card->getSubcards()){
                guzheng << card_id;
            }

            erzhang->tag["Guzheng"] = guzheng;
        }

        return false;
    }
};

class GuzhengGet: public PhaseChangeSkill{
public:
    GuzhengGet():PhaseChangeSkill("#guzheng-get"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill("guzheng");
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->isDead())
            return false;

        Room *room = player->getRoom();
        ServerPlayer *erzhang = room->findPlayerBySkillName(objectName());
        if(erzhang == NULL)
            return false;

        QVariantList guzheng_cards = erzhang->tag["Guzheng"].toList();
        erzhang->tag.remove("Guzheng");

        QList<int> cards;
        foreach(QVariant card_data, guzheng_cards){
            int card_id = card_data.toInt();
            if(room->getCardPlace(card_id) == Player::DiscardedPile)
                cards << card_id;
        }

        if(cards.isEmpty())
            return false;

        if(erzhang->askForSkillInvoke("guzheng", cards.length())){
            room->fillAG(cards, erzhang);

            int to_back = room->askForAG(erzhang, cards, false, objectName());
            player->obtainCard(Sanguosha->getCard(to_back));

            cards.removeOne(to_back);

            erzhang->invoke("clearAG");

            foreach(int card_id, cards)
                erzhang->obtainCard(Sanguosha->getCard(card_id));
        }

        return false;
    }
};

class Xiangle: public TriggerSkill{
public:
    Xiangle():TriggerSkill("xiangle"){
        events << CardEffected;

        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();

        if(effect.card->inherits("Slash")){
            Room *room = player->getRoom();

            room->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#Xiangle";
            log.from = effect.from;
            log.to << effect.to;
            room->sendLog(log);

            return !room->askForCard(effect.from, ".basic", "@xiangle-discard", data);
        }

        return false;
    }
};

class Fangquan: public PhaseChangeSkill{
public:
    Fangquan():PhaseChangeSkill("fangquan"){

    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool onPhaseChange(ServerPlayer *liushan) const{
        switch(liushan->getPhase()){
        case Player::Play: {
                bool invoked = liushan->askForSkillInvoke(objectName());
                if(invoked)
                    liushan->setFlags("fangquan");
                return invoked;
            }

        case Player::Finish: {
                if(liushan->hasFlag("fangquan")){
                    Room *room = liushan->getRoom();

                    if(liushan->isKongcheng())
                        return false;

                    room->askForDiscard(liushan, "fangquan", 1);

                    ServerPlayer *player = room->askForPlayerChosen(liushan, room->getOtherPlayers(liushan), objectName());

                    QString name = player->getGeneralName();
                    if(name == "zhugeliang" || name == "shenzhugeliang" || name == "wolong")
                        room->playSkillEffect("fangquan", 1);
                    else
                        room->playSkillEffect("fangquan", 2);

                    LogMessage log;
                    log.type = "#Fangquan";
                    log.from = liushan;
                    log.to << player;
                    room->sendLog(log);

                    room->setCurrent(player);
                    room->getThread()->trigger(TurnStart, player);
                    room->setCurrent(liushan);
                }

                break;
            }

        default:
            break;
        }

        return false;
    }
};

class Ruoyu: public PhaseChangeSkill{
public:
    Ruoyu():PhaseChangeSkill("ruoyu$"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::Start
                && target->hasLordSkill("ruoyu")
                && target->isAlive()
                && target->getMark("ruoyu") == 0;
    }

    virtual bool onPhaseChange(ServerPlayer *liushan) const{
        Room *room = liushan->getRoom();

        bool can_invoke = true;
        foreach(ServerPlayer *p, room->getAllPlayers()){
            if(liushan->getHp() > p->getHp()){
                can_invoke = false;
                break;
            }
        }

        if(can_invoke){
            room->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#RuoyuWake";
            log.from = liushan;
            log.arg = QString::number(liushan->getHp());
            room->sendLog(log);

            room->setPlayerMark(liushan, "ruoyu", 1);
            room->setPlayerProperty(liushan, "maxhp", liushan->getMaxHP() + 1);

            RecoverStruct recover;
            recover.who = liushan;
            room->recover(liushan, recover);

            room->acquireSkill(liushan, "zhangshi");
        }

        return false;
    }
};

class Huashen: public GameStartSkill{
public:
    Huashen():GameStartSkill("huashen"){

    }

    static void AcquireGenerals(ServerPlayer *zuoci, int n){
        QStringList list = GetAvailableGenerals(zuoci);
        qShuffle(list);

        QStringList acquired = list.mid(0, n);
        QVariantList huashens = zuoci->tag["Huashens"].toList();
        foreach(QString huashen, acquired){
            huashens << huashen;
            const General *general = Sanguosha->getGeneral(huashen);
            foreach(const TriggerSkill *skill, general->getTriggerSkills()){
                zuoci->getRoom()->getThread()->addTriggerSkill(skill);
            }
        }

        zuoci->tag["Huashens"] = huashens;

        zuoci->invoke("animate", "huashen:" + acquired.join(":"));

        LogMessage log;
        log.type = "#GetHuashen";
        log.from = zuoci;
        log.arg = QString::number(n);
        log.arg2 = QString::number(huashens.length());
        zuoci->getRoom()->sendLog(log);
    }

    static QStringList GetAvailableGenerals(ServerPlayer *zuoci){
        QSet<QString> all = Sanguosha->getLimitedGeneralNames().toSet();
        QSet<QString> huashen_set, room_set;
        QVariantList huashens = zuoci->tag["Huashens"].toList();
        foreach(QVariant huashen, huashens)
            huashen_set << huashen.toString();

        Room *room = zuoci->getRoom();
        QList<const ServerPlayer *> players = room->findChildren<const ServerPlayer *>();
        foreach(const ServerPlayer *player, players){
            room_set << player->getGeneralName();
            if(player->getGeneral2())
                room_set << player->getGeneral2Name();
        }

        static QSet<QString> banned;
        if(banned.isEmpty()){
            banned << "zuoci" << "zuocif" << "guzhielai" << "dengshizai" << "caochong";
        }

        return (all - banned - huashen_set - room_set).toList();
    }

    static QString SelectSkill(ServerPlayer *zuoci, bool acquire_instant = true){
        Room *room = zuoci->getRoom();

        QString huashen_skill = zuoci->tag["HuashenSkill"].toString();
        if(!huashen_skill.isEmpty())
            room->detachSkillFromPlayer(zuoci, huashen_skill);

        QVariantList huashens = zuoci->tag["Huashens"].toList();
        if(huashens.isEmpty())
            return QString();

        QStringList huashen_generals;
        foreach(QVariant huashen, huashens)
            huashen_generals << huashen.toString();

        QString general_name = room->askForGeneral(zuoci, huashen_generals);
        const General *general = Sanguosha->getGeneral(general_name);
        QString kingdom = general->getKingdom();
        if(zuoci->getKingdom() != kingdom){
            if(kingdom == "god")
                kingdom = room->askForKingdom(zuoci);
            room->setPlayerProperty(zuoci, "kingdom", kingdom);
        }
        if(zuoci->getGeneral()->isMale() != general->isMale())
            room->setPlayerProperty(zuoci, "general", general->isMale() ? "zuoci" : "zuocif");

        QStringList skill_names;
        foreach(const Skill *skill, general->getVisibleSkillList()){
            if(skill->isLordSkill() || skill->getFrequency() == Skill::Limited
               || skill->getFrequency() == Skill::Wake)
                continue;

            skill_names << skill->objectName();
        }

        if(skill_names.isEmpty())
            return QString();

        QString skill_name;
        if(skill_names.length() == 1)
            skill_name = skill_names.first();
        else
            skill_name = room->askForChoice(zuoci, "huashen", skill_names.join("+"));

        zuoci->tag["HuashenSkill"] = skill_name;

        if(acquire_instant)
            room->acquireSkill(zuoci, skill_name);

        return skill_name;
    }

    virtual void onGameStart(ServerPlayer *zuoci) const{
        AcquireGenerals(zuoci, 2);
        SelectSkill(zuoci);
    }

    virtual QDialog *getDialog() const{
        static HuashenDialog *dialog;

        if(dialog == NULL)
            dialog = new HuashenDialog;

        return dialog;
    }
};

HuashenDialog::HuashenDialog()
{
    setWindowTitle(tr("Incarnation"));
}

void HuashenDialog::popup(){
    QVariantList huashen_list = Self->tag["Huashens"].toList();
    QList<const General *> huashens;
    foreach(QVariant huashen, huashen_list)
        huashens << Sanguosha->getGeneral(huashen.toString());

    fillGenerals(huashens);

    show();
}

class HuashenBegin: public PhaseChangeSkill{
public:
    HuashenBegin():PhaseChangeSkill("#huashen-begin"){

    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::Start;
    }

    virtual bool onPhaseChange(ServerPlayer *zuoci) const{
        QString skill_name = Huashen::SelectSkill(zuoci, false);
        if(!skill_name.isEmpty())
            zuoci->getRoom()->acquireSkill(zuoci, skill_name);

        return false;
    }
};

class HuashenEnd: public PhaseChangeSkill{
public:
    HuashenEnd():PhaseChangeSkill("#huashen-end"){

    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target) && target->getPhase() == Player::NotActive;
    }

    virtual bool onPhaseChange(ServerPlayer *zuoci) const{
        Huashen::SelectSkill(zuoci);

        return false;
    }
};

class Xinsheng: public MasochismSkill{
public:
    Xinsheng():MasochismSkill("xinsheng"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *zuoci, const DamageStruct &damage) const{
        int n = damage.damage;
        if(n == 0)
            return;

        if(zuoci->getRoom()->askForSkillInvoke(zuoci, objectName()))
            Huashen::AcquireGenerals(zuoci, n);
    }
};

CGDKPackage::CGDKPackage()
    :Package("CGDK")
{
    General *wuyong = new General(this, "wuyong", "kou", 3);
    wuyong->addSkill(new YunchouSelect);
    wuyong->addSkill(new Yunchou);
    wuyong->addSkill(new ZhiquN);
    wuyong->addSkill(new ZhiquC);

    General *ruanxiaoqi = new General(this, "ruanxiaoqi", "min");
    ruanxiaoqi->addSkill(new Jueming);
    ruanxiaoqi->addSkill(new Jiuhan);

    General *xiebao = new General(this, "xiebao", "min");
    xiebao->addSkill(new Liehuo);

    General *zuoci = new General(this, "zuoci", "kou", 3);
    zuoci->addSkill(new Huashen);
    zuoci->addSkill(new HuashenBegin);
    zuoci->addSkill(new HuashenEnd);
    zuoci->addSkill(new Xinsheng);

    General *zuocif = new General(this, "zuocif", "kou", 3, false, true);
    zuocif->addSkill("huashen");
    zuocif->addSkill("#huashen-begin");
    zuocif->addSkill("#huashen-end");
    zuocif->addSkill("xinsheng");

    related_skills.insertMulti("huashen", "#huashen-begin");
    related_skills.insertMulti("huashen", "#huashen-end");

    addMetaObject<TiaoxinCard>();
    addMetaObject<ZhijianCard>();
    addMetaObject<YunchouCard>();
}

ADD_PACKAGE(CGDK)
