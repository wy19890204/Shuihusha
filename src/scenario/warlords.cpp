#include "warlords.h"
#include "engine.h"

class WarlordsScenarioRule: public ScenarioRule{
public:
    WarlordsScenarioRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStarted << Damage << Death << GameOverJudge;
    }

    static QList<ServerPlayer *> getPlayersbyRole(Room *room, const QString &role){
        QList<ServerPlayer *> players;
        foreach(ServerPlayer *player, room->getAlivePlayers()){
            if(player->getRole() == role)
                players << player;
        }
        return players;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
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
            if(damage.from && !getPlayersbyRole(room, "loyalist").isEmpty())
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
            if((damage && damage->from) && (player->getRole() == "loyalist" || player->getRole() == "lord")) {
                if((player->getRole() == "lord" && damage->from->getRole() == "loyalist") ||
                   (player->getRole() == "loyalist" && damage->from->getRole() == "lord")) {
                    damage->from->throwAllEquips();
                    damage->from->throwAllHandCards();
                    room->setPlayerProperty(damage->from, "role", "rebel");
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

            if(getPlayersbyRole(room, "lord").isEmpty()){
                QList<ServerPlayer *> players = room->getAlivePlayers();
                qShuffle(players);

                ServerPlayer *target = players.at(0);
                room->setPlayerProperty(target, "role", "lord");
            }
            else if(room->getLord() && getPlayersbyRole(room, "loyalist").isEmpty() && !player->isLord()) {
                QList<ServerPlayer *> players = room->getAlivePlayers();
                players.removeOne(room->getLord());
                if(players.length() > 1) {
                    ServerPlayer *junshi = room->askForPlayerChosen(room->getLord(), players, "getJunShi");
                    room->setPlayerProperty(getPlayersbyRole(room, "loyalist").first(), "role", "rebel");
                    room->setPlayerProperty(junshi, "role", "loyalist");
                }
            }

            if(getPlayersbyRole(room, "rebel").isEmpty())
                room->gameOver("lord+loyalist");
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

void WarlordsScenario::onTagSet(Room *room, const QString &key) const{
    // dummy
}

AI::Relation WarlordsScenario::relationTo(const ServerPlayer *a, const ServerPlayer *b) const{
    if(a->getRole() == "rebel" && b->getRole() == "rebel" &&
       WarlordsScenarioRule::getPlayersbyRole(a->getRoom(), "rebel").length() > 5)
        return AI::Neutrality;
    else
        return AI::GetRelation(a, b);
}

WarlordsScenario::WarlordsScenario()
    :Scenario("warlords")
{
    rule = new WarlordsScenarioRule(this);
}

ADD_SCENARIO(Warlords)
