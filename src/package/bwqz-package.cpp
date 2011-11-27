#include "bwqz-package.h"
#include "general.h"
#include "skill.h"
#include "carditem.h"
#include "engine.h"
#include "standard.h"
#include "clientplayer.h"
#include "plough.h"
#include "tocheck.h"

class Zaochuan: public OneCardViewAsSkill{
public:
    Zaochuan():OneCardViewAsSkill("zaochuan"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("TrickCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        IronChain *chain = new IronChain(card->getSuit(), card->getNumber());
        chain->addSubcard(card);
        chain->setSkillName(objectName());
        return chain;
    }
};

YuanyinCard::YuanyinCard(){
}

bool YuanyinCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2 || targets.length() == 1;
}

bool YuanyinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.isEmpty()){
        return to_select->getWeapon() && to_select != Self;
    }else if(targets.length() == 1){
        const Player *first = targets.first();
        return to_select != Self && first->getWeapon() && Self->canSlash(to_select);
    }else
        return false;
}

void YuanyinCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target;
    if(targets.length() > 1)
        target = targets.at(1);
    else if(targets.length() == 1 && source->canSlash(targets.first())){
        target = targets.first();
    }
    else
        return;

    const Card *weapon = targets.first()->getWeapon();
    if(weapon){
        room->throwCard(weapon->getId());
        Slash *slash = new Slash(weapon->getSuit(), weapon->getNumber());
        slash->setSkillName("yuanyin");
        slash->addSubcard(weapon);
        CardUseStruct use;
        use.card = slash;
        use.from = source;
        use.to << target;
        room->useCard(use);
    }
}

class YuanyinViewAsSkill:public ZeroCardViewAsSkill{
public:
    YuanyinViewAsSkill():ZeroCardViewAsSkill("yuanyin"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual const Card *viewAs() const{
        return new YuanyinCard;
    }
};

class Yuanyin: public TriggerSkill{
public:
    Yuanyin():TriggerSkill("yuanyin"){
        events << CardAsked;
        view_as_skill = new YuanyinViewAsSkill;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        QString asked = data.toString();
        if(asked != "slash" && asked != "jink")
            return false;
        Room *room = player->getRoom();
        QList<ServerPlayer *> playerAs, playerBs;
        foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
            if(asked == "slash" && tmp->getWeapon())
                playerAs << tmp;
            if(asked == "jink" &&
                    (tmp->getArmor() || tmp->getOffensiveHorse() || tmp->getDefensiveHorse()))
                playerBs << tmp;
        }
        if((asked == "slash" && playerAs.isEmpty()) || (asked == "jink" && playerBs.isEmpty()))
            return false;
        if(room->askForSkillInvoke(player, objectName(), data)){
            ServerPlayer *target = asked == "slash" ?
                                   room->askForPlayerChosen(player, playerAs, objectName()) :
                                   room->askForPlayerChosen(player, playerBs, objectName());
            int card_id = asked == "slash" ?
                          target->getWeapon()->getId() :
                          room->askForCardChosen(player, target, "e", objectName());
            if(asked == "jink" && target->getWeapon() && target->getWeapon()->getId() == card_id)
                return false;
            const Card *card = Sanguosha->getCard(card_id);
            if(asked == "slash"){
                Slash *yuanyin_card = new Slash(card->getSuit(), card->getNumber());
                yuanyin_card->setSkillName(objectName());
                yuanyin_card->addSubcard(card);
                room->provide(yuanyin_card);
                room->setEmotion(player, "good");
                return true;
            }
            else if(asked == "jink"){
                Jink *yuanyin_card = new Jink(card->getSuit(), card->getNumber());
                yuanyin_card->setSkillName(objectName());
                yuanyin_card->addSubcard(card);
                room->provide(yuanyin_card);
                room->setEmotion(player, "good");
                return true;
            }
        }
        return false;
    }
};

class Xiaofang: public TriggerSkill{
public:
    Xiaofang():TriggerSkill("xiaofang"){
        events << Predamaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        ServerPlayer *water = room->findPlayerBySkillName(objectName());
        if(!water || water->isKongcheng() || damage.nature != DamageStruct::Fire)
            return false;
        if(water->askForSkillInvoke(objectName()) && room->askForDiscard(water, objectName(), 1)){
            LogMessage log;
            log.type = "#Xiaofang";
            log.from = water;
            log.arg = objectName();
            log.to << damage.to;
            room->sendLog(log);

            damage.nature = DamageStruct::Normal;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

NushaCard::NushaCard(){
    once = true;
}

bool NushaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int i = 0;
    foreach(const Player *player, Self->getSiblings()){
        if(player->getHandcardNum() >= i){
            i = player->getHandcardNum();
        }
    }
    return targets.isEmpty() && to_select->getHandcardNum() == i && to_select != Self;
}

void NushaCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    DamageStruct damage;
    damage.from = source;
    damage.to = targets.first();
    damage.card = this;
    room->damage(damage);
}

class Nusha: public OneCardViewAsSkill{
public:
    Nusha():OneCardViewAsSkill("nusha"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("NushaCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Slash");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new NushaCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Wanku:public PhaseChangeSkill{
public:
    Wanku():PhaseChangeSkill("wanku"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *zhugeliang) const{
        if(zhugeliang->getPhase() == Player::Finish &&
           zhugeliang->getHp() > zhugeliang->getHandcardNum() &&
           zhugeliang->askForSkillInvoke(objectName())){
            zhugeliang->getRoom()->playSkillEffect(objectName());
            zhugeliang->drawCards(zhugeliang->getHp() - zhugeliang->getHandcardNum());
        }
        return false;
    }
};

class Zhuying: public FilterSkill{
public:
    Zhuying():FilterSkill("zhuying"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getCard()->objectName() == "analeptic";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *c = card_item->getCard();
        Peach *peach = new Peach(c->getSuit(), c->getNumber());
        peach->setSkillName(objectName());
        peach->addSubcard(card_item->getCard());

        return peach;
    }
};

class Banzhuang: public OneCardViewAsSkill{
public:
    Banzhuang():OneCardViewAsSkill("banzhuang"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Heart;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        ExNihilo *ex_nihilo = new ExNihilo(card->getSuit(), card->getNumber());
        ex_nihilo->addSubcard(card->getId());
        ex_nihilo->setSkillName(objectName());
        return ex_nihilo;
    }
};

ShougeCard::ShougeCard(){
    will_throw = false;
    target_fixed = true;
    mute = true;
}

void ShougeCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->playSkillEffect("shouge", qrand() % 2 + 1);
    source->addToPile("vege", this->getSubcards().first());
}

class ShougeViewAsSkill: public OneCardViewAsSkill{
public:
    ShougeViewAsSkill():OneCardViewAsSkill("shouge"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Peach") ||
                to_select->getCard()->inherits("Analeptic");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ShougeCard *card = new ShougeCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Shouge: public TriggerSkill{
public:
    Shouge():TriggerSkill("shouge"){
        view_as_skill = new ShougeViewAsSkill;
        events << CardLost << HpLost;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getPhase() == Player::NotActive;
    }

    static bool doDraw(Room *room, ServerPlayer *vgqq){
        room->throwCard(vgqq->getPile("vege").last());
        room->playSkillEffect("shouge", qrand() % 2 + 3);
        vgqq->drawCards(3);
        return vgqq->getPile("vege").isEmpty();
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *vgqq, QVariant &data) const{
        Room *room = vgqq->getRoom();
        if(vgqq->getPile("vege").isEmpty())
            return false;
        if(event == CardLost){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Hand && move->to != vgqq
               && vgqq->isAlive() && vgqq->askForSkillInvoke(objectName())){
                doDraw(room, vgqq);
            }
        }
        else{
            int lose = data.toInt();
            for(; lose > 0; lose --){
                if(vgqq->isAlive() && vgqq->askForSkillInvoke(objectName()))
                    if(doDraw(room, vgqq))
                        break;
            }
        }
        return false;
    }
};

class Qiongtu: public PhaseChangeSkill{
public:
    Qiongtu():PhaseChangeSkill("qiongtu"){
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        ServerPlayer *zhangqing = room->findPlayerBySkillName(objectName());
        if(zhangqing && target->getPhase() == Player::Finish){
            if(target->getHandcardNum() <= 1 && !target->isNude()
                && zhangqing->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());
                int card_id = room->askForCardChosen(zhangqing, target, "he", objectName());
                room->obtainCard(zhangqing, card_id);
            }
        }
        return false;
    }
};

QiaogongCard::QiaogongCard(){
    once = true;
}

void QiaogongCard::swapEquip(ServerPlayer *first, ServerPlayer *second, int index) const{
    const EquipCard *e1 = first->getEquip(index);
    const EquipCard *e2 = second->getEquip(index);

    Room *room = first->getRoom();

    if(e1)
        first->obtainCard(e1);

    if(e2)
        room->moveCardTo(e2, first, Player::Equip);

    if(e1)
        room->moveCardTo(e1, second, Player::Equip);
}

bool QiaogongCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

bool QiaogongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    switch(targets.length()){
    case 0: return true;
    case 1: {
            int n1 = targets.first()->getEquips().length();
            int n2 = to_select->getEquips().length();
            return qAbs(n1-n2) <= Self->getLostHp();
        }

    default:
        return false;
    }
}

void QiaogongCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *first = targets.first();
    ServerPlayer *second = targets.at(1);

    int i;
    for(i=0; i<4; i++)
        swapEquip(first, second, i);

    LogMessage log;
    log.type = "#QiaogongSwap";
    log.from = source;
    log.to = targets;
    room->sendLog(log);
}

class Qiaogong: public ZeroCardViewAsSkill{
public:
    Qiaogong():ZeroCardViewAsSkill("qiaogong"){

    }

    virtual const Card *viewAs() const{
        return new QiaogongCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QiaogongCard");
    }
};

class Manli: public TriggerSkill{
public:
    Manli():TriggerSkill("manli"){
        events << Predamage;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *turtle, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        const Card *reason = damage.card;
        if(reason == NULL)
            return false;
        Room *room = turtle->getRoom();
        if((reason->inherits("Slash") || reason->inherits("Duel"))
            && turtle->getWeapon() && turtle->getArmor()
            && turtle->askForSkillInvoke(objectName())){
            LogMessage log;
            log.type = "#ManliBuff";
            log.from = turtle;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            room->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class Menghan: public OneCardViewAsSkill{
public:
    Menghan():OneCardViewAsSkill("menghan"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getPhase() == Player::Play;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->getSuit() == Card::Spade;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Ecstasy *ecstasy = new Ecstasy(card->getSuit(), card->getNumber());
        ecstasy->setSkillName(objectName());
        ecstasy->addSubcard(card->getId());

        return ecstasy;
    }
};

class ShudanClear: public TriggerSkill{
public:
    ShudanClear():TriggerSkill("#shudan-clear"){
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &) const{
        if(player->getPhase() == Player::NotActive)
            player->getRoom()->setTag("Shudan", QVariant());
        return false;
    }
};

class Shudan: public TriggerSkill{
public:
    Shudan():TriggerSkill("shudan"){
        events << Damaged << CardEffected;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        if(player->getPhase() != Player::NotActive)
            return false;
        if(event == Damaged){
            room->setTag("Shudan", player->objectName());
            room->playSkillEffect(objectName());

            LogMessage log;
            log.type = "#ShudanDamaged";
            log.from = player;
            room->sendLog(log);

        }else if(event == CardEffected){
            if(room->getTag("Shudan").toString() != player->objectName())
                return false;
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if(effect.card->inherits("Slash") || effect.card->getTypeId() == Card::Trick){
                LogMessage log;
                log.type = "#ShudanAvoid";
                log.arg = objectName();
                log.from = player;
                room->sendLog(log);
                return true;
            }
        }
        return false;
    }
};

class Aoxiang: public TriggerSkill{
public:
    Aoxiang():TriggerSkill("aoxiang"){
        events << HpChanged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        player->tag["AoxiangStore"] = player->getGeneralName();
        if(player->isWounded()){
            room->setPlayerProperty(player, "general", "tongguanf");
        }
        else{
            QString gen_name = player->tag.value("AoxiangStore", "tongguanf").toString();
            room->setPlayerProperty(player, "general", gen_name);
        }
        return false;
    }
};

JiaomieCard::JiaomieCard(){
    mute = true;
}

bool JiaomieCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int tarGets = Self->getHp();
    if(targets.length() >= tarGets)
        return false;
    return Self->canSlash(to_select);
}

void JiaomieCard::onEffect(const CardEffectStruct &effect) const{
    DamageStruct damage;
    damage.from = effect.from;
    damage.to = effect.to;
    effect.from->getRoom()->damage(damage);
}

class Jiaomie: public ZeroCardViewAsSkill{
public:
    Jiaomie():ZeroCardViewAsSkill("jiaomie"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@zhengFa";
    }

    virtual const Card *viewAs() const{
        return new JiaomieCard;
    }
};

ZhengfaCard::ZhengfaCard(){
    once = true;
    will_throw = false;
    mute = true;
}

bool ZhengfaCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getGender() != Self->getGender()
            && !to_select->isWounded() && !to_select->isKongcheng() && to_select != Self;
}

void ZhengfaCard::use(Room *room, ServerPlayer *tonguan, const QList<ServerPlayer *> &targets) const{
    bool success = tonguan->pindian(targets.first(), "zhengfa", this);
    if(success){
        if(tonguan->getGeneral()->isMale())
            room->playSkillEffect("jiaomie", qrand() % 2 + 1);
        else
            room->playSkillEffect("jiaomie", qrand() % 2 + 3);
        room->askForUseCard(tonguan, "@@zhengFa", "@jiaomie-effect");
    }else{
        if(tonguan->getGeneral()->isMale())
            room->playSkillEffect("zhengfa", 1);
        else
            room->playSkillEffect("zhengfa", 2);
        tonguan->turnOver();
    }
}

class Zhengfa: public OneCardViewAsSkill{
public:
    Zhengfa():OneCardViewAsSkill("zhengfa"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ZhengfaCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new ZhengfaCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

BWQZPackage::BWQZPackage()
    :Package("BWQZ")
{
    General *houjian = new General(this, "houjian", "min", 3);
    houjian->addSkill(new Yuanyin);

    General *mengkang = new General(this, "mengkang", "kou", 4);
    mengkang->addSkill(new Zaochuan);
    mengkang->addSkill(new Skill("mengchong", Skill::Compulsory));

    General *jiaoting = new General(this, "jiaoting", "kou", 4);
    jiaoting->addSkill(new Skill("qinlong"));

    General *shantinggui = new General(this, "shantinggui", "jiang", 5, true, true);
    shantinggui->addSkill(new Xiaofang);
    shantinggui->addSkill(new Skill("shuizhan", Skill::Compulsory));

    General *qingzhang = new General(this, "qingzhang", "kou", 3);
    qingzhang->addSkill(new Shouge);
    qingzhang->addSkill(new Qiongtu);

    General *kongliang = new General(this, "kongliang", "kou", 3);
    kongliang->addSkill(new Nusha);
    kongliang->addSkill(new Wanku);

    General *jiashi = new General(this, "jiashi", "min", 3, false);
    jiashi->addSkill(new Zhuying);
    jiashi->addSkill(new Banzhuang);

    General *taozongwang = new General(this, "taozongwang", "min", 3);
    taozongwang->addSkill(new Qiaogong);
    taozongwang->addSkill(new Manli);

    General *baisheng = new General(this, "baisheng", "min", 4);
    baisheng->addSkill(new Menghan);
    baisheng->addSkill(new Shudan);
    baisheng->addSkill(new ShudanClear);
    related_skills.insertMulti("shudan", "#shudan-clear");

    General *tongguan = new General(this, "tongguan", "guan", 4);
    tongguan->addSkill(new Aoxiang);
    tongguan->addSkill(new Zhengfa);
    tongguan->addSkill(new Jiaomie);

    General *tongguanf = new General(this, "tongguanf", "guan", 4, false, true);

    addMetaObject<YuanyinCard>();
    addMetaObject<ShougeCard>();
    addMetaObject<NushaCard>();
    addMetaObject<QiaogongCard>();
    addMetaObject<ZhengfaCard>();
    addMetaObject<JiaomieCard>();
}

ADD_PACKAGE(BWQZ);
