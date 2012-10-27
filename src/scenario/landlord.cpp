#include "landlord.h"
#include "skill.h"
#include "engine.h"
#include "room.h"
#include "settings.h"

class LandlordScenarioRule: public ScenarioRule{
public:
    LandlordScenarioRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart << Death;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        //const LandlordScenario *scenario = qobject_cast<const LandlordScenario *>(parent());
        switch(event){
        case GameStart:{
            if(!player->getWakeSkills().isEmpty()){
                player->setFlags("init_wake");
                player->setFlags("-init_wake");
            }

            player->drawCards(4, false);

            QList<ServerPlayer *> players;
            ServerPlayer *lord = room->getLord();
            players << lord;
            players << lord->getNextAlive();
            players << lord->getNextAlive()->getNextAlive();

            bool dizhu = false;
            QStringList choices;
            choices << "2" << "3" << "pass";
            QString choice = "pass";
            foreach(ServerPlayer *fmr, players){
                choice = room->askForChoice(fmr, "landlord", choices.join("+"));
                fmr->setMark("Land", choice == "pass" ? 0 : atoi(choice.toLocal8Bit().data()));
                if(choice == "3"){
                    lord->setRole(fmr->getRole());
                    fmr->setRole("lord");
                    room->updateStateItem();
                    //room->broadcastProperty(player, "role");
                    lord = fmr;
                    dizhu = true;
                }
                else if(choice == "2")
                    choices.removeOne("2");
            }
            if(!dizhu){
                foreach(ServerPlayer *fmr, players){
                    if(fmr->getMark("Land") == 2){
                        lord->setRole(fmr->getRole());
                        fmr->setRole("lord");
                        room->updateStateItem();
                        //room->broadcastProperty(player, "role");
                        lord = fmr;
                        dizhu = true;
                    }
                }
            }

            QStringList lord_list = Sanguosha->getRandomGenerals(qMin(5, Config.value("MaxChoice", 3).toInt()));
            QString general = room->askForGeneral(lord, lord_list);
            lord->setGeneral2Name(general);
            room->setPlayerProperty(lord, "maxhp", qMin(lord->getGeneral()->getMaxHp(), lord->getGeneral2()->getMaxHp()) + 1);
            room->setPlayerProperty(lord, "hp", lord->getMaxHp());
            lord->acquireSkill("linse");

            return true;
        }
        case Death:{
            if(!player->isLord()){
                ServerPlayer *lord = room->getLord();
                room->loseMaxHp(lord);
            }
        }
        default:
            break;
        }
        return false;
    }
};

void LandlordScenario::assign(QStringList &generals, QStringList &roles) const{
    Q_UNUSED(generals);

    roles << "lord" << "rebel" << "rebel";
}

int LandlordScenario::getPlayerCount() const{
    return 3;
}

void LandlordScenario::getRoles(char *roles) const{
    strcpy(roles, "ZFF");
}

void LandlordScenario::onTagSet(Room *, const QString &) const{
    // dummy
}

bool LandlordScenario::lordWelfare(const ServerPlayer *) const{
    return false;
}

bool LandlordScenario::generalSelection(Room *room) const{
    ServerPlayer *the_lord = room->getLord();
    //QList<const General *> generals = Sanguosha->getAllGeneral()
    QStringList lord_list = Sanguosha->getRandomGenerals(qMin(5, Config.value("MaxChoice", 3).toInt()));
    QString general = room->askForGeneral(the_lord, lord_list);
    the_lord->setGeneralName(general);
    //generals.removeOne(the_lord)

    room->broadcastProperty(the_lord, "general", general);

    foreach(ServerPlayer *player, room->getOtherPlayers(the_lord)){
        QStringList choices = Sanguosha->getRandomGenerals(qMin(5, Config.value("MaxChoice", 3).toInt()));
        QString name = room->askForGeneral(player, choices);
        room->setPlayerProperty(player, "general", name);
        //generals.removeOne(name);
    }
    return false;
}

class Exploit: public DrawCardsSkill{
public:
    Exploit():DrawCardsSkill("exploit"){
    }

    virtual int getDrawNum(ServerPlayer *x, int n) const{
        Room *room = x->getRoom();
        if(room->askForSkillInvoke(x, objectName())){
            return n + 1;
        }else
            return n;
    }
};

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

LandlordScenario::LandlordScenario()
    :Scenario("landlord")
{
    rule = new LandlordScenarioRule(this);
    skills << new Exploit;
}

ADD_SCENARIO(Landlord)
