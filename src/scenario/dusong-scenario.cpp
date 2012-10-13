#include "dusong-scenario.h"
#include "skill.h"
#include "engine.h"
#include "standard.h"
#include "carditem.h"
#include "gamerule.h"

static int Transfiguration = 1;

class Douzhan: public ViewAsSkill{
public:
    Douzhan():ViewAsSkill("douzhan"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const{
        return pattern == "slash";
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.isEmpty())
            return !to_select->isEquipped();
        else if(selected.length() == 1){
            const Card *card = selected.first()->getFilteredCard();
            return !to_select->isEquipped() && to_select->getCard()->getSuit() == card->getSuit();
        }else
            return false;
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;

        const Card *first = cards.at(0)->getFilteredCard();
        const Card *second = cards.at(1)->getFilteredCard();

        Slash *slash = new Slash(first->getSuit(), 0);
        slash->setSkillName(objectName());
        slash->addSubcard(first);
        slash->addSubcard(second);

        return slash;
    }
};

class Guzhu: public SlashBuffSkill{
public:
    Guzhu():SlashBuffSkill("guzhu"){
        frequency = Compulsory;
    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        Room *room = effect.from->getRoom();

        if(effect.from->getHp() <= 2){
            room->playSkillEffect(objectName());
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = effect.from;
            log.to << effect.to;
            log.arg = objectName();
            room->sendLog(log);

            room->slashResult(effect, NULL);
            return true;
        }
        return false;
    }
};

class Zhengzhuang: public DrawCardsSkill{
public:
    Zhengzhuang():DrawCardsSkill("zhengzhuang"){
        frequency = Frequent;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        if(player->hasEquip() && player->askForSkillInvoke(objectName())){
            player->playSkillEffect(objectName());
            return n + player->getEquips().count();
        }
        return n;
    }
};

class Feizhi: public TriggerSkill{
public:
    Feizhi():TriggerSkill("feizhi"){
        events << PhaseChange << CardDiscarded;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(event == CardDiscarded){
            if(player->getPhase() == Player::Discard){
                CardStar card = data.value<CardStar>();
                int n = card->subcardsLength();
                if(n > 0)
                    room->setPlayerMark(player, "@true", n);
            }
            return false;
        }
        if(player->getPhase() == Player::Finish && player->hasMark("@true")){
            int totalcard = 0;
            foreach(ServerPlayer *tmp, room->getOtherPlayers(player))
                totalcard += tmp->getCardCount(true);
            if(totalcard >= player->getMark("@true") && player->askForSkillInvoke(objectName())){
                room->playSkillEffect(objectName());
                QList<ServerPlayer *> tarc;
                for(int i = player->getMark("@true"); i > 0; i--){
                    QList<ServerPlayer *> targets, players;
                    players = tarc.length() == 3 ? tarc : room->getOtherPlayers(player);
                    foreach(ServerPlayer *tmp, players){
                        if(!tmp->isNude())
                            targets << tmp;
                    }
                    if(targets.isEmpty())
                        continue;
                    ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
                    if(!tarc.contains(target) && tarc.length() < 3)
                        tarc << target;
                    int card_id = room->askForCardChosen(player, target, "he", objectName());
                    room->throwCard(card_id, target, player);
                    room->setPlayerMark(player, "@true", i - 1);
                }
            }
            room->setPlayerMark(player, "@true", 0);
        }

        return false;
    }
};

class DusongRule: public ScenarioRule{
public:
    DusongRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart << TurnStart << CardUsed << HpChanged << Death;
        default_choice = "recover";
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        switch(event){
        case GameStart:{
                if(player->isLord()){
                    if(setjmp(env) == Transfiguration){
                        player = room->getLord();

                        room->broadcastInvoke("animate", "lightbox:$vqdp:3000");
                        room->broadcastInvoke("playAudio", "zombify-male");
                        room->getThread()->delay(1500);
                        room->transfigure(player, "zhang2dong", true, true, "zhang1dong");

                        QList<const Card *> tricks = player->getJudgingArea();
                        foreach(const Card *trick, tricks)
                            room->throwCard(trick);
                        if(player->isChained())
                            room->setPlayerProperty(player, "chained", false);

                        room->getThread()->delay(1500);
                    }else
                        player->drawCards(8, false);
                }else
                    player->drawCards(player->getSeat() + 1, false);

                return true;
            }

        case CardUsed:{
                CardUseStruct use = data.value<CardUseStruct>();
                if(use.card->inherits("Weapon") && player->askForSkillInvoke("weapon_recast", data)){
                    player->playCardEffect("@recast");
                    room->throwCard(use.card, use.from);
                    player->drawCards(1, false);
                    return true;
                }

                break;
            }

        case HpChanged:{
                if(player->getGeneralName() == "zhang1dong" && player->getHp() <= 4){
                    longjmp(env, Transfiguration);
                }

                break;
            }

        case Death:{
                if(player->isLord()){
                    room->gameOver("rebel");
                }else{
                    if(room->aliveRoles(player).length() == 1)
                        room->gameOver("lord");

                    LogMessage log;
                    log.type = "#Reforming";
                    log.from = player;
                    room->sendLog(log);

                    player->bury();
                    room->setPlayerProperty(player, "hp", 0);

                    foreach(ServerPlayer *player, room->getOtherPlayers(room->getLord())){
                        if(player->askForSkillInvoke("draw_1v3"))
                            player->drawCards(1, false);
                    }
                }

                return false;
            }

        case TurnStart:{
                if(player->isLord()){
                    if(!player->faceUp())
                        player->turnOver();
                    else
                        player->play();
                }else{
                    if(player->isDead()){
                        if(player->getHp() + player->getHandcardNum() == 6){
                            LogMessage log;
                            log.type = "#ReformingRevive";
                            log.from = player;
                            room->sendLog(log);

                            room->revivePlayer(player);
                        }else if(player->isWounded()){
                            if(player->getHp() > 0 && (room->askForChoice(player, "dusong", "recover1hp+draw1card") == "draw1card")){
                                LogMessage log;
                                log.type = "#ReformingDraw";
                                log.from = player;
                                room->sendLog(log);
                                player->drawCards(1, false);
                                return false;
                            }

                            LogMessage log;
                            log.type = "#ReformingRecover";
                            log.from = player;
                            room->sendLog(log);

                            room->setPlayerProperty(player, "hp", player->getHp() + 1);
                        }else
                            player->drawCards(1, false);
                    }else if(!player->faceUp())
                        player->turnOver();
                    else
                        player->play();
                }

                return true;
            }

        default:
            break;
        }

        return false;
        //return GameRule::trigger(event, room, player, data);
    }

private:
    mutable jmp_buf env;
};

void DusongScenario::run(Room *room) const{
    RoomThread *thread = room->getThread();
    ServerPlayer *shenlvbu = room->getLord();
    if(shenlvbu->getGeneralName() == "zhang1dong"){
        QList<ServerPlayer *> league = room->getPlayers();
        league.removeOne(shenlvbu);

        forever{
            foreach(ServerPlayer *player, league){
                if(player->hasFlag("actioned"))
                    room->setPlayerFlag(player, "-actioned");
            }

            foreach(ServerPlayer *player, league){
                room->setCurrent(player);
                thread->trigger(TurnStart, room, room->getCurrent());

                if(!player->hasFlag("actioned"))
                    room->setPlayerFlag(player, "actioned");

                if(shenlvbu->getGeneralName() == "zhang2dong")
                    goto second_phase;

                if(player->isAlive()){
                    room->setCurrent(shenlvbu);
                    thread->trigger(TurnStart, room, room->getCurrent());

                    if(shenlvbu->getGeneralName() == "zhang2dong")
                        goto second_phase;
                }
            }
        }

    }else{
        second_phase:

        foreach(ServerPlayer *player, room->getPlayers()){
            if(player != shenlvbu){
                if(player->hasFlag("actioned"))
                    room->setPlayerFlag(player, "-actioned");

                if(player->getPhase() != Player::NotActive){
                    PhaseChangeStruct phase;
                    phase.from = player->getPhase();
                    room->setPlayerProperty(player, "phase", "not_active");
                    phase.to = player->getPhase();
                    QVariant data = QVariant::fromValue(phase);
                    thread->trigger(PhaseChange, room, player, data);
                }
            }
        }

        room->setCurrent(shenlvbu);

        forever{
            thread->trigger(TurnStart, room, room->getCurrent());
            room->setCurrent(room->getCurrent()->getNext());
        }
    }
}

bool DusongScenario::exposeRoles() const{
    return true;
}

void DusongScenario::assign(QStringList &generals, QStringList &roles) const{
    Q_UNUSED(generals);

    roles << "lord";
    int i;
    for(i=0; i<3; i++)
        roles << "rebel";

    qShuffle(roles);
}

int DusongScenario::getPlayerCount() const{
    return 4;
}

void DusongScenario::getRoles(char *roles) const{
    strcpy(roles, "ZFFF");
}

void DusongScenario::onTagSet(Room *room, const QString &key) const{
    // dummy
}

bool DusongScenario::lordWelfare(const ServerPlayer *) const{
    return false;
}

bool DusongScenario::setCardPiles(const Card *card) const{
    if(card->getPackage() != "standard_cards" && card->getPackage() != "plough")
        return true;
    return card->inherits("Disaster");
}

bool DusongScenario::generalSelection(Room *room) const{
    ServerPlayer *lord = room->getPlayers().first();
    room->setPlayerProperty(lord, "general", "zhang1dong");

    const Package *stdpack = Sanguosha->findChild<const Package *>("standard");
    const Package *ratpack = Sanguosha->findChild<const Package *>("rat");

    QList<const General *> generals = stdpack->findChildren<const General *>();
    generals << ratpack->findChildren<const General *>();

    QStringList names;
    foreach(const General *general, generals){
        names << general->objectName();
    }

    foreach(ServerPlayer *player, room->getPlayers()){
        if(player == lord)
            continue;

        qShuffle(names);
        QStringList choices = names.mid(0, 3);
        QString name = room->askForGeneral(player, choices);

        room->setPlayerProperty(player, "general", name);
        names.removeOne(name);
    }
    return false;
}

DusongScenario::DusongScenario()
    :Scenario("dusong")
{
    lord = "zhang1dong";
    rule = new DusongRule(this);

    General *zhang1dong = new General(this, "zhang1dong", "god", 8, true, true);
    zhang1dong->addSkill("huqi");
    zhang1dong->addSkill(new Douzhan);

    General *zhang2dong = new General(this, "zhang2dong", "god", 4, true, true);
    zhang2dong->addSkill("huqi");
    zhang2dong->addSkill(new Guzhu);
    zhang2dong->addSkill("douzhan");
    zhang2dong->addSkill(new Zhengzhuang);
    zhang2dong->addSkill(new Feizhi);

}

ADD_SCENARIO(Dusong)
