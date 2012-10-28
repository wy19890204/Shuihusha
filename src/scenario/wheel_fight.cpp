#include "wheel_fight.h"
#include "skill.h"
#include "engine.h"
#include "room.h"
#include "carditem.h"
#include "settings.h"

class WheelFightScenarioRule: public ScenarioRule{
public:
    WheelFightScenarioRule(Scenario *scenario)
        :ScenarioRule(scenario){
        events << GameStart << PreDeath;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        //const WheelFightScenario *scenario = qobject_cast<const WheelFightScenario *>(parent());
        switch(event){
        case PreDeath:{
            DamageStar damage = data.value<DamageStar>();
            ServerPlayer *killer = damage ? damage->from : NULL;
            if(killer)
                killer->drawCards(3);
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
            room->transfigure(player, next_general, true, true, player->getGeneralName());
            player->gainMark("@skull");
            if(player->getMark("@skull") >= Config.value("WheelCount", 10).toInt())
                room->gameOver(room->getOtherPlayers(player).first()->objectName());
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

void WheelFightScenario::onTagSet(Room *, const QString &) const{
    // dummy
}

WheelFightScenario::WheelFightScenario()
    :Scenario("wheel_fight"){
    rule = new WheelFightScenarioRule(this);
}

ADD_SCENARIO(WheelFight)
