#include "xzzd.h"
#include "general.h"
#include "skill.h"
#include "room.h"
#include "carditem.h"
#include "tocheck.h"
#include "clientplayer.h"
#include "engine.h"

DuijueCard::DuijueCard(){
}

bool DuijueCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    if(to_select->hasSkill("fangzhen") && Self->getHp() > to_select->getHp())
        return false;
    return to_select != Self;
}

void DuijueCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(spade):(.*)");
    judge.good = true;
    judge.reason = "duijue";
    judge.who = effect.to;

    room->judge(judge);
    if(judge.isBad()){
        Duel *duel = new Duel(judge.card->getSuit(), judge.card->getNumber());
        duel->setSkillName("duijue");
        duel->setCancelable(false);

        CardUseStruct use;
        use.from = effect.from;
        use.to << effect.to;
        use.card = duel;
        room->useCard(use);
    }
}

class DuijueViewAsSkill:public ZeroCardViewAsSkill{
public:
    DuijueViewAsSkill():ZeroCardViewAsSkill("duijue"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@duijue";
    }

    virtual const Card *viewAs() const{
        return new DuijueCard;
    }
};

class Duijue: public TriggerSkill{
public:
    Duijue():TriggerSkill("duijue"){
        view_as_skill = new DuijueViewAsSkill;
        events << Damage << Damaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.card || !damage.card->inherits("Slash"))
            return false;
        Room *room = player->getRoom();
        room->askForUseCard(player, "@@duijue", "@duijue");
        return false;
    }
};

class Yixian: public TriggerSkill{
public:
    Yixian():TriggerSkill("yixian"){
        events << Predamage;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.to == damage.from || damage.damage < 1)
            return false;
        if(!damage.to->isNude() && player->askForSkillInvoke(objectName(), data)){
            Room *room = player->getRoom();
            room->playSkillEffect(objectName());
            int dust = room->askForCardChosen(player, damage.to, "he", objectName());
            room->throwCard(dust);

            LogMessage log;
            log.type = "$Yixian";
            log.from = player;
            log.to << damage.to;
            log.card_str = QString::number(dust);
            room->sendLog(log);
            player->drawCards(1);
            return true;
        }
        return false;
    }
};

MaidaoCard::MaidaoCard(){
    will_throw = false;
    target_fixed = true;
    mute = true;
}

void MaidaoCard::use(Room *, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    source->getRoom()->playSkillEffect("maidao", qrand() % 2 + 1);
    source->addToPile("knife", this->getSubcards().first());
}

class MaidaoViewAsSkill: public OneCardViewAsSkill{
public:
    MaidaoViewAsSkill():OneCardViewAsSkill("maidao"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Weapon");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        MaidaoCard *card = new MaidaoCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Maidao: public GameStartSkill{
public:
    Maidao():GameStartSkill("maidao"){
        view_as_skill = new MaidaoViewAsSkill;
    }

    virtual void onGameStart(ServerPlayer *yangvi) const{
        Room *room = yangvi->getRoom();
        QList<ServerPlayer *> players = room->getAlivePlayers();
        foreach(ServerPlayer *player, players){
            room->attachSkillToPlayer(player, "maida0");
        }
    }
};

class Fengmang: public PhaseChangeSkill{
public:
    Fengmang():PhaseChangeSkill("fengmang"){

    }

    virtual bool onPhaseChange(ServerPlayer *yang) const{
        Room *room = yang->getRoom();
        if(yang->getPhase() != Player::Start)
            return false;
        while(!yang->getPile("knife").isEmpty()){
            if(!yang->askForSkillInvoke(objectName()))
                return false;
            const QList<int> &knife = yang->getPile("knife");
            int card_id;
            if(knife.length() == 1)
                card_id = knife.first();
            else{
                room->fillAG(knife, yang);
                card_id = room->askForAG(yang, knife, false, objectName());
                yang->invoke("clearAG");
            }

            QList<ServerPlayer *> players;
            foreach(ServerPlayer *tmp, room->getOtherPlayers(yang)){
                if(yang->canSlash(tmp, false))
                    players << tmp;
            }
            if(players.isEmpty())
                return false;
            ServerPlayer *target = room->askForPlayerChosen(yang, players, objectName());

            const Card *card = Sanguosha->getCard(card_id);
            const Weapon *weapon = qobject_cast<const Weapon *>(card);
            yang->tag["Daozi"] = weapon->getRange();
            room->throwCard(card_id);
            Slash *slash = new Slash(Card::NoSuit, 0);
            slash->setSkillName(objectName());
            slash->addSubcard(Sanguosha->getCard(card_id));
            CardUseStruct use;
            use.card = slash;
            use.from = yang;
            use.to << target;
            room->playSkillEffect(objectName());

            room->useCard(use);
        }
        return false;
    }
};

class FengmangBuff: public SlashBuffSkill{
public:
    FengmangBuff():SlashBuffSkill("#fengmangbuff"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        if(effect.slash && effect.slash->getSkillName() != "fengmang")
            return false;
        int weapon_range = effect.from->tag.value("Daozi", 0).toInt();
        int range = effect.from->distanceTo(effect.to);
        Room *room = effect.from->getRoom();
        if(range <= weapon_range){
            room->playSkillEffect(objectName());
            room->slashResult(effect, NULL);
            return true;
        }
        return false;
    }
};

Maida0Card::Maida0Card(){
    will_throw = false;
    mute = true;
}

bool Maida0Card::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty() || to_select == Self)
        return false;
    return to_select->hasSkill("maidao") && !to_select->getPile("knife").isEmpty();
}

void Maida0Card::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *target = targets.first();
    room->playSkillEffect("maidao", qrand() % 2 + 3);
    target->obtainCard(this);

    const QList<int> &knife = target->getPile("knife");
    if(knife.isEmpty())
        return;
    int card_id;
    if(knife.length() == 1)
        card_id = knife.first();
    else{
        room->fillAG(knife, source);
        card_id = room->askForAG(source, knife, false, "maida0");
        source->invoke("clearAG");
    }
    source->obtainCard(Sanguosha->getCard(card_id));
}

class Maida0: public ViewAsSkill{
public:
    Maida0():ViewAsSkill("maida0"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() > 2)
            return false;
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        Maida0Card *card = new Maida0Card();
        card->addSubcards(cards);
        return card;
    }
};

class Shalu: public TriggerSkill{
public:
    Shalu():TriggerSkill("shalu"){
        events << Damage << PhaseChange;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent e, ServerPlayer *likui, QVariant &data) const{
        Room *room = likui->getRoom();
        if(e == PhaseChange){
            if(likui->getPhase() == Player::NotActive)
                room->setPlayerMark(likui, "shalu", 0);
            return false;
        }
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.card || damage.from != likui)
            return false;
        if(damage.card->inherits("Slash")){
            if(likui->getMark("shalu") > 0 && !likui->hasWeapon("crossbow")
                && !likui->hasSkill("paoxiao") && !likui->hasSkill("qinlong")
                && !likui->hasFlag("SlashbySlash"))
                room->setPlayerMark(likui, "shalu", likui->getMark("shalu") - 1);
            if(!room->askForSkillInvoke(likui, objectName(), data))
                return false;
            room->playSkillEffect(objectName(), 1);
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(spade|club):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = likui;

            room->judge(judge);
            if(judge.isGood()){
                room->playSkillEffect(objectName(), 2);
                likui->obtainCard(judge.card);
                room->setPlayerMark(likui, "shalu", likui->getMark("shalu") + 1);
            }
        }
        return false;
    }
};

class Shunshui: public TriggerSkill{
public:
    Shunshui():TriggerSkill("shunshui"){
        events << CardAsked;
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        QString asked = data.toString();
        if(asked == "jink"){
            Room *room = player->getRoom();
            QList<ServerPlayer *> targets;
            foreach(ServerPlayer *tmp, room->getAllPlayers()){
                if(!tmp->getJudgingArea().isEmpty())
                    targets << tmp;
            }
            if(!targets.isEmpty() && room->askForSkillInvoke(player, objectName())){
                ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
                int card_id = room->askForCardChosen(player, target, "j", objectName());
                if(card_id > -1){
                    room->throwCard(card_id);
                    Jink *jink = new Jink(Card::NoSuit, 0);
                    jink->setSkillName(objectName());
                    room->provide(jink);
                    room->setEmotion(player, "good");
                    room->playSkillEffect(objectName());
                    return true;
                }
            }
        }
        return false;
    }
};

class Lihun: public TriggerSkill{
public:
    Lihun():TriggerSkill("lihun"){
        events << Dying;
    }

    static int GetCard(ServerPlayer *from, ServerPlayer *to){
        int first = to->getEquips().isEmpty()?
                    to->getRandomHandCardId():
                    from->getRoom()->askForCardChosen(from, to, "he", "lihun");
        from->obtainCard(Sanguosha->getCard(first));
        return first;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *shun, QVariant &data) const{
        DyingStruct dying = data.value<DyingStruct>();
        DamageStruct *damage = dying.damage;
        if(damage->from && !damage->from->isNude() && shun->askForSkillInvoke(objectName())){
            Room *room = shun->getRoom();
            room->playSkillEffect(objectName());
            DummyCard *dummy = new DummyCard;
            //int first = room->askForCardChosen(shun, damage->from, "he", objectName());
            //shun->obtainCard(Sanguosha->getCard(first));
            dummy->addSubcard(GetCard(shun, damage->from));
            if(!damage->from->isNude() && shun->askForSkillInvoke(objectName())){
                //first = room->askForCardChosen(shun, damage->from, "he", objectName());
                //shun->obtainCard(Sanguosha->getCard(first));
                dummy->addSubcard(GetCard(shun, damage->from));
            }
            ServerPlayer *target = room->askForPlayerChosen(shun, room->getOtherPlayers(damage->from), objectName());
            room->moveCardTo(dummy, target, Player::Hand);
            delete dummy;
        }
        return false;
    }
};

class Fenhui: public TriggerSkill{
public:
    Fenhui():TriggerSkill("fenhui"){
        frequency = Compulsory;
        events << Predamage << Predamaged;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        LogMessage log;
        log.from = player;
        log.arg2 = objectName();

        if(event == Predamage){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature != DamageStruct::Fire){
                Room *room = player->getRoom();
                damage.nature = DamageStruct::Fire;

                log.type = "#FenhuiFire";
                log.arg = QString::number(damage.damage);
                room->sendLog(log);
                room->playSkillEffect(objectName(), qrand() % 2 + 1);

                data = QVariant::fromValue(damage);
                return false;
            }
       }else if(event == Predamaged){
           DamageStruct damage = data.value<DamageStruct>();
           if(damage.nature == DamageStruct::Fire){
               Room *room = player->getRoom();
               log.type = "#FenhuiProtect";
               log.arg = QString::number(damage.damage);
               room->sendLog(log);

               room->playSkillEffect(objectName(), qrand() % 2 + 3);
               return true;
           }else
               return false;
       }
       return false;
    }
};

class ShenhuoViewAsSkill: public OneCardViewAsSkill{
public:
    ShenhuoViewAsSkill():OneCardViewAsSkill("shenhuo"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getFilteredCard();
        return card->isRed() && card->inherits("TrickCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        FireAttack *fire_attack = new FireAttack(card->getSuit(), card->getNumber());
        fire_attack->addSubcard(card->getId());
        fire_attack->setSkillName(objectName());
        return fire_attack;
    }
};

class Shenhuo:public TriggerSkill{
public:
    Shenhuo():TriggerSkill("shenhuo"){
        view_as_skill = new ShenhuoViewAsSkill;
        events << CardUsed;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *weidingguo, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        CardStar card = use.card;
        if(card->inherits("FireAttack")){
            Room *room = weidingguo->getRoom();
            if(room->askForSkillInvoke(weidingguo, objectName())){
                room->playSkillEffect(objectName());
                weidingguo->drawCards(2);
            }
        }
        return false;
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
                ServerPlayer *target = room->askForPlayerChosen(hx, room->getAllPlayers(), objectName());
                if(!target)
                    target = hx;
                //room->takeAG(target, card_id);
                const Card *card = Sanguosha->getCard(card_id);
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

class Shenpan: public TriggerSkill{
public:
    Shenpan():TriggerSkill("shenpan"){
        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return TriggerSkill::triggerable(target);
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();

        if(player->askForSkillInvoke(objectName(), data)){
            player->obtainCard(judge->card);
            room->playSkillEffect(objectName());
            int card_id = room->drawCard();
            room->moveCardTo(Sanguosha->getCard(card_id), NULL, Player::Special, true);
            room->getThread()->delay();

            judge->card = Sanguosha->getCard(card_id);
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = player;
            log.to << judge->who;
            log.card_str = QString::number(card_id);
            room->sendLog(log);

            room->sendJudgeResult(judge);
            return true;
        }
        return false;
    }
};

BinggongCard::BinggongCard(){
    will_throw = false;
}

bool BinggongCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;
    return to_select != Self;
}

void BinggongCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    int num = this->getSubcards().length();
    ServerPlayer *target = targets.first();
    target->obtainCard(this);
    if(num >= 3){
        RecoverStruct rev;
        rev.who = source;
        room->recover(source, rev);
    }
}

class BinggongViewAsSkill: public ViewAsSkill{
public:
    BinggongViewAsSkill():ViewAsSkill("binggong"){

    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        int num = Self->getMark("Bingo");
        return !to_select->isEquipped() && selected.length() < num;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != Self->getMark("Bingo"))
            return NULL;

        BinggongCard *card = new BinggongCard();
        card->addSubcards(cards);
        return card;
    }
};

class Binggong: public PhaseChangeSkill{
public:
    Binggong():PhaseChangeSkill("binggong"){
        view_as_skill = new BinggongViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *pei) const{
        Room *room = pei->getRoom();
        if(pei->getPhase() == Player::Start && pei->getHandcardNum() > pei->getHp() &&
           pei->askForSkillInvoke(objectName())){
            int num = pei->getHandcardNum() - pei->getHp();
            room->setPlayerMark(pei, "Bingo", num);
            room->askForUseCard(pei, "@@binggong", "@binggong");
        }
        return false;
    }
};

class Linse: public ProhibitSkill{
public:
    Linse():ProhibitSkill("linse"){
    }

    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->inherits("Snatch") || card->inherits("Dismantlement");
    }
};

class FeiQPattern: public CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const{
        return card->objectName() == "jink";
    }
};

FeiqiangCard::FeiqiangCard(){
    once = true;
    mute = true;
}

bool FeiqiangCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void FeiqiangCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if(!room->askForCard(effect.to, ".FeiQ", "@feiqiang:" + effect.from->objectName())){
        QString choice = effect.to->getCards("e").isEmpty() ? "gong"
            : room->askForChoice(effect.from, "feiqiang", "gong+wang");
        if(choice == "gong"){
            room->playSkillEffect("feiqiang", 1);
            room->loseHp(effect.to);
        }else{
            room->playSkillEffect("feiqiang", 2);
            effect.to->throwAllEquips();
        }
    }
}

class Feiqiang:public OneCardViewAsSkill{
public:
    Feiqiang():OneCardViewAsSkill("feiqiang"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("FeiqiangCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Weapon");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new FeiqiangCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Feiyan: public ProhibitSkill{
public:
    Feiyan():ProhibitSkill("feiyan"){
    }

    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->inherits("Snatch") || card->inherits("SupplyShortage");
    }
};

class Shentou: public OneCardViewAsSkill{
public:
    Shentou():OneCardViewAsSkill("shentou"){
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

XZDDPackage::XZDDPackage()
    :Package("XZDD"){ //guan == wei, jiang == shu, min == wu, kou == qun

    General *linchong = new General(this, "linchong", "jiang");
    linchong->addSkill(new Duijue);

    General *zhutong = new General(this, "zhutong", "min");
    zhutong->addSkill(new Yixian);

    General *yangzhi = new General(this, "yangzhi", "guan");
    yangzhi->addSkill(new Maidao);
    skills << new Maida0;
    yangzhi->addSkill(new Fengmang);
    yangzhi->addSkill(new FengmangBuff);
    related_skills.insertMulti("fengmang", "#fengmangbuff");

    General *likui = new General(this, "likui", "jiang");
    likui->addSkill(new Shalu);

    General *zhangshun = new General(this, "zhangshun", "kou", 3);
    zhangshun->addSkill(new Shunshui);
    zhangshun->addSkill(new Lihun);

    General *weidingguo = new General(this, "weidingguo", "jiang", 3);
    weidingguo->addSkill(new Fenhui);
    weidingguo->addSkill(new Shenhuo);

    General *huangxin = new General(this, "huangxin", "jiang");
    huangxin->addSkill(new Tongxia);

    General *yanshun = new General(this, "yanshun", "jiang");
    yanshun->addSkill(new Huxiao);

    General *peixuan = new General(this, "peixuan", "guan", 3);
    peixuan->addSkill(new Shenpan);
    peixuan->addSkill(new Binggong);

    General *lizhong = new General(this, "lizhong", "kou", 4);
    lizhong->addSkill("#losthp");
    lizhong->addSkill(new Linse);

    General *gongwang = new General(this, "gongwang", "jiang");
    gongwang->addSkill(new Feiqiang);
    patterns[".FeiQ"] = new FeiQPattern;

    General *shiqian = new General(this, "shiqian", "kou", 3);
    shiqian->addSkill(new Feiyan);
    shiqian->addSkill(new Shentou);

    addMetaObject<DuijueCard>();
    addMetaObject<MaidaoCard>();
    addMetaObject<Maida0Card>();
    addMetaObject<BinggongCard>();
    addMetaObject<FeiqiangCard>();
}

ADD_PACKAGE(XZDD)
