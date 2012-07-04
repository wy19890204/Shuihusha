#include "standard.h"
#include "skill.h"
#include "chem-cards.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"

class Piaoyong: public PhaseChangeSkill{
public:
    Piaoyong():PhaseChangeSkill("piaoyong"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Start || !player->askForSkillInvoke(objectName()))
            return false;
        Room *room = player->getRoom();
        player->skip(Player::Judge);
        if(room->askForChoice(player, objectName(), "first+second") == "first")
            player->skip(Player::Draw);
        else
            player->skip(Player::Play);
        foreach(const Card *card, player->getJudgingArea())
            room->throwCard(card);
        ServerPlayer *target = room->askForPlayerChosen(player, room->getAllPlayers(), objectName());
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName(objectName());
        CardUseStruct use;
        use.card = slash;
        use.from = player;
        use.to << target;
        room->useCard(use, false);
        return false;
    }
};

class Wuzong: public PhaseChangeSkill{
public:
    Wuzong():PhaseChangeSkill("wuzong"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Start && player->askForSkillInvoke(objectName())){
            player->drawCards(3);
            room->acquireSkill(player, "wusheng");
            room->acquireSkill(player, "paoxiao");
            room->setPlayerFlag(player, "wuzong");
            return false;
        }
        if(player->getPhase() == Player::NotActive && player->hasFlag("wuzong"))
            room->killPlayer(player);
        return false;
    }
};

//fugui
//zizhu

class Qiuhe: public TriggerSkill{
public:
    Qiuhe():TriggerSkill("qiuhe"){
        events << CardEffected;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if(!effect.from || (!effect.card->inherits("Slash") && !effect.card->isNDTrick()))
            return false;
        if(!effect.from->hasFlag("qiuhe") && player->askForSkillInvoke(objectName())){
            effect.from->setFlags("qiuhe");
            effect.from->drawCards(1);
            player->obtainCard(effect.card);
            return true;
        }
        return false;
    }
};

class Duanbing: public TriggerSkill{
public:
    Duanbing():TriggerSkill("duanbing"){
        events << SlashEffect << SlashProceed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(event == SlashEffect){
            if(!player->getWeapon())
                effect.to->addMark("qinggang");
        }
        else{
            if(player->getArmor())
                return false;
            Room *room = player->getRoom();
            QString slasher = player->objectName();

            const Card *first_jink = NULL, *second_jink = NULL;
            first_jink = room->askForCard(effect.to, "jink", "@duanbing-jink-1:" + slasher);
            if(first_jink)
                second_jink = room->askForCard(effect.to, "jink", "@duanbing-jink-2:" + slasher);

            Card *jink = NULL;
            if(first_jink && second_jink){
                jink = new DummyCard;
                jink->addSubcard(first_jink);
                jink->addSubcard(second_jink);
            }
            room->slashResult(effect, jink);
            return true;
        }
        return false;
    }
};

GuiouCard::GuiouCard(){
}

bool GuiouCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void GuiouCard::onEffect(const CardEffectStruct &effect) const{
    effect.from->getRoom()->acquireSkill(effect.to, "#guioupro");
    effect.from->tag["GuiouTarget"] = QVariant::fromValue(effect.to);
}

class GuiouViewAsSkill: public OneCardViewAsSkill{
public:
    GuiouViewAsSkill():OneCardViewAsSkill("guiou"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@guiou";
    }

    virtual bool viewFilter(const CardItem *watch) const{
        return watch->getCard()->isBlack();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        GuiouCard *card = new GuiouCard;
        card->addSubcard(card_item->getCard()->getId());
        return card;
    }
};

class GuiouPro: public ProhibitSkill{
public:
    GuiouPro():ProhibitSkill("#guioupro"){
    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        return card->inherits("TrickCard") && card->isRed();
    }
};

class Guiou: public PhaseChangeSkill{
public:
    Guiou():PhaseChangeSkill("guiou"){
        view_as_skill = new GuiouViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Finish){
            room->askForUseCard(player, "@@guiou", "@guiou");
        }
        else if(player->getPhase() == Player::Start){
            PlayerStar taregt = player->tag["GuiouTarget"].value<PlayerStar>();
            if(taregt)
                room->detachSkillFromPlayer(taregt, "#guioupro");
        }
        return false;
    }
};

class Xiaoguo: public TriggerSkill{
public:
    Xiaoguo():TriggerSkill("xiaoguo"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        DamageStruct damage = data.value<DamageStruct>();
        if(!damage.card->inherits("Slash"))
            return false;
        if(player->askForSkillInvoke(objectName())){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(spade|club):(.*)");
            judge.good = true;
            judge.reason = objectName();
            judge.who = player;

            room->judge(judge);
            if(judge.isGood()){
                if(judge.card->getSuit() == Card::Spade)
                    player->obtainCard(judge.card);
                else
                    room->askForUseCard(player, "slash", "@xiaoguo");
            }
        }
        return false;
    }
};

class Huace:public MasochismSkill{
public:
    Huace():MasochismSkill("huace"){
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &damage) const{
        Room *room = player->getRoom();
        if(damage.damage < 1)
            return;
        for(int i = 0; i < damage.damage; i ++){
            if(player->askForSkillInvoke(objectName())){
                ServerPlayer *target = room->askForPlayerChosen(player, room->getAllPlayers(), objectName());
                target->drawCards(target->getLostHp());
            }
            else
                break;
        }
    }
};

ZhonglianCard::ZhonglianCard(){
    target_fixed = true;
}

void ZhonglianCard::onUse(Room *room, const CardUseStruct &card_use) const{
    Peach *peach = new Peach(Card::NoSuit, 0);
    peach->setSkillName("zhonglian");
    foreach(int x, getSubcards())
        peach->addSubcard(Sanguosha->getCard(x));
    CardUseStruct use;
    use.card = peach;
    use.from = card_use.from;
    use.to << card_use.from->tag["ZhonglianTarget"].value<PlayerStar>();
    room->useCard(use);
}

class ZhonglianViewAsSkill: public ViewAsSkill{
public:
    ZhonglianViewAsSkill():ViewAsSkill("zhonglian"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        return selected.length() < 3;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@zhonglian";
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2 ||
           cards.first()->getCard()->getNumber() != cards.last()->getCard()->getNumber())
            return NULL;
        ZhonglianCard *card = new ZhonglianCard;
        card->addSubcards(cards);
        return card;
    }
};

class Zhonglian: public TriggerSkill{
public:
    Zhonglian():TriggerSkill("zhonglian"){
        events << Dying;
        view_as_skill = new ZhonglianViewAsSkill;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        QList<ServerPlayer *> helpers;
        foreach(ServerPlayer *tmp, room->getAlivePlayers()){
            if(tmp->getCardCount(true) > 1)
                helpers << tmp;
        }
        if(helpers.isEmpty())
            return false;
        foreach(ServerPlayer *helper, helpers){
            helper->tag["ZhonglianTarget"] = QVariant::fromValue(player);
            room->askForUseCard(helper, "@@zhonglian", "@zhonglian:" + player->objectName());
            if(player->getHp() > 0)
                return true;
        }
        return false;
    }
};

MingwangCard::MingwangCard(){
    once = true;
    will_throw = false;
}

void MingwangCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->obtainCard(this);

    Room *room = effect.to->getRoom();
    QList<ServerPlayer *> targets;
    foreach(ServerPlayer *player, room->getOtherPlayers(effect.to)){
        if(player->getHandcardNum() > effect.from->getHandcardNum())
            targets << player;
        if(player->getHp() > effect.from->getHp())
            targets << player;
    }
    if(!targets.isEmpty()){
        ServerPlayer *target = room->askForPlayerChosen(effect.from, targets, "mingwang");
        DamageStruct damage;
        damage.from = effect.to;
        damage.to = target;
        damage.card = this;
        room->damage(damage);
    }
}

class Mingwang: public OneCardViewAsSkill{
public:
    Mingwang():OneCardViewAsSkill("mingwang"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("MingwangCard");
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        MingwangCard *card = new MingwangCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Dancer: public TriggerSkill{
public:
    Dancer():TriggerSkill("dancer"){
        events << SlashProceed;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *lubu, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        Room *room = lubu->getRoom();
        room->playSkillEffect(objectName());

        QString slasher = lubu->objectName();

        const Card *first_jink = NULL, *second_jink = NULL;
        first_jink = room->askForCard(effect.to, "jink", "@dancer-jink-1:" + slasher);
        if(first_jink)
            second_jink = room->askForCard(effect.to, ".", "@dancer-jink-2:" + slasher);

        Card *jink = NULL;
        if(first_jink && second_jink){
            jink = new DummyCard;
            jink->addSubcard(first_jink);
            jink->addSubcard(second_jink);
        }
        room->slashResult(effect, jink);
        return true;
    }
};

class Fuckmoon: public PhaseChangeSkill{
public:
    Fuckmoon():PhaseChangeSkill("fuckmoon"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        //Room *room = player->getRoom();
        if(player->getPhase() == Player::NotActive && player->askForSkillInvoke(objectName())){
            int num = player->getMaxHP() - player->getHandcardNum();
            if(num > 0)
                player->drawCards(num);
        }
        return false;
    }
};

ChemCardsPackage::ChemCardsPackage()
    :Package("chem_cards")
{
    General *guanzhang = new General(this, "guanzhang", "jiang");
    guanzhang->addSkill(new Piaoyong);
    guanzhang->addSkill(new Wuzong);

    General *mizhu = new General(this, "mizhu", "jiang", 3);
    //zhugejin->addSkill(new Fugui);
    //zhugejin->addSkill(new Zizhu);

    General *zhugejin = new General(this, "zhugejin", "min", 3);
    zhugejin->addSkill(new Qiuhe);
    //zhugejin->addSkill(new Kuanhp);

    General *dingfeng = new General(this, "dingfeng", "min");
    dingfeng->addSkill(new Duanbing);

    General *yuejin = new General(this, "yuejin", "guan");
    yuejin->addSkill(new Guiou);
    skills << new GuiouPro;
    yuejin->addSkill(new Xiaoguo);

    General *xunyou = new General(this, "xunyou", "guan", 3);
    xunyou->addSkill(new Huace);

    //General *beimihu = new General(this, "beimihu", "qun", 3, false);

    General *wangyun = new General(this, "wangyun", "kou", 3);
    wangyun->addSkill(new Zhonglian);
    wangyun->addSkill(new Mingwang);
    //wangyun->addSkill(new Lixin);

    General *lvlingqi = new General(this, "lvlingqi", "kou", 3);
    lvlingqi->addSkill(new Dancer);
    lvlingqi->addSkill(new Fuckmoon);

    addMetaObject<GuiouCard>();
    addMetaObject<ZhonglianCard>();
    addMetaObject<MingwangCard>();
}

ADD_PACKAGE(ChemCards);
