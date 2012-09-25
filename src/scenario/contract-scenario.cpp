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
                        room->acquireSkill(tmp, "#joint_attack", false);
                        room->acquireSkill(tmp, "#protection", false);
                    }
                }
                break;
            }
        case PhaseChange:{
                if(player->isLord() && player->getPhase() == Player::Play){
                    player->setRole("renegade");
                    room->broadcastProperty(player, "role");
                }
                break;
            }
        case GameOverJudge:{
                QList<ServerPlayer *> players = room->getAlivePlayers();
                if(players.length() == 2){
                    ServerPlayer *survivor = players.first();
                    ServerPlayer *spouse = players.last();
                    if(scenario->getSpouse(survivor) == spouse){
                        room->gameOver(QString("%1+%2").arg(survivor->objectName()).arg(spouse->objectName()));
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
                if(damage){
                    if(scenario->getSpouse(damage->from) || scenario->getSpouse(player))
                        break;
                    const Card *card = room->askForCard(damage->from, "..", "@contract:" + player->objectName(), false, data, NonTrigger);
                    if(card){
                        player->obtainCard(card);
                        scenario->marry(damage->from, damage->to);
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
                    if(scenario->getSpouse(player) && scenario->getSpouse(killer) == player){
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
                scenario->divorce(player);
            }
        default:
            break;
        }
        return false;
    }
};

void ContractScenario::setSpouse(ServerPlayer *player, ServerPlayer *spouse) const{
    if(spouse)
        player->tag["spouse"] = QVariant::fromValue(spouse);
    else
        player->tag.remove("spouse");
}

void ContractScenario::marry(ServerPlayer *husband, ServerPlayer *wife) const{
    if(getSpouse(husband) == wife)
        return;
    Room *room = husband->getRoom();

    LogMessage log;
    log.type = "#CMarry";
    log.from = husband;
    log.to << wife;
    room->sendLog(log);

    setSpouse(husband, wife);
    setSpouse(wife, husband);

    QVariant n = room->getTag("ConTotal").toInt() + 1;
    room->setTag("ConTotal", n);
    for(int i = 1; i <= 4; i ++){
        bool hasused = false;
        foreach(ServerPlayer *tmp, getMarried(room)){
            if(tmp->getMark("@ctt" + QString::number(i)) > 0){
                hasused = true;
                break;
            }
        }
        if(hasused)
            continue;
        room->setPlayerMark(husband, "@ctt" + QString::number(i), 1);
        room->setPlayerMark(wife, "@ctt" + QString::number(i), 1);
        break;
    }
}

void ContractScenario::divorce(ServerPlayer *enkemann, ServerPlayer *widow) const{
    if(!widow)
        widow = getSpouse(enkemann);
    if(getSpouse(enkemann) != widow)
        return;
    Room *room = enkemann->getRoom();

    ServerPlayer *ex_husband = getSpouse(widow);
    setSpouse(ex_husband, NULL);
    LogMessage log;
    log.type = "#CDivorse";
    log.from = enkemann;
    log.to << widow;
    room->sendLog(log);

    setSpouse(enkemann);
    setSpouse(widow);

    QVariant n = room->getTag("ConTotal").toInt() - 1;
    room->setTag("ConTotal", n);
    for(int i = 1; i <= 4; i ++){
        if(widow->getMark("@ctt" + QString::number(i)) > 0){
            room->setPlayerMark(widow, "@ctt" + QString::number(i), 0);
            break;
        }
    }
}

ServerPlayer *ContractScenario::getSpouse(const ServerPlayer *player){
    return player->tag["spouse"].value<PlayerStar>();
}

QList<ServerPlayer *> ContractScenario::getMarried(Room *room) const{
    QList<ServerPlayer *> ss;
    foreach(ServerPlayer *tmp, room->getAlivePlayers()){
        if(getSpouse(tmp))
            ss << tmp;
    }
    return ss;
}

void ContractScenario::assign(QStringList &generals, QStringList &roles) const{
    Q_UNUSED(generals);

    roles << "lord";
    int i;
    for(i=0; i<7; i++)
        roles << "renegade";
}

int ContractScenario::getPlayerCount() const{
    return 8;
}

void ContractScenario::getRoles(char *roles) const{
    strcpy(roles, "ZNNNNNNN");
}

void ContractScenario::onTagSet(Room *room, const QString &key) const{
    // dummy
}

bool ContractScenario::generalSelection() const{
    return true;
}

class JointAttack: public TriggerSkill{
public:
    JointAttack():TriggerSkill("#joint_attack"){
        events << SlashProceed;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        //ContractScenario *contra = new ContractScenario();
        ServerPlayer *jiyou = ContractScenario::getSpouse(player);
        if(jiyou && jiyou->inMyAttackRange(effect.to) &&
           room->askForCard(jiyou, "Slash", "@attack:" + player->objectName() + ":" + effect.to->objectName(), false, data)){
            LogMessage log;
            log.type = "#Attack";
            log.from = effect.to;
            log.to << player;
            log.to << jiyou;
            room->sendLog(log);
            //room->playSkillEffect(objectName());

            room->slashResult(effect, NULL);
            return true;
        }
        return false;
    }
};

class Protection: public TriggerSkill{
public:
    Protection():TriggerSkill("#protection"){
        events << DamagedProceed;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        //ContractScenario *contra = new ContractScenario();
        ServerPlayer *jiyou = ContractScenario::getSpouse(player);
        if(jiyou && jiyou->inMyAttackRange(player) &&
           room->askForCard(jiyou, "Jink", "@protect:" + player->objectName(), false, data)){
            LogMessage log;
            log.type = "#Protect";
            log.from = jiyou;
            log.to << player;
            room->sendLog(log);
            //room->playSkillEffect(objectName());

            damage.damage --;
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
    if(getSpouse(a) == b)
        return AI::Friend;

    if(getSpouse(a) && getSpouse(b))
        return AI::Neutrality;

    return AI::Enemy;
}

ADD_SCENARIO(Contract);
