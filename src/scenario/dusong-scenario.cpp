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

class Zhengzhuang:public DrawCardsSkill{
public:
    Zhengzhuang():DrawCardsSkill("zhengzhuang"){
        frequency = Frequent;
    }

    virtual int getDrawNum(ServerPlayer *player, int n) const{
        if(player->hasEquip() && player->askForSkillInvoke(objectName())){
            player->getRoom()->playSkillEffect(objectName());
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
                    player->gainMark("@true", n);
            }
            return false;
        }
        if(player->getPhase() == Player::Finish && player->getMark("@true") > 0 && player->askForSkillInvoke(objectName())){
            room->playSkillEffect(objectName());
            QList<ServerPlayer *> tarc;
            for(int i = player->getMark("@true"); i > 0; i--){
                QList<ServerPlayer *> targets, players;
                players = tarc.length() == 3 ? tarc : room->getAlivePlayers();
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
                room->obtainCard(player, card_id, room->getCardPlace(card_id) != Player::Hand);
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
                        room->transfigure(player, "zhang2dong", true, true);

                        QList<const Card *> tricks = player->getJudgingArea();
                        foreach(const Card *trick, tricks)
                            room->throwCard(trick);

                    }else
                        player->drawCards(8, false);
                }else
                    player->drawCards(player->getSeat() + 1, false);

                if(player->getGeneralName() == "zhangchunhua"){
                    if(qrand() % 3 == 0)
                        room->killPlayer(player);
                }

                return true;
            }

        case CardUsed:{
                CardUseStruct use = data.value<CardUseStruct>();
                if(use.card->inherits("Weapon") && player->askForSkillInvoke("weapon_recast", data)){
                    player->playCardEffect("@recast");
                    room->throwCard(use.card);
                    player->drawCards(1, false);
                    return false;
                }

                break;
            }

        case HpChanged:{
                if(player->getGeneralName() == "zhang1dong" && player->getHp() <= 4){
                    longjmp(env, Transfiguration);
                }

                return false;
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

                return false;
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

bool DusongScenario::generalSelection() const{
    return true;
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

ADD_SCENARIO(Dusong);
