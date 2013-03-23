#include "impasse_fight.h"
#include <QTime>

class Silue: public PhaseChangeSkill{
public:
    Silue():PhaseChangeSkill("silue"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Draw)
            return false;
        Room *room = player->getRoom();
        QList<ServerPlayer *> players = room->getOtherPlayers(player);
        bool has_frantic = player->hasMark("@frantic");

        if(has_frantic){
            foreach(ServerPlayer *target, players){
                if(target->getCards("he").length() == 0)
                    continue;
                int card_id = room->askForCardChosen(player, target, "he", objectName());
                room->obtainCard(player, card_id, room->getCardPlace(card_id) != Player::Hand);
            }
            room->playSkillEffect(objectName(), 1);
            return true;
        }
        else{
            player->drawCards(player->getHp());
            room->playSkillEffect(objectName(), 2);
            return true;
        }
        return false;
    }
};

class Kedi: public MasochismSkill{
public:
    Kedi():MasochismSkill("kedi"){
        frequency = Frequent;
    }

    virtual void onDamaged(ServerPlayer *player, const DamageStruct &) const{
        Room *room = player->getRoom();
        bool has_frantic = player->hasMark("@frantic");

        if(!room->askForSkillInvoke(player, objectName()))
            return;

        room->playSkillEffect(objectName(), has_frantic ? 1 : 2);
        QList<ServerPlayer *> players = room->getAlivePlayers();
        int n = 0;
        if(has_frantic)
            n = players.length();
        else
            n = (player->getHp());
        player->drawCards(n);
    }
};

class YuanyinEx: public ClientSkill{
public:
    YuanyinEx():ClientSkill("#yuanyin-extra"){
    }

    virtual int getExtra(const Player *target) const{
        if(!target->hasSkill("yuanyin"))
            return 0;
        else{
            int extra = 1;
            foreach(const Player *player, target->getSiblings()){
                if(player->isAlive())
                    extra ++;
            }
            return - extra;
        }
    }
};

class Yuanyin: public PhaseChangeSkill{
public:
    Yuanyin():PhaseChangeSkill("yuanyin"){
        frequency = Compulsory;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return target->isLord();
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        Room *room = target->getRoom();
        QList<ServerPlayer *> players = room->getAlivePlayers();
        QList<ServerPlayer *> others = room->getOtherPlayers(target);
        bool has_frantic = target->hasMark("@frantic");

        if(target->getPhase() == Player::Start){
            bool invoke_skill = false;
            if(has_frantic){
                if(target->getHandcardNum()<=(target->getMaxHP()+players.length()))
                    invoke_skill = true;
            }
            else{
                if(target->getHandcardNum()<=target->getHp())
                    invoke_skill = true;
            }
            if(!invoke_skill)
                return false;

            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = target;
            log.arg = "yuanyin";
            room->sendLog(log);

            foreach(ServerPlayer *player, others){
                if(player->getHandcardNum() == 0){
                    if(has_frantic)
                        room->loseHp(player, 2);
                    else
                        room->loseHp(player);
                }
                else{
                    int card_id = room->askForCardChosen(target, player, "h", "yuanyin");
                    room->obtainCard(target, card_id, false);
                }
            }
        }
        return false;
    }
};

class Tiemu: public TriggerSkill{
public:
    Tiemu():TriggerSkill("tiemu"){
        events << Damaged << PhaseChange << CardEffected << Predamaged;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        room->playSkillEffect(objectName());
        QList<ServerPlayer *> players = room->getAlivePlayers();
        bool has_frantic = player->getMark("@frantic")>0;

        if(event == PhaseChange && player->getPhase() == Player::Finish){
            if(has_frantic)
                player->drawCards(players.length());
            else
                player->drawCards(player->getMaxHP());
        }

        if(has_frantic && event == CardEffected){
            if(player->isWounded()){
                CardEffectStruct effect = data.value<CardEffectStruct>();
                if(!effect.multiple && effect.card->inherits("TrickCard") && player->getPhase() == Player::NotActive){
                    LogMessage log;
                    log.type = "#TiemuAvoid";
                    log.from = effect.from;
                    log.to << player;
                    log.arg = effect.card->objectName();
                    log.arg2 = objectName();

                    room->sendLog(log);

                    return true;
                }
            }
        }

        if(event == Predamaged){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.damage > 1){
                damage.damage = damage.damage-1;
                data = QVariant::fromValue(damage);

                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = player;
                log.arg = objectName();
                room->sendLog(log);
                return false;
            }
        }
        return false;
    }
};

class Guzhan: public ClientSkill{
public:
    Guzhan():ClientSkill("guzhan"){
        frequency = NotFrequent;
    }

    virtual int getSlashResidue(const Player *zom) const{
        if(zom->hasSkill(objectName()) && !zom->getWeapon())
            return 998;
        else
            return 0;
    }
};

class Jizhan: public TriggerSkill{
public:
    Jizhan():TriggerSkill("jizhan"){
        events << Damage << CardLost;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() != Player::Play)
            return false;

        if(player->getHp() != player->getMaxHP() && event == Damage){
            RecoverStruct recover;
            recover.who = player;
            recover.recover = 1;
            room->recover(player, recover);
        }
        else{
            QList<ServerPlayer *> players = room->getAlivePlayers();
            if(player->getHandcardNum() < players.length())
                player->drawCards(1);
        }
        return false;
    }
};

class Duduan: public ClientSkill{
public:
    Duduan():ClientSkill("duduan"){
    }

    virtual bool isProhibited(const Player *from, const Player *to, const Card *card) const{
        return card->inherits("DelayedTrick");
    }
};

class ImpasseRule: public ScenarioRule{
public:
    ImpasseRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart << TurnStart << PhaseChange
               << Death << GameOverJudge << HpChanged << RewardAndPunish;

        boss_banlist << "songjiang" << "yanxijiao" << "shijin" << "qiongying" << "caijing" << "zhuwu" << "gaoqiu" << "zhaoji"
                     << "suochao" << "tianhu" << "qiongyaonayan";

        boss_skillbanned << "ganlin" << "huakui" << "yuanpei" << "shalu" << "fangzhen" << "cuju" << "shemi"
                         << "chongfeng" << "jiaozhen" << "wuzhou";

        dummy_skills << "mozhang" << "maidao" << "fengmang" << "shouge" << "jielue"
                     << "qimen" << "wuzhou" << "ganlin" << "beishui" << "huatian"
                     << "fushang" << "zuohua" << "jiuhan" << "lianma" << "zhongjia"
                     << "huaxian" << "linmo" << "sheyan" << "jiayao";
    }

    void getRandomSkill(ServerPlayer *player, bool need_trans = false) const{
        Room *room = player->getRoom();
        qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

        QStringList all_generals = Sanguosha->getLimitedGeneralNames();
        QList<ServerPlayer *> players = room->getAllPlayers();
        foreach(ServerPlayer *player, players)
            all_generals.removeOne(player->getGeneralName());

        if(need_trans){
            QString new_lord;

            do{
                int seed = qrand() % all_generals.length();
                new_lord = all_generals[seed];
            }while(boss_banlist.contains(new_lord));

            room->transfigure(player, new_lord, false);
            return;
        }

        QStringList all_skills;
        foreach(QString one, all_generals){
            const General *general = Sanguosha->getGeneral(one);
            if(general->isHidden())
                continue;
            QList<const Skill *> skills = general->findChildren<const Skill *>();
            foreach(const Skill *skill, skills){
                if(!skill->isLordSkill()){
                    if(dummy_skills.contains(skill->objectName()))
                        continue;
                    if(skill->getFrequency() == Skill::NotSkill ||
                       skill->getFrequency() == Skill::Wake ||
                       skill->getFrequency() == Skill::Limited)
                        continue;

                    if(!skill->objectName().startsWith("#"))
                        all_skills << skill->objectName();
                }
            }
        }

        QString got_skill;
        do{
            if(all_skills.isEmpty())
                break;
            int index;
            do{
                index = qrand() % all_skills.length();
            }while(player->isLord() && boss_skillbanned.contains(all_skills[index]));
            got_skill = all_skills[index];
            all_skills.removeAt(index);
        }while(hasSameSkill(room, got_skill));

        if(!got_skill.isEmpty())
            room->acquireSkill(player, got_skill);
        else
            room->acquireSkill(player, "wusheng");
    }

    bool hasSameSkill(Room *room, QString skill_name) const{
        foreach(ServerPlayer *player, room->getAllPlayers()){
            QList<const Skill *> skills = player->getVisibleSkillList();
            const Skill *skill = Sanguosha->getSkill(skill_name);
            if(skills.contains(skill))
                return true;
        }

        return false;
    }

    void startDeadJudge(ServerPlayer *lord) const{
        Room *room = lord->getRoom();

        LogMessage log;
        log.type = "#BaozouOver";
        log.from = lord;
        room->sendLog(log);

        QList<ServerPlayer *> players = room->getOtherPlayers(lord);
        foreach(ServerPlayer *player, players){
            JudgeStruct judge;
            judge.pattern = QRegExp("(Peach|Analeptic):(.*):(.*)");
            judge.good = true;
            judge.who = player;

            room->judge(judge);
            if(judge.isBad())
                room->loseHp(player, player->getHp());
        }
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        switch(event){
        case GameStart:{
                if(player->isLord()){
                    if(boss_banlist.contains(player->getGeneralName()))
                        getRandomSkill(player, true);

                    room->installEquip(player, "silver_lion");
                    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
                    if((qrand() % 2) == 1){
                        room->acquireSkill(player, "silue");
                        room->acquireSkill(player, "kedi");
                    }
                    else{
                        room->acquireSkill(player, "yuanyin");
                        room->acquireSkill(player, "tiemu");
                    }

                    int maxhp = 8 - ((player->getMaxHp() % 3) % 2);
                    room->setPlayerProperty(player, "maxhp", maxhp);
                    room->setPlayerProperty(player, "hp", maxhp);
                }

                getRandomSkill(player);
                room->setTag("FirstRound", true);
                break;
            }

        case TurnStart:{
                if(player->isLord() && player->faceUp()){
                    bool hasLoseMark = false;
                    if(player->hasMark("@frantic")){
                        player->loseMark("@frantic");
                        hasLoseMark = true;
                    }

                    if(hasLoseMark && !player->hasMark("@frantic")){
                        startDeadJudge(player);
                        player->addMark("frantic_over");
                    }
                }
                break;
            }

        case PhaseChange:{
                if(player->isLord() && player->hasMark("frantic_over") && player->getPhase() == Player::Finish)
                   player->getRoom()->killPlayer(player);
                break;
            }

        case GameOverJudge:{
                return true;
                break;
            }

        case Death:{
            QList<ServerPlayer *> players = room->getAlivePlayers();
            bool hasRebel = false, hasLord = false;
            foreach(ServerPlayer *each, players){
                if(each->getRole() == "rebel")
                    hasRebel = true;
                if(each->getRole() == "lord"){
                    hasLord = true;
                    if(each->getMaxHp() > 3)
                        room->setPlayerProperty(each, "maxhp", each->getMaxHp()-1);

                    if(each->getMark("@frantic") > (players.length()-1))
                        each->loseMark("@frantic");
                }
            }
            if(!hasRebel)
                room->gameOver("lord");
            if(!hasLord)
                room->gameOver("rebel");
            break;
        }

        case RewardAndPunish:{
            if(data.canConvert<DamageStar>()){
                DamageStar damage = data.value<DamageStar>();
                if(damage && damage->from){
                    QString torole = damage->to ? damage->to->getRole() : "unknown";
                    if(damage->from->getRole() == torole)
                        damage->from->throwAllHandCards();
                    else
                        damage->from->drawCards(2);

                    damage = NULL;
                    data = QVariant::fromValue(damage);
                }
            }
            return true;
        }

        case HpChanged:{
            if(player->isLord()){
                if(player->getHp() <= 3 && !player->hasMark("@frantic")){
                    LogMessage log;
                    log.type = "#Baozou";
                    log.from = player;
                    room->sendLog(log);

                    QList<ServerPlayer *> others = room->getOtherPlayers(player);
                    player->gainMark("@frantic", others.length());
                    room->setPlayerProperty(player, "maxhp", 3);
                    room->acquireSkill(player, "guzhan");
                    room->acquireSkill(player, "duduan");
                    room->acquireSkill(player, "jizhan");

                    QList<const Card *> judges = player->getCards("j");
                    foreach(const Card *card, judges)
                        room->throwCard(card->getEffectiveId());
                }
            }
            break;
        }

        default:
            break;
        }

        return false;
    }

private:
    QStringList boss_banlist, boss_skillbanned;
    QStringList dummy_skills;
};

bool ImpasseScenario::exposeRoles() const{
    return true;
}

void ImpasseScenario::assign(QStringList &generals, QStringList &roles) const{
    Q_UNUSED(generals);

    roles << "lord";
    int i;
    for(i=0; i<7; i++)
        roles << "rebel";

    qShuffle(roles);
}

int ImpasseScenario::getPlayerCount() const{
    return 8;
}

bool ImpasseScenario::unloadLordSkill() const{
    return true;
}

void ImpasseScenario::getRoles(char *roles) const{
    strcpy(roles, "ZFFFFFFF");
}

ImpasseScenario::ImpasseScenario()
    :Scenario("impasse_fight")
{
    rule = new ImpasseRule(this);

    skills << new Silue << new Kedi
            << new Tiemu << new Yuanyin << new YuanyinEx
            << new Guzhan << new Jizhan << new Duduan;

}

ADD_SCENARIO(Impasse)
