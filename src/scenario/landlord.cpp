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

            if(player->isLord() && !room->getTag("LandLordDone").toBool()){
                QList<ServerPlayer *> players;
                PlayerStar lord = player;
                players << lord;
                players << lord->getNextAlive();
                players << lord->getNextAlive()->getNextAlive();
                foreach(ServerPlayer *fmr, players)
                    fmr->drawCards(4, false);

                QStringList choices;
                choices << "2" << "3" << "pass";
                QString choice = "pass";
                foreach(ServerPlayer *fmr, players){
                    choice = room->askForChoice(fmr, "landlord", choices.join("+"));
                    LogMessage log;
                    log.type = choice != "pass" ? "#LandLord1" : "#LandLord2";
                    log.from = fmr;
                    log.arg = choice;
                    room->sendLog(log);
                    fmr->setMark("Land", choice == "pass" ? 0 : atoi(choice.toLocal8Bit().data()));
                    if(choice == "3"){
                        lord->setRole(fmr->getRole());
                        fmr->setRole("lord");
                        lord = fmr;
                        room->setTag("LandLordDone", true);
                        break;
                    }
                    else if(choice == "2")
                        choices.removeOne("2");
                }
                if(!room->getTag("LandLordDone").toBool()){
                    foreach(ServerPlayer *fmr, players){
                        if(fmr->getMark("Land") == 2){
                            lord->setRole(fmr->getRole());
                            fmr->setRole("lord");
                            lord = fmr;
                            room->setTag("LandLordDone", true);
                            break;
                        }
                    }
                }

                LogMessage log;
                log.type = "#LandLordDone";
                log.from = lord;
                room->sendLog(log);
                room->setTag("LandLordDone", true);

                QStringList ban = room->getTag("LandlordBan").toStringList();
                QStringList lord_list = Sanguosha->getRandomGenerals(qMin(5, Config.value("MaxChoice", 3).toInt()), ban.toSet());
                QString general = room->askForGeneral(lord, lord_list);

                QString trans = QString("%1:%2").arg("anjiang").arg(general);
                lord->invoke("transfigure", trans);
                room->setPlayerProperty(lord, "general2", general);

                room->setPlayerProperty(lord, "maxhp", qMin(lord->getGeneral()->getMaxHp(), lord->getGeneral2()->getMaxHp()) + 1);
                room->setPlayerProperty(lord, "hp", lord->getMaxHp());
                room->acquireSkill(lord, "linse");

                room->updateStateItem();
                foreach(ServerPlayer *p, players)
                    room->broadcastProperty(p, "role");
            }

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
    ServerPlayer *lord = room->getPlayers().first();
    QStringList ban;
    QStringList lord_list = Sanguosha->getRandomGenerals(qMin(5, Config.value("MaxChoice", 3).toInt()));
    QString general = room->askForGeneral(lord, lord_list);
    room->setPlayerProperty(lord, "general", general);
    ban << general;

    foreach(ServerPlayer *player, room->getPlayers()){
        if(player == lord)
            continue;
        QStringList choices = Sanguosha->getRandomGenerals(qMin(5, Config.value("MaxChoice", 3).toInt()), ban.toSet());
        QString name = room->askForGeneral(player, choices);
        room->setPlayerProperty(player, "general", name);
        ban << name;
    }
    room->setTag("LandlordBan", ban);
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
