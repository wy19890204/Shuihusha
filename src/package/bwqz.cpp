#include "bwqz.h"
#include "general.h"
#include "skill.h"
#include "carditem.h"
#include "engine.h"
#include "standard.h"
#include "clientplayer.h"
#include "plough.h"
#include "tocheck.h"

class Fushang: public MasochismSkill{
public:
    Fushang():MasochismSkill("fushang"){
        frequency = Compulsory;
    }

    virtual void onDamaged(ServerPlayer *hedgehog, const DamageStruct &damage) const{
        if(hedgehog->getMaxHP() > 3){
            LogMessage log;
            Room *room = hedgehog->getRoom();
            log.type = "#TriggerSkill";
            log.from = hedgehog;
            log.arg = objectName();
            room->sendLog(log);
            room->playSkillEffect(objectName());
            room->loseMaxHp(hedgehog);
        }
    }
};

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

class Mengchong: public DistanceSkill{
public:
    Mengchong():DistanceSkill("mengchong"){
    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        bool mengkang = from->hasSkill(objectName());
        foreach(const Player *player, from->getSiblings()){
            if(player->isAlive() && player->hasSkill(objectName())){
                mengkang = true;
                break;
            }
        }
        if(mengkang && !from->isChained() && to->isChained())
            return +1;
        else
            return 0;
    }
};

YuanyinCard::YuanyinCard(){
    mute = true;
}

bool YuanyinCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(targets.length() == 2)
        return true;
    return targets.length() == 1 && Self->canSlash(targets.first());
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

void YuanyinCard::onUse(Room *room, const CardUseStruct &card_use) const{
    QList<ServerPlayer *> targets = card_use.to;
    PlayerStar target; PlayerStar source = card_use.from;
    if(targets.length() > 1)
        target = targets.at(1);
    else if(targets.length() == 1 && source->canSlash(targets.first())){
        target = targets.first();
    }
    else
        return;

    const Card *weapon = targets.first()->getWeapon();
    if(weapon){
        Slash *slash = new Slash(weapon->getSuit(), weapon->getNumber());
        slash->setSkillName("yuanyin");
        slash->addSubcard(weapon);
        room->throwCard(weapon->getId());
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
            const Card *card = NULL;
            if(asked == "slash")
                card = target->getWeapon();
            else if(asked == "jink"){
                if(target->getEquips().length() == 1 && !target->getWeapon())
                    card = target->getEquips().first();
                else if(target->getWeapon() && target->getEquips().length() == 2)
                    card = target->getEquips().at(1);
                else
                    card = Sanguosha->getCard(room->askForCardChosen(player, target, "e", objectName()));
            }
            if(asked == "jink" && target->getWeapon() && target->getWeapon()->getId() == card->getId())
                return false;
            if(asked == "slash"){
                Slash *yuanyin_card = new Slash(card->getSuit(), card->getNumber());
                yuanyin_card->setSkillName(objectName());
                yuanyin_card->addSubcard(card);
                room->provide(yuanyin_card);
            }
            else{
                Jink *yuanyin_card = new Jink(card->getSuit(), card->getNumber());
                yuanyin_card->setSkillName(objectName());
                yuanyin_card->addSubcard(card);
                room->provide(yuanyin_card);
            }
            room->setEmotion(player, "good");
            return true;
        }
        return false;
    }
};

class Xiaofang: public TriggerSkill{
public:
    Xiaofang():TriggerSkill("xiaofang"){
        events << Predamaged << Damaged;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        ServerPlayer *water = room->findPlayerBySkillName(objectName());
        if(!water || water->isKongcheng())
            return false;
        if(event == Predamaged){
            if(damage.nature == DamageStruct::Fire &&
               water->askForSkillInvoke(objectName()) &&
               room->askForDiscard(water, objectName(), 1)){
                LogMessage log;
                log.type = "#Xiaofan";
                log.from = water;
                log.arg = objectName();
                log.to << damage.to;
                room->sendLog(log);

                return true;
            }
        }
        else{
            if(damage.nature == DamageStruct::Thunder &&
               water->askForSkillInvoke(objectName()) &&
               room->askForDiscard(water, objectName(), 1)){
                ServerPlayer *forbider = damage.to;
                foreach(ServerPlayer *tmp, room->getOtherPlayers(water)){
                    if(tmp == forbider)
                        continue;
                    DamageStruct ailue;
                    ailue.from = water;
                    ailue.to = tmp;
                    ailue.nature = DamageStruct::Thunder;
                    room->damage(ailue);
                }
            }
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

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        PlayerStar target = player;
        QList<ServerPlayer *> zhangqings = room->findPlayersBySkillName(objectName());
        if(zhangqings.isEmpty() || target->getPhase() != Player::Finish)
            return false;
        foreach(ServerPlayer *zhangqing, zhangqings){
            if(target->getHandcardNum() <= 1 && !target->isNude()
                && zhangqing->askForSkillInvoke(objectName(), QVariant::fromValue(target))){
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
            && turtle->askForSkillInvoke(objectName(), data)){
            room->playSkillEffect(objectName());
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

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
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
        return false;
    }
};

class AoxiangChange: public TriggerSkill{
public:
    AoxiangChange():TriggerSkill("#aox_cg"){
        events << GameStart;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &) const{
        if(player->getGeneral2Name() == "tongguan"){
            player->getRoom()->setPlayerProperty(player, "general2", player->getGeneralName());
            player->getRoom()->setPlayerProperty(player, "general", "tongguan");
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
    if(!Self->hasUsed("ZhengfaCard")){
        bool success = tonguan->pindian(targets.first(), "zhengfa", this);
        if(success){
            room->playSkillEffect("zhengfa", tonguan->getGeneral()->isMale()? 1: 3);
            room->askForUseCard(tonguan, "@@zhengfa", "@zhengfa-effect");
        }else{
            room->playSkillEffect("zhengfa", tonguan->getGeneral()->isMale()? 5: 6);
            tonguan->turnOver();
        }
    }
    else{
        foreach(ServerPlayer *tarmp, targets)
            room->cardEffect(this, tonguan, tarmp);
        room->playSkillEffect("zhengfa", tonguan->getGeneral()->isMale()? 2: 4);
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

class Kongying: public TriggerSkill{
public:
    Kongying():TriggerSkill("kongying"){
        events << CardResponsed;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardStar card_star = data.value<CardStar>();
        if(!card_star->inherits("Jink"))
            return false;

        if(player->askForSkillInvoke(objectName())){
            Room *room = player->getRoom();
            room->playSkillEffect(objectName());
            ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName());
            if(!room->askForCard(target, "jink", "@kongying:" + player->objectName())){
                DamageStruct damage;
                damage.from = player;
                damage.to = target;
                room->damage(damage);
            }
        }
        return false;
    }
};

class Jibu: public DistanceSkill{
public:
    Jibu():DistanceSkill("jibu"){
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

YongleCard::YongleCard(){
}

int YongleCard::getKingdoms(const Player *Self) const{
    QSet<QString> kingdom_set;
    QList<const Player *> players = Self->getSiblings();
    players << Self;
    foreach(const Player *player, players){
        if(player->isDead())
            continue;
        kingdom_set << player->getKingdom();
    }
    return kingdom_set.size();
}

bool YongleCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int x = getKingdoms(Self);
    return targets.length() < x && !to_select->isKongcheng();
}

bool YongleCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    int x = getKingdoms(Self);
    return targets.length() <= x && !targets.isEmpty();
}

void YongleCard::use(Room *room, ServerPlayer *fangla, const QList<ServerPlayer *> &targets) const{
    foreach(ServerPlayer *tmp, targets){
        const Card *card = tmp->getRandomHandCard();
        fangla->obtainCard(card);
    }
    foreach(ServerPlayer *tmp, targets){
        const Card *card = room->askForCardShow(fangla, tmp, "yongle");
        tmp->obtainCard(card);
    }
}

class Yongle: public ZeroCardViewAsSkill{
public:
    Yongle():ZeroCardViewAsSkill("yongle"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("YongleCard");
    }

    virtual const Card *viewAs() const{
        return new YongleCard;
    }
};

class ZhiYPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return !player->hasEquip(card);
    }
    virtual bool willThrow() const{
        return false;
    }
};

class Zhiyuan: public TriggerSkill{
public:
    Zhiyuan():TriggerSkill("zhiyuan$"){
        events << CardLost;
        //frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasLordSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *fang1a, QVariant &data) const{
        if(fang1a->isKongcheng()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(move->from_place == Player::Hand){
                Room *room = fang1a->getRoom();
                QList<ServerPlayer *> lieges = room->getLieges("jiang", fang1a);
                foreach(ServerPlayer *tmp, lieges){
                    const Card *card = room->askForCard(tmp, ".Zy", "@zhiyuan:" + fang1a->objectName(), data);
                    if(card){
                        room->playSkillEffect(objectName());
                        LogMessage lo;
                        lo.type = "#InvokeSkill";
                        lo.from = tmp;
                        lo.arg = objectName();
                        room->sendLog(lo);
                        room->obtainCard(fang1a, card, false);
                        break;
                    }
                }
            }
        }
        return false;
    }
};

class Qiaogongplus: public TriggerSkill{
public:
    Qiaogongplus():TriggerSkill("qiaog"){
        events << CardLost << CardFinished;
    }

    virtual bool triggerable(const ServerPlayer *ta) const{
        //return !ta->hasSkill(objectName());
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *gui = room->findPlayerBySkillName(objectName());
        if(!gui)
            return false;
        if(event == CardLost){ // 失去装备
            CardMoveStar move = data.value<CardMoveStar>();
            QList<ServerPlayer *> hasweapon;
            bool tao = false;
            if(move->from->isDead() || move->from_place != Player::Equip)
                return false;
            int card_id = move->card_id;
            const Card *equ = Sanguosha->getCard(card_id);
            if(player == gui){ // 如果是阿龟自己失去装备，则须用自己装备覆盖原装备主人的装备区
                foreach(ServerPlayer *tmp, room->getOtherPlayers(gui)){
                    foreach(const Card *e, tmp->getEquips()){
                        if(e == equ){ //寻找主人
                            const EquipCard *equipped = qobject_cast<const EquipCard *>(equ);
                            QList<ServerPlayer *> targets;
                            targets << tmp;
                            equipped->use(room, tmp, targets);
                            //room->moveCardTo(equ, tmp, Player::Equip, false);
                            return false;
                        }
                    }
                }
                return false;
            }
            foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
                //如果出现有某人失去装备后满足阿龟条件的情况
                foreach(const Card *e, tmp->getEquips()){
                    if(e->getSubtype() == equ->getSubtype()){
                        if(tmp != gui && !hasweapon.contains(tmp))
                            hasweapon << tmp; //将除了阿龟外还有该类装备的人存储
                        else if(tmp == gui)
                            tao = true; //阿龟有该类装备
                    }
                }
            }
            if(hasweapon.isEmpty() && tao){ // 场上只有阿龟有该类装备
                //room->moveCardTo(equ, gui, Player::Equip, false);
                room->throwCard(card_id);
            }
            else if(hasweapon.length() == 1 && !gui->getEquips().contains(equ)){
     /*           //场上除了阿龟外是否有且只有一人有该类装备
        // 两种情况，一种是该人和阿龟装备不同，这是因为阿龟装的是自己的装备
      // 还有一种情况，装备相同，那是不可能的，因为只有唯一才可能出现装备相同的情况
      // 两种情况都不用考虑，所以注释掉。
                ServerPlayer *target = hasweapon.first(); //找到该人
                const Card *bequ = NULL; //找到该装备
                foreach(const Card *e, target->getEquips()){
                    if(e->getSubtype() == equ->getSubtype()){
                        bequ = e;
                        break;
                    }
                }
                bool flag = false;
                foreach(const Card *tmp, gui->getEquips()){ //阿龟卸载装备
                    if(tmp->getSubtype() == bequ->getSubtype() && bequ != tmp){
                        room->throwCard(tmp->getId());
                        flag = true;
                    }
                }
                if(!flag)
                    return false; // 如果阿龟装的是自己本来就有的装备，则中止
                QList<int> card_ids;
                int card_id = bequ->getId();
                card_ids << card_id;
                room->fillAG(card_ids, gui);
                room->takeAG(gui, card_id);
                card_ids.removeOne(card_id);
                room->broadcastInvoke("clearAG");
                gui->obtainCard(bequ);
                room->moveCardTo(bequ, gui, Player::Equip, false);
                ~*room->throwCard(card_id);

                room->moveCardTo(bequ, gui, Player::Equip, false);
                QList<int> card_ids;
                card_ids << bequ->getId();
                int card_id = bequ->getId();
                room->fillAG(card_ids, gui);
                //card_ids.removeOne(card_id);
                room->takeAG(gui, card_id);
                room->broadcastInvoke("clearAG");
                room->moveCardTo(equ, NULL, Player::DrawPile);
                gui->drawCards(1);
                room->moveCardTo(equ, target, Player::Equip, false);
                //room->throwCard(card_id);*~
            */}
        }
        else if(event == CardFinished){ //其他角色装备装备后
            CardUseStruct card_use = data.value<CardUseStruct>();
            const Card *equ = card_use.card;
            int card_id = card_use.card->getId();
            if(!equ->inherits("EquipCard"))
                return false;
            if(player == gui){ //如果是阿龟自己，中止，因为原装备被顶掉后会自动触发CardLost
                return false;
            }
            bool flag = false; //标记，false为该类型装备在场上只有一件
            foreach(ServerPlayer *tmp, room->getOtherPlayers(player)){
                //if(tmp == gui) //不考虑阿龟装备情况
                //    continue;
                QList<const Card*> equipos;
                foreach(const Card *e, tmp->getEquips()){
                    if(e->getSubtype() == equ->getSubtype()){
                        flag = true; //若除自己外还有人有同类装备，标记
                        equipos << e; //记录现有装备
                    }
                }
                if(flag){ // 若不满足条件，则阿龟复制的装备也不能存在
                    foreach(const Card *e, gui->getEquips()){
                        foreach(const Card *tmp, equipos){
                            if(e == tmp){
                                room->throwCard(e->getId());
                                break;
                            }
                        }
                    }
                    break;
                }
            }
            if(!flag){ //场上只存在一个同类装备（就是自己装备的那个）
                //room->moveCardTo(equ, gui, Player::Equip, false);
                QList<int> card_ids;
                QList<const Card *> cards = player->getEquips();
                foreach(const Card *tmp, cards)
                    if(tmp->getId() == card_id)
                        card_ids << tmp->getId();
                room->fillAG(card_ids, gui);
                //card_id = room->askForAG(gui, card_ids, false, objectName());
                room->takeAG(gui, card_id);
                card_ids.removeOne(card_id);
                room->broadcastInvoke("clearAG");
                gui->obtainCard(equ);
                room->moveCardTo(equ, gui, Player::Equip, false);
                //room->throwCard(card_id);
            }
        }
        return false;
    }
};

BWQZPackage::BWQZPackage()
    :Package("BWQZ")
{
    General *dingdesun = new General(this, "dingdesun", "jiang", 6);
    dingdesun->addSkill(new Skill("beizhan"));
    dingdesun->addSkill(new Fushang);

    General *houjian = new General(this, "houjian", "kou", 2);
    houjian->addSkill(new Yuanyin);

    General *mengkang = new General(this, "mengkang", "kou");
    mengkang->addSkill(new Zaochuan);
    mengkang->addSkill(new Mengchong);

    General *jiaoting = new General(this, "jiaoting", "kou");
    jiaoting->addSkill(new Skill("qinlong"));

    General *shantinggui = new General(this, "shantinggui", "jiang", 5, true, true);
    shantinggui->addSkill(new Xiaofang);

    General *qingzhang = new General(this, "qingzhang", "kou", 3);
    qingzhang->addSkill(new Shouge);
    qingzhang->addSkill(new Qiongtu);

    General *kongliang = new General(this, "kongliang", "kou", 3);
    kongliang->addSkill(new Nusha);
    kongliang->addSkill(new Wanku);

    General *taozongwang = new General(this, "taozongwang", "min", 3);
    taozongwang->addSkill(new Qiaogong);
    taozongwang->addSkill(new Manli);
    //taozongwang->addSkill(new Qiaogongplus);

    General *baisheng = new General(this, "baisheng", "min", 3);
    baisheng->addSkill(new Menghan);
    baisheng->addSkill(new Shudan);
    baisheng->addSkill(new ShudanClear);
    related_skills.insertMulti("shudan", "#shudan-clear");

    General *tongguan = new General(this, "tongguan", "guan");
    tongguan->addSkill(new Aoxiang);
    tongguan->addSkill(new AoxiangChange);
    related_skills.insertMulti("aoxiang", "#aox_cg");
    tongguan->addSkill(new Zhengfa);

    tongguan = new General(this, "tongguanf", "yan", 4, false, true);
    tongguan->addSkill("aoxiang");
    tongguan->addSkill("zhengfa");
    tongguan->addSkill("zhengfa");

    General *wangdingliu = new General(this, "wangdingliu", "kou", 3);
    wangdingliu->addSkill(new Kongying);
    wangdingliu->addSkill(new Jibu);

    General *fangla = new General(this, "fangla$", "jiang");
    fangla->addSkill(new Yongle);
    fangla->addSkill(new Zhiyuan);
    patterns[".Zy"] = new ZhiYPattern;

    addMetaObject<YuanyinCard>();
    addMetaObject<ShougeCard>();
    addMetaObject<NushaCard>();
    addMetaObject<QiaogongCard>();
    addMetaObject<ZhengfaCard>();
    addMetaObject<YongleCard>();
    skills << new Qiaogongplus;
}

ADD_PACKAGE(BWQZ);
