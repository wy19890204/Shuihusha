#include "god.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "settings.h"
#include "tocheck.h"

class Jinqiang:public TriggerSkill{
public:
    Jinqiang():TriggerSkill("jinqiang"){
        events << Predamage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *xuning, QVariant &data) const{

        DamageStruct damage = data.value<DamageStruct>();
        if((damage.to->isChained()) &&(damage.nature == DamageStruct::Normal)){
            damage.damage = damage.damage + 1;
            data = QVariant::fromValue(damage);
            return false;
        }
        return false;
    }
};

class Jiebei: public TriggerSkill{
public:
    Jiebei():TriggerSkill("jiebei"){
        events << CardLost << FinishJudge;

        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->getRoom()->findPlayerBySkillName(objectName());
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *xuning = room->findPlayerBySkillName(objectName());
        if(event == CardLost){
            CardMoveStar move = data.value<CardMoveStar>();
            const Card *armor = Sanguosha->getCard(move->card_id);
            if(!armor->inherits("Armor"))
                return false;
            if(move->from_place == Player::Equip && move->from == xuning){
                if(room->askForSkillInvoke(xuning, objectName())){
                    room->playSkillEffect(objectName());
                    xuning->drawCards(2);
                }
            }else if(room->getCardPlace(move->card_id) == Player::DiscardedPile){
                if(room->askForDiscard(xuning, objectName(), 1, false, false)){
                    xuning->obtainCard(armor);
                    QList<ServerPlayer *> tos;
                    foreach(ServerPlayer *p, room->getAllPlayers()){
                        if(!p->getArmor())
                            tos << p;
                    }
                    tos.removeOne(move->from);
                    ServerPlayer *to = room->askForPlayerChosen(xuning, tos, objectName());
                    room->moveCardTo(armor, to, Player::Equip, true);
                }
                return false;
            }
            return false;
        }else if(event == FinishJudge){
            JudgeStar judge = data.value<JudgeStar>();
            if(room->getCardPlace(judge->card->getEffectiveId()) == Player::DiscardedPile && judge->card->inherits("Armor")){
                if(room->askForDiscard(xuning, objectName(), 1, false, false)){
                    xuning->obtainCard(judge->card);
                    QList<ServerPlayer *> tos;
                    foreach(ServerPlayer *p, room->getAllPlayers()){
                        if(!p->getArmor())
                            tos << p;
                    }
                    //tos.removeOne(move->from);
                    ServerPlayer *to = room->askForPlayerChosen(xuning, tos, objectName());
                    room->moveCardTo(judge->card, to, Player::Equip, true);

                }
                return false;
            }
        }

        return false;
    }
};

class Shenchou:public TriggerSkill{
public:
    Shenchou():TriggerSkill("shenchou"){
        events << Damage;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *wusong, QVariant &data) const{
        Room *room = wusong->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        room->playSkillEffect(objectName());
        if(damage.card && wusong->getMark("shenchou") == 0){
            if(wusong->getPile("chou_pile").isEmpty())
                wusong->addToPile("chou_pile", damage.card->getEffectiveId(), true);
            else{
                bool getit = true;
                foreach(int cdid, wusong->getPile("chou_pile")){
                    if(damage.card->getEffectiveId() == cdid)
                        getit = false;
                }
                if(getit)
                    wusong->addToPile("chou_pile", damage.card->getEffectiveId(), true);
                else
                    return false;
            }
        }
        else
            return false;
        return false;
    }
};

class Wujie: public PhaseChangeSkill{
public:
    Wujie():PhaseChangeSkill("wujie"){
        frequency = Wake;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return PhaseChangeSkill::triggerable(target)
                && target->getPhase() == Player::Start
                && target->getMark("wujie") == 0
                && target->getPile("chou_pile").length() >= 3;
    }

    virtual bool onPhaseChange(ServerPlayer *wusong) const{
        Room *room = wusong->getRoom();

        room->setPlayerMark(wusong, "wujie", 1);
        room->setPlayerProperty(wusong, "maxhp", QVariant(wusong->getMaxHP() + 1));

        room->playSkillEffect(objectName());

        room->acquireSkill(wusong, "zhusha");

        return false;
    }
};

ZhushaCard::ZhushaCard(){
    target_fixed = true;
}

void ZhushaCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *wusong = source;
    QList<int> chous = wusong->getPile("chou_pile");
    if(chous.isEmpty())
        return ;

    int card_id;
    if(chous.length() == 1)
        card_id = chous.first();
    else{
        room->fillAG(chous, wusong);
        card_id = room->askForAG(wusong, chous, true, "zhusha");
        wusong->invoke("clearAG");

        if(card_id == -1)
            return;
    }

    const Card *card = Sanguosha->getCard(card_id);
    room->moveCardTo(card, NULL, Player::DiscardedPile, true);
    room->loseMaxHp(wusong, 1);
    wusong->setFlags("zhusha_effect");
}

class ZhushaDiscard: public ZeroCardViewAsSkill{
public:
    ZhushaDiscard():ZeroCardViewAsSkill("zhusha"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ZhushaCard") && player->getPile("chou_pile").length() != 0;
    }

    virtual const Card *viewAs() const{
        return new ZhushaCard;
    }
};

class Zhusha:public TriggerSkill{
public:
    Zhusha():TriggerSkill("zhusha"){
        events << CardUsed;
        view_as_skill = new ZhushaDiscard;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *wusong, QVariant &data) const{
        Room *room = wusong->getRoom();
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Slash") && wusong->hasFlag("zhusha_effect")){
            room->playSkillEffect(objectName());
            wusong->setFlags("-zhusha_effect");
            room->throwCard(use.card);
            foreach(ServerPlayer *p, room->getOtherPlayers(wusong)){
                room->cardEffect(use.card, wusong, p);
            }
            return true;
        }
        return false;
    }
};

DuanbiCard::DuanbiCard(){

}

bool DuanbiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void DuanbiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    ServerPlayer *target = targets.first();
    room->setPlayerProperty(source, "maxhp", 1);
    DamageStruct damage;
    damage.card = NULL;
    damage.from = source;
    damage.to = target;
    damage.damage = 2;
    room->damage(damage);
    room->detachSkillFromPlayer(source, "shenchou");
    source->addMark("shenchou");
    source->loseMark("@duanbi");
}

class Duanbi: public ZeroCardViewAsSkill{
public:
    Duanbi():ZeroCardViewAsSkill("duanbi"){
    }

    virtual const Card *viewAs() const{
        return new DuanbiCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@duanbi") == 1
                && player->getHp() == 1
                && player->getMaxHP() > 1;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  false;
    }
};

class XianjiClear: public PhaseChangeSkill{
public:
    XianjiClear():PhaseChangeSkill("#xianji-clear"){

    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::NotActive){
            Room *room = target->getRoom();
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer *player, players){
                if(player->hasFlag("jilei")){
                    player->jilei(".");
                    player->invoke("jilei");

                    LogMessage log;
                    log.type = "#XianjiClear";
                    log.from = player;
                    room->sendLog(log);
                }
            }
        }

        return false;
    }
};

class Xianji: public PhaseChangeSkill{
public:
    Xianji():PhaseChangeSkill("xianji"){
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        PlayerStar target = player;
        if(target->getPhase() != Player::Start)
            return false;
        Room *room = target->getRoom();
        ServerPlayer *waste = room->findPlayerBySkillName(objectName());
        const Card *card = NULL;
        if(waste && !waste->isNude())
            card = room->askForCard(waste, "..", "@xianji", QVariant::fromValue(target));
        if(!waste || !card)
           return false;
        QString choice = card->getType();
        room->playSkillEffect(objectName());

        target->jilei(choice);
        target->invoke("jilei", choice);
        target->setFlags("jilei");

        LogMessage log;
        log.type = "#Xianji";
        log.from = waste;
        log.to << target;
        log.arg2 = objectName();
        log.arg = choice;
        room->sendLog(log);

        return false;
    }
};

class Houlue: public TriggerSkill{
public:
    Houlue():TriggerSkill("houlue"){
        events << CardLost;
        frequency = Frequent;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *lese = room->findPlayerBySkillName(objectName());
        if(!lese)
            return false;
        CardMoveStar move = data.value<CardMoveStar>();
        if(move->to_place == Player::DiscardedPile){
            const Card *card = Sanguosha->getCard(move->card_id);
            if(card->isNDTrick() && lese->askForSkillInvoke(objectName())){
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(.*):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = lese;

                room->judge(judge);
                if(!judge.card->inherits("TrickCard")){
                    lese->obtainCard(card);
                    room->playSkillEffect(objectName(), 1);
                }
                else{
                    lese->obtainCard(judge.card);
                    room->playSkillEffect(objectName(), 2);
                }
            }
        }
        return false;
    }
};

FeihuangCard::FeihuangCard(){
    target_fixed = true;
    will_throw = false;
}

void FeihuangCard::use(Room *, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    foreach(int card_id, this->getSubcards())
        source->addToPile("stone", card_id);
}

class FeihuangViewAsSkill: public ViewAsSkill{
public:
    FeihuangViewAsSkill(): ViewAsSkill("feihuang"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        int n = Self->getHandcardNum() - Self->getHp();
        if(selected.length() >= n)
            return false;

        if(to_select->isEquipped())
            return false;

        return true;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != Self->getHandcardNum() - Self->getHp())
            return NULL;

        FeihuangCard *card = new FeihuangCard;
        card->addSubcards(cards);
        return card;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@feihuang";
    }
};

class Feihuang: public PhaseChangeSkill{
public:
    Feihuang():PhaseChangeSkill("feihuang"){
        view_as_skill = new FeihuangViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *vq) const{
        if(vq->getPhase() != Player::Discard)
            return false;
        Room *room = vq->getRoom();
        if(vq->getHandcardNum() > vq->getHp() && room->askForUseCard(vq, "@@feihuang", "@feihuang"))
            return true;
        return false;
    }
};

MeiyuCard::MeiyuCard(){
}

bool MeiyuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->canSlash(to_select, false);
}

void MeiyuCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->throwCard(effect.from->getPile("stone").last());
    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("meiyu");
    CardUseStruct use;
    use.card = slash;
    use.from = effect.from;
    use.to << effect.to;
    room->useCard(use, false);
}

class Meiyu: public ZeroCardViewAsSkill{
public:
    Meiyu():ZeroCardViewAsSkill("meiyu"){
    }

    virtual const Card *viewAs() const{
        return new MeiyuCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->getPile("stone").isEmpty();
    }
};

class HuafoSlash: public OneCardViewAsSkill{
public:
    HuafoSlash():OneCardViewAsSkill("hua1fo"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("BasicCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        Slash *s = new Slash(card->getSuit(), card->getNumber());
        s->addSubcard(card);
        s->setSkillName(objectName());
        return s;
    }
};

class HuafoAnale: public OneCardViewAsSkill{
public:
    HuafoAnale():OneCardViewAsSkill("hua2fo"){
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("BasicCard");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getFilteredCard();
        Analeptic *a = new Analeptic(card->getSuit(), card->getNumber());
        a->addSubcard(card);
        a->setSkillName(objectName());
        return a;
    }
};

class Dunwu: public TriggerSkill{
public:
    Dunwu():TriggerSkill("dunwu"){
        events << Damage;
        frequency = Compulsory;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        Room *room = player->getRoom();
        if(damage.damage > 0){
            LogMessage log;
            log.type = "#Dunwu";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = objectName();
            room->sendLog(log);

            room->playSkillEffect(objectName());
        }
        for(int i = 0; i < damage.damage; i++){
            int card_id = room->drawCard();
            const Card *card = Sanguosha->getCard(card_id);
            room->moveCardTo(card, NULL, Player::Special, true);
            room->getThread()->delay();

            if(card->inherits("Tsunami"))
                room->killPlayer(player);
            else
                player->addToPile("jiyan", card_id);

            if(player->getPile("jiyan").length() >= 4){
                LogMessage log;
                log.type = "#DunwuB";
                log.from = player;
                log.arg = "jiyan";
                log.arg2 = QString::number(4);
                room->sendLog(log);

                foreach(int i, player->getPile("jiyan"))
                    room->throwCard(i);
                room->loseMaxHp(player);
            }
        }
        return false;
    }
};

GodPackage::GodPackage()
    :Package("god")
{
    General *shenwusong = new General(this, "shenwusong", "god", 5);
    shenwusong->addSkill(new Shenchou);
    shenwusong->addSkill(new Wujie);
    skills << new Zhusha;
    shenwusong->addSkill(new Duanbi);
    shenwusong->addSkill(new MarkAssignSkill("@bi", 1));
    related_skills.insertMulti("duanbi", "#@bi-1");

    General *shenwuyong = new General(this, "shenwuyong", "god", 3);
    shenwuyong->addSkill(new Xianji);
    shenwuyong->addSkill(new XianjiClear);
    shenwuyong->addSkill(new Houlue);
    related_skills.insertMulti("xianji", "#xianji-clear");

    General *shenzhangqing = new General(this, "shenzhangqing", "god", 4);
    shenzhangqing->addSkill(new Feihuang);
    shenzhangqing->addSkill(new Meiyu);

    General *shenluzhishen = new General(this, "shenluzhishen", "god", 5);
    shenluzhishen->addSkill(new Dunwu);
    shenluzhishen->addSkill(new HuafoSlash);
    shenluzhishen->addSkill(new HuafoAnale);

    General *shenxuning = new General(this, "shenxuning", "god");
    shenxuning->addSkill(new Jiebei);
    shenxuning->addSkill(new Jinqiang);

    addMetaObject<ZhushaCard>();
    addMetaObject<DuanbiCard>();
    addMetaObject<FeihuangCard>();
    addMetaObject<MeiyuCard>();
}

ADD_PACKAGE(God)
