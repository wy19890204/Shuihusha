#include "landlord.h"
#include "skill.h"
#include "engine.h"
#include "room.h"
#include "carditem.h"
#include "settings.h"

class LandlordScenarioRule: public ScenarioRule{
public:
    LandlordScenarioRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart << Death;
    }

    static QStringList getLandSkills(Room *room){
        QStringList landskills;
        landskills << "linse" << "duoming" << "shemi";
        QStringList sameskills;
        foreach(QString skill, landskills){
            foreach(ServerPlayer *p, room->getAllPlayers()){
                if(p->hasSkill(skill))
                    sameskills << skill;
            }
        }
        foreach(QString skill, sameskills){
            landskills.removeOne(skill);
        }
        landskills << "lesuo" << "bizhai" << "boxue" << "fangdai";

        qShuffle(landskills);
        return landskills;
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

                LogMessage log;
                log.type = "#LLBegin";
                room->sendLog(log);

                foreach(ServerPlayer *fmr, players){
                    QString choice = room->askForChoice(fmr, "landlord", "light+pass");
                    room->broadcastInvoke("playAudio", QString("landlord-%1-%2").arg(choice).arg(fmr->getGenderString()));
                    log.type = "#LandLord_" + choice;
                    log.from = fmr;
                    log.arg = choice;
                    room->sendLog(log);
                    if(choice == "light"){
                        lord->setRole(fmr->getRole());
                        fmr->setRole("lord");
                        lord = fmr;
                        room->setTag("LandLordDone", true);
                        break;
                    }
                }

                log.type = "#LandLordDone";
                log.from = lord;
                room->sendLog(log);
                room->setTag("LandLordDone", true);

                QStringList ban = room->getTag("LandlordBan").toStringList();
                QStringList lord_list = Sanguosha->getRandomGenerals(qMin(5, Config.value("MaxChoice", 3).toInt()), ban.toSet());
                QString general = room->askForGeneral(lord, lord_list);

                lord->invoke("transfigure", "anjiang:" + general);
                room->setPlayerProperty(lord, "general2", general);

                room->setPlayerProperty(lord, "maxhp", qMin(lord->getGeneral()->getMaxHp(), lord->getGeneral2()->getMaxHp()) + 1);
                room->setPlayerProperty(lord, "hp", lord->getMaxHp());

                room->updateStateItem();
                foreach(ServerPlayer *p, players)
                    room->broadcastProperty(p, "role");

                room->getThread()->addPlayerSkills(lord, true);
                QStringList landskills = getLandSkills(room);
                room->acquireSkill(lord, landskills.first());

                log.type = "#LLEnd";
                room->sendLog(log);
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
    qShuffle(roles);
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

LesuoCard::LesuoCard(){
    will_throw = false;
    mute = true;
}

bool LesuoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void LesuoCard::use(Room *o, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    ServerPlayer *yanp = targets.first();
    int index = source->getGender() == General::Male ? qrand() % 2 + 1 : qrand() % 2 + 4;
    o->playSkillEffect(skill_name, index);
    source->pindian(yanp, skill_name, this);
}

class LesuoPindian: public OneCardViewAsSkill{
public:
    LesuoPindian():OneCardViewAsSkill("lesuo"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("LesuoCard") && !player->isKongcheng();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        LesuoCard *card = new LesuoCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return ! to_select->isEquipped();
    }
};

class Lesuo: public TriggerSkill{
public:
    Lesuo():TriggerSkill("lesuo"){
        events << Pindian;
        view_as_skill = new LesuoPindian;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        PindianStar pindian = data.value<PindianStar>();
        if(pindian->reason == objectName()){
            if(pindian->isSuccess()){
                room->playSkillEffect(objectName(), player->getGender() == General::Male ? 3 : 6);
                player->obtainCard(pindian->from_card);
                player->obtainCard(pindian->to_card);
            }
        }
        return false;
    }
};

class Boxue: public PhaseChangeSkill{
public:
    Boxue():PhaseChangeSkill("boxue"){
        frequency = Frequent;
    }

    virtual bool onPhaseChange(ServerPlayer *x) const{
        Room *room = x->getRoom();
        if(x->getPhase() == Player::Start || x->getPhase() == Player::Finish){
            if(room->askForSkillInvoke(x, objectName())){
                int index = x->getGender() == General::Male ? qrand() % 2 + 1 : qrand() % 2 + 3;
                room->playSkillEffect(objectName(), index);
                x->drawCards(1);
            }
        }
        return false;
    }
};

FangdaiCard::FangdaiCard(){
    target_fixed = true;
    once = true;
    mute = true;
}

void FangdaiCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &) const{
    int index = source->getGender() == General::Male ? qrand() % 2 + 1 : qrand() % 2 + 3;
    room->playSkillEffect(skill_name, index);
    room->throwCard(this, source);
    if(source->isAlive())
        room->drawCards(source, subcards.length() + 1);
}

class Fangdai:public ViewAsSkill{
public:
    Fangdai():ViewAsSkill("fangdai"){
    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        FangdaiCard *fangdai_card = new FangdaiCard;
        fangdai_card->addSubcards(cards);
        return fangdai_card;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return ! player->hasUsed("FangdaiCard");
    }
};

LandlordScenario::LandlordScenario()
    :Scenario("landlord")
{
    rule = new LandlordScenarioRule(this);
    skills << new Lesuo << new Skill("bizhai") << new Boxue << new Fangdai;
    addMetaObject<LesuoCard>();
    addMetaObject<FangdaiCard>();
}

ADD_SCENARIO(Landlord)
