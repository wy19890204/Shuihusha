#include "contract-scenario.h"
#include "skill.h"
#include "engine.h"
#include "room.h"

class ContractScenarioRule: public ScenarioRule{
public:
    ContractScenarioRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart << PhaseChange << GameOverJudge << PreDeath << Death;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        const ContractScenario *scenario = qobject_cast<const ContractScenario *>(parent());

        switch(event){
        case GameStart:{
                if(player->isLord()){
                    room->setTag("ConTotal", 0);
                    foreach(ServerPlayer *tmp, room->getAllPlayers()){
                        room->acquireSkill(tmp, "joint_attack", false);
                        room->acquireSkill(tmp, "protection", false);
                    }
                }
                break;
            }
        case PhaseChange:{
                if(player->isLord() && player->getPhase() == Player::Play){
                    player->setRole("rebel");
                    room->broadcastProperty(player, "role");
                }
                break;
            }
        case GameOverJudge:{
                QList<ServerPlayer *> players = room->getAlivePlayers();
                if(players.length() == 2){
                    ServerPlayer *survivor = players.first();
                    ServerPlayer *comrade = players.last();
                    if(scenario->getComrade(survivor) == comrade){
                        room->gameOver(QString("%1+%2").arg(survivor->objectName()).arg(comrade->objectName()));
                        return true;
                    }
                }else if(players.length() == 1){
                    room->gameOver(players.first()->objectName());
                    return true;
                }
                return true;
            }
        case PreDeath:{
                DamageStar damage = data.value<DamageStar>();
                if(room->getAlivePlayers().length() == 2)
                    break;
                if(damage && damage->from){
                    if(scenario->getComrade(damage->from) || scenario->getComrade(player))
                        break;
                    const Card *card = room->askForCard(damage->from, "..", "@contract:" + player->objectName(), false, data, NonTrigger);
                    if(card){
                        player->obtainCard(card);
                        scenario->annex(damage->from, damage->to);
                        if(player->getMaxHp() < 1)
                            room->setPlayerProperty(player, "maxhp", player->getGeneral()->getMaxHp());
                        RecoverStruct rs;
                        room->recover(player, rs, true);
                        player->drawCards(2);
                        return true;
                    }
                }
                break;
            }
        case Death:{
                DamageStar damage = data.value<DamageStar>();
                if(damage && damage->from){
                    ServerPlayer *killer = damage->from;
                    if(scenario->getComrade(player) && scenario->getComrade(killer) == player){
                        LogMessage log;
                        log.type = "#Uron";
                        log.from = killer;
                        log.to << player;
                        room->sendLog(log);

                        killer->throwAllCards();
                        room->loseMaxHp(killer);
                    }
                    else
                        killer->drawCards(3);
                }
                scenario->rupture(player);
            }
        default:
            break;
        }
        return false;
    }
};

void ContractScenario::setComrade(ServerPlayer *player, ServerPlayer *comrade) const{
    if(comrade)
        player->tag["comrade"] = QVariant::fromValue(comrade);
    else
        player->tag.remove("comrade");
}

void ContractScenario::annex(ServerPlayer *seme, ServerPlayer *uke) const{
    if(getComrade(seme) == uke)
        return;
    Room *room = seme->getRoom();

    LogMessage log;
    log.type = "#Annex";
    log.from = seme;
    log.to << uke;
    room->sendLog(log);

    setComrade(seme, uke);
    setComrade(uke, seme);

    QVariant n = room->getTag("ConTotal").toInt() + 1;
    room->setTag("ConTotal", n);
    for(int i = 1; i <= 4; i ++){
        bool hasused = false;
        foreach(ServerPlayer *tmp, getComraded(room)){
            if(tmp->getMark("@ctt" + QString::number(i)) > 0){
                hasused = true;
                break;
            }
        }
        if(hasused)
            continue;
        room->setPlayerMark(seme, "@ctt" + QString::number(i), 1);
        room->setPlayerMark(uke, "@ctt" + QString::number(i), 1);
        break;
    }
}

void ContractScenario::rupture(ServerPlayer *seme, ServerPlayer *uke) const{
    if(!uke)
        uke = getComrade(seme);
    if(!uke || getComrade(seme) != uke)
        return;
    Room *room = seme->getRoom();

    ServerPlayer *ex_seme = getComrade(uke);
    setComrade(ex_seme, NULL);
    LogMessage log;
    log.type = "#Rupture";
    log.from = seme;
    log.to << uke;
    room->sendLog(log);

    setComrade(seme);
    setComrade(uke);

    QVariant n = room->getTag("ConTotal").toInt() - 1;
    room->setTag("ConTotal", n);
    for(int i = 1; i <= 4; i ++){
        if(uke->getMark("@ctt" + QString::number(i)) > 0){
            room->setPlayerMark(uke, "@ctt" + QString::number(i), 0);
            break;
        }
    }
}

ServerPlayer *ContractScenario::getComrade(const ServerPlayer *player){
    return player->tag["comrade"].value<PlayerStar>();
}

QList<ServerPlayer *> ContractScenario::getComraded(Room *room) const{
    QList<ServerPlayer *> ss;
    foreach(ServerPlayer *tmp, room->getAlivePlayers()){
        if(getComrade(tmp))
            ss << tmp;
    }
    return ss;
}

void ContractScenario::assign(QStringList &generals, QStringList &roles) const{
    Q_UNUSED(generals);

    roles << "lord";
    int i;
    for(i=0; i<7; i++)
        roles << "rebel";
}

int ContractScenario::getPlayerCount() const{
    return 8;
}

void ContractScenario::getRoles(char *roles) const{
    strcpy(roles, "FFFFFFFF");
}

void ContractScenario::onTagSet(Room *room, const QString &key) const{
    // dummy
}

bool ContractScenario::generalSelection() const{
    return true;
}

class JointAttack: public TriggerSkill{
public:
    JointAttack():TriggerSkill("joint_attack"){
        events << SlashProceed;
        frequency = NotSkill;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        //ContractScenario *contra = new ContractScenario();
        ServerPlayer *jiyou = ContractScenario::getComrade(player);
        if(jiyou && jiyou->inMyAttackRange(effect.to) &&
           room->askForCard(jiyou, "Slash", "@attack:" + player->objectName() + ":" + effect.to->objectName(), false, data)){
            LogMessage log;
            log.type = "#Attack";
            log.from = effect.to;
            log.to << player;
            log.to << jiyou;
            room->sendLog(log);
            //room->playSkillEffect(objectName());

            room->setEmotion(effect.to, "bad");
            room->getThread()->delay();
            room->slashResult(effect, NULL);
            return true;
        }
        return false;
    }
};

class Protection: public TriggerSkill{
public:
    Protection():TriggerSkill("protection"){
        events << DamagedProceed;
        frequency = NotSkill;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        //ContractScenario *contra = new ContractScenario();
        ServerPlayer *jiyou = ContractScenario::getComrade(player);
        if(jiyou && jiyou->inMyAttackRange(player) &&
           room->askForCard(jiyou, "Jink", "@protect:" + player->objectName(), false, data)){
            LogMessage log;
            log.type = "#Protect";
            log.from = jiyou;
            log.to << player;
            room->sendLog(log);
            //room->playSkillEffect(objectName());

            damage.damage --;
            room->setEmotion(player, "good");
            room->getThread()->delay();
            data = QVariant::fromValue(damage);;
        }
        return false;
    }
};

ContractScenario::ContractScenario()
    :Scenario("contract")
{
    rule = new ContractScenarioRule(this);
    skills << new JointAttack << new Protection;
}

AI::Relation ContractScenario::relationTo(const ServerPlayer *a, const ServerPlayer *b) const{
    if(getComrade(a) == b)
        return AI::Friend;
    else if(!getComrade(a) && !getComrade(b))
        return AI::Enemy;
    else if(getComrade(a) && getComrade(b))
        return AI::Neutrality;
    else
        return AI::Enemy;
}

ADD_SCENARIO(Contract)
