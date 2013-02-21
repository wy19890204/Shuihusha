#include "dragon.h"
#include "maneuvering.h"
#include "plough.h"

class Xixue: public TriggerSkill{
public:
    Xixue():TriggerSkill("xixue"){
        events << Predamage << Death;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == Death){
            QList<ServerPlayer *> yanes = room->findPlayersBySkillName(objectName());
            if(player->getGender() == General::Female){
                RecoverStruct rec;
                rec.who = player;
                foreach(ServerPlayer *yn, yanes){
                    if(yn->isWounded()){
                        room->playSkillEffect(objectName(), qrand() % 2 + 4);
                        LogMessage log;
                        log.type = "#TriggerSkill";
                        log.from = yn;
                        log.arg = objectName();
                        room->sendLog(log);
                        room->recover(yn, rec, true);
                    }
                }
            }
            return false;
        }
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to->isDead() || !damage.from)
            return false;
        int index = 0;
        PlayerStar source = player;
        if(damage.to->getGender() == General::Female && damage.from->hasSkill(objectName())){
            source = damage.from;
            index = 1;
        }
        else if(damage.from->getGender() == General::Female && damage.to->hasSkill(objectName())){
            source = damage.to;
            index = qrand() % 2 + 2;
        }
        if(index > 0){
            room->playSkillEffect(objectName(), index);
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = source;
            log.arg = objectName();
            room->sendLog(log);
            room->loseHp(damage.to, damage.damage);
            return true;
        }
        return false;
    }
};

class Jiandiao: public MasochismSkill{
public:
    Jiandiao():MasochismSkill("jiandiao"){
    }

    virtual QString getDefaultChoice(ServerPlayer *player) const{
        if(player->getHandcardNum() < 2)
            return "diao";
        else
            return "jian";
    }

    virtual void onDamaged(ServerPlayer *lese, const DamageStruct &damage) const{
        Room *room = lese->getRoom();
        if(!damage.from || damage.from == lese)
            return;
        int lstn = qAbs(lese->getHp() - damage.from->getHp());
        lstn = qMax(qMin(lstn, 3), 1);
        QString choice = !damage.from->isNude() ?
                         room->askForChoice(lese, objectName(), "jian+diao+nil", QVariant::fromValue(damage)):
                         room->askForChoice(lese, objectName(), "diao+nil");
        if(choice == "nil")
            return;
        LogMessage log;
        log.from = lese;
        log.arg = objectName();
        if(choice == "jian"){
            room->playSkillEffect(objectName(), qrand() % 2 + 1);
            int card_id = room->askForCardChosen(lese, damage.from, "he", objectName());
            room->obtainCard(lese, card_id, room->getCardPlace(card_id) != Player::Hand);
            log.to << damage.from;
            log.type = "#UseSkill";
            room->sendLog(log);
        }
        else{
            room->playSkillEffect(objectName(), qrand() % 2 + 3);
            lese->drawCards(lstn);
            log.type = "#InvokeSkill";
            room->sendLog(log);
        }
    }
};

class JiaozhenViewAsSkill: public OneCardViewAsSkill{
public:
    JiaozhenViewAsSkill():OneCardViewAsSkill("jiaozhen"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->isBlack() && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *c = card_item->getCard();
        Duel *d = new Duel(c->getSuit(), c->getNumber());
        d->setSkillName(objectName());
        d->addSubcard(c);
        return d;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->hasFlag("Bark");
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return qrand() % 2 + 4;
    }
};

class Jiaozhen: public TriggerSkill{
public:
    Jiaozhen():TriggerSkill("jiaozhen"){
        events << PhaseChange << Predamage << DrawNCards;
        view_as_skill = new JiaozhenViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == Predamage){
            if(player->hasFlag("Bark")){
                DamageStruct damage = data.value<DamageStruct>();
                damage.nature = DamageStruct::Thunder;

                LogMessage log;
                log.type = "#JZThunder";
                log.from = player;
                log.arg = objectName();
                log.arg2 = QString::number(damage.damage);
                room->sendLog(log);
                room->playSkillEffect(objectName(), qrand() % 2 + 6);

                data = QVariant::fromValue(damage);
            }
            return false;
        }
        else if(event == DrawNCards){
            int n = data.toInt();
            if(player->hasFlag("Bark")){
                room->playSkillEffect(objectName(), 3);
                n--;
            }
            data = qMax(n, 0);
            return false;
        }
        else{
            if(player->getPhase() == Player::RoundStart && player->askForSkillInvoke(objectName())){
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(club|spade):(.*)");;
                judge.good = true;
                judge.reason = objectName();
                judge.who = player;
                room->playSkillEffect(objectName(), 1);

                room->judge(judge);
                if(judge.isGood()){
                    LogMessage log;
                    log.type = "#Jiaozhen";
                    log.from = player;
                    room->sendLog(log);
                    room->setPlayerFlag(player, "Bark");
                }
                else
                    room->playSkillEffect(objectName(), 2);
            }
        }
        return false;
    }
};

TaolueCard::TaolueCard(){
    once = true;
    mute = true;
    will_throw = false;
}

bool TaolueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void TaolueCard::use(Room *room, ServerPlayer *player, const QList<ServerPlayer *> &targets) const{
    room->playSkillEffect(skill_name, 1);
    bool success = player->pindian(targets.first(), skill_name, this);
    if(!success){
        room->playSkillEffect(skill_name, 3);
        if(!player->isNude())
            room->askForDiscard(player, skill_name, 1, false, true);
        return;
    }
    PlayerStar from = targets.first();
    if(from->getCards("ej").isEmpty())
        return;

    room->playSkillEffect(skill_name, 2);
    int card_id = room->askForCardChosen(player, from , "ej", skill_name);
    const Card *card = Sanguosha->getCard(card_id);
    Player::Place place = room->getCardPlace(card_id);

    int equip_index = -1;
    const DelayedTrick *trick = NULL;
    if(place == Player::Equip){
        const EquipCard *equip = qobject_cast<const EquipCard *>(card);
        equip_index = static_cast<int>(equip->location());
    }else
        trick = DelayedTrick::CastFrom(card);

    QList<ServerPlayer *> tos;
    foreach(ServerPlayer *p, room->getAlivePlayers()){
        if(equip_index != -1){
            if(p->getEquip(equip_index) == NULL)
                tos << p;
        }else{
            if(!player->isProhibited(p, trick) && !p->containsTrick(trick->objectName(), false))
                tos << p;
        }
    }
    if(trick && trick->isVirtualCard())
        delete trick;

    room->setTag("TaolueTarget", QVariant::fromValue(from));
    ServerPlayer *to = room->askForPlayerChosen(player, tos, skill_name);
    if(to)
        room->moveCardTo(card, to, place);
    room->removeTag("TaolueTarget");

    room->playSkillEffect(skill_name, place == Player::Equip ? 4 : 5);
}

class Taolue: public OneCardViewAsSkill{
public:
    Taolue():OneCardViewAsSkill("taolue"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("TaolueCard") && !player->isKongcheng();
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new TaolueCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Changsheng: public TriggerSkill{
public:
    Changsheng():TriggerSkill("changsheng"){
        events << Pindian;
        frequency = Compulsory;
    }

    virtual int getPriority(TriggerEvent) const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *, QVariant &data) const{
        QList<ServerPlayer *> aokoo = room->findPlayersBySkillName(objectName());
        if(aokoo.isEmpty())
            return false;
        PindianStar pindian = data.value<PindianStar>();
        if(!aokoo.contains(pindian->from) && !aokoo.contains(pindian->to))
            return false;
        Card *pdcd;
        foreach(ServerPlayer *aoko, aokoo){
            bool invoke = false;
            if(pindian->from != aoko && pindian->to != aoko)
                continue;
            if(pindian->from != aoko && pindian->to_card->getSuit() == Card::Spade){
                pdcd = Sanguosha->cloneCard(pindian->to_card->objectName(), pindian->to_card->getSuit(), 13);
                pdcd->addSubcard(pindian->to_card);
                pdcd->setSkillName(objectName());
                pindian->to_card = pdcd;
                invoke = true;
            }
            else if(pindian->to != aoko && pindian->from_card->getSuit() == Card::Spade){
                pdcd = Sanguosha->cloneCard(pindian->from_card->objectName(), pindian->from_card->getSuit(), 13);
                pdcd->addSubcard(pindian->from_card);
                pdcd->setSkillName(objectName());
                pindian->from_card = pdcd;
                invoke = true;
            }

            if(invoke){
                LogMessage log;
                log.type = "#Changsheng";
                log.from = aoko;
                log.arg = objectName();
                room->sendLog(log);
            }

            data = QVariant::fromValue(pindian);
        }
        return false;
    }
};

AnxiCard::AnxiCard(){
}

bool AnxiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getHp() >= Self->getHp() && to_select != Self;
}

void AnxiCard::onEffect(const CardEffectStruct &effect) const{
    DamageStruct dm;
    dm.from = effect.from;
    dm.to = effect.to;
    effect.from->getRoom()->damage(dm);
}

class AnxiViewAsSkill: public OneCardViewAsSkill{
public:
    AnxiViewAsSkill():OneCardViewAsSkill("anxi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@anxi";
    }

    virtual bool viewFilter(const CardItem *d) const{
        return d->getCard()->isRed() && !d->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        AnxiCard *card = new AnxiCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Anxi: public TriggerSkill{
public:
    Anxi():TriggerSkill("anxi"){
        events << DamageConclude;
        view_as_skill = new AnxiViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.card || !damage.card->isKindOf("TrickCard"))
            return false;
        QList<ServerPlayer *> ruang0rou = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *ruangorou, ruang0rou){
            if(player->isDead() || ruangorou->isKongcheng())
                break;
            QList<ServerPlayer *> targets;
            foreach(ServerPlayer *tmp, room->getOtherPlayers(ruangorou)){
                if(tmp->getHp() >= ruangorou->getHp())
                    targets << tmp;
            }
            if(!targets.isEmpty())
                room->askForUseCard(ruangorou, "@@anxi", "@anxi:" + player->objectName(), true);
        }
        return false;
    }
};

class Shuilao: public OneCardViewAsSkill{
public:
    Shuilao():OneCardViewAsSkill("shuilao"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("EquipCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *first = card_item->getCard();
        Indulgence *indulgence = new Indulgence(first->getSuit(), first->getNumber());
        indulgence->addSubcard(first->getId());
        indulgence->setSkillName(objectName());
        return indulgence;
    }
};

class Shuizhen: public TriggerSkill{
public:
    Shuizhen():TriggerSkill("shuizhen"){
        events << DamagedProceed << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual int getPriority(TriggerEvent event) const{
        return event == DamagedProceed ? 2 : -1;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        QList<ServerPlayer *> waters = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *water, waters){
            if(water->isKongcheng())
                continue;
            if(event == DamagedProceed){
                if(damage.nature == DamageStruct::Fire &&
                   room->askForCard(water, ".", "@shuizhen1", true, data, CardDiscarded)){
                    room->playSkillEffect(objectName(), qrand() % 3 + 1);
                    LogMessage log;
                    log.type = "#Shuizhen";
                    log.from = water;
                    log.arg = objectName();
                    log.to << damage.to;
                    room->sendLog(log);

                    return true;
                }
            }
            else{
                if(damage.nature == DamageStruct::Thunder &&
                   room->askForCard(water, ".", "@shuizhen2", true, data, CardDiscarded)){
                    ServerPlayer *forbider = damage.to;
                    foreach(ServerPlayer *tmp, room->getOtherPlayers(water)){
                        if(tmp == forbider)
                            continue;
                        DamageStruct ailue;
                        ailue.from = water;
                        ailue.to = tmp;
                        ailue.nature = DamageStruct::Thunder;
                        room->playSkillEffect(objectName(), qrand() % 3 + 3);
                        room->damage(ailue);
                    }
                }
            }
        }
        return false;
    }
};

class Yanmo: public MasochismSkill{
public:
    Yanmo():MasochismSkill("yanmo"){
    }

    virtual void onDamaged(ServerPlayer *ubun, const DamageStruct &damage) const{
        Room *room = ubun->getRoom();
        if(damage.nature != DamageStruct::Normal)
            return;
        QList<ServerPlayer *> yan, mo;
        foreach(ServerPlayer *tmp, room->getAlivePlayers()){
            if(tmp->getWeapon() || tmp->getArmor())
                yan << tmp;
            if(tmp->getOffensiveHorse() || tmp->getDefensiveHorse())
                mo << tmp;
        }
        if(yan.isEmpty() && mo.isEmpty())
            return;
        QString choices = yan.isEmpty() ? "mo+nil" :
                          mo.isEmpty() ? "yan+nil" : "yan+mo+nil";
        QString ssj = room->askForChoice(ubun, objectName(), choices);
        if(ssj == "nil")
            return;
        room->playSkillEffect(objectName());
        LogMessage log;
        log.from = ubun;
        log.arg = objectName();
        if(ssj == "yan"){
            log.type = "#YanmoYan";
            foreach(ServerPlayer *tmp, yan){
                room->throwCard(tmp->getWeapon(), tmp, ubun);
                room->throwCard(tmp->getArmor(), tmp, ubun);
            }
        }
        else{
            log.type = "#YanmoMo";
            foreach(ServerPlayer *tmp, mo){
                room->throwCard(tmp->getOffensiveHorse(), tmp, ubun);
                room->throwCard(tmp->getDefensiveHorse(), tmp, ubun);
            }
        }
        room->sendLog(log);
    }
};

ShexinCard::ShexinCard(){
    once = true;
}

bool ShexinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void ShexinCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.value(0, source);
    CardEffectStruct effect;
    effect.card = this;
    effect.from = source;
    effect.to = target;

    room->cardEffect(effect);
}

void ShexinCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    QList<int> cardes = effect.to->handCards();
    room->fillAG(cardes, effect.from);
    room->askForAG(effect.from, cardes, true, "shexin");
    QList<const Card *> cards = effect.to->getHandcards();
    foreach(const Card *card, cards){
        if(!card->inherits("BasicCard")){
            room->showCard(effect.to, card->getEffectiveId());
            room->getThread()->delay();
            room->throwCard(card, effect.to, effect.from);
        }else{
            room->showCard(effect.to, card->getEffectiveId());
            room->getThread()->delay();
        }
        effect.from->invoke("clearAG");
    }
}

class Shexin: public OneCardViewAsSkill{
public:
    Shexin():OneCardViewAsSkill("shexin"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("ShexinCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("EquipCard") || to_select->getCard()->isNDTrick();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        ShexinCard *card = new ShexinCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Wugou:public ViewAsSkill{
public:
    Wugou():ViewAsSkill("wugou"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty())
            return to_select->getCard()->inherits("BasicCard");
        else if(selected.length() == 1){
            const Card *card = selected.first()->getFilteredCard();
            return to_select->getCard()->inherits("BasicCard") && to_select->getFilteredCard()->isRed() == card->isRed();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() == 2){
            const Card *first = cards.first()->getCard();
            int secondnum = cards.last()->getCard()->getNumber();
            Assassinate *a = new Assassinate(first->getSuit(), qMin(13, first->getNumber() + secondnum));
            a->addSubcards(cards);
            a->setSkillName(objectName());
            return a;
        }else
            return NULL;
    }
};

class Qiaojiang:public OneCardViewAsSkill{
public:
    Qiaojiang():OneCardViewAsSkill("qiaojiang"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();

        switch(ClientInstance->getStatus()){
        case Client::Playing:{
                // black trick as slash
                return card->inherits("TrickCard") && card->isBlack();
            }
        case Client::Responsing:{
                QString pattern = ClientInstance->getPattern();
                if(pattern == "slash")
                    return card->inherits("TrickCard") && card->isBlack();
                else if(pattern == "jink")
                    return card->inherits("TrickCard") && card->isRed();
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
        if(!card->inherits("TrickCard"))
            return NULL;
        if(card->isRed()){
            Jink *jink = new Jink(card->getSuit(), card->getNumber());
            jink->addSubcard(card);
            jink->setSkillName(objectName());
            return jink;
        }else{
            Slash *slash = new Slash(card->getSuit(), card->getNumber());
            slash->addSubcard(card);
            slash->setSkillName(objectName());
            return slash;
        }
    }
};

QianxianCard::QianxianCard(){
    once = true;
}

bool QianxianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select == Self)
        return false;
    if(targets.isEmpty())
        return true;
    if(targets.length() == 1){
        int max1 = targets.first()->getMaxHP();
        return to_select->getMaxHP() != max1;
    }
    return false;
}

bool QianxianCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(targets.length() != 2)
        return false;
    int max1 = targets.first()->getMaxHP();
    int max2 = targets.last()->getMaxHP();
    return max1 != max2;
}

void QianxianCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    const Card *club = room->askForCard(effect.to, ".C", "@qianxian:" + effect.from->objectName(), QVariant::fromValue(effect), NonTrigger);
    if(club){
        effect.from->obtainCard(club);
        if(!effect.to->faceUp())
            effect.to->turnOver();
        room->setPlayerProperty(effect.to, "chained", false);
    }
    else{
        if(effect.to->faceUp())
            effect.to->turnOver();
        room->setPlayerProperty(effect.to, "chained", true);
    }
}

class Qianxian: public OneCardViewAsSkill{
public:
    Qianxian():OneCardViewAsSkill("qianxian"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("QianxianCard");
    }

    virtual bool viewFilter(const CardItem *blackfeiyanshitrick) const{
        const Card *card = blackfeiyanshitrick->getCard();
        return card->isBlack() && card->isNDTrick();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        QianxianCard *card = new QianxianCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

class Meicha: public OneCardViewAsSkill{
public:
    Meicha():OneCardViewAsSkill("meicha"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Analeptic::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern.contains("analeptic");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped() && to_select->getFilteredCard()->getSuit() == Card::Club;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        Analeptic *analeptic = new Analeptic(card->getSuit(), card->getNumber());
        analeptic->setSkillName(objectName());
        analeptic->addSubcard(card->getId());
        return analeptic;
    }
};

class Chuqiao: public TriggerSkill{
public:
    Chuqiao():TriggerSkill("chuqiao"){
        frequency = Frequent;
        events << CardEffected;
    }

    virtual int getPriority(TriggerEvent) const{
        return 3;
    }

    virtual bool trigger(TriggerEvent, Room *, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!effect.card->inherits("Slash"))
            return false;
        if(player->getHandcardNum() < player->getMaxHp() && player->askForSkillInvoke(objectName(), data)){
            player->playSkillEffect(objectName());
            player->drawCards(player->getMaxHp() - player->getHandcardNum());
        }
        return false;
    }
};

class Jianwu: public MasochismSkill{
public:
    Jianwu():MasochismSkill("jianwu"){
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        PlayerStar target = damage.from;
        if(!target || target->isDead() || target == player)
            return;

        QList<ServerPlayer *> others = room->getOtherPlayers(player);
        others.removeOne(target);
        if(!others.isEmpty() && player->askForSkillInvoke(objectName(), QVariant::fromValue(target))){
            foreach(ServerPlayer *p, others){
                QString prompt = QString("@jianwu-slash:%1:%2").arg(player->objectName()).arg(target->objectName());
                const Card *slash = room->askForCard(p, "slash", prompt, QVariant::fromValue(target));
                if(slash) {
                    CardUseStruct use;
                    use.card = slash;
                    use.to << target;
                    use.from = player;
                    room->useCard(use);
                    break;
                }
            }
        }
    }
};

XiashuCard::XiashuCard(){
}

bool XiashuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void XiashuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.to->drawCards(3);
    room->askForDiscard(effect.to, "xiashu", qMin(2, effect.to->getHandcardNum()));
}

class Xiashu: public OneCardViewAsSkill{
public:
    Xiashu():OneCardViewAsSkill("xiashu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("XiashuCard");
    }

    virtual bool viewFilter(const CardItem*) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *originalCard) const{
        Card *card = new XiashuCard;
        card->addSubcard(originalCard->getFilteredCard());
        return card;
    }
};

class Xiaozhan: public TriggerSkill{
public:
    Xiaozhan():TriggerSkill("xiaozhan") {
        events << CardUsed << CardAsked << Damage << CardFinished;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        if(triggerEvent == CardUsed){
            if(player->isNude())
                return false;
            CardUseStruct use = data.value<CardUseStruct>();
            if(!use.card->isKindOf("Slash"))
                return false;
            bool invoke = false;
            foreach(ServerPlayer *p, use.to){
                if(!p->isKongcheng()) {
                    invoke = true;
                    break;
                }
            }
            if(!invoke)
                return false;

            if(room->askForCard(player, "..", "@xiaozhan", true, data, CardDiscarded)){
                room->playSkillEffect(objectName(), qrand() % 2 + 1);
                LogMessage log;
                log.type = "#InvokeSkill";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);
                foreach(ServerPlayer *p, use.to)
                    if(!p->isKongcheng())
                        room->askForDiscard(p, objectName(), 1);
            }
        }
        else if(triggerEvent == CardAsked) {
            QString pattern = data.toString();
            if(pattern != "jink")
                return false;

            room->setPlayerFlag(player, "xiaozhanusing");
            if(room->askForUseCard(player, "slash", "@xiaozhan-slash", true)){
                room->setPlayerFlag(player, "-xiaozhanusing");
                if(player->hasFlag("xiaozhansuccess")) {
                    room->setPlayerFlag(player, "-xiaozhansuccess");
                    Jink *jink = new Jink(Card::NoSuit, 0);
                    jink->setSkillName(objectName());
                    room->provide(jink);
                }
            }
            room->setPlayerFlag(player, "-xiaozhanusing");
        }
        else if(triggerEvent == Damage && player->hasFlag("xiaozhanusing")){
            room->playSkillEffect(objectName(), qrand() % 2 + 3);
            room->setPlayerFlag(player, "xiaozhansuccess");
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return 0;
    }
};

class Chongfeng: public TriggerSkill{
public:
    Chongfeng():TriggerSkill("chongfeng") {
        events << PhaseChange << Damage << Damaged;
    }

    virtual bool trigger(TriggerEvent e, Room* room, ServerPlayer *suoch, QVariant &data) const{
        if(e == PhaseChange){
            if(suoch->getPhase() == Player::RoundStart)
                room->setPlayerMark(suoch, "@hatchet", 0);
            else if(suoch->getPhase() == Player::NotActive){
                int fist = suoch->getMark("@hatchet");
                room->setPlayerMark(suoch, "@hatchet", 0);
                if(fist >= 2 && suoch->askForSkillInvoke(objectName())){
                    room->playSkillEffect(objectName());
                    suoch->gainAnExtraTurn(suoch);
                }
            }
            return false;
        }
        else if(suoch->getPhase() != Player::NotActive){
            DamageStruct damage = data.value<DamageStruct>();
            suoch->gainMark("@hatchet", damage.damage);
        }
        return false;
    }
};

DragonPackage::DragonPackage()
    :GeneralPackage("dragon")
{
    General *liutang = new General(this, "liutang", "kou");
    liutang->addSkill(new Xiashu);

    General *hantao = new General(this, "hantao", "guan");
    hantao->addSkill(new Taolue);
    hantao->addSkill(new Changsheng);

    General *shibao = new General(this, "shibao", "jiang");
    shibao->addSkill(new Xiaozhan);

    General *ruanxiaowu = new General(this, "ruanxiaowu", "min");
    ruanxiaowu->addSkill("#hp-1");
    ruanxiaowu->addSkill(new Anxi);
    ruanxiaowu->addSkill(new Shuilao);

    General *zhengtianshou = new General(this, "zhengtianshou", "kou", 3);
    zhengtianshou->addSkill(new Wugou);
    zhengtianshou->addSkill(new Qiaojiang);

    General *gaoyanei = new General(this, "gaoyanei", "guan", 3);
    gaoyanei->addSkill(new Xixue);
    gaoyanei->addSkill(new Jiandiao);

    General *shantinggui = new General(this, "shantinggui", "jiang", 3, true, true);
    shantinggui->addSkill(new Shuizhen);
    shantinggui->addSkill(new Yanmo);
    shantinggui->addSkill(new Skill("shuizhan", Skill::Compulsory));

    General *lizhu = new General(this, "lizhu", "min", 3);
    lizhu->addSkill(new Chuqiao);
    lizhu->addSkill(new Jianwu);

    General *yangchun = new General(this, "yangchun", "kou");
    yangchun->addSkill(new Shexin);

    General *qiongyaonayan = new General(this, "qiongyaonayan", "guan");
    qiongyaonayan->addSkill(new Jiaozhen);

    General *suochao = new General(this, "suochao", "jiang");
    suochao->addSkill(new Chongfeng);

    General *wangpo = new General(this, "wangpo", "min", 3, false);
    wangpo->addSkill(new Qianxian);
    wangpo->addSkill(new Meicha);

    addMetaObject<TaolueCard>();
    addMetaObject<AnxiCard>();
    addMetaObject<ShexinCard>();
    addMetaObject<QianxianCard>();
    addMetaObject<XiashuCard>();
}

ADD_PACKAGE(Dragon)
