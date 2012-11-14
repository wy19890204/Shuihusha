#include "warlords.h"
#include "engine.h"

class WarlordsScenarioRule: public ScenarioRule{
public:
    WarlordsScenarioRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << PhaseChange << Damage << Death << GameOverJudge;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        switch(triggerEvent){
        case PhaseChange:{
            if(player->isLord() && player->getPhase() == Player::RoundStart){
                player->setRole("renegade");
                room->broadcastProperty(player, "role");
            }
            break;
        }
        case Damage:{
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.from)
                damage.from->gainMark("@blood", damage.damage);

            return false;
            break;
        }

        case GameOverJudge:{
            return true;
            break;
        }

        case Death:{
            DamageStar damage = data.value<DamageStar>();
            if((player->getRole() == "loyalist" || player->getRole() == "lord") && damage && damage->from) {
                if((player->getRole() == "lord" && damage->from->getRole() == "loyalist") || (player->getRole() == "loyalist" && damage->from->getRole() == "lord")) {
                    damage->from->throwAllEquips();
                    damage->from->throwAllHandCards();
                    room->setPlayerProperty(damage->from, "role", "renegade");
                }
                else
                    room->setPlayerProperty(damage->from, "role", player->getRole());
                    
                if(player->getRole() == "lord") {
                    int maxblood = -1;
                    foreach(ServerPlayer *p, room->getOtherPlayers(damage->from))
                        if(p->getMark("@blood") > maxblood)
                            maxblood = p->getMark("@blood");

                    QList<ServerPlayer *> targets;
                    foreach(ServerPlayer *p, room->getOtherPlayers(damage->from))
                        if(p->getMark("@blood") == maxblood)
                            targets << p;
                            
                    ServerPlayer *junshi = room->askForPlayerChosen(damage->from, targets, "getJunShi");
                    room->setPlayerProperty(junshi, "role", "loyalist");
                }
            }

            if(room->getLord() == NULL || (player->isLord() && (!damage || !damage->from))) {
                QList<ServerPlayer *> players = room->getAllPlayers();
                qShuffle(players);

                ServerPlayer *target = players.at(0);
                room->setPlayerProperty(player, "role", "lord");
                players.removeOne(target);
                if(!players.isEmpty()) {
                    ServerPlayer *junshi = room->askForPlayerChosen(target, players, "getJunShi");
                    room->setPlayerProperty(junshi, "role", "loyalist");
                }
                else
                    room->gameOver("lord+loyalist");
            }
                break;
        }

        default:
            break;
        }
        
        return false;
    }
};

void WarlordsScenario::assign(QStringList &generals, QStringList &roles) const{
    Q_UNUSED(generals);

    roles << "lord";
    for(int i = 0; i < 7; i++)
        roles << "renegade";
}

int WarlordsScenario::getPlayerCount() const{
    return 8;
}

void WarlordsScenario::getRoles(char *roles) const{
    strcpy(roles, "NNNNNNNN");
}

bool WarlordsScenario::lordWelfare(const ServerPlayer *player) const{
    return false;
}

void WarlordsScenario::onTagSet(Room *room, const QString &key) const{
    // dummy
}

WarlordsScenario::WarlordsScenario()
    :Scenario("warlords")
{
    rule = new WarlordsScenarioRule(this);
}

//ADD_SCENARIO(Warlords)
