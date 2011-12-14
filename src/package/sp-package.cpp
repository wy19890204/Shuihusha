#include "standard.h"
#include "skill.h"
#include "sp-package.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"

// skill cards

GuidaoCard::GuidaoCard(){
    target_fixed = true;
    will_throw = false;
}

void GuidaoCard::use(Room *room, ServerPlayer *zhangjiao, const QList<ServerPlayer *> &targets) const{

}

LeijiCard::LeijiCard(){

}

bool LeijiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty();
}

void LeijiCard::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *zhangjiao = effect.from;
    ServerPlayer *target = effect.to;

    Room *room = zhangjiao->getRoom();
    room->setEmotion(target, "bad");

    JudgeStruct judge;
    judge.pattern = QRegExp("(.*):(.*):(.*)");
    judge.good = false;
    judge.reason = "leiji";
    judge.who = target;

    room->judge(judge);

    if(judge.isBad()){
        DamageStruct damage;
        damage.card = NULL;
        damage.damage = 2;
        damage.from = zhangjiao;
        damage.to = target;
        damage.nature = DamageStruct::Thunder;

        room->damage(damage);
    }else
        room->setEmotion(zhangjiao, "bad");
}

HuangtianCard::HuangtianCard(){
    once = true;
}

void HuangtianCard::use(Room *room, ServerPlayer *, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *zhangjiao = targets.first();
    if(zhangjiao->hasSkill("huangtian")){
        zhangjiao->obtainCard(this);
        room->setEmotion(zhangjiao, "good");
    }
}

bool HuangtianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->hasLordSkill("huangtian") && to_select != Self;
}

class GuidaoViewAsSkill:public OneCardViewAsSkill{
public:
    GuidaoViewAsSkill():OneCardViewAsSkill(""){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@guidao";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isBlack();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        GuidaoCard *card = new GuidaoCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Guidao: public TriggerSkill{
public:
    Guidao():TriggerSkill("guidao"){
        view_as_skill = new GuidaoViewAsSkill;

        events << AskForRetrial;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        if(!TriggerSkill::triggerable(target))
            return false;

        if(target->isKongcheng()){
            bool has_black = false;
            int i;
            for(i=0; i<4; i++){
                const EquipCard *equip = target->getEquip(i);
                if(equip && equip->isBlack()){
                    has_black = true;
                    break;
                }
            }

            return has_black;
        }else
            return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        JudgeStar judge = data.value<JudgeStar>();

        QStringList prompt_list;
        prompt_list << "@guidao-card" << judge->who->objectName()
                << "" << judge->reason << judge->card->getEffectIdString();
        QString prompt = prompt_list.join(":");

        player->tag["Judge"] = data;
        const Card *card = room->askForCard(player, "@guidao", prompt, data);

        if(card){
            // the only difference for Guicai & Guidao
            player->obtainCard(judge->card);

            judge->card = Sanguosha->getCard(card->getEffectiveId());
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$ChangedJudge";
            log.from = player;
            log.to << judge->who;
            log.card_str = card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);
        }

        return false;
    }
};

class HuangtianViewAsSkill: public OneCardViewAsSkill{
public:
    HuangtianViewAsSkill():OneCardViewAsSkill("huangtianv"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("HuangtianCard") && player->getKingdom() == "kou";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        const Card *card = to_select->getCard();
        return card->objectName() == "jink" || card->objectName() == "lightning";
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        HuangtianCard *card = new HuangtianCard;
        card->addSubcard(card_item->getFilteredCard());

        return card;
    }
};

class Huangtian: public GameStartSkill{
public:
    Huangtian():GameStartSkill("huangtian$"){

    }

    virtual void onGameStart(ServerPlayer *zhangjiao) const{
        Room *room = zhangjiao->getRoom();
        QList<ServerPlayer *> players = room->getAlivePlayers();
        foreach(ServerPlayer *player, players){
            room->attachSkillToPlayer(player, "huangtianv");
        }
    }
};

class LeijiViewAsSkill: public ZeroCardViewAsSkill{
public:
    LeijiViewAsSkill():ZeroCardViewAsSkill("leiji"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@leiji";
    }

    virtual const Card *viewAs() const{
        return new LeijiCard;
    }
};

class Leiji: public TriggerSkill{
public:
    Leiji():TriggerSkill("leiji"){
        events << CardResponsed;
        view_as_skill = new LeijiViewAsSkill;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zhangjiao, QVariant &data) const{
        CardStar card_star = data.value<CardStar>();
        if(!card_star->inherits("Jink"))
            return false;

        Room *room = zhangjiao->getRoom();
        room->askForUseCard(zhangjiao, "@@leiji", "@leiji");

        return false;
    }
};

ShensuCard::ShensuCard(){
    mute = true;
}

bool ShensuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(!targets.isEmpty())
        return false;

    if(to_select->hasSkill("kongcheng") && to_select->isKongcheng())
        return false;

    if(to_select == Self)
        return false;

    return true;
}

void ShensuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);

    Slash *slash = new Slash(Card::NoSuit, 0);
    slash->setSkillName("shensu");
    CardUseStruct use;
    use.card = slash;
    use.from = source;
    use.to = targets;

    room->useCard(use);
}

class ShensuViewAsSkill: public ViewAsSkill{
public:
    ShensuViewAsSkill():ViewAsSkill("shensu"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(ClientInstance->getPattern().endsWith("1"))
            return false;
        else
            return selected.isEmpty() && to_select->getCard()->inherits("EquipCard");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern.startsWith("@@shensu");
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(ClientInstance->getPattern().endsWith("1")){
            if(cards.isEmpty())
                return new ShensuCard;
            else
                return NULL;
        }else{
            if(cards.length() != 1)
                return NULL;

            ShensuCard *card = new ShensuCard;
            card->addSubcards(cards);

            return card;
        }
    }
};

class Shensu: public PhaseChangeSkill{
public:
    Shensu():PhaseChangeSkill("shensu"){
        view_as_skill = new ShensuViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *xiahouyuan) const{
        Room *room = xiahouyuan->getRoom();

        if(xiahouyuan->getPhase() == Player::Judge){
            if(room->askForUseCard(xiahouyuan, "@@shensu1", "@shensu1")){
                xiahouyuan->skip(Player::Draw);
                return true;
            }
        }else if(xiahouyuan->getPhase() == Player::Play){
            if(room->askForUseCard(xiahouyuan, "@@shensu2", "@shensu2")){
                return true;
            }
        }

        return false;
    }
};

class Jushou: public PhaseChangeSkill{
public:
    Jushou():PhaseChangeSkill("jushou"){

    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish){
            Room *room = target->getRoom();
            if(room->askForSkillInvoke(target, objectName())){
                target->drawCards(15);
                target->turnOver();

                room->playSkillEffect(objectName());
            }
        }

        return false;
    }
};

class Liegong: public SlashBuffSkill{
public:
    Liegong():SlashBuffSkill("liegong"){

    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *huangzhong = effect.from;
        Room *room = huangzhong->getRoom();
        if(huangzhong->getPhase() != Player::Play)
            return false;

        int num = effect.to->getHandcardNum();
        if(num >= huangzhong->getHp() || num <= huangzhong->getAttackRange()){
            if(huangzhong->askForSkillInvoke(objectName(), QVariant::fromValue(effect))){
                room->playSkillEffect(objectName());
                room->slashResult(effect, NULL);

                return true;
            }
        }

        return false;
    }
};

class Kuanggu: public TriggerSkill{
public:
    Kuanggu():TriggerSkill("kuanggu"){
        frequency = Compulsory;
        events << Damage << DamageDone;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        if(event == DamageDone && damage.from && damage.from->hasSkill("kuanggu") && damage.from->isAlive()){
            ServerPlayer *weiyan = damage.from;
            weiyan->tag["InvokeKuanggu"] = weiyan->distanceTo(damage.to) <= 1;
        }else if(event == Damage && player->hasSkill("kuanggu") && player->isAlive()){
            bool invoke = player->tag.value("InvokeKuanggu", false).toBool();
            if(invoke){
                Room *room = player->getRoom();

                room->playSkillEffect(objectName());

                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);

                RecoverStruct recover;
                recover.who = player;
                recover.recover = damage.damage;
                room->recover(player, recover);
            }
        }

        return false;
    }
};
SPPackage::SPPackage()
    :Package("sp")
{/*
    General *yangxiu = new General(this, "yangxiu", "wei", 3);
    yangxiu->addSkill(new Jilei);
    yangxiu->addSkill(new JileiClear);
    yangxiu->addSkill(new Danlao);

    related_skills.insertMulti("jilei", "#jilei-clear");

    General *gongsunzan = new General(this, "gongsunzan", "qun");
    gongsunzan->addSkill(new Yicong);

    General *yuanshu = new General(this, "yuanshu", "qun");
    yuanshu->addSkill(new Yongsi);
    yuanshu->addSkill(new Weidi);

    General *sp_diaochan = new General(this, "sp_diaochan", "qun", 3, false, true);
    sp_diaochan->addSkill("lijian");
    sp_diaochan->addSkill("biyue");
    sp_diaochan->addSkill(new Xuwei);

    General *sp_sunshangxiang = new General(this, "sp_sunshangxiang", "shu", 3, false, true);
    sp_sunshangxiang->addSkill("jieyin");
    sp_sunshangxiang->addSkill("xiaoji");

    General *shenlvbu1 = new General(this, "shenlvbu1", "god", 8, true, true);
    shenlvbu1->addSkill("mashu");
    shenlvbu1->addSkill("wushuang");

    General *shenlvbu2 = new General(this, "shenlvbu2", "god", 4, true, true);
    shenlvbu2->addSkill("mashu");
    shenlvbu2->addSkill("wushuang");
    shenlvbu2->addSkill(new Xiuluo);
    shenlvbu2->addSkill(new Shenwei);
    shenlvbu2->addSkill(new Skill("shenji"));

    General *sp_guanyu = new General(this, "sp_guanyu", "wei", 4);
    sp_guanyu->addSkill("wusheng");
    sp_guanyu->addSkill(new Danji);

    General *sp_caiwenji = new General(this, "sp_caiwenji", "wei", 3, false, true);
    sp_caiwenji->addSkill("beige");
    sp_caiwenji->addSkill("duanchang");

    General *sp_machao = new General(this, "sp_machao", "qun", 4, true, true);
    sp_machao->addSkill("mashu");
    sp_machao->addSkill("tieji");

    General *sp_jiaxu = new General(this, "sp_jiaxu", "wei", 3, true, true);
    sp_jiaxu->addSkill("wansha");
    sp_jiaxu->addSkill("luanwu");
    sp_jiaxu->addSkill("weimu");
    sp_jiaxu->addSkill("#@chaos-1");*/
}

ADD_PACKAGE(SP);
