#include "tiger.h"
#include "plough.h"

class Guzong:public TriggerSkill{
public:
    Guzong():TriggerSkill("guzong"){
        events << CardAsked;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        QString pattern = data.toString();
        if(pattern != "jink")
            return false;

        QList<ServerPlayer *> nein = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *neinhg, nein){
            bool canivk = false;
            foreach(const Card *card, neinhg->getCards("he")){
                if(card->isKindOf("TrickCard")){
                    canivk = true;
                    break;
                }
            }
            if(!canivk)
                continue;
            QVariant tohelp = QVariant::fromValue((PlayerStar)player);
            const Card *yjk = room->askForCard(neinhg, "TrickCard,EquipCard", "@guzong:" + player->objectName(), true, tohelp, CardDiscarded);
            if(yjk){
                LogMessage log;
                log.type = "$Guzong";
                log.card_str = yjk->getEffectIdString();
                log.from = neinhg;
                log.arg = objectName();
                log.to << player;
                room->sendLog(log);
                Jink *jink = new Jink(Card::NoSuit, 0);
                jink->setSkillName(objectName());
                room->provide(jink);
                if(!player->isNude() && neinhg->askForSkillInvoke(objectName(), tohelp)){
                    int first = room->askForCardChosen(neinhg, player, "he", objectName());
                    room->throwCard(first, player, neinhg);
                    if(!player->isNude() && neinhg->askForSkillInvoke(objectName(), tohelp)){
                        first = room->askForCardChosen(neinhg, player, "he", objectName());
                        room->throwCard(first, player, neinhg);
                    }
                }
            }
        }
        return false;
    }
};

NeiyingCard::NeiyingCard(){
    mute = true;
}

bool NeiyingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;
    return true;
}

bool NeiyingCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return targets.length() == 2;
}

void NeiyingCard::weAreFriends(Room *room, ServerPlayer *you, ServerPlayer *me) const{
    QList<int> all1 = you->handCards();
    QList<int> all2 = me->handCards();
    room->playSkillEffect(skill_name, qrand() % 2 + 5);
    room->fillAG(all1, me);
    room->fillAG(all2, you);
    room->getThread()->delay(4000);
    //room->askForAG(you, me, true, skill_name);
    me->invoke("clearAG");
    you->invoke("clearAG");
}

void NeiyingCard::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &targets) const{
    weAreFriends(room, targets.first(), targets.last());
    LogMessage log;
    log.type = "#Neiying";
    log.to = targets;
    room->sendLog(log);
}

class Neiying:public ViewAsSkill{
public:
    Neiying():ViewAsSkill("neiying"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(ClientInstance->getPattern() == "nulliplot"){
            if(selected.isEmpty())
                return true;
            else if(selected.length() == 1){
                const Card *card = selected.first()->getCard();
                return to_select->getCard()->getColor() == card->getColor();
            }
        }
        return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(ClientInstance->getPattern() == "nulliplot"){
            if(cards.length() == 2){
                const Card *first = cards.first()->getCard();
                Counterplot *aa = new Counterplot(first->getSuit(), 0);
                aa->addSubcards(cards);
                aa->setSkillName(objectName());
                return aa;
            }else
                return NULL;
        }
        else{
            return new NeiyingCard;
        }
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("NeiyingCard");
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "nulliplot";
    }

    virtual bool isEnabledAtNullification(const ServerPlayer *player, bool include_counterplot) const{
        return player->getCardCount(true) > 1 && include_counterplot;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return qrand() % 4 + 1;
    }
};

JintangCard::JintangCard(){
    mute = true;
}

bool JintangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void JintangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    CardStar card = effect.from->tag["Jintg"].value<CardStar>();
    effect.from->tag.remove("Jintg");
    const EquipCard *equipped = qobject_cast<const EquipCard *>(card);
    QList<ServerPlayer *> targets;
    targets << effect.to;
    equipped->use(room, effect.from, targets);
}

class JintangViewAsSkill: public ZeroCardViewAsSkill{
public:
    JintangViewAsSkill():ZeroCardViewAsSkill("jintang"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@jintang!";
    }

    virtual const Card *viewAs() const{
        return new JintangCard;
    }
};

class Jintang: public TriggerSkill{
public:
    Jintang(): TriggerSkill("jintang"){
        events << Predamaged << Death;
        view_as_skill = new JintangViewAsSkill;
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == Predamaged){
            DamageStruct damage = data.value<DamageStruct>();
            LogMessage log;
            log.from = player;
            log.arg = objectName();
            log.arg2 = QString::number(damage.damage);
            if(player->getHp() == 1 && damage.nature == DamageStruct::Normal){
                log.type = "#JintangForb";
                room->sendLog(log);
                room->playSkillEffect(objectName(), qrand() % 2 + 2);
                room->setEmotion(damage.to, "avoid");
                return true;
            }
            if(player->getHp() <= 2 && damage.damage > 1){
                log.type = "#JintangCut";
                room->sendLog(log);
                damage.damage = 1;
                room->playSkillEffect(objectName(), 1);
                data = QVariant::fromValue(damage);
            }
        }
        else if(event == Death){
            if(player->hasEquip()){
                room->playSkillEffect(objectName(), qrand() % 2 + 4);
                foreach(CardStar equip, player->getEquips()){
                    player->tag["Jintg"] = QVariant::fromValue(equip);
                    room->askForUseCard(player, "@@jintang!", "@jintang:::" + equip->objectName(), true);
                }
                //room->getThread()->delay(1500);
            }
        }
        return false;
    }
};

class Pinming: public TriggerSkill{
public:
    Pinming():TriggerSkill("pinming"){
        events << DamageConclude;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        QList<ServerPlayer *> sanlangs = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *sanlang, sanlangs){
            if(damage.from->isAlive() && player != sanlang && sanlang->askForSkillInvoke(objectName(), data)){
                room->playSkillEffect(objectName(), qrand() % 3 + 1);
                room->loseMaxHp(sanlang);
                DamageStruct dag = damage;
                dag.from = sanlang;
                dag.to = player;
                dag.card = NULL;
                dag.chain = false;
                room->damage(dag);
            }
        }
        return false;
    }
};

class PinmingDie: public TriggerSkill{
public:
    PinmingDie():TriggerSkill("#pinming-die"){
        events << Dying;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *, QVariant &data) const{
        DyingStruct dying = data.value<DyingStruct>();
        DamageStar damage = dying.damage;
        if(damage && damage->from && damage->from->hasSkill("pinming") && damage->to->isAlive()){
            damage->from->setFlags("PinmingDie");
            if(damage->from->askForSkillInvoke("pinming", QVariant::fromValue(damage))){
                room->playSkillEffect("pinming", qrand() % 2 + 4);
                room->getThread()->delay(500);
                room->killPlayer(damage->to, damage);
                room->getThread()->delay(1000);
                room->killPlayer(damage->from);
            }
            damage->from->setFlags("-PinmingDie");
            return true;
        }
        return false;
    }
};

LiejiCard::LiejiCard(){
    will_throw = false;
}

bool LiejiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 2)
        return false;
    return to_select != Self;
}

void LiejiCard::onUse(Room *room, const CardUseStruct &card_use) const{
    room->throwCard(this, card_use.from);
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName(skill_name);
    CardUseStruct use;
    use.card = slash;
    use.from = card_use.from;
    use.to = card_use.to;
    room->useCard(use);
}

class LiejiViewAsSkill: public OneCardViewAsSkill{
public:
    LiejiViewAsSkill():OneCardViewAsSkill("lieji"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@lieji";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("BasicCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new LiejiCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Lieji:public PhaseChangeSkill{
public:
    Lieji():PhaseChangeSkill("lieji"){
        view_as_skill = new LiejiViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *lvfang) const{
        if(lvfang->getPhase() == Player::Play){
            if(lvfang->isKongcheng())
                return false;
            if(lvfang->getRoom()->askForUseCard(lvfang, "@@lieji", "@lieji", true))
                return true;
        }
        return false;
    }
};

class Wuzhou:public TriggerSkill{
public:
    Wuzhou():TriggerSkill("wuzhou"){
        events << CardLostDone << CardGotDone << PhaseChange;
        frequency = Frequent;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &) const{
        if(room->getCurrent() == player && player->getPhase() == Player::Play){
            if(!player->hasEquip())
                return false;
            int x = 5 - player->getEquips().count();
            if(player->getHandcardNum() < x && player->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());
                player->drawCards(x - player->getHandcardNum());
            }
        }
        return false;
    }
};

HuweiCard::HuweiCard(){
    will_throw = false;
}

void HuweiCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *frog = effect.to;
    if(frog->hasLordSkill("huwei")){
        const Card *card = Sanguosha->getCard(getSubcards().first());
        const EquipCard *equipped = qobject_cast<const EquipCard *>(card);
        QList<ServerPlayer *> targets;
        targets << frog;
        equipped->use(frog->getRoom(), effect.from, targets);
        effect.from->drawCards(1);
    }
}

bool HuweiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("huwei")
            && to_select != Self && !to_select->hasEquip();
}

class HuweiViewAsSkill: public OneCardViewAsSkill{
public:
    HuweiViewAsSkill():OneCardViewAsSkill("huweiv"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getKingdom() == "jiang";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("EquipCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        HuweiCard *card = new HuweiCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Huwei: public GameStartSkill{
public:
    Huwei():GameStartSkill("huwei$"){
    }

    virtual void onGameStart(ServerPlayer *tigger) const{
        if(!tigger->isLord())
            return;
        Room *room = tigger->getRoom();
        if(ServerInfo.EnableAnzhan || room->isNoLordSkill())
            return;
        QList<ServerPlayer *> players = room->getAlivePlayers();
        foreach(ServerPlayer *player, players){
            room->attachSkillToPlayer(player, "huweiv");
        }
    }

    virtual void onIdied(ServerPlayer *tigger) const{
        Room *room = tigger->getRoom();
        if(room->findPlayerBySkillName("huwei"))
            return;
        QList<ServerPlayer *> players = room->getAlivePlayers();
        foreach(ServerPlayer *player, players){
            room->detachSkillFromPlayer(player, "huweiv", false);
        }
    }
};

class Jielue:public TriggerSkill{
public:
    Jielue():TriggerSkill("jielue"){
        events << FinishJudge;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &) const{
        //JudgeStar judge = data.value<JudgeStar>();
        QList<ServerPlayer *> zhangs = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *zhah, zhangs){
            if(zhah == player || zhah->hasMark("fuhun_wake") || player->isKongcheng())
                continue;
            if(zhah->askForSkillInvoke(objectName(), QVariant::fromValue((PlayerStar)player))){
                room->playSkillEffect(objectName());
                room->obtainCard(zhah, room->askForCardChosen(zhah, player, "h", objectName()), false);
            }
        }
        return false;
    }
};

class Fuhun: public TriggerSkill{
public:
    Fuhun():TriggerSkill("fuhun"){
        events << Death;
        frequency = Wake;
    }

    virtual int getPriority(TriggerEvent) const{
        return -1;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        QList<ServerPlayer *> zhangs = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *zhang, zhangs){
            if(zhang == player || zhang->hasMark("fuhun_wake"))
                continue;

            room->awake(zhang, objectName(), "1500", 1500);

            room->detachSkillFromPlayer(zhang, "jielue");
            room->acquireSkill(zhang, "lihun");
            int count = 0;
            foreach(const Skill *skill, player->getVisibleSkillList()){
                if(skill->getLocation() == Skill::Right &&
                   skill->getFrequency() != Skill::Limited &&
                   skill->getFrequency() != Skill::Wake &&
                   !skill->isLordSkill()){
                    room->acquireSkill(zhang, skill->objectName());
                    count ++;
                }
            }
            if(count > 1)
                room->loseMaxHp(zhang);
        }
        return false;
    }
};

class Liehuo: public TriggerSkill{
public:
    Liehuo():TriggerSkill("liehuo"){
        events << SlashMissed << Damage;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *bao, QVariant &data) const{
        PlayerStar target;
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
           target->getHandcardNum() >= bao->getHandcardNum() &&
           room->askForSkillInvoke(bao, objectName(), QVariant::fromValue(target))){
            room->playSkillEffect(objectName());
            bao->obtainCard(target->getRandomHandCard(), false);
        }
        return false;
    }
};

class Longluo:public TriggerSkill{
public:
    Longluo():TriggerSkill("longluo"){
        events << CardEffected;
        frequency = Frequent;
    }

    virtual int getPriority(TriggerEvent) const{
        return 3;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *shien, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!effect.card->inherits("Slash"))
            return false;

        if(room->askForSkillInvoke(shien, objectName(), data)){
            bool xoxo = false;
            for(int i = 0; i < 2; i++){
                CardStar card = room->peek(); // get the first card of drawpile(not draw)
                room->getThread()->delay();

                LogMessage lolo;
                lolo.from = shien;
                lolo.card_str = card->getEffectIdString();
                if(!card->inherits("BasicCard")){
                    lolo.type = "$Longluo1";
                    room->sendLog(lolo);
                    room->throwCard(card);
                }else{
                    room->showCard(shien, card->getEffectiveId());
                    if(!xoxo){
                        room->playSkillEffect(objectName());
                        xoxo = true;
                    }
                    lolo.type = "$Longluo2";
                    shien->tag["LongluoCard"] = QVariant::fromValue(card);
                    ServerPlayer *target = room->askForPlayerChosen(shien, room->getAllPlayers(), objectName());
                    if(!target)
                        target = shien;
                    room->obtainCard(target, card);
                    lolo.to << target;
                    room->sendLog(lolo);
                    shien->tag.remove("LongluoCard");
                }
            }
        }
        return false;
    }
};

XiaozaiCard::XiaozaiCard(){
    will_throw = false;
}

bool XiaozaiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select == Self)
        return false;

    return !to_select->hasFlag("Xiaozai");
}

void XiaozaiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    PlayerStar target = targets.first();;
    target->obtainCard(this, false);
    DamageStruct damage = source->tag["XiaozaiDamage"].value<DamageStruct>();
    damage.to = target;
    room->setPlayerFlag(source, "-Xiaozai");
    room->damage(damage);
}

class XiaozaiViewAsSkill: public ViewAsSkill{
public:
    XiaozaiViewAsSkill():ViewAsSkill("xiaozai"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return !to_select->isEquipped() && selected.length() < 2;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@xiaozai";
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;
        XiaozaiCard *card = new XiaozaiCard;
        card->addSubcards(cards);
        return card;
    }
};

class Xiaozai: public TriggerSkill{
public:
    Xiaozai():TriggerSkill("xiaozai"){
        events << DamagedProceed;
        view_as_skill = new XiaozaiViewAsSkill;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.from)
            room->setPlayerFlag(damage.from, "Xiaozai");
        if(player->getHandcardNum() > 1){
            player->tag["XiaozaiDamage"] = data;
            if(room->askForUseCard(player, "@@xiaozai", "@xiaozai", true))
                return true;
            player->tag.remove("XiaozaiDamage");
        }
        if(damage.from)
            room->setPlayerFlag(damage.from, "-Xiaozai");
        return false;
    }
};

class Huxiao: public OneCardViewAsSkill{
public:
    Huxiao():OneCardViewAsSkill("huxiao"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->inherits("EquipCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        SavageAssault *savage_assault = new SavageAssault(card->getSuit(), card->getNumber());
        savage_assault->addSubcard(card->getId());
        savage_assault->setSkillName(objectName());
        return savage_assault;
    }
};

class Tanse:public TriggerSkill{
public:
    Tanse():TriggerSkill("tanse"){
        events << CardEffected;
    }

    virtual int getPriority(TriggerEvent) const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(effect.card->isNDTrick() && effect.from->getGeneral()->isFemale() && player->askForSkillInvoke(objectName(), data)){
            const Card *equip = room->askForCard(player, "EquipCard", "@tanse:" + effect.from->objectName(), data, NonTrigger);
            if(equip){
                room->playSkillEffect(objectName(), qrand() % 2 + 1);
                effect.from->obtainCard(equip);
            }
            else{
                if(!effect.from->hasEquip())
                    return false;
                room->playSkillEffect(objectName(), qrand() % 2 + 3);
                room->obtainCard(player, room->askForCardChosen(player, effect.from, "e", objectName()));
            }
        }
        return false;
    }
};

class HoufaViewAsSkill: public ViewAsSkill{
public:
    HoufaViewAsSkill():ViewAsSkill("houfa"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 2)
            return false;
        return to_select->getCard()->inherits("Slash");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;
        const Card *card1 = cards.first()->getCard();
        const Card *card2 = cards.last()->getCard();
        Card::Suit suit = card1->getColor() == card2->getColor() ?
                          card1->getSuit() : Card::NoSuit;
        Slash *slash = new Slash(suit, 0);
        slash->addSubcards(cards);
        slash->setSkillName(objectName());
        return slash;
    }
};

class Houfa: public TriggerSkill{
public:
    Houfa():TriggerSkill("houfa"){
        events << CardDiscarded;
        //frequency = Frequent;
        view_as_skill = new HoufaViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual int getPriority(TriggerEvent) const{
        return 3;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        QList<ServerPlayer *> se1ang = room->findPlayersBySkillName(objectName());
        foreach(ServerPlayer *selang, se1ang){
            if(selang == player)
                continue;
            CardStar slash = data.value<CardStar>();
            if(slash->isVirtualCard()){
                bool hasslash = false;
                foreach(int card_id, slash->getSubcards()){
                    if(Sanguosha->getCard(card_id)->inherits("Slash")){
                        hasslash = true;
                        break;
                    }
                }
                if(hasslash && selang->askForSkillInvoke(objectName())){
                    room->playSkillEffect(objectName(), qrand() % 2 + 1);
                    foreach(int card_id, slash->getSubcards()){
                        if(Sanguosha->getCard(card_id)->inherits("Slash"))
                            room->obtainCard(selang, card_id);
                    }
                    break;
                }
            }
            else if(slash->inherits("Slash") && selang->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName(), qrand() % 2 + 1);
                selang->obtainCard(slash);
                break;
            }
        }
        return false;
    }

    virtual int getEffectIndex(const ServerPlayer *, const Card *) const{
        return qrand() % 2 + 3;
    }
};

class HoufaSlash: public TriggerSkill{
public:
    HoufaSlash():TriggerSkill("#houfa-slash"){
        events << SlashProceed;
    }

    virtual bool trigger(TriggerEvent , Room* room, ServerPlayer *, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.slash->getSkillName() == "houfa"){
            room->slashResult(effect, NULL);
            return true;
        }
        return false;
    }
};

class HoufaSlash2: public SlashSkill{
public:
    HoufaSlash2():SlashSkill("#houfa-2slash"){
    }

    virtual int getSlashRange(const Player *from, const Player *, const Card *card) const{
        if(from->hasSkill("houfa") && card && card->getSkillName() == "houfa")
            return 998;
        else
            return 0;
    }
};

class Linse: public ClientSkill{
public:
    Linse():ClientSkill("linse"){
    }

    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->inherits("Snatch") || card->inherits("Dismantlement");
    }

    virtual int getExtra(const Player *target) const{
        if(target->hasSkill(objectName()))
            return - target->getMaxHp();
        else
            return 0;
    }
};

class LinseEffect: public PhaseChangeSkill{
public:
    LinseEffect():PhaseChangeSkill("#linse-effect"){
    }

    virtual bool onPhaseChange(ServerPlayer *lz) const{
        Room *room = lz->getRoom();
        if(lz->getPhase() == Player::Discard &&
           lz->getHandcardNum() > lz->getHp() && lz->getHandcardNum() <= lz->getMaxCards()){
            int index = 0;
            if(room->getMode() == "landlord" && lz->isLord())
                index = lz->getGender() == General::Male ? qrand() % 2 + 3 : qrand() % 2 + 5;
            else
                index = qrand() % 2 + 1;
            lz->playSkillEffect("linse", index);
        }
        return false;
    }
};

TigerPackage::TigerPackage()
    :GeneralPackage("tiger")
{/*
    General *leiheng = new General(this, "leiheng", "guan");
    leiheng->addSkill(new Guzong);
*/
    General *sunli = new General(this, "sunli", "guan");
    sunli->addSkill(new Neiying);

    General *wuyanguang = new General(this, "wuyanguang", "guan");
    wuyanguang->addSkill(new Jintang);

    General *shixiu = new General(this, "shixiu", "jiang", 6);
    shixiu->addSkill(new CutHpSkill(2));
    shixiu->addSkill(new Pinming);
    shixiu->addSkill(new PinmingDie);
    related_skills.insertMulti("pinming", "#pinming-die");

    General *lvfang = new General(this, "lvfang", "jiang");
    lvfang->addSkill(new Lieji);

    General *tianhu = new General(this, "tianhu$", "jiang");
    tianhu->addSkill(new Wuzhou);
    tianhu->addSkill(new Huwei);
    skills << new HuweiViewAsSkill;

    General *zhangheng = new General(this, "zhangheng", "min", 3);
    zhangheng->addSkill(new Jielue);
    zhangheng->addSkill(new Fuhun);

    General *xiebao = new General(this, "xiebao", "min");
    xiebao->addSkill(new Liehuo);

    General *shien = new General(this, "shien", "min", 3);
    shien->addSkill(new Longluo);
    shien->addSkill(new Xiaozai);

    General *yanshun = new General(this, "yanshun", "kou");
    yanshun->addSkill(new Huxiao);

    General *wangying = new General(this, "wangying", "kou", 3);
    wangying->addSkill(new Tanse);
    wangying->addSkill(new Houfa);
    wangying->addSkill(new HoufaSlash);
    related_skills.insertMulti("houfa", "#houfa-slash");
    skills << new HoufaSlash2;

    General *lizhong = new General(this, "lizhong", "kou", 4);
    lizhong->addSkill(new CutHpSkill(1));
    lizhong->addSkill(new Linse);
    lizhong->addSkill(new LinseEffect);
    related_skills.insertMulti("linse", "#linse-effect");

    addMetaObject<NeiyingCard>();
    addMetaObject<JintangCard>();
    addMetaObject<LiejiCard>();
    addMetaObject<HuweiCard>();
    addMetaObject<XiaozaiCard>();
}

ADD_PACKAGE(Tiger)
