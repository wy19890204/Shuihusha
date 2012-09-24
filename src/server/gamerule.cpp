#include "gamerule.h"
#include "serverplayer.h"
#include "room.h"
#include "standard.h"
#include "engine.h"
#include "settings.h"

#include <QTime>

GameRule::GameRule(QObject *)
    :TriggerSkill("game_rule")
{
    //@todo: this setParent is illegitimate in QT and is equivalent to calling
    // setParent(NULL). So taking it off at the moment until we figure out
    // a way to do it.
    //setParent(parent);

    events << GameStart << TurnStart << PhaseChange << CardUsed
            << CardAsk << CardUseAsk << CardFinished
            << CardEffected << HpRecover << HpLost << AskForPeachesDone
            << AskForPeaches << Death << Dying << GameOverJudge
            << PreDeath << AskForRetrial
            << SlashHit << SlashMissed << SlashEffected << SlashProceed
            << DamageDone << DamageComplete << Predamaged
            << StartJudge << FinishJudge << Pindian;
}

bool GameRule::triggerable(const ServerPlayer *) const{
    return true;
}

int GameRule::getPriority() const{
    return 0;
}

void GameRule::onPhaseChange(ServerPlayer *player) const{
    Room *room = player->getRoom();
    switch(player->getPhase()){
    case Player::RoundStart:{
            if(player->getMark("poison") > 0 && !player->isAllNude()){
                LogMessage log;
                log.from = player;
                log.type = "$Poison_lost";
                int index = qrand() % player->getCards("hej").length();
                const Card *card = player->getCards("hej").at(index);
                log.card_str = card->getEffectIdString();
                room->throwCard(card);
                room->sendLog(log);
            }
            break;
        }
    case Player::Start: {
            player->setMark("SlashCount", 0);
            if(player->getMark("@shang") > 0)
                room->loseHp(player, player->getMark("@shang"));
            break;
        }
    case Player::Judge: {
            QList<const DelayedTrick *> tricks = player->delayedTricks();
            while(!tricks.isEmpty() && player->isAlive()){
                const DelayedTrick *trick = tricks.takeLast();
                bool on_effect = room->cardEffect(trick, NULL, player);
                if(!on_effect)
                    trick->onNullified(player);
            }

            break;
        }
    case Player::Draw: {
            QVariant num = 2;
            if(room->getTag("FirstRound").toBool()){
                room->setTag("FirstRound", false);
                if(room->getMode() == "02_1v1")
                    num = 1;
            }

            room->getThread()->trigger(DrawNCards, room, player, num);
            int n = num.toInt();
            if(n > 0)
                player->drawCards(n, false);
            break;
        }

    case Player::Play: {
            player->clearHistory();

            while(player->isAlive()){
                CardUseStruct card_use;
                room->activate(player, card_use);
                if(card_use.isValid()){
                    room->useCard(card_use);
                }else
                    break;
            }
            break;
        }

    case Player::Discard:{
            int discard_num = player->getHandcardNum() - player->getMaxCards();
            if(player->hasFlag("jilei")){
                QSet<const Card *> jilei_cards;
                QList<const Card *> handcards = player->getHandcards();
                foreach(const Card *card, handcards){
                    if(player->isJilei(card))
                        jilei_cards << card;
                }

                if(jilei_cards.size() > player->getMaxCards()){
                    // show all his cards
                    room->showAllCards(player);

                    DummyCard *dummy_card = new DummyCard;
                    foreach(const Card *card, handcards.toSet() - jilei_cards){
                        dummy_card->addSubcard(card);
                    }
                    room->throwCard(dummy_card, player);

                    return;
                }
            }

            if(discard_num > 0)
                room->askForDiscard(player, "gamerule", discard_num);
            break;
        }
    case Player::Finish: {
            break;
        }

    case Player::NotActive:{
            if(player->hasFlag("drank")){
                LogMessage log;
                log.type = "#UnsetDrankEndOfTurn";
                log.from = player;
                room->sendLog(log);

                room->setPlayerFlag(player, "-drank");
            }
            foreach(ServerPlayer *tmp, room->getAllPlayers()){
                if(tmp->hasFlag("ecst")){
                    LogMessage log;
                    log.type = "#UnsetEcstEndOfTurn";
                    log.from = player;
                    log.to << tmp;
                    room->sendLog(log);
                    room->setPlayerFlag(tmp, "-ecst");
                }
                if(tmp->hasFlag("Guibing"))
                    room->setPlayerFlag(tmp, "-Guibing");
            }

            player->clearFlags();
            player->clearHistory();

            if(!Config.BanPackages.contains("events")){
                ServerPlayer *source = room->findPlayerWhohasEventCard("jiefachang");
                if(source && player == source){
                    bool face = false;
                    foreach(ServerPlayer *tmp, room->getAlivePlayers())
                        if(!tmp->faceUp()){
                            face = true;
                            break;
                        }
                    if(face)
                        room->askForUseCard(player, "Jiefachang", "@jiefachang");
                }
            }
            if(Config.EnableReincarnation){
                int count = Sanguosha->getPlayerCount(room->getMode());
                if(count < 4)
                    return;
                int max = count > 5 ? 4 : 3;
                ServerPlayer *next = player->getNext();
                while(next->isDead()){
                    if(next->getHandcardNum() >= max){
                        LogMessage log;
                        log.type = "#ReincarnRevive";
                        log.from = next;
                        room->sendLog(log);

                        room->broadcastInvoke("playAudio", "reincarnation");
                        room->revivePlayer(next);

                        QString oldname = next->getGeneralName();
                        QString newname = Sanguosha->getRandomGenerals(1).first();
                        room->transfigure(next, newname, false, true, oldname);
                        if(next->getMaxHp() == 0)
                            room->setPlayerProperty(next, "maxhp", 1);
                        room->setPlayerProperty(next, "hp", 1);

                        room->getThread()->delay(1500);
                        room->attachSkillToPlayer(next, "sacrifice");
                        room->setPlayerMark(next, "@skull", 1);
                        room->setPlayerProperty(next, "isDead", true);
                    }
                    next = next->getNext();
                }
            }
            return;
        }
    }
}

void GameRule::setGameProcess(Room *room) const{
    int good = 0, bad = 0;
    QList<ServerPlayer *> players = room->getAlivePlayers();
    foreach(ServerPlayer *player, players){
        switch(player->getRoleEnum()){
        case Player::Lord:
        case Player::Loyalist: good ++; break;
        case Player::Rebel: bad++; break;
        case Player::Renegade: break;
        }
    }

    QString process;
    if(good == bad)
        process = "Balance";
    else if(good > bad)
        process = "LordSuperior";
    else
        process = "RebelSuperior";

    room->setTag("GameProcess", process);
}

bool GameRule::trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
    if(room->getTag("SkipGameRule").toBool()){
        room->removeTag("SkipGameRule");
        return false;
    }

    switch(event){
    case GameStart: {
        if(player->getGeneral()->getKingdom() == "god" && player->getGeneralName() != "anjiang"){
                QString new_kingdom = room->askForKingdom(player);
                room->setPlayerProperty(player, "kingdom", new_kingdom);

                LogMessage log;
                log.type = "#ChooseKingdom";
                log.from = player;
                log.arg = new_kingdom;
                room->sendLog(log);
            }

            if(Config.EnableAnzhan){
                PlayerStar head = room->getTag("StandsOutBird").value<PlayerStar>();
                if(player == head)
                    setGameProcess(room);
            }
            else{
                if(player->isLord())
                    setGameProcess(room);
            }

            if(Config.EnableReincarnation){
                int count = Sanguosha->getPlayerCount(room->getMode());
                if(count > 3)
                    room->attachSkillToPlayer(player, "sacrifice");
            }

            room->setTag("FirstRound", true);
            int init = player->hasSkill("beizhan") ? 6 : 4;
            player->drawCards(init, false);

            break;
        }

    case TurnStart:{
            player = room->getCurrent();
            if(Config.value("FreeShowRole", false).toBool()){
                LogMessage log;
                log.type = "#ShowRole";
                log.from = player;
                log.arg = player->getRole();
                room->sendLog(log);
            }
            if(!player->faceUp())
                player->turnOver();
            else if(player->isAlive())
                player->play();

            break;
        }

    case PhaseChange: onPhaseChange(player); break;
    case CardUsed: {
            if(data.canConvert<CardUseStruct>()){
                CardUseStruct card_use = data.value<CardUseStruct>();
                const Card *card = card_use.card;

                bool mute = card_use.mute;
                if(card->inherits("Slash") && Config.EnableEquipEffects){
                    if(player->hasSkill("shuangzhan") && card_use.to.count() == 2){
                        room->playSkillEffect("shuangzhan", qrand() % 2 + 1);
                        mute = true;
                    }
                    if(player->hasSkill("douzhan") && card_use.to.count() == 2){
                        room->playSkillEffect("douzhan");
                        mute = true;
                    }
                    if(player->hasWeapon("crossbow") && player->getPhase() == Player::Play && player->getMark("SlashCount") > 0)
                        mute = true;
                    if(card->getSkillName() == "spear"){
                        player->playCardEffect("Espear", "weapon");
                        mute = true;
                    }
                    else if(player->hasWeapon("halberd") &&
                            player->isLastHandCard(card) && card_use.to.count() > 1){
                        player->playCardEffect("Ehalberd", "weapon");
                        mute = true;
                    }
                    else if(player->hasWeapon("sun_bow") && card->objectName() == "slash" && card_use.to.count() > 1){
                        player->playCardEffect("Esun_bow", "weapon");
                        mute = true;
                    }
                }

                card_use.from->playCardEffect(card, mute);
                card->use(room, card_use.from, card_use.to);
            }

            break;
        }
    case CardAsk :
    case CardUseAsk: {
            if(player->hasFlag("ecst") && (data.toString() == "slash" || data.toString() == "jink")){
                LogMessage log;
                log.type = "#EcstasyEffect";
                log.from = player;
                log.arg = data.toString();
                room->sendLog(log);
                return true;
            }
            if(player->hasFlag("Guibing") && data.toString() == "slash")
                return true;
            break;
        }
    case CardFinished: {
            CardUseStruct use = data.value<CardUseStruct>();
            if(data.canConvert<CardUseStruct>() && !Config.BanPackages.contains("events")){
                if(use.card->inherits("Snatch")){
                    ServerPlayer *source = room->findPlayerWhohasEventCard("daojia");
                    if(source){
                        room->setPlayerFlag(source, "Daojia");
                        room->askForUseCard(source, "Daojia", "@daojia");
                        room->setPlayerFlag(source, "-Daojia");
                    }
                }
                if(use.card->inherits("Analeptic")){
                    ServerPlayer *source = room->findPlayerWhohasEventCard("tifanshi");
                    if(source && source == use.from){
                        room->setPlayerFlag(source, "Tifanshi");
                        room->askForUseCard(source, "Tifanshi", "@tifanshi");
                        room->setPlayerFlag(source, "-Tifanshi");
                    }
                }
                if(use.card->inherits("Ecstasy")){
                    ServerPlayer *source = room->findPlayerWhohasEventCard("nanastars");
                    if(source){
                        bool invoke = false;
                        foreach(ServerPlayer *tmp, room->getAlivePlayers()){
                            if(tmp->containsTrick("treasury", false)){
                                invoke = true;
                                break;
                            }
                        }
                        if(invoke)
                            room->askForUseCard(source, "NanaStars", "@nanastars");
                    }
                }
            }

            room->clearCardFlag(use.card);
            break;
        }
    case HpRecover:{
            RecoverStruct recover_struct = data.value<RecoverStruct>();
            int recover = recover_struct.recover;

            room->setPlayerStatistics(player, "recover", recover);

            int new_hp = qMin(player->getHp() + recover, player->getMaxHP());
            room->setPlayerProperty(player, "hp", new_hp);
            room->broadcastInvoke("hpChange", QString("%1:%2").arg(player->objectName()).arg(recover));

            if(player->getMark("poison") > 0){
                int index = qrand() % 5;
                if(index == 4){
                    room->setPlayerMark(player, "poison", 0);
                    room->setEmotion(player, "good");
                    LogMessage log;
                    log.type = "#Poison_out";
                    log.from = player;
                    room->sendLog(log);
                }
            }
            break;
        }

    case HpLost:{
            int lose = data.toInt();

            if(room->getCurrent()->hasSkill("jueqing"))
                return true;

            LogMessage log;
            log.type = "#LoseHp";
            log.from = player;
            log.arg = QString::number(lose);
            room->sendLog(log);

            room->setPlayerProperty(player, "hp", player->getHp() - lose);
            QString str = QString("%1:%2L").arg(player->objectName()).arg(-lose);
            room->broadcastInvoke("hpChange", str);

            if(player->getHp() <= 0)
                room->enterDying(player, NULL);

            if(Config.EnableEndless){
                player->gainMark("@endless", lose);
            }
            break;
    }

    case Dying:{
            if(player->getHp() > 0){
                player->setFlags("-dying");
                break;
            }

            DyingStruct dying = data.value<DyingStruct>();

            LogMessage log;
            log.type = "#AskForPeaches";
            log.from = player;
            log.to = dying.savers;
            log.arg = QString::number(1 - player->getHp());
            room->sendLog(log);

            RoomThread *thread = room->getThread();
            foreach(ServerPlayer *saver, dying.savers){
                if(player->getHp() > 0)
                    break;

                thread->trigger(AskForPeaches, room, saver, data);
            }

            player->setFlags("-dying");
            thread->trigger(AskForPeachesDone, room, player, data);

            break;
        }

    case AskForPeaches:{
            DyingStruct dying = data.value<DyingStruct>();

            while(dying.who->getHp() <= 0){
                if(dying.who->isDead())
                    break;
                const Card *peach = room->askForSinglePeach(player, dying.who);
                if(!peach)
                    break;

                CardUseStruct use;
                use.card = peach;
                use.from = player;
                if(player != dying.who)
                    use.to << dying.who;

                room->useCard(use, false);

                if(player != dying.who && dying.who->getHp() > 0)
                    room->setPlayerStatistics(player, "save", 1);
            }

            break;
        }

    case AskForPeachesDone:{
            if(player->getHp() <= 0 && player->isAlive()){
                DyingStruct dying = data.value<DyingStruct>();
                room->killPlayer(player, dying.damage);
            }

            break;
        }

    case DamageDone:{
            DamageStruct damage = data.value<DamageStruct>();
            room->sendDamageLog(damage);

            if(damage.to->hasFlag("ecst")){
                LogMessage log;
                log.type = "#UnsetEcst";
                log.from = damage.to;
                room->sendLog(log);

                room->setPlayerFlag(damage.to, "-ecst");
            }
            if(damage.from)
                room->setPlayerStatistics(damage.from, "damage", damage.damage);

            room->applyDamage(player, damage);
            if(player->getHp() <= 0){
                room->enterDying(player, &damage);
            }

            break;
        }

    case DamageComplete:{
            if(room->getMode() == "02_1v1" && player->isDead()){
                QString new_general = player->tag["1v1ChangeGeneral"].toString();
                if(!new_general.isEmpty())
                    changeGeneral1v1(player);
            }
            //nanastars
            if(!Config.BanPackages.contains("events")){
                DamageStruct damage = data.value<DamageStruct>();
                ServerPlayer *source = room->findPlayerWhohasEventCard("nanastars");
                if(damage.from && damage.from != player && source == player && !damage.from->isNude()){
                    if(room->askForCard(source, "NanaStars", "@7stars:" + damage.from->objectName(), false, data, CardDiscarded)){
                        int x = qMax(qAbs(source->getHp() - damage.from->getHp()), 1);
                        source->playCardEffect("@nanastars2");

                        LogMessage log;
                        log.type = "#NanaStars";
                        log.from = source;
                        log.to << damage.from;
                        log.arg = "nanastars";
                        room->sendLog(log);
                        while(!damage.from->isNude()){
                            int card_id = room->askForCardChosen(source, damage.from, "he", "nanastars");
                            room->obtainCard(source, card_id, room->getCardPlace(card_id) != Player::Hand);
                            x --;
                            if(x == 0)
                                break;
                        }
                    }
                }
            }

            bool chained = player->isChained();
            if(!chained)
                break;

            DamageStruct damage = data.value<DamageStruct>();
            if(damage.nature != DamageStruct::Normal){
                room->setPlayerProperty(player, "chained", false);

                // iron chain effect
                QList<ServerPlayer *> chained_players = room->getOtherPlayers(player);
                foreach(ServerPlayer *chained_player, chained_players){
                    if(chained_player->isChained()){
                        room->getThread()->delay();
                        room->setPlayerProperty(chained_player, "chained", false);

                        LogMessage log;
                        log.type = "#IronChainDamage";
                        log.from = chained_player;
                        room->sendLog(log);

                        DamageStruct chain_damage = damage;
                        chain_damage.to = chained_player;
                        chain_damage.chain = true;

                        room->damage(chain_damage);
                    }
                }
            }

            if(Config.EnableEndless){
                if(damage.from)
                    damage.from->gainMark("@endless", damage.damage);
                else
                    damage.to->gainMark("@endless", damage.damage);
            }

            break;
        }

    case Predamaged:{
            //ninegirl
            if(!Config.BanPackages.contains("events")){
                DamageStruct damage = data.value<DamageStruct>();
                if(damage.damage > 1){
                    ServerPlayer *source = room->findPlayerWhohasEventCard("ninedaygirl");
                    if(source == damage.to){
                        room->setPlayerFlag(damage.to, "NineGirl");
                        QString prompt = QString("@ninedaygirl:::%1").arg(damage.damage);
                        bool girl = room->askForUseCard(damage.to, "NinedayGirl", prompt);
                        room->setPlayerFlag(damage.to, "-NineGirl");
                        if(girl){
                            LogMessage log;
                            log.from = damage.to;
                            log.type = "#NineGirl";
                            log.arg = QString::number(damage.damage);
                            room->sendLog(log);
                            return true;
                        }
                    }
                }
            }
        }
    case CardEffected:{
            if(data.canConvert<CardEffectStruct>()){
                CardEffectStruct effect = data.value<CardEffectStruct>();
                if(room->isCanceled(effect))
                    return true;

                effect.card->onEffect(effect);
            }

            break;
        }

    case SlashEffected:{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();

            QVariant data = QVariant::fromValue(effect);
            room->getThread()->trigger(SlashProceed, room, effect.from, data);

            break;
        }

    case SlashProceed:{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();

            QString slasher = effect.from->objectName();
            const Card *jink = room->askForCard(effect.to, "jink", "slash-jink:" + slasher, false, data, JinkUsed);
            room->slashResult(effect, jink);

            break;
        }

    case SlashHit:{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();

            DamageStruct damage;
            damage.card = effect.slash;

            damage.damage = 1;
            if(effect.drank){
                LogMessage log;
                log.type = "#AnalepticBuff";
                log.from = effect.from;
                log.to << effect.to;
                log.arg = "analeptic";
                room->sendLog(log);

                damage.damage ++;
            }
            if(effect.from->hasFlag("drunken")){
                LogMessage log;
                log.type = "#DrunkenBuff";
                log.from = effect.from;
                log.to << effect.to;
                log.arg = "jiangjieshi";
                room->sendLog(log);

                damage.damage ++;
                room->setPlayerFlag(player, "-drunken");
            }

            if(effect.to->hasSkill("jueqing") || effect.to->getGeneralName() == "zhangchunhua")
                damage.damage ++;

            damage.from = effect.from;
            damage.to = effect.to;
            damage.nature = effect.nature;

            room->damage(damage);

            effect.to->removeMark("qinggang");

            break;
        }

    case SlashMissed:{
            SlashEffectStruct effect = data.value<SlashEffectStruct>();
            effect.to->removeMark("qinggang");

            break;
        }

    case GameOverJudge:{
            if(room->getMode() == "02_1v1"){
                QStringList list = player->tag["1v1Arrange"].toStringList();

                if(!list.isEmpty())
                    return false;
            }

            QString winner = getWinner(player);
            if(!winner.isNull()){
                room->gameOver(winner);
                return true;
            }

            break;
        }

    case PreDeath:{
            DamageStar damage = data.value<DamageStar>();
            ServerPlayer *killer = damage ? damage->from : NULL;

            if(Config.EnableEndless){
                if(player->getMaxHp() <= 0)
                    room->setPlayerProperty(player, "maxhp", player->getGeneral()->getMaxHp());
                if(player->getHp() <= 0)
                    room->setPlayerProperty(player, "hp", 1);
                if(killer && !player->isKongcheng())
                    killer->gainMark("@endless", qMin(3, player->getHandcardNum()));
                if(player->getMark("@endless") > 0)
                    player->loseMark("@endless", player->getMark("@endless") / 2);
                return true;
            }

            if(player->getState() == "online" && Config.value("FreeUnDead", false).toBool()){
                if(player->getMaxHp() <= 0)
                    room->setPlayerProperty(player, "maxhp", player->getGeneral()->getMaxHp());
                if(player->getHp() <= 0)
                    room->setPlayerProperty(player, "hp", 1);
                LogMessage log;
                log.type = "#Undead";
                log.from = player;
                room->sendLog(log);
                return true;
            }
            break;
        }

    case Death:{
            player->bury();

            if(room->getTag("SkipNormalDeathProcess").toBool())
                return false;

            DamageStar damage = data.value<DamageStar>();
            ServerPlayer *killer = damage ? damage->from : NULL;
            if(killer)
                rewardAndPunish(killer, player);
            else if(player->hasSkill("zuohua"))
                room->playSkillEffect("zuohua", 2);

            setGameProcess(room);

            if(room->getMode() == "02_1v1"){
                QStringList list = player->tag["1v1Arrange"].toStringList();

                if(!list.isEmpty()){
                    player->tag["1v1ChangeGeneral"] = list.takeFirst();
                    player->tag["1v1Arrange"] = list;

                    DamageStar damage = data.value<DamageStar>();

                    if(damage == NULL){
                        changeGeneral1v1(player);
                        return false;
                    }
                }
            }
            break;
        }

    case StartJudge:{
            int card_id = room->drawCard();

            JudgeStar judge = data.value<JudgeStar>();
            judge->card = Sanguosha->getCard(card_id);
            room->moveCardTo(judge->card, NULL, Player::Special);

            LogMessage log;
            log.type = "$InitialJudge";
            log.from = player;
            log.card_str = judge->card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);

            int delay = Config.AIDelay;
            if(judge->time_consuming)
                delay /= 4;
            room->getThread()->delay(delay);

            break;
        }

    case AskForRetrial:{
            if(!Config.BanPackages.contains("events")){
                ServerPlayer *source = room->findPlayerWhohasEventCard("fuckgaolian");
                if(source && source == player){
                    room->setPlayerFlag(player, "FuckLian");
                    const Card *fuck = room->askForCard(player, "FuckGaolian", "@fuckl", false, data);
                    if(fuck){
                        player->playCardEffect("@fuckgaolian2");
                        JudgeStar judge = data.value<JudgeStar>();
                        source->obtainCard(judge->card);
                        judge->card = fuck;
                        room->moveCardTo(judge->card, NULL, Player::Special);
                        LogMessage log;
                        log.type = "$ChangedJudge";
                        log.from = player;
                        log.to << judge->who;
                        log.card_str = QString::number(fuck->getId());
                        room->sendLog(log);
                        room->sendJudgeResult(judge);
                    }
                    room->setPlayerFlag(player, "-FuckLian");
                }
            }

            break;
        }

    case FinishJudge:{
            JudgeStar judge = data.value<JudgeStar>();
            room->throwCard(judge->card);

            LogMessage log;
            log.type = "$JudgeResult";
            log.from = player;
            log.card_str = judge->card->getEffectIdString();
            room->sendLog(log);

            room->sendJudgeResult(judge);

            if(!Config.BanPackages.contains("events")){
                if(judge->card->getSuit() == Card::Spade){
                    ServerPlayer *source = room->findPlayerWhohasEventCard("fuckgaolian");
                    if(source){
                        room->setPlayerFlag(source, "FuckGao");
                        room->askForUseCard(source, "FuckGaolian", "@fuckg");
                        room->setPlayerFlag(source, "-FuckGao");
                    }
                }
                if(judge->card->inherits("Analeptic") && room->getCardPlace(judge->card->getEffectiveId()) == Player::DiscardedPile){
                    ServerPlayer *sour = room->findPlayerWhohasEventCard("jiangjieshi");
                    if(sour && sour != room->getCurrent()){
                        const Card *fight = room->askForCard(sour, "Jiangjieshi", "@jiangshi", false, data, CardDiscarded);
                        if(fight){
                            sour->playCardEffect("@jiangjieshi2");
                            LogMessage log;
                            log.type = "#Jiangjs";
                            log.from = sour;
                            log.arg = "jiangjieshi";
                            log.arg2 = judge->card->objectName();
                            room->sendLog(log);
                            sour->obtainCard(judge->card);
                        }
                    }
                }
            }
            room->getThread()->delay();

            break;
        }

    case Pindian:{
            PindianStar pindian = data.value<PindianStar>();

            LogMessage log;

            room->throwCard(pindian->from_card);
            log.type = "$PindianResult";
            log.from = pindian->from;
            log.card_str = pindian->from_card->getEffectIdString();
            room->sendLog(log);
            room->getThread()->delay();

            room->throwCard(pindian->to_card);
            log.type = "$PindianResult";
            log.from = pindian->to;
            log.card_str = pindian->to_card->getEffectIdString();
            room->sendLog(log);
            room->getThread()->delay();

            break;
        }

    default:
        ;
    }

    return false;
}

void GameRule::changeGeneral1v1(ServerPlayer *player) const{
    Room *room = player->getRoom();
    QString new_general = player->tag["1v1ChangeGeneral"].toString();
    player->tag.remove("1v1ChangeGeneral");
    room->transfigure(player, new_general, true, true);
    room->revivePlayer(player);

    if(player->getKingdom() != player->getGeneral()->getKingdom())
        room->setPlayerProperty(player, "kingdom", player->getGeneral()->getKingdom());

    room->broadcastInvoke("revealGeneral",
                          QString("%1:%2").arg(player->objectName()).arg(new_general),
                          player);

    if(!player->faceUp())
        player->turnOver();

    if(player->isChained())
        room->setPlayerProperty(player, "chained", false);

    player->drawCards(4);
}

void GameRule::rewardAndPunish(ServerPlayer *killer, ServerPlayer *victim) const{
    if(killer->isDead())
        return;

    Room *room = victim->getRoom();
    if(Config.EnableReincarnation && victim->property("isDead").toBool())
        return;

    if(victim->hasSkill("zuohua")){
        LogMessage log;
        log.type = "#Zuohua";
        log.from = victim;
        log.to << killer;
        log.arg = "zuohua";
        room->playSkillEffect("zuohua", 1);
        room->sendLog(log);
        return;
    }

    if(room->getMode() == "06_3v3"){
        if(Config.value("3v3/UsingNewMode", false).toBool())
            killer->drawCards(2);
        else
            killer->drawCards(3);
    }
    else{
        if(victim->getRole() == "rebel" && killer != victim){
            killer->drawCards(3);
        }else if(victim->getRole() == "loyalist" && killer->getRole() == "lord"){
            killer->throwAllEquips();
            killer->throwAllHandCards();
        }
    }
}

QString GameRule::getWinner(ServerPlayer *victim) const{
    Room *room = victim->getRoom();
    QString winner;

    if(room->getMode() == "06_3v3"){
        switch(victim->getRoleEnum()){
        case Player::Lord: winner = "renegade+rebel"; break;
        case Player::Renegade: winner = "lord+loyalist"; break;
        default:
            break;
        }
    }else if(Config.EnableHegemony){
        bool has_anjiang = false, has_diff_kingdoms = false;
        QString init_kingdom;
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            if(room->getTag(p->objectName()).toStringList().size()){
                has_anjiang = true;
            }

            if(init_kingdom.isEmpty()){
                init_kingdom = p->getKingdom();
            }
            else if(init_kingdom != p->getKingdom()){
                has_diff_kingdoms = true;
            }
        }

        if(!has_anjiang && !has_diff_kingdoms){
            QStringList winners;
            QString aliveKingdom = room->getAlivePlayers().first()->getKingdom();
            foreach(ServerPlayer *p, room->getPlayers()){
                if(p->isAlive())winners << p->objectName();
                if(p->getKingdom() == aliveKingdom)
                {
                    QStringList generals = room->getTag(p->objectName()).toStringList();
                    if(generals.size()&&!Config.Enable2ndGeneral)continue;
                    if(generals.size()>1)continue;

                    //if someone showed his kingdom before death,
                    //he should be considered victorious as well if his kingdom survives
                    winners << p->objectName();
                }
            }

            winner = winners.join("+");
        }
    }else{
        QStringList alive_roles = room->aliveRoles(victim);
        switch(victim->getRoleEnum()){
        case Player::Lord:{
                if(alive_roles.length() == 1 && alive_roles.first() == "renegade")
                    winner = room->getAlivePlayers().first()->objectName();
                else
                    winner = "rebel";
                break;
            }

        case Player::Rebel:
        case Player::Renegade:
            {
                if(!alive_roles.contains("rebel") && !alive_roles.contains("renegade")){
                    winner = "lord+loyalist";
                    if(victim->getRole() == "renegade" && !alive_roles.contains("loyalist"))
                        room->setTag("RenegadeInFinalPK", true);
                }
                break;
            }

        default:
            break;
        }
    }

    return winner;
}

BasaraMode::BasaraMode(QObject *parent)
    :GameRule(parent)
{
    setObjectName("basara_mode");

    events << CardLost << Predamaged;
    skill_mark["shenchou"] = "@chou";
}

QString BasaraMode::getMappedRole(const QString &role){
    static QMap<QString, QString> roles;
    if(roles.isEmpty()){
        roles["guan"] = "lord";
        roles["jiang"] = "loyalist";
        roles["min"] = "rebel";
        roles["kou"] = "renegade";
    }
    return roles[role];
}

int BasaraMode::getPriority() const{
    return 5;
}

void BasaraMode::playerShowed(ServerPlayer *player) const{
    Room *room = player->getRoom();
    QStringList names = room->getTag(player->objectName()).toStringList();
    if(names.isEmpty())
        return;

    if(Config.EnableHegemony){
        QMap<QString, int> kingdom_roles;
        foreach(ServerPlayer *p, room->getOtherPlayers(player)){
            kingdom_roles[p->getKingdom()]++;
        }

        if(kingdom_roles[Sanguosha->getGeneral(names.first())->getKingdom()] >= 2
                && player->getGeneralName() == "anjiang")
            return;
    }

    QString answer = room->askForChoice(player, "RevealGeneral", "yes+no");
    if(answer == "yes"){

        QString general_name = room->askForGeneral(player,names);

        generalShowed(player,general_name);
        if (Config.EnableHegemony) room->getThread()->trigger(GameOverJudge, room, player);
        playerShowed(player);
    }
}

void BasaraMode::generalShowed(ServerPlayer *player, QString general_name) const
{
    Room * room = player->getRoom();
    QStringList names = room->getTag(player->objectName()).toStringList();
    if(names.isEmpty())return;

    if(player->getGeneralName() == "anjiang")
    {
        QString transfigure_str = QString("%1:%2").arg(player->getGeneralName()).arg(general_name);
        player->invoke("transfigure", transfigure_str);
        room->setPlayerProperty(player,"general",general_name);

        foreach(QString skill_name, skill_mark.keys()){
            if(player->hasSkill(skill_name))
                room->setPlayerMark(player, skill_mark[skill_name], 1);
        }
    }
    else{
        QString transfigure_str = QString("%1:%2").arg(player->getGeneral2Name()).arg(general_name);
        player->invoke("transfigure", transfigure_str);
        room->setPlayerProperty(player,"general2",general_name);
    }

    room->setPlayerProperty(player, "kingdom", player->getGeneral()->getKingdom());
    if(Config.EnableHegemony)room->setPlayerProperty(player, "role", getMappedRole(player->getGeneral()->getKingdom()));

    names.removeOne(general_name);
    room->setTag(player->objectName(),QVariant::fromValue(names));

    LogMessage log;
    log.type = "#BasaraReveal";
    log.from = player;
    log.arg  = player->getGeneralName();
    log.arg2 = player->getGeneral2Name();

    room->sendLog(log);
    room->broadcastInvoke("playAudio","choose-item");
}

bool BasaraMode::trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
    player->tag["event"] = event;
    player->tag["event_data"] = data;

    switch(event){
    case GameStart:{
        if(player->isLord()){
            if(Config.EnableHegemony)
                room->setTag("SkipNormalDeathProcess", true);

            foreach(ServerPlayer* sp, room->getAlivePlayers())
            {
                QString transfigure_str = QString("%1:%2").arg(sp->getGeneralName()).arg("anjiang");
                sp->invoke("transfigure", transfigure_str);
                room->setPlayerProperty(sp,"general","anjiang");
                room->setPlayerProperty(sp,"kingdom","god");

                LogMessage log;
                log.type = "#BasaraGeneralChosen";
                log.arg = room->getTag(sp->objectName()).toStringList().at(0);

                if(Config.Enable2ndGeneral)
                {

                    transfigure_str = QString("%1:%2").arg(sp->getGeneral2Name()).arg("anjiang");
                    sp->invoke("transfigure", transfigure_str);
                    room->setPlayerProperty(sp,"general2","anjiang");

                    log.arg2 = room->getTag(sp->objectName()).toStringList().at(1);
                }

                sp->invoke("log",log.toString());
                sp->tag["roles"] = room->getTag(sp->objectName()).toStringList().join("+");
            }
        }

        break;
    }
    case CardEffected:{
        if(player->getPhase() == Player::NotActive){
            CardEffectStruct ces = data.value<CardEffectStruct>();
            if(ces.card)
                if(ces.card->inherits("TrickCard") ||
                        ces.card->inherits("Slash"))
                playerShowed(player);

            const ClientSkill* prohibit = room->isProhibited(ces.from,ces.to,ces.card);
            if(prohibit)
            {
                LogMessage log;
                log.type = "#SkillAvoid";
                log.from = ces.to;
                log.arg  = prohibit->objectName();
                log.arg2 = ces.card->objectName();

                room->sendLog(log);

                return true;
            }
        }
        break;
    }

    case PhaseChange:{
        if(player->getPhase() == Player::Start)
            playerShowed(player);

        break;
    }
    case Predamaged:{
        playerShowed(player);
        break;
    }
    case GameOverJudge:{
        if(Config.EnableHegemony){
            if(player->getGeneralName() == "anjiang"){
                QStringList generals = room->getTag(player->objectName()).toStringList();
                room->setPlayerProperty(player, "general", generals.at(0));
                if(Config.Enable2ndGeneral)room->setPlayerProperty(player, "general2", generals.at(1));
                room->setPlayerProperty(player, "kingdom", player->getGeneral()->getKingdom());
                room->setPlayerProperty(player, "role", getMappedRole(player->getKingdom()));
            }
        }
        break;
    }

    case Death:{
        if(Config.EnableHegemony){
            DamageStar damage = data.value<DamageStar>();
            ServerPlayer *killer = damage ? damage->from : NULL;
            if(killer && killer->getKingdom() == damage->to->getKingdom()){
                killer->throwAllEquips();
                killer->throwAllHandCards();
            }
            else if(killer && killer->isAlive()){
                killer->drawCards(3);
            }
        }

        break;
    }

    default:
        break;
    }

    return false;
}
