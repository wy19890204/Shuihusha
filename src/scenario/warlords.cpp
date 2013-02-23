#include "warlords.h"

class WarlordsScenarioRule: public ScenarioRule{
public:
    WarlordsScenarioRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStarted << Damage << Death << GameOverJudge;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        const WarlordsScenario *scenario = qobject_cast<const WarlordsScenario *>(parent());
        switch(triggerEvent){
        case GameStarted:{
            if(player->isLord()){
                player->setRole("rebel");
                room->broadcastProperty(player, "role");
            }
            break;
        }

        case Damage:{
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.from && !scenario->getPlayersbyRole(room, "loyalist").isEmpty())
                damage.from->gainMark("@blood", damage.damage);

            break;
        }

        case Death:{
            if(scenario->getPlayersbyRole(room, "lord").isEmpty()){
                QList<ServerPlayer *> players = room->getAlivePlayers();
                qShuffle(players);

                ServerPlayer *target = players.at(0);

                LogMessage log;
                log.type = player->isLord() ? "#NewLord" : "#NewLord2";
                log.from = target;
                log.arg = target->getScreenRole();
                room->sendLog(log);

                room->setPlayerProperty(target, "role", "lord");
            }
            else if(room->getLord() && scenario->getPlayersbyRole(room, "loyalist").isEmpty() && !player->isLord()) {
                QList<ServerPlayer *> players = room->getAlivePlayers();
                players.removeOne(room->getLord());
                if(players.length() > 1) {
                    ServerPlayer *junshi = room->askForPlayerChosen(room->getLord(), players, "getJunShi");

                    LogMessage log;
                    log.type = "#NewLoya";
                    log.from = junshi;
                    log.arg = junshi->getScreenRole();
                    room->sendLog(log);

                    if(!scenario->getPlayersbyRole(room, "loyalist").isEmpty())
                        room->setPlayerProperty(scenario->getPlayersbyRole(room, "loyalist").first(), "role", "rebel");
                    room->setPlayerProperty(junshi, "role", "loyalist");
                }
            }

            if(scenario->getPlayersbyRole(room, "rebel").isEmpty()){
                QStringList players;
                foreach(ServerPlayer *tmp, room->getAlivePlayers())
                    players << tmp->objectName();
                room->gameOver(players.join("+"));
                return true;
            }
            break;
        }

        case GameOverJudge:{
            DamageStar damage = data.value<DamageStar>();
            if((damage && damage->from) && (player->getRole() == "loyalist" || player->getRole() == "lord")) {
                if((player->getRole() == "lord" && damage->from->getRole() == "loyalist") ||
                   (player->getRole() == "loyalist" && damage->from->getRole() == "lord")) {

                    LogMessage log;
                    log.type = "#InternalStrife";
                    log.from = damage->from;
                    log.to << player;
                    log.arg = damage->from->getScreenRole();
                    log.arg2 = player->getScreenRole();
                    room->sendLog(log);

                    damage->from->throwAllEquips();
                    damage->from->throwAllHandCards();
                    room->setPlayerProperty(damage->from, "role", "rebel");
                }
                else{
                    LogMessage log;
                    log.type = "#SeizePower";
                    log.from = damage->from;
                    log.to << player;
                    log.arg = damage->from->getScreenRole();
                    log.arg2 = player->getScreenRole();
                    room->sendLog(log);

                    room->setPlayerProperty(damage->from, "role", player->getRole());

                    if(player->getRole() == "lord" && damage->from->getRole() == "lord") {
                        int maxblood = 0;
                        foreach(ServerPlayer *p, room->getOtherPlayers(damage->from))
                            if(p->getMark("@blood") > maxblood)
                                maxblood = p->getMark("@blood");

                        QList<ServerPlayer *> targets;
                        foreach(ServerPlayer *p, room->getOtherPlayers(damage->from))
                            if(p->getMark("@blood") == maxblood)
                                targets << p;

                        if(damage->from->askForSkillInvoke("getJunShi")){
                            ServerPlayer *junshi = room->askForPlayerChosen(damage->from, targets, "getJunShi");

                            log.type = "#Empower";
                            log.from = damage->from;
                            log.to.clear();
                            log.to << junshi;
                            log.arg = damage->from->getScreenRole();
                            log.arg2 = junshi->getScreenRole();
                            room->sendLog(log);

                            room->setPlayerProperty(junshi, "role", "loyalist");
                            foreach(ServerPlayer *loy, room->getOtherPlayers(junshi))
                                if(loy->getRole() == "loyalist")
                                    room->setPlayerProperty(loy, "role", "rebel");
                        }
                    }
                }
            }

            return true;
        }

        default:
            break;
        }
        
        return false;
    }
};

QList<ServerPlayer *> WarlordsScenario::getPlayersbyRole(Room *room, const QString &role){
    QList<ServerPlayer *> players;
    foreach(ServerPlayer *player, room->getAlivePlayers()){
        if(player->getRole() == role)
            players << player;
    }
    return players;
}

void WarlordsScenario::assign(QStringList &generals, QStringList &roles) const{
    Q_UNUSED(generals);

    roles << "lord";
    for(int i = 0; i < 7; i++)
        roles << "rebel";
}

int WarlordsScenario::getPlayerCount() const{
    return 8;
}

void WarlordsScenario::getRoles(char *roles) const{
    strcpy(roles, "FFFFFFFF");
}

bool WarlordsScenario::lordWelfare(const ServerPlayer *player) const{
    return false;
}

int WarlordsScenario::lordGeneralCount() const{
    return Config.value("MaxChoice", 5).toInt();
}

bool WarlordsScenario::unloadLordSkill() const{
    return true;
}

AI::Relation WarlordsScenario::relationTo(const ServerPlayer *a, const ServerPlayer *b) const{
    return AI::Enemy;
    if(a->getRole() == "rebel" && b->getRole() == "rebel" &&
       getPlayersbyRole(a->getRoom(), "rebel").length() > 5){
        switch(qrand() % 4){
        case 0:
        case 1: return AI::Neutrality;
        case 2: return AI::Enemy;
        default: return AI::Friend;
        }
    }
    else if(a->getRole() == "loyalist" && b->isLord())
        return AI::Friend;
    else
        return AI::GetRelation(a, b);
}

WarlordsScenario::WarlordsScenario()
    :Scenario("warlords")
{
    rule = new WarlordsScenarioRule(this);
}

//AF

class ArthurFerrisScenarioRule: public ScenarioRule{
public:
    ArthurFerrisScenarioRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameOverJudge;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        switch(triggerEvent){
        case GameOverJudge:{
            if(room->getAlivePlayers().length() == 2){
                room->gameOver(player->getNextAlive()->objectName());
                return true;
            }
            return true;
        }

        default:
            break;
        }

        return false;
    }
};

bool ArthurFerrisScenario::setCardPiles(const Card *card) const{
    return card->inherits("Halberd") || card->inherits("KylinBow");
}

void ArthurFerrisScenario::assign(QStringList &generals, QStringList &roles) const{
    Q_UNUSED(generals);

    roles << "lord" << "renegade" << "rebel";
}

int ArthurFerrisScenario::getPlayerCount() const{
    return 3;
}

void ArthurFerrisScenario::getRoles(char *roles) const{
    strcpy(roles, "ZNF");
}

int ArthurFerrisScenario::lordGeneralCount() const{
    return Config.value("MaxChoice", 5).toInt();
}

AI::Relation ArthurFerrisScenario::relationTo(const ServerPlayer *a, const ServerPlayer *b) const{
    if(a->getNextAlive() == b)
        return AI::Friend;
    else if(b->getNextAlive() == a)
        return AI::Enemy;
    else
        return AI::Neutrality;
}

class Youxia: public TriggerSkill{
public:
    Youxia():TriggerSkill("youxia"){
        events << CardLost;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *jinge = room->findPlayerBySkillName(objectName());
        if(player->isKongcheng() && jinge && !jinge->isKongcheng() && jinge->isWounded()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(player->isAlive() && move->from_place == Player::Hand && jinge->askForSkillInvoke(objectName(), data)){
                const Card *card = room->askForCardShow(jinge, player, "youxia");
                player->obtainCard(card, false);
                RecoverStruct o;
                o.card = card;
                room->recover(jinge, o);
            }
        }
        return false;
    }
};

ArthurFerrisScenario::ArthurFerrisScenario()
    :Scenario("arthur_ferris")
{
    rule = new ArthurFerrisScenarioRule(this);
}

ADD_SCENARIO(Warlords)
ADD_SCENARIO(ArthurFerris)
