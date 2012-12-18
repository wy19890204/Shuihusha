#include "wheel_fight.h"
#include "engine.h"
#include "settings.h"

class WheelFightScenarioRule: public ScenarioRule{
public:
    WheelFightScenarioRule(Scenario *scenario)
        :ScenarioRule(scenario){
        events << HpLost << PreDeath << GameOverJudge;
    }

    static void Domo(Room *room, ServerPlayer *player){
        if(!player->faceUp())
            player->turnOver();
        room->setPlayerFlag(player, "-ecst");
        room->setPlayerFlag(player, "-init_wake");
        player->clearFlags();
        player->clearHistory();
        player->throwAllCards();
        player->throwAllMarks();
        player->clearPrivatePiles();
    }

    static void Oyasumi(Room *room, ServerPlayer *player){
        //player->loseAllSkills();
        QStringList ban = room->getTag("WheelBan").toStringList();
        foreach(ServerPlayer *tmp, room->getAllPlayers()){
            if(!ban.contains(tmp->getGeneralName()))
                ban << tmp->getGeneralName();
            if(!ban.contains(tmp->getGeneral2Name()))
                ban << tmp->getGeneral2Name();
        }
        room->setTag("WheelBan", ban);
        QStringList list = Sanguosha->getRandomGenerals(qMin(5, Config.value("MaxChoice", 3).toInt()), ban.toSet());
        QString next_general = room->askForGeneral(player, list);
        //room->transfigure(player, next_general, true, true, player->getGeneralName());
        room->transfigure(player, next_general, true, true);
        room->setPlayerProperty(player, "kingdom", player->getGeneral()->getKingdom());

        Domo(room, player);
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        //const WheelFightScenario *scenario = qobject_cast<const WheelFightScenario *>(parent());
        switch(event){
        case HpLost:{
            DamageStruct damage;
            damage.to = player;
            damage.from = player->getNextAlive();
            damage.damage = data.toInt();
            room->damage(damage);
            return true;
        }
        case PreDeath:{
            DamageStar damage = data.value<DamageStar>();
            ServerPlayer *killer = damage ? damage->from : NULL;
            if(killer){
                killer->addMark("wheelon");
                if(killer->getMark("wheelon") >= 3){
                    int wheel = killer->getMark("@skull");
                    LogMessage log;
                    log.type = "#KillerB";
                    log.from = killer;
                    log.arg = killer->getGeneralName();
                    room->sendLog(log);

                    Oyasumi(room, killer);
                    room->setPlayerMark(killer, "@skull", wheel);
                }
                killer->drawCards(3);
                room->setPlayerStatistics(killer, "kill", 1);
            }

            room->setPlayerMark(player, "wheelon", 0);
            player->addMark("@skull");
            if(player->getMark("@skull") >= Config.value("WheelCount", 10).toInt()){
                room->gameOver(room->getOtherPlayers(player).first()->objectName());
                return true;
            }
            int wheel = player->getMark("@skull");

            LogMessage log;
            log.type = "#VictimB";
            log.from = player;
            log.arg = QString::number(wheel);
            room->sendLog(log);
            int maxwheel = Config.value("WheelCount", 10).toInt();
            if((float)wheel / (float)maxwheel > 0.7){
                log.type = "#VictimC";
                log.from = player;
                log.arg = QString::number(maxwheel);
                log.arg2 = QString::number(maxwheel - wheel);
                room->sendLog(log);
            }

            data = QVariant();
            Oyasumi(room, player);

            room->setPlayerMark(player, "@skull", wheel);
            player->drawCards(4);
            return true;
        }
        case GameOverJudge:{
            room->gameOver(".");
            return true;
        }
        default:
            break;
        }
        return false;
    }
};

void WheelFightScenario::assign(QStringList &generals, QStringList &roles) const{
    Q_UNUSED(generals);
    roles << "lord" << "renegade";
    qShuffle(roles);
}

int WheelFightScenario::getPlayerCount() const{
    return 2;
}

void WheelFightScenario::getRoles(char *roles) const{
    strcpy(roles, "ZN");
}

int WheelFightScenario::lordGeneralCount() const{
    return Config.value("MaxChoice", 5).toInt();
}

WheelFightScenario::WheelFightScenario()
    :Scenario("wheel_fight"){
    rule = new WheelFightScenarioRule(this);
}

ADD_SCENARIO(WheelFight)
