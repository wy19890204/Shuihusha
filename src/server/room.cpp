#include "room.h"
#include "engine.h"
#include "settings.h"
#include "standard.h"
#include "ai.h"
#include "scenario.h"
#include "gamerule.h"
#include "scenerule.h"
#include "contestdb.h"
#include "banpair.h"
#include "roomthread3v3.h"
#include "roomthread1v1.h"
#include "server.h"
#include "generalselector.h"
#include "jsonutils.h"

#include <QStringList>
#include <QMessageBox>
#include <QHostAddress>
#include <QTimer>
#include <QMetaEnum>
#include <QTimerEvent>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

using namespace QSanProtocol;
using namespace QSanProtocol::Utils;

Room::Room(QObject *parent, const QString &mode)
    :QThread(parent), mode(mode), current(NULL), pile1(Sanguosha->getRandomCards()),
    draw_pile(&pile1), discard_pile(&pile2),
    game_started(false), game_finished(false), L(NULL), thread(NULL),
    thread_3v3(NULL), sem(new QSemaphore), _m_semRaceRequest(0), _m_semRoomMutex(1),
    _m_raceStarted(false), provided(NULL), has_provided(false),
    m_surrenderRequestReceived(false), _virtual(false)
{
    player_count = Sanguosha->getPlayerCount(mode);
    scenario = Sanguosha->getScenario(mode);

    initCallbacks();

    L = CreateLuaState();
    QStringList scripts;
    scripts << "lua/sanguosha.lua" << "lua/ai/smart-ai.lua";
    DoLuaScripts(L, scripts);
}

void Room::initCallbacks(){
    // init request response pair
    m_requestResponsePair[S_COMMAND_PLAY_CARD] = S_COMMAND_USE_CARD;
    m_requestResponsePair[S_COMMAND_NULLIFICATION] = S_COMMAND_RESPONSE_CARD;
    m_requestResponsePair[S_COMMAND_SHOW_CARD] = S_COMMAND_RESPONSE_CARD;
    m_requestResponsePair[S_COMMAND_ASK_PEACH] = S_COMMAND_RESPONSE_CARD;
    m_requestResponsePair[S_COMMAND_PINDIAN] = S_COMMAND_RESPONSE_CARD;
    m_requestResponsePair[S_COMMAND_EXCHANGE_CARD] = S_COMMAND_DISCARD_CARD;
    m_requestResponsePair[S_COMMAND_CHOOSE_DIRECTION] = S_COMMAND_MULTIPLE_CHOICE;
    m_requestResponsePair[S_COMMAND_SHOW_ALL_CARDS] = S_COMMAND_SKILL_GONGXIN;

    // client request handlers
    m_callbacks[S_COMMAND_SURRENDER] = &Room::processRequestSurrender;
    m_callbacks[S_COMMAND_CHEAT] = &Room::processRequestCheat;

    // init callback table
    callbacks["arrangeCommand"] = &Room::arrangeCommand;
    callbacks["takeGeneralCommand"] = &Room::takeGeneralCommand;

    // Client notifications
    callbacks["toggleReadyCommand"] = &Room::toggleReadyCommand;
    callbacks["addRobotCommand"] = &Room::addRobotCommand;
    callbacks["fillRobotsCommand"] = &Room::fillRobotsCommand;

    callbacks["speakCommand"] = &Room::speakCommand;
    callbacks["trustCommand"] = &Room::trustCommand;
    callbacks["kickCommand"] = &Room::kickCommand;

    //Client request
    callbacks["networkDelayTestCommand"] = &Room::networkDelayTestCommand;
}

ServerPlayer *Room::getCurrent() const{
    return current;
}

void Room::setCurrent(ServerPlayer *current){
    this->current = current;
}

int Room::alivePlayerCount() const{
    return m_alivePlayers.count();
}

QList<ServerPlayer *> Room::getPlayers() const{
    return m_players;
}

QList<ServerPlayer *> Room::getAllPlayers(bool include_dead) const{ //current is start
    QList <ServerPlayer *> count_players = include_dead ? m_players : m_alivePlayers;
    if (current == NULL)
        return count_players;

    int index = count_players.indexOf(current);
    if (index == -1)
        return count_players;

    QList<ServerPlayer *> all_players;
    for (int i = index; i < count_players.length(); i++)
        all_players << count_players[i];

    for (int i = 0; i < index; i++)
        all_players << count_players[i];

    return all_players;
}

QList<ServerPlayer *> Room::getOtherPlayers(ServerPlayer *except, bool include_dead) const{
    QList<ServerPlayer *> other_players = getAllPlayers(include_dead);
    if (except && (except->isAlive() || include_dead))
        other_players.removeOne(except);
    return other_players;
}

QList<ServerPlayer *> Room::getAlivePlayers() const{
    return m_alivePlayers;
}

void Room::output(const QString &message){
    emit room_message(message);
}

void Room::outputEventStack(){
    QString msg;

    foreach(EventTriplet triplet, *thread->getEventStack()){
        msg.prepend(triplet.toString());
    }

    output(msg);
}

void Room::enterDying(ServerPlayer *player, DamageStruct *reason){
    player->setFlags("dying");

    QString sos_filename;
    if(player->getGender() == General::Male)
        sos_filename = "male-sos";
    else{
        int r = qrand() % 2 + 1;
        sos_filename = QString("female-sos%1").arg(r);
    }
    broadcastInvoke("playAudio", sos_filename);

    QList<ServerPlayer *> savers;
    ServerPlayer *current = getCurrent();
    if(current->hasSkill("wansha") && current->isAlive()){
        playSkillEffect("wansha");

        savers << current;

        LogMessage log;
        log.from = current;
        log.arg = "wansha";
        if(current != player){
            savers << player;
            log.type = "#WanshaTwo";
            log.to << player;
        }else{
            log.type = "#WanshaOne";
        }

        sendLog(log);

    }else
        savers = getAllPlayers();

    DyingStruct dying;
    dying.who = player;
    dying.damage = reason;
    dying.savers = savers;

    QVariant dying_data = QVariant::fromValue(dying);
    thread->trigger(Dying, this, player, dying_data);
}

void Room::revivePlayer(ServerPlayer *player, bool invoke_start){
    player->setAlive(true);
    broadcastProperty(player, "alive");

    m_alivePlayers.clear();
    foreach(ServerPlayer *player, m_players){
        if(player->isAlive())
            m_alivePlayers << player;
    }

    int i;
    for(i = 0; i < m_alivePlayers.length(); i++){
        m_alivePlayers.at(i)->setSeat(i+1);
        broadcastProperty(m_alivePlayers.at(i), "seat");
    }

    broadcastInvoke("revivePlayer", player->objectName());
    updateStateItem();
    setEmotion(player, "revive");

    thread->addPlayerSkills(player, invoke_start);
}

static bool CompareByRole(ServerPlayer *player1, ServerPlayer *player2){
    int role1 = player1->getRoleEnum();
    int role2 = player2->getRoleEnum();

    if(role1 != role2)
        return role1 < role2;
    else
        return player1->isAlive();
}

void Room::updateStateItem(){
    QList<ServerPlayer *> players = this->m_players;
    qSort(players.begin(), players.end(), CompareByRole);
    QString roles;
    foreach(ServerPlayer *p, players){
        QChar c = "ZCFN"[p->getRoleEnum()];
        if(p->isDead())
            c = c.toLower();

        roles.append(c);
    }

    broadcastInvoke("updateStateItem", roles);
}

void Room::killPlayer(ServerPlayer *victim, DamageStruct *reason, bool force){
    QVariant data = QVariant::fromValue(reason);
    if(!force && thread->trigger(PreDeath, this, victim, data))
        return;
    ServerPlayer *killer = reason ? reason->from : NULL;
    if(Config.ContestMode && killer)
        killer->addVictim(victim);

    victim->setAlive(false);

    int index = m_alivePlayers.indexOf(victim);
    for (int i = index + 1; i < m_alivePlayers.length(); i++) {
        ServerPlayer *p = m_alivePlayers.at(i);
        p->setSeat(p->getSeat() - 1);
        broadcastProperty(p, "seat");
    }

    m_alivePlayers.removeOne(victim);

    LogMessage log;
    log.to << victim;
	QString rol = Config.EnableHegemony ? victim->getKingdom() :
                victim->property("panxin").toBool() ? "unknown" : victim->getScreenRole();
    log.arg = rol;
    log.from = killer;

    updateStateItem();

    if(killer){
        if(killer == victim)
            log.type = "#Suicide";
        else
            log.type = "#Murder";
        setPlayerStatistics(killer, "kill", 1);
    }
    else
        log.type = "#Contingency";

    sendLog(log);

    broadcastProperty(victim, "alive");

    thread->trigger(GameOverJudge, this, victim, data);

    broadcastProperty(victim, "role");
    thread->delay(300);
    broadcastInvoke("killPlayer", victim->objectName());

    thread->trigger(Death, this, victim, data);
    victim->loseAllSkills();

    if(Config.EnableAI){
        bool expose_roles = true;
        foreach(ServerPlayer *player, m_alivePlayers){
            if(player->getState() != "robot" && player->getState() != "offline"){
                expose_roles = false;
                break;
            }
        }

        if(expose_roles){
            foreach(ServerPlayer *player, m_alivePlayers){
                if(Config.EnableHegemony){
                    QString role = player->getKingdom();
                    if(role == "god")
                        role = Sanguosha->getGeneral(getTag(player->objectName()).toStringList().at(0))->getKingdom();
                    role = BasaraMode::getMappedRole(role);
                    broadcast(QString("#%1 role %2").arg(player->objectName()).arg(role));
                }
                else
                    broadcastProperty(player, "role");
            }
        }
    }
}

void Room::judge(JudgeStruct &judge_struct){
    Q_CHECK_PTR(judge_struct.who);

    JudgeStar judge_star = &judge_struct;

    QVariant data = QVariant::fromValue(judge_star);

    thread->trigger(StartJudge, this, judge_star->who, data);

    QList<ServerPlayer *> players = getAllPlayers();
    foreach(ServerPlayer *player, players){
        if(thread->trigger(AskForRetrial, this, player, data))
            break;
    }

    thread->trigger(FinishJudge, this, judge_star->who, data);
}

void Room::sendJudgeResult(const JudgeStar judge, bool fin){
    QString who = judge->who->objectName();
    QString result = judge->isGood() ? "good" : "bad";
    if(fin)
        setEmotion(judge->who, "judge" + result);
    else
        broadcastInvoke("judgeResult", QString("%1:%2:%3").arg(who).arg(result).arg(judge->reason));
}

void Room::sendJudgeResult(ServerPlayer *moder){
    broadcastInvoke("judgeResult", moder->objectName() + ":retrial:-");
}

QList<int> Room::getNCards(int n, bool update_pile_number){
    QList<int> card_ids;
    for(int i = 0; i < n; i++){
        card_ids << drawCard();
    }

    if(update_pile_number)
        broadcastInvoke("setPileNumber", QString::number(draw_pile->length()));

    return card_ids;
}

QStringList Room::aliveRoles(ServerPlayer *except) const{
    QStringList roles;
    foreach(ServerPlayer *player, m_alivePlayers){
        if(Config.EnableReincarnation && player->property("isDead").toBool())
            continue;
        if(player != except)
            roles << player->getRole();
    }

    return roles;
}

void Room::gameOver(const QString &winner){
    QStringList all_roles;
    foreach(ServerPlayer *player, m_players)
        all_roles << player->getRole();

    game_finished = true;

    if(Config.ContestMode){
        foreach(ServerPlayer *player, m_players){
            QString screen_name = player->screenName().toUtf8().toBase64();
            broadcastInvoke("setScreenName", QString("%1:%2").arg(player->objectName()).arg(screen_name));
        }

        ContestDB *db = ContestDB::GetInstance();
        db->saveResult(m_players, winner);
    }

    //broadcastInvoke("gameOver", QString("%1:%2").arg(winner).arg(all_roles.join("+")));

    // save records
    if(Config.ContestMode){
        bool only_lord = Config.value("Contest/OnlySaveLordRecord", true).toBool();
        QString start_time = tag.value("StartTime").toDateTime().toString(ContestDB::TimeFormat);

        if(only_lord && getLord())
            getLord()->saveRecord(QString("records/%1.txt").arg(start_time));
        else{
            foreach(ServerPlayer *player, m_players){
                QString filename = QString("records/%1-%2.txt").arg(start_time).arg(player->getGeneralName());
                player->saveRecord(filename);
            }
        }

        ContestDB *db = ContestDB::GetInstance();
        if(!Config.value("Contest/Sender").toString().isEmpty())
            db->sendResult(this);
    }

    emit game_over(winner);

    if(mode.contains("_mini_"))
    {
        ServerPlayer * playerWinner = NULL;
        QStringList winners =winner.split("+");
        foreach(ServerPlayer * sp, m_players)
        {
            if(sp->getState() != "robot" &&
                (winners.contains(sp->getRole()) ||
                winners.contains(sp->objectName()))
                )
            {
                playerWinner = sp;
                break;
            }
        }

        if(playerWinner){
            QString id = Config.GameMode;
            id.replace("_mini_","");
            int stage = Config.value("MiniSceneStage", Config.S_MINI_STAGE_START).toInt();
            int current = id.toInt();
            if(stage == current && stage < Config.S_MINI_MAX_COUNT){
                Config.setValue("MiniSceneStage",current+1);
                id = QString::number(stage+1).rightJustified(2,'0');
                id.prepend("_mini_");
                Config.setValue("GameMode",id);
                Config.GameMode = id;
            }
            else if(current < Config.S_MINI_MAX_COUNT){
                id = QString::number(current + 1).rightJustified(2, '0');
                id.prepend("_mini_");
                Config.setValue("GameMode", id);
                Config.GameMode = id;
            }
        }
    }

    Json::Value arg(Json::arrayValue);
    arg[0] = toJsonString(winner);
    arg[1] = toJsonArray(all_roles);
    doBroadcastNotify(S_COMMAND_GAME_OVER, arg);
}

void Room::slashEffect(const SlashEffectStruct &effect){
    effect.from->addMark("SlashCount");

    QVariant data = QVariant::fromValue(effect);
    playExtra(SlashEffect, data);

    switch(effect.nature){
    case DamageStruct::Thunder:{
        setEmotion(effect.from, "thunder_slash");
        break;
    }
    case DamageStruct::Fire:{
        setEmotion(effect.from, "fire_slash");
        break;
    }
    default:
        if(effect.slash->isBlack())
            setEmotion(effect.from, "slash_black");
        else if(effect.slash->isRed())
            setEmotion(effect.from, "slash_red");
        else
            setEmotion(effect.from, "killer");
        break;
    }
    setEmotion(effect.to, effect.to->getGeneral()->isMale() ? "victim" : "victimf");

    setTag("LastSlashEffect", data);
    bool broken = thread->trigger(SlashEffect, this, effect.from, data);
    if(!broken)
        thread->trigger(SlashEffected, this, effect.to, data);
}

void Room::slashResult(const SlashEffectStruct &effect, const Card *jink){
    if (!effect.to->isAlive())
        return;

    SlashEffectStruct result_effect = effect;
    result_effect.jink = jink;

    QVariant data = QVariant::fromValue(result_effect);

    if(jink == NULL)
        thread->trigger(SlashHit, this, effect.from, data);
    else{
        //if (!(jink->getSkillName() == "eight_diagram")
            setEmotion(effect.to, "jink");
        thread->trigger(SlashMissed, this, effect.from, data);
    }
}

void Room::attachSkillToPlayer(ServerPlayer *player, const QString &skill_name){
    player->acquireSkill(skill_name);
    player->invoke("attachSkill", skill_name);
}

void Room::detachSkillFromPlayer(ServerPlayer *player, const QString &skill_name, bool showlog){
    if(!player->hasSkill(skill_name))
        return;

    const Skill *skill = Sanguosha->getSkill(skill_name);
    if(!skill)
        return;
    if(mode == "wheel_fight" && skill->getLocation() == Skill::Right){
        if(!player->getGeneral()->hasSkill(skill_name))
            return;
    }

    player->loseSkill(skill_name);
    broadcastInvoke("detachSkill",
        QString("%1:%2").arg(player->objectName()).arg(skill_name));

    if(skill->isVisible()){
        foreach(const Skill *skill, Sanguosha->getRelatedSkills(skill_name))
            detachSkillFromPlayer(player, skill->objectName());

        if(showlog){
            LogMessage log;
            log.type = "#LoseSkill";
            log.from = player;
            log.arg = skill_name;
            sendLog(log);
        }
    }
}

bool Room::obtainable(const Card *card, ServerPlayer *player){
    if(card == NULL)
        return false;

    if(card->isVirtualCard()){
        QList<int> subcards = card->getSubcards();
        if(subcards.isEmpty())
            return false;
    }else{
        ServerPlayer *owner = getCardOwner(card->getId());
        Player::Place place = getCardPlace(card->getId());
        if(owner == player && place == Player::Hand)
            return false;
    }

    return true;
}

bool Room::doRequest(ServerPlayer* player, QSanProtocol::CommandType command, const Json::Value &arg,
    bool moveFocus, bool wait)
{
    time_t timeOut = getCommandTimeout(command);
    return doRequest(player, command, arg, timeOut, moveFocus, wait);
}

bool Room::doRequest(ServerPlayer* player, QSanProtocol::CommandType command, const Json::Value &arg, time_t timeOut,
                        bool moveFocus, bool wait)
{
    QSanGeneralPacket packet(S_SERVER_REQUEST, command);
    packet.setMessageBody(arg);
    player->acquireLock(ServerPlayer::SEMA_MUTEX);
    player->m_isClientResponseReady = false;
    player->drainLock(ServerPlayer::SEMA_COMMAND_INTERACTIVE);
    player->setClientReply(Json::Value::null);
    player->setClientReplyString(QString());
    player->m_isWaitingReply = true;
    player->m_expectedReplySerial = packet.m_globalSerial;
    if (m_requestResponsePair.contains(command))
        player->m_expectedReplyCommand = m_requestResponsePair[command];
    else
        player->m_expectedReplyCommand = command;

    if(moveFocus)
        doBroadcastNotify(S_COMMAND_MOVE_FOCUS, toJsonString(player->objectName()));

    player->invoke(&packet);
    player->releaseLock(ServerPlayer::SEMA_MUTEX);
    if (wait) return getResult(player, timeOut);
    else return true;
}

bool Room::doBroadcastRequest(QList<ServerPlayer*> &players, QSanProtocol::CommandType command)
{
   time_t timeOut = getCommandTimeout(command);
   return doBroadcastRequest(players, command, timeOut);
}

bool Room::doBroadcastRequest(QList<ServerPlayer*> &players, QSanProtocol::CommandType command, time_t timeOut)
{
    foreach (ServerPlayer* player, players)
    {
        doRequest(player, command, player->m_commandArgs, timeOut, false, false);
    }
    QTime timer;
    time_t remainTime = timeOut;
    timer.start();
    foreach (ServerPlayer* player, players)
    {
        remainTime = timeOut - timer.elapsed();
        if (remainTime < 0) remainTime = 0;
        getResult(player, remainTime);
    }
    return true;
}

ServerPlayer* Room::doBroadcastRaceRequest(QList<ServerPlayer*> &players, QSanProtocol::CommandType command,
                                            time_t timeOut, ResponseVerifyFunction validateFunc, void* funcArg)
{
    _m_semRoomMutex.acquire();
    _m_raceStarted = true;
    _m_raceWinner = NULL;
    while (_m_semRaceRequest.tryAcquire(1)) ; //drain lock
    _m_semRoomMutex.release();
    foreach (ServerPlayer* player, players)
    {
        doRequest(player, command, player->m_commandArgs, timeOut, false, false);
    }
    return getRaceResult(players, command, timeOut, validateFunc, funcArg);
}

ServerPlayer* Room::getRaceResult(QList<ServerPlayer*> &players, QSanProtocol::CommandType , time_t timeOut,
                                    ResponseVerifyFunction validateFunc, void* funcArg)
{
    QTime timer;
    timer.start();
    bool validResult = false;
    for (int i = 0; i < players.size(); i++)
    {
        time_t timeRemain = timeOut - timer.elapsed();
        if (timeRemain < 0) timeRemain = 0;
        bool tryAcquireResult = true;
        if (Config.OperationNoLimit)
            _m_semRaceRequest.acquire();
        else
            tryAcquireResult = _m_semRaceRequest.tryAcquire(1, timeRemain);

        if (!tryAcquireResult)
            _m_semRoomMutex.tryAcquire(1);
        // So that processResponse cannot update raceWinner when we are reading it.

        if (_m_raceWinner == NULL)
        {
            _m_semRoomMutex.release();
            continue;
        }

        if (validateFunc == NULL || (_m_raceWinner->m_isClientResponseReady &&
            (this->*validateFunc)(_m_raceWinner, _m_raceWinner->getClientReply(), funcArg)))
        {
            validResult = true;
            break;
        }
        else
        {
            if (_m_raceWinner != NULL) // Don't give this player any more chance for this race
                _m_raceWinner->m_isWaitingReply = false;
            _m_raceWinner = NULL;
            _m_semRoomMutex.release();
        }
    }

    if (!validResult) _m_semRoomMutex.acquire();
    _m_raceStarted = false;
    foreach (ServerPlayer* player, players)
    {
        player->acquireLock(ServerPlayer::SEMA_MUTEX);
        player->m_expectedReplyCommand = S_COMMAND_UNKNOWN;
        player->m_isWaitingReply = false;
        player->m_expectedReplySerial = -1;
        player->releaseLock(ServerPlayer::SEMA_MUTEX);
    }
    _m_semRoomMutex.release();
    return _m_raceWinner;
}

bool Room::doNotify(ServerPlayer* player, QSanProtocol::CommandType command, const Json::Value &arg)
{
    QSanGeneralPacket packet(S_SERVER_NOTIFICATION, command);
    packet.setMessageBody(arg);
    player->invoke(&packet);
    return true;
}

bool Room::doBroadcastNotify(QSanProtocol::CommandType command, const Json::Value &arg)
{
    foreach (ServerPlayer* player, m_players)
    {
        doNotify(player, command, arg);
    }
    return true;
}

void Room::broadcastInvoke(const char *method, const QString &arg, ServerPlayer *except){
    broadcast(QString("%1 %2").arg(method).arg(arg), except);
}

void Room::broadcastInvoke(const QSanProtocol::QSanPacket* packet, ServerPlayer *except)
{
    broadcast(QString(packet->toString().c_str()), except);
}

bool Room::getResult(ServerPlayer* player, time_t timeOut){
    Q_ASSERT(player->m_isWaitingReply);
    bool validResult = false;
    player->acquireLock(ServerPlayer::SEMA_MUTEX);

    if (player->isOnline())
    {
        player->releaseLock(ServerPlayer::SEMA_MUTEX);

        if (Config.OperationNoLimit)
            player->acquireLock(ServerPlayer::SEMA_COMMAND_INTERACTIVE);
        else
            player->tryAcquireLock(ServerPlayer::SEMA_COMMAND_INTERACTIVE, timeOut) ;

        // Note that we rely on processResponse to filter out all unrelevant packet.
        // By the time the lock is released, m_clientResponse must be the right message
        // assuming the client side is not tampered.

        // Also note that lock can be released when a player switch to trust or offline status.
        // It is ensured by trustCommand and reportDisconnection that the player reports these status
        // is the player waiting the lock. In these cases, the serial number and command type doesn't matter.
        player->acquireLock(ServerPlayer::SEMA_MUTEX);
        validResult = player->m_isClientResponseReady;
    }
    player->m_expectedReplyCommand = S_COMMAND_UNKNOWN;
    player->m_isWaitingReply = false;
    player->m_expectedReplySerial = -1;
    player->releaseLock(ServerPlayer::SEMA_MUTEX);
    return validResult;
}

bool Room::askForSkillInvoke(ServerPlayer *player, const QString &skill_name, const QVariant &data){
    bool invoked = false;
    const Skill *skill = Sanguosha->getSkill(skill_name);
    if(player->property("scarecrow").toBool() && skill->getFrequency() != Skill::NotSkill)
        return false;
    AI *ai = player->getAI();
    if(ai){
        invoked = ai->askForSkillInvoke(skill_name, data);
        if(invoked)
            thread->delay(Config.AIDelay);
    }else{

        Json::Value skillCommand;
        if(data.type() == QVariant::String)
            skillCommand = toJsonArray(skill_name, data.toString());
        else
            skillCommand = toJsonArray(skill_name, QString());

        if (!doRequest(player, S_COMMAND_INVOKE_SKILL, skillCommand))
        {
            invoked = false;
        }
        else
        {
            Json::Value clientReply = player->getClientReply();
            if (clientReply.isBool())
                invoked = clientReply.asBool();
        }
    }

    if(invoked){
        if(Config.EnableSkillEmotion)
            setEmotion(player, "skill/" + skill_name);
        Json::Value msg = toJsonArray(skill_name, player->objectName());
        doBroadcastNotify(S_COMMAND_INVOKE_SKILL, msg);
    }

    QVariant decisionData = QVariant::fromValue("skillInvoke:"+skill_name+":"+(invoked ? "yes" : "no"));
    thread->trigger(ChoiceMade, this, player, decisionData);
    return invoked;
}

QString Room::askForChoice(ServerPlayer *player, const QString &skill_name, const QString &choices, const QVariant &data){
    AI *ai = player->getAI();
    QString answer;
    if(ai){
        answer = ai->askForChoice(skill_name, choices, data);
        thread->delay(Config.AIDelay);
    }
    else{
        bool success = doRequest(player, S_COMMAND_MULTIPLE_CHOICE, toJsonArray(skill_name, choices), true);
        Json::Value clientReply = player->getClientReply();
        if (!success || !clientReply.isString())
        {
            answer = ".";
            const Skill *skill = Sanguosha->getSkill(skill_name);
            if(skill)
                return skill->getDefaultChoice(player);
        }
        else answer = toQString(clientReply);
    }
    QVariant decisionData = QVariant::fromValue("skillChoice:"+skill_name+":"+answer);
    thread->trigger(ChoiceMade, this, player, decisionData);
    return answer;
}

void Room::obtainCard(ServerPlayer *target, const Card *card, bool unhide){
    if(card == NULL)
        return;

    if(card->isVirtualCard()){
        moveCardTo(card, target, Player::Hand, unhide);

    }else
        obtainCard(target, card->getId(), unhide);
}

void Room::obtainCard(ServerPlayer *target, int card_id, bool unhide){
    moveCardTo(Sanguosha->getCard(card_id), target, Player::Hand, unhide);
}

bool Room::isCanceled(const CardEffectStruct &effect){
    if(!effect.card->isCancelable(effect))
        return false;

    const TrickCard *trick = qobject_cast<const TrickCard *>(effect.card);
    if(trick){
        QVariant decisionData = QVariant::fromValue(effect.to);
        setTag("NullifyingTarget",decisionData);
        decisionData = QVariant::fromValue(effect.from);
        setTag("NullifyingSource",decisionData);
        decisionData = QVariant::fromValue(effect.card);
        setTag("NullifyingCard",decisionData);
        setTag("NullifyingTimes",0);
        return askForNullification(trick, effect.from, effect.to, true);
    }else
        return false;
}

bool Room::verifyNullificationResponse(ServerPlayer* player, const Json::Value& response, void* )
{
    const Card* card = NULL;
    if (player != NULL && response.isString())
        card = Card::Parse(toQString(response));
    return card != NULL;
}

bool Room::askForNullification(const TrickCard *trick, ServerPlayer *from, ServerPlayer *to, bool positive){
    _NullificationAiHelper aiHelper;
    aiHelper.m_from = from;
    aiHelper.m_to = to;
    aiHelper.m_trick = trick;
    return _askForNullification(trick, from, to, positive, aiHelper);
}

bool Room::_askForNullification(const TrickCard *trick, ServerPlayer *from, ServerPlayer *to, bool positive, _NullificationAiHelper aiHelper){
    QString trick_name = trick->objectName();
    QList<ServerPlayer *> validHumanPlayers;
    QList<ServerPlayer *> validAiPlayers;

    Json::Value arg(Json::arrayValue);
    arg[0] = toJsonString(trick_name);
    arg[1] = from ? toJsonString(from->objectName()) : Json::Value::null;
    arg[2] = to ? toJsonString(to->objectName()) : Json::Value::null;

    foreach (ServerPlayer *player, m_alivePlayers){
        if(player->hasNullification(trick->inherits("SingleTargetTrick") && trick->isNDTrick())){
            if (player->isOnline()){
                player->m_commandArgs = arg;
                validHumanPlayers << player;
            }
            else
                validAiPlayers << player;
        }
    }

    ServerPlayer* repliedPlayer = NULL;
    time_t timeOut = getCommandTimeout(S_COMMAND_NULLIFICATION);
    if (!validHumanPlayers.empty())
        repliedPlayer = doBroadcastRaceRequest(validHumanPlayers, S_COMMAND_NULLIFICATION, timeOut, &Room::verifyNullificationResponse);

    const Card *card = NULL;
    if (repliedPlayer != NULL && repliedPlayer->getClientReply().isString())
        card = Card::Parse(toQString(repliedPlayer->getClientReply()));
    if (card == NULL) {
        foreach (ServerPlayer *player, validAiPlayers) {
            AI *ai = player->getAI();
            if (ai == NULL) continue;
            card = ai->askForNullification(aiHelper.m_trick, aiHelper.m_from, aiHelper.m_to, positive);
            if (card != NULL) {
                thread->delay(Config.AIDelay);
                repliedPlayer = player;
                break;
            }
        }
    }

    if (card == NULL) return false;

    bool continuable = false;
    card = card->validateInResposing(repliedPlayer, &continuable);
    if (card == NULL) return false;

    CardUseStruct use;
    use.card = card;
    use.from = repliedPlayer;
    useCard(use);

    LogMessage log;
    log.type = "#NullificationDetails";
    log.from = from;
    log.to << to;
    log.arg = trick_name;
    sendLog(log);

    QString animation_str = QString(card->objectName() + ":%1:%2")
                            .arg(repliedPlayer->objectName()).arg(to->objectName());
    broadcastInvoke("animate", animation_str);
    const Card *last_trick = trick;

    QVariant decisionData = QVariant::fromValue("Nullification:"+QString(trick->getClassName())+":"+to->objectName()+":"+(positive?"true":"false"));
    thread->trigger(ChoiceMade, this, repliedPlayer, decisionData);
    setTag("NullifyingTimes",getTag("NullifyingTimes").toInt()+1);
    if(repliedPlayer->hasSkill("pozhen") ||
       (card->getSkillName() == "neiying" && card->hasSameSuit())){
        if(card->objectName() == "counterplot" && repliedPlayer->isAlive())
            repliedPlayer->obtainCard(last_trick);
        return true;
    }
    bool istrue = false;
    istrue = !_askForNullification((TrickCard*)card, repliedPlayer, to, !positive, aiHelper);
    if(istrue && card->objectName() == "counterplot")
        repliedPlayer->obtainCard(last_trick);
    return istrue;
}

int Room::askForCardChosen(ServerPlayer *player, ServerPlayer *who, const QString &flags, const QString &reason){

    //@todo: whoever wrote this had better put a explantory note here
    if(!player->hasFlag("loot") && who != player && !who->hasFlag("dongchaee")){
        if(flags == "h" || (flags == "he" && !who->hasEquip()))
            return who->getRandomHandCardId();
    }

    int card_id;

    AI *ai = player->getAI();
    if(ai){
        thread->delay(Config.AIDelay);
        card_id = ai->askForCardChosen(who, flags, reason);
    }else{
        bool success = doRequest(player, S_COMMAND_CHOOSE_CARD, toJsonArray(who->objectName(), flags, reason));
        //@todo: check if the card returned is valid
        Json::Value clientReply = player->getClientReply();
        if (!success || !clientReply.isInt()){
            // randomly choose a card
            QList<const Card *> cards = who->getCards(flags);
            int r = qrand() % cards.length();
            card_id = cards.at(r)->getId();
        }else{
            card_id = clientReply.asInt();
        }
    }

    if(card_id == -1)
        card_id = who->getRandomHandCardId();

    QVariant decisionData = QVariant::fromValue("cardChosen:"+reason+":"+QString::number(card_id));
    thread->trigger(ChoiceMade, this, player, decisionData);
    return card_id;
}

const Card *Room::askForCard(ServerPlayer *player, const QString &pattern, const QString &prompt,
                             bool is_skill, const QVariant &data, TriggerEvent trigger_event){
    if(player->isDead())
        return NULL;
    if(is_skill && player->property("scarecrow").toBool())
        return NULL;

    const Card *card = NULL;

    QVariant asked = pattern;
    if(thread->trigger(CardAsk, this, player, asked))
        return NULL;
    if(thread->trigger(CardAsked, this, player, asked))
        return NULL;
    if(has_provided){
        card = provided;
        provided = NULL;
        has_provided = false;
    }else if(pattern.startsWith("@") || !player->isNude()){
        AI *ai = player->getAI();
        if(ai){
            card = ai->askForCard(pattern, prompt, data);
            if(card)
                thread->delay(Config.AIDelay);
        }else{
            bool success = doRequest(player, S_COMMAND_RESPONSE_CARD, toJsonArray(pattern, prompt));
            Json::Value clientReply = player->getClientReply();
            if (success && !clientReply.isNull()){
                card = Card::Parse(toQString(clientReply));
            }
        }
    }

    if(card == NULL)
    {
        QVariant decisionData = QVariant::fromValue("cardResponsed:"+pattern+":"+prompt+":_"+"nil"+"_");
        thread->trigger(ChoiceMade, this, player, decisionData);
        return NULL;
    }

    bool continuable = false;
    CardUseStruct card_use;
    card_use.card = card;
    card_use.from = player;
    card = card->validateInResposing(player, &continuable);

    if(card){
        if(card->getTypeId() != Card::Skill){
            const CardPattern *card_pattern = Sanguosha->getPattern(pattern);
            if(card_pattern == NULL || card_pattern->willThrow())
                moveCardTo(card, NULL, Player::DiscardedPile, true);
                //throwCard(card, trigger_event == CardDiscarded ? player: NULL);
        }else if(card->willThrow())
            moveCardTo(card, NULL, Player::DiscardedPile, true);
            //throwCard(card, trigger_event == CardDiscarded ? player: NULL);

        if(card->getSkillName() == "spear")
            player->playCardEffect("Espear", "weapon");

        QVariant decisionData = QVariant::fromValue("cardResponsed:"+pattern+":"+prompt+":_"+card->toString()+"_");
        thread->trigger(ChoiceMade, this, player, decisionData);

        CardStar card_ptr = card;
        QVariant card_star = QVariant::fromValue(card_ptr);

        if(trigger_event == CardResponsed || trigger_event == JinkUsed){
            LogMessage log;
            log.card_str = card->toString();
            log.from = player;
            log.type = QString("#%1").arg(card->getClassName());
            sendLog(log);

            bool mute = false;
            if(card->getSkillName() == "eight_diagram" && Config.EnableEquipEffects)
                mute = true;

            player->playCardEffect(card, mute);

            if(trigger_event == JinkUsed)
                thread->trigger(CardResponsed, this, player, card_star);
        }

        thread->trigger(trigger_event, this, player, card_star);

    }else if(continuable)
        return askForCard(player, pattern, prompt);

    return card;
}

const Card *Room::askForCard(ServerPlayer *player, const QString &pattern, const QString &prompt,
                             const QVariant &data, TriggerEvent trigger_event){
    return askForCard(player, pattern, prompt, false, data, trigger_event);
}

bool Room::askForUseCard(ServerPlayer *player, const QString &pattern, const QString &prompt, bool is_skill){
    if(player->isDead())
        return NULL;
    if(is_skill && player->property("scarecrow").toBool())
        return NULL;

    QVariant asked = pattern;
    if(thread->trigger(CardUseAsk, this, player, asked))
        return NULL;

    CardUseStruct card_use;
    bool isCardUsed = false;
    AI *ai = player->getAI();
    if(ai){
        //@todo: update ai interface to use the new protocol
        QString answer = ai->askForUseCard(pattern, prompt);
        if(answer != ".")
        {
            isCardUsed = true;
            card_use.from = player;
            card_use.parse(answer, this);
            thread->delay(Config.AIDelay);
        }
    }
    else if (doRequest(player, S_COMMAND_USE_CARD, toJsonArray(pattern, prompt)))
    {
        Json::Value clientReply = player->getClientReply();
        isCardUsed = !clientReply.isNull();
        if (isCardUsed && card_use.tryParse(clientReply, this))
            card_use.from = player;
    }

    if (isCardUsed && card_use.isValid()){
        QVariant decisionData = QVariant::fromValue(card_use);
        thread->trigger(ChoiceMade, this, player, decisionData);
        useCard(card_use);
        return true;
    }else{
        QVariant decisionData = QVariant::fromValue("askForUseCard:"+pattern+":"+prompt+":nil");
        thread->trigger(ChoiceMade, this, player, decisionData);
    }

    return false;
}

int Room::askForAG(ServerPlayer *player, const QList<int> &card_ids, bool refusable, const QString &reason){

    Q_ASSERT(card_ids.length()>0);

    if(card_ids.length() == 1 && !refusable)
        return card_ids.first();

    int card_id;

    AI *ai = player->getAI();
    if(ai){
        thread->delay(Config.AIDelay);
        card_id = ai->askForAG(card_ids, refusable, reason);
    }else{
        player->invoke("disableAG", "false");
        bool success = doRequest(player, S_COMMAND_AMAZING_GRACE, refusable);
        Json::Value clientReply = player->getClientReply();
        if (!success || !clientReply.isInt() || !card_ids.contains(clientReply.asInt()))
            card_id = refusable ? -1 : card_ids.first();
        else card_id = clientReply.asInt();
    }

    QVariant decisionData = QVariant::fromValue("AGChosen:"+reason+":"+QString::number(card_id));
    thread->trigger(ChoiceMade, this, player, decisionData);

    return card_id;
}

const Card *Room::askForCardShow(ServerPlayer *player, ServerPlayer *requestor, const QString &reason){

    if(player->getHandcardNum() == 1)
        return player->getHandcards().first();

    const Card *card = NULL;

    AI *ai = player->getAI();
    if(ai)
        card = ai->askForCardShow(requestor, reason);
    else{
        bool success = doRequest(player, S_COMMAND_SHOW_CARD, toJsonString(requestor->getGeneralName()));
        Json::Value clientReply = player->getClientReply();
        if (success && clientReply.isString())
        {
            card = Card::Parse(toQString(clientReply));
        }

        if (card == NULL)
            card = player->getRandomHandCard();
    }

    QVariant decisionData = QVariant::fromValue("cardShow:" + reason + ":_" + card->toString() + "_");
    thread->trigger(ChoiceMade, this, player, decisionData);
    return card;
}

const Card *Room::askForSinglePeach(ServerPlayer *player, ServerPlayer *dying){

    //@todo: put this into AI!!!!!!!!!!!!!!!!!
    if(player->isKongcheng()){
        /* jijiu special case
        if(player->hasSkill("jijiu") && player->getPhase() == Player::NotActive){
            bool has_red = false;
            foreach(const Card *equip, player->getEquips()){
                if(equip->isRed()){
                    has_red = true;
                    break;
                }
            }

            if(!has_red)
                return NULL;
        }else if(player->hasSkill("jiushi")){
            if(!player->faceUp())
                return NULL;
        }else if(player->hasSkill("longhun")){
            bool has_heart = false;
            foreach(const Card *equip, player->getEquips()){
                if(equip->getSuit() == Card::Heart){
                    has_heart = true;
                    break;
                }
            }

            if(!has_heart)
                return NULL;
        }else*/
            return NULL;
    }

    const Card * card;
    bool continuable = false;

    AI *ai = player->getAI();
    if(ai)
        card= ai->askForSinglePeach(dying);
    else{
        int peaches = 1 - dying->getHp();
        Json::Value arg(Json::arrayValue);
        arg[0] = toJsonString(dying->objectName());
        arg[1] = peaches;
        bool success = doRequest(player, S_COMMAND_ASK_PEACH, arg);
        Json::Value clientReply = player->getClientReply();
        if (!success || !clientReply.isString()) return NULL;

        card = Card::Parse(toQString(clientReply));

        if (card != NULL)
            card = card->validateInResposing(player, &continuable);
    }
    if(card){
        QVariant decisionData = QVariant::fromValue("peach:"+
            QString("%1:%2:%3").arg(dying->objectName()).arg(1 - dying->getHp()).arg(card->toString()));
        thread->trigger(ChoiceMade, this, player, decisionData);
        return card;
    }else if(continuable)
        return askForSinglePeach(player, dying);
    else
        return NULL;
}

void Room::setPlayerFlag(ServerPlayer *player, const QString &flag){
    player->setFlags(flag);
    broadcast(QString("#%1 flags %2").arg(player->objectName()).arg(flag));
}

void Room::setPlayerProperty(ServerPlayer *player, const char *property_name, const QVariant &value){
    player->setProperty(property_name, value);
    broadcastProperty(player, property_name);

    if (strcmp(property_name, "hp") == 0)
        thread->trigger(HpChanged, this, player);

    if (strcmp(property_name, "maxhp") == 0)
        thread->trigger(MaxHpChanged, this, player);

    if (strcmp(property_name, "chained") == 0)
        thread->trigger(ChainStateChanged, this, player);
}

void Room::setPlayerMark(ServerPlayer *player, const QString &mark, int value){
    player->setMark(mark, value);
    broadcastInvoke("setMark", QString("%1.%2=%3").arg(player->objectName()).arg(mark).arg(value));
}

void Room::addPlayerHistory(ServerPlayer *player, const QString &name, int times){
    player->addHistory(name, times);
    if(times < 0)
        times = 100 - times;
    player->invoke("addHistory", QString("%1#%2").arg(name).arg(times));
}

void Room::setPlayerCardLock(ServerPlayer *player, const QString &name){
    player->setCardLocked(name);
    player->invoke("cardLock", name);
}

void Room::clearPlayerCardLock(ServerPlayer *player){
    player->setCardLocked(".");
    player->invoke("cardLock", ".");
}

void Room::setPlayerStatistics(ServerPlayer *player, const QString &property_name, const QVariant &value){
    StatisticsStruct *statistics = player->getStatistics();
    if(!statistics->setStatistics(property_name, value))
        return;

    player->setStatistics(statistics);
    QString prompt = QString("%1.%2:").arg(player->objectName()).arg(property_name);

    bool ok;
    int add = value.toInt(&ok);
    if(ok)
        prompt += QString::number(add);
    else
        prompt += value.toString();

    broadcastInvoke("setStatistics", prompt);
}

void Room::setCardFlag(const Card *card, const QString &flag, ServerPlayer *who){
    if(flag.isEmpty()) return;

    card->setFlags(flag);

    if(!card->isVirtualCard())
        setCardFlag(card->getEffectiveId(), flag, who);
}

void Room::setCardFlag(int card_id, const QString &flag, ServerPlayer *who){
    if(flag.isEmpty()) return;

    Sanguosha->getCard(card_id)->setFlags(flag);

    QString pattern = QString::number(card_id) + ":" + flag;
    if(who)
        who->invoke("setCardFlag", pattern);
    else
        broadcastInvoke("setCardFlag", pattern);
}

void Room::clearCardFlag(const Card *card, ServerPlayer *who){
    card->clearFlags();

    if(!card->isVirtualCard())
        clearCardFlag(card->getEffectiveId(), who);
}

void Room::clearCardFlag(int card_id, ServerPlayer *who){
    Sanguosha->getCard(card_id)->clearFlags();

    QString pattern = QString::number(card_id) + ":.";
    if(who)
        who->invoke("setCardFlag", pattern);
    else
        broadcastInvoke("setCardFlag", pattern);
}

ServerPlayer *Room::addSocket(ClientSocket *socket){
    ServerPlayer *player = new ServerPlayer(this);
    player->setSocket(socket);
    m_players << player;

    connect(player, SIGNAL(disconnected()), this, SLOT(reportDisconnection()));
    connect(player, SIGNAL(request_got(QString)), this, SLOT(processClientPacket(QString)));

    return player;
}

bool Room::isFull() const{
    return m_players.length() == player_count;
}

bool Room::isFinished() const{
    return game_finished;
}

bool Room::isPCConsole() const{
    int human = 0;
    foreach(ServerPlayer *robot, getAllPlayers()){
        if(robot->getState() != "robot")
            human ++;
    }
    return human <= 1;
}

int Room::getLack() const{
    return player_count - m_players.length();
}

int Room::getPlayerCount() const{
    return player_count;
}

QString Room::getMode() const{
    return mode;
}

const Scenario *Room::getScenario() const{
    return scenario;
}

void Room::broadcast(const QString &message, ServerPlayer *except){
    foreach(ServerPlayer *player, m_players){
        if(player != except){
            player->unicast(message);
        }
    }
}

void Room::swapPile(){
    if(discard_pile->isEmpty()){
        // the standoff
        gameOver(".");
    }

    int times = tag.value("SwapPile", 0).toInt();
    tag.insert("SwapPile", ++times);
    if(Config.EnableEndless){
        int time = Config.value("EndlessTimes", 3).toInt();
        if(times == time){
            QList<int> winners;
            foreach(ServerPlayer *player, getAllPlayers())
                winners << player->getMark("@endless");
            qSort(winners.begin(), winners.end(), qGreater<int>());
            QStringList winner_names;
            foreach(ServerPlayer *player, getAllPlayers())
                if(player->getMark("@endless") == winners.first())
                    winner_names << player->objectName();
            gameOver(winner_names.join("+"));
        }
    }
    else if(scenario){
        if(times == scenario->swapCount())
            gameOver(".");
    }
    else{
        if(times == Config.value("SwapCount", 6).toInt())
            gameOver(".");
    }

    qSwap(draw_pile, discard_pile);

    broadcastInvoke("clearPile");
    broadcastInvoke("setPileNumber", QString::number(draw_pile->length()));

    qShuffle(*draw_pile);

    foreach(int card_id, *draw_pile)
        setCardMapping(card_id, NULL, Player::DrawPile);
}

QList<int> Room::getDiscardPile(){
    return *discard_pile;
}

QList<int> Room::getDrawPile(){
    return *draw_pile;
}

ServerPlayer *Room::findPlayer(const QString &general_name, bool include_dead) const{
    const QList<ServerPlayer *> &list = include_dead ? m_players : m_alivePlayers;

    if(general_name.contains("+")){
        QStringList names = general_name.split("+");
        foreach(ServerPlayer *player, list){
            if(names.contains(player->getGeneralName()) || names.contains(player->objectName()))
                return player;
        }

        return NULL;
    }

    foreach(ServerPlayer *player, list){
        if(player->getGeneralName() == general_name || player->objectName() == general_name)
            return player;
    }

    return NULL;
}

QList<ServerPlayer *>Room::findPlayersByProperty(const char *key, const QVariant &value, bool include_dead) const{
    QList<ServerPlayer *> list;
    foreach(ServerPlayer *player, include_dead ? m_players : m_alivePlayers){
        if(QString(QLatin1String(key)) == "mark"){
            if(player->hasMark(key))
                list << player;
        }
        else if(QString(QLatin1String(key)) == "flag"){
            if(player->hasFlag(key))
                list << player;
        }
        if(player->property(key) == value)
            list << player;
    }
    return list;
}

QList<ServerPlayer *>Room::findPlayersBySkillName(const QString &skill_name, bool include_dead) const{
    QList<ServerPlayer *> list;
    foreach(ServerPlayer *player, include_dead ? m_players : m_alivePlayers){
        if(player->hasSkill(skill_name))
            list << player;
    }
    return list;
}

ServerPlayer *Room::findPlayerBySkillName(const QString &skill_name, bool include_dead) const{
    QList<ServerPlayer *> list = findPlayersBySkillName(skill_name, include_dead);
    if(list.isEmpty())
        return NULL;
    else
        return list.first();
}

ServerPlayer *Room::findPlayerWhohasCard(const QString &card) const{
    foreach(ServerPlayer *player, m_alivePlayers){
        if(player->isKongcheng())
            continue;
        if(player->hasCard(card))
            return player;
    }
    return NULL;
}

QList<ServerPlayer *>Room::findPlayersWhohasCard(const QString &card) const{
    QList<ServerPlayer *> list;
    foreach(ServerPlayer *player, m_alivePlayers){
        if(player->isKongcheng())
            continue;
        if(player->hasCard(card))
            list << player;
    }
    return list;
}

QList<ServerPlayer *>Room::findOnlinePlayers() const{
    QList<ServerPlayer *> list;
    foreach(ServerPlayer *player, m_alivePlayers){
        if(player->getState() == "online")
            list << player;
    }
    return list;
}

void Room::installEquip(ServerPlayer *player, const QString &equip_name){
    if(player == NULL)
        return;

    int card_id = getCardFromPile(equip_name);
    if(card_id == -1)
        return;

    moveCardTo(Sanguosha->getCard(card_id), player, Player::Equip, true);

    thread->delay(800);
}

void Room::resetAI(ServerPlayer *player){
    AI *smart_ai = player->getSmartAI();
    if(smart_ai){
        ais.removeOne(smart_ai);
        smart_ai->deleteLater();
        player->setAI(cloneAI(player));
    }
}

void Room::transfigure(ServerPlayer *player, const QString &new_general, bool full_state, bool invoke_start){
    QString general = new_general;
    if(new_general.contains(":"))
        general = new_general.split(":").last();
    if(new_general.contains("%"))
        general = new_general.split("%").last();
    if(new_general.contains("~"))
        general = new_general.split("~").last();
    if(!Sanguosha->getGeneral(general))
        return;

    if(new_general.contains(":")){
        QString object = new_general.split(":").first();
        foreach(ServerPlayer *p, m_players){
            if(p->objectName() == object){
                player = p;
                break;
            }
        }
    }
    if(new_general.contains("%")){
        QString object = new_general.split("%").first();
        foreach(ServerPlayer *p, m_players){
            if(p->objectName() == object){
                player = p;
                break;
            }
        }
        player->invoke("transfigure", player->getGeneral2Name() + ":" + general);
        setPlayerProperty(player, "general2", general);
        thread->addPlayerSkills(player, true);
        return;
    }
    if(new_general.contains("`")){
        QString object = new_general.split("`").first();
        foreach(ServerPlayer *p, m_players){
            if(p->objectName() == object){
                player = p;
                break;
            }
        }
        if(new_general.contains("`~")){
            const General *gen = Sanguosha->getGeneral(general);
            foreach(const Skill *skill, gen->getVisibleSkillList())
                acquireSkill(player, skill->objectName());
        }
        else{
            QString skill_name = new_general.split("`").last().split("~").first();
            acquireSkill(player, skill_name);
        }
        return;
    }

    LogMessage log;
    log.type = "#Transfigure";
    log.from = player;
    log.arg = general;
    sendLog(log);

    QString transfigure_str = QString("%1:%2").arg(player->getGeneralName()).arg(general);
    player->invoke("transfigure", transfigure_str);

    setPlayerProperty(player, "general", general);
    broadcastProperty(player, "general");
    thread->addPlayerSkills(player, invoke_start);

    player->setMaxHP(player->getGeneralMaxHP());
    broadcastProperty(player, "maxhp");

    if(full_state)
        player->setHp(player->getMaxHP());
    broadcastProperty(player, "hp");

    resetAI(player);
}

lua_State *Room::getLuaState() const{
    return L;
}

void Room::setFixedDistance(Player *from, const Player *to, int distance){
    QString a = from->objectName();
    QString b = to->objectName();
    QString set_str = QString("%1~%2=%3").arg(a).arg(b).arg(distance);
    from->setFixedDistance(to, distance);
    broadcastInvoke("setFixedDistance", set_str);
}

void Room::reverseFor3v3(const Card *card, ServerPlayer *player, QList<ServerPlayer *> &list){
    bool isClockwise = false;
    if(player->isOnline()){
        bool success = doRequest(player, S_COMMAND_CHOOSE_DIRECTION, Json::Value::null, true);
        Json::Value clientReply = player->getClientReply();
        if (success && clientReply.isString())
        {
            isClockwise = (clientReply.asString() == "cw");
        }
    }else{
        //@todo: nice if this thing is encapsulated in AI
        const TrickCard *trick = qobject_cast<const TrickCard *>(card);
        if(trick->isAggressive()){ //AOE
            if(mode == "06_3v3")
                isClockwise = AI::GetRelation3v3(player, player->getNextAlive()) != AI::Enemy;
            else
                isClockwise = AI::GetRelation(player, player->getNextAlive()) != AI::Enemy;
        }else{
            if(mode == "06_3v3") //Global
                isClockwise = AI::GetRelation3v3(player, player->getNextAlive()) != AI::Friend;
            else
                isClockwise = AI::GetRelation(player, player->getNextAlive()) != AI::Friend;
        }
    }

    LogMessage log;
    log.type = "#TrickDirection";
    log.from = player;
    log.arg = isClockwise ? "cw" : "ccw";
    log.arg2 = card->objectName();
    sendLog(log);

    if(isClockwise){
        QList<ServerPlayer *> new_list;

        while(!list.isEmpty())
            new_list << list.takeLast();

        if(card->inherits("GlobalEffect")){
            ServerPlayer *last = new_list.takeLast();
            //new_list.removeLast();
            new_list.prepend(last);
        }

        list = new_list;
    }
}

const ClientSkill *Room::isProhibited(const Player *from, const Player *to, const Card *card) const{
    return Sanguosha->isProhibited(from, to, card);
}

int Room::drawCard(){
    if(draw_pile->isEmpty())
        swapPile();

    return draw_pile->takeFirst();
}

QList<int> Room::drawCards(int num)
{
    QList<int> cards;
    for (int i = 0; i < num; i++)
    {
        cards.push_back(drawCard());
    }
    return cards;
}

const Card *Room::peek(){
    if(draw_pile->isEmpty())
        swapPile();

    int card_id = draw_pile->first();
    return Sanguosha->getCard(card_id);
}

void Room::prepareForStart(){
    if(scenario){
        QStringList generals, roles;
        scenario->assign(generals, roles);

        bool expose_roles = scenario->exposeRoles();
        for (int i = 0; i < m_players.length(); i++){
            ServerPlayer *player = m_players.at(i);
            if(generals.length()>0)
            {
                player->setGeneralName(generals.at(i));
                broadcastProperty(player, "general");
            }
            player->setRole(roles.at(i));

            if(player->isLord())
                broadcastProperty(player, "role");

            if(expose_roles)
                broadcastProperty(player, "role");
            else
                notifyProperty(player, player, "role");
        }
    }else if(mode == "06_3v3"){
        return;
    }else if(mode == "02_1v1"){
        if(qrand() % 2 == 0)
            m_players.swap(0, 1);

        m_players.at(0)->setRole("lord");
        m_players.at(1)->setRole("renegade");

        for(int i = 0; i < 2; i++){
            broadcastProperty(m_players.at(i), "role");
        }
    }else if(Config.value("Cheat/FreeAssign", false).toBool()){
        ServerPlayer *owner = getOwner();
        if(owner && owner->isOnline()){
            bool success = doRequest(owner, S_COMMAND_CHOOSE_ROLE, Json::Value::null);
            //executeCommand(owner, "askForAssign", "assignRolesCommand", ".", ".");
            Json::Value clientReply = owner->getClientReply();
            if(!success || !clientReply.isArray() || clientReply.size() != 2){
                if(Config.RandomSeat)
                    qShuffle(m_players);
                assignRoles();
            }else if (Config.FreeAssignSelf){
                QString name = toQString(clientReply[0][0]);
                QString role = toQString(clientReply[1][0]);
                ServerPlayer *player_self = findChild<ServerPlayer *>(name);
                setPlayerProperty(player_self, "role", role);
                if(role == "lord")
                    broadcastProperty(player_self, "role", "lord");

                QList<ServerPlayer *> all_players = m_players;
                all_players.removeOne(player_self);
                int n = all_players.count(), i;
                QStringList roles = Sanguosha->getRoleList(mode);
                roles.removeOne(role);
                qShuffle(roles);

                for(i = 0; i < n; i++){
                    ServerPlayer *player = all_players[i];
                    QString role = roles.at(i);

                    player->setRole(role);
                    if(role == "lord")
                        broadcastProperty(player, "role", "lord");
                    else
                        notifyProperty(player, player, "role");
                }
            }
            else{
                for(unsigned int i = 0; i < clientReply[0].size(); i++){
                    QString name = toQString(clientReply[0][i]);
                    QString role = toQString(clientReply[1][i]);

                    ServerPlayer *player = findChild<ServerPlayer *>(name);
                    setPlayerProperty(player, "role", role);

                    m_players.swap(i, m_players.indexOf(player));
                }
            }
        }else{
            if(Config.RandomSeat)
                qShuffle(m_players);
            assignRoles();
        }
    }else{
        if(Config.RandomSeat)
            qShuffle(m_players);
        assignRoles();
    }

    adjustSeats();
}

void Room::reportDisconnection(){
    ServerPlayer *player = qobject_cast<ServerPlayer*>(sender());

    if(player == NULL)
        return;

    // send disconnection message to server log
    emit room_message(player->reportHeader() + tr("disconnected"));

    // the 4 kinds of circumstances
    // 1. Just connected, with no object name : just remove it from player list
    // 2. Connected, with an object name : remove it, tell other clients and decrease signup_count
    // 3. Game is not started, but role is assigned, give it the default general(general2) and others same with fourth case
    // 4. Game is started, do not remove it just set its state as offline
    // all above should set its socket to NULL

    player->setSocket(NULL);

    if(player->objectName().isEmpty()){
        // first case
        player->setParent(NULL);
        m_players.removeOne(player);
    }else if(player->getRole().isEmpty()){
        // second case
        if(m_players.length() < player_count){
            player->setParent(NULL);
            m_players.removeOne(player);

            if(player->getState() != "robot"){
                QString screen_name = Config.ContestMode ? tr("Contestant") : player->screenName();
                QString leaveStr = tr("<font color=#000000>Player <b>%1</b> left the game</font>").arg(screen_name);
                speakCommand(player, leaveStr.toUtf8().toBase64());
            }

            broadcastInvoke("removePlayer", player->objectName());
        }
    }else{
        // fourth case
        if (player->m_isWaitingReply)
        {
            player->releaseLock(ServerPlayer::SEMA_COMMAND_INTERACTIVE);
        }
        setPlayerProperty(player, "state", "offline");

        bool someone_is_online = false;
        foreach(ServerPlayer *player, m_players){
            if(player->getState() == "online" || player->getState() == "trust"){
                someone_is_online = true;
                break;
            }
        }

        if(!someone_is_online){
            game_finished = true;
            return;
        }
    }

    if(player->isOwner()){
        foreach(ServerPlayer *p, m_players){
            if(p->getState() == "online"){
                p->setOwner(true);
                broadcastProperty(p, "owner");
                break;
            }
        }
    }
}

void Room::trustCommand(ServerPlayer *player, const QString &){
    player->acquireLock(ServerPlayer::SEMA_MUTEX);
    if (player->isOnline()){
        player->setState("trust");
        if (player->m_isWaitingReply) {
            player->releaseLock(ServerPlayer::SEMA_MUTEX);
            player->releaseLock(ServerPlayer::SEMA_COMMAND_INTERACTIVE);
        }
    }else
    {
        player->setState("online");
    }
    player->releaseLock(ServerPlayer::SEMA_MUTEX);
    broadcastProperty(player, "state");
}

bool Room::processRequestCheat(ServerPlayer *player, const QSanProtocol::QSanGeneralPacket *packet)
{
    if (!Config.value("Cheat/EnableCheatMenu", false).toBool()) return false;
    Json::Value arg = packet->getMessageBody();
    if (!arg.isArray() || !arg[0].isInt()) return false;
    //@todo: synchronize this
    player->m_cheatArgs = arg;
    player->releaseLock(ServerPlayer::SEMA_COMMAND_INTERACTIVE);
    if(Config.EnableCheatRing)
        broadcastInvoke("playAudio", "cheat");
    setPlayerStatistics(player, "cheat", 1);
    return true;
}

bool Room::makeSurrender(ServerPlayer* initiator)
{
    bool loyalGiveup = true; int loyalAlive = 0;
    bool renegadeGiveup = true; int renegadeAlive = 0;
    bool rebelGiveup = true; int rebelAlive = 0;

    // broadcast polling request
    QList<ServerPlayer*> playersAlive;
    foreach(ServerPlayer *player, m_players)
    {
        QString playerRole = player->getRole();
        if ((playerRole == "loyalist" || playerRole == "lord") && player->isAlive()) loyalAlive++;
        else if (playerRole == "rebel" && player->isAlive()) rebelAlive++;
        else if (playerRole == "renegade" && player->isAlive()) renegadeAlive++;
        if (player != initiator && player->isAlive() && player->isOnline())
        {
            player->m_commandArgs = toJsonString(initiator->getGeneral()->objectName());
            playersAlive << player;
        }
    }
    doBroadcastRequest(playersAlive, S_COMMAND_SURRENDER);

    // collect polls
    foreach (ServerPlayer* player, playersAlive)
    {
        bool result = false;
        if (!player->m_isClientResponseReady
            || !player->getClientReply().isBool())
            result = !player->isOnline();
        else
            result = player->getClientReply().asBool();

        QString playerRole = player->getRole();
        if (playerRole == "loyalist" || playerRole == "lord")
        {
            loyalGiveup &= result;
            if (player->isAlive()) loyalAlive++;
        }
        else if (playerRole == "rebel")
        {
            rebelGiveup &= result;
            if (player->isAlive()) rebelAlive++;
        }
        else if (playerRole == "renegade")
        {
            renegadeGiveup &= result;
            if (player->isAlive()) renegadeAlive++;
        }
    }

    // vote counting
    if (loyalGiveup && renegadeGiveup && !rebelGiveup)
        gameOver("rebel");
    else if (loyalGiveup && !renegadeGiveup && rebelGiveup)
        gameOver("renegade");
    else if (!loyalGiveup && renegadeGiveup && rebelGiveup)
        gameOver("lord+loyalist");
    else if (loyalGiveup && renegadeGiveup && rebelGiveup)
    {
        // if everyone give up, then ensure that the initiator doesn't win.
        QString playerRole = initiator->getRole();
        if (playerRole == "lord" || playerRole == "loyalist")
        {
            gameOver(renegadeAlive >= rebelAlive ? "renegade" : "rebel");
        }
        else if (playerRole == "renegade")
        {
            gameOver(loyalAlive >= rebelAlive ? "loyalist+lord" : "rebel");
        }
        else if (playerRole == "rebel")
        {
            gameOver(renegadeAlive >= loyalAlive ? "renegade" : "loyalist+lord");
        }
    }

    return true;
}

bool Room::processRequestSurrender(ServerPlayer *player, const QSanProtocol::QSanGeneralPacket *)
{
    //@todo: Strictly speaking, the client must be in the PLAY phase
    //@todo: return false for 3v3 and 1v1!!!
    if (player == NULL || !player->m_isWaitingReply)
        return false;
    if (!_m_isFirstSurrenderRequest
        && _m_timeSinceLastSurrenderRequest.elapsed() <= Config.S_SURRNDER_REQUEST_MIN_INTERVAL)
    {
        //@todo: warn client here after new protocol has been enacted on the warn request
        return false;
    }
    _m_isFirstSurrenderRequest = false;
    _m_timeSinceLastSurrenderRequest.restart();
    m_surrenderRequestReceived = true;
    player->releaseLock(ServerPlayer::SEMA_COMMAND_INTERACTIVE);
    return true;
}

void Room::processClientPacket(const QString &request){
    QSanGeneralPacket packet;
    //@todo: remove this thing after the new protocol is fully deployed
    if (packet.parse(request.toAscii().constData()))
    {
        ServerPlayer *player = qobject_cast<ServerPlayer*>(sender());
        if (packet.getPacketType() == S_CLIENT_REPLY)
        {
            if (player == NULL) return;
            player->setClientReplyString(request);
            processResponse(player, &packet);
        }
        //@todo: make sure that cheat is binded to Config.FreeChoose, better make
        // a seperate variable called EnableCheat
        else if (packet.getPacketType() == S_CLIENT_REQUEST)
        {
            CallBack callback = m_callbacks[packet.getCommandType()];
            if (!callback) return;
            (this->*callback)(player, &packet);
        }
    }
    else
    {
        QStringList args = request.split(" ");
        QString command = args.first();
        ServerPlayer *player = qobject_cast<ServerPlayer*>(sender());
        if(player == NULL)
            return;

        if(game_finished){
            if (player->isOnline())
                player->invoke("warn", "GAME_OVER");
            return;
        }

        command.append("Command");
        Callback callback = callbacks.value(command, NULL);
        if(callback){

            (this->*callback)(player, args.at(1));

#ifndef QT_NO_DEBUG
            // output client command only in debug version
            emit room_message(player->reportHeader() + request);
#endif

        }else
            emit room_message(tr("%1: %2 is not invokable").arg(player->reportHeader()).arg(command));
    }
}

void Room::addRobotCommand(ServerPlayer *player, const QString &){
    if(player && !player->isOwner())
        return;

    if(isFull())
        return;

    QString robot_name;
    if(!Config.value("AINames", true).toBool()){
        int n = 0;
        foreach(ServerPlayer *player, m_players){
            if(player->getState() == "robot")
                n++;
        }
        robot_name = tr("Computer %1").arg(QChar('A' + n));
    }

    ServerPlayer *robot = new ServerPlayer(this);
    robot->setState("robot");

    m_players << robot;

    if(Config.value("AINames", true).toBool()){
        QStringList robot_names = GetConfigFromLuaState(Sanguosha->getLuaState(), "ai_names").toStringList();
        foreach(ServerPlayer *p, m_players){
            if(robot_names.contains(p->screenName()))
                robot_names.removeOne(p->screenName());
        }
        robot_name = robot_names.at(qrand() % robot_names.length());
    }
    const QString robot_avatar = Sanguosha->getRandomGeneralName();
    signup(robot, robot_name, robot_avatar, true);

    if(!mode.contains("_mini_")){
        QString greeting = tr("Hello, I'm a robot").toUtf8().toBase64();
        speakCommand(robot, greeting);
    }

    broadcastProperty(robot, "state");
}

void Room::fillRobotsCommand(ServerPlayer *player, const QString &){
    int left = player_count - m_players.length();
    for(int i=0; i<left; i++){
        addRobotCommand(player, QString());
    }
}

ServerPlayer *Room::getOwner() const{
    foreach(ServerPlayer *player, m_players){
        if(player->isOwner())
            return player;
    }

    return NULL;
}

void Room::toggleReadyCommand(ServerPlayer *player, const QString &){
    if(game_started)
        return;

    setPlayerProperty(player, "ready", ! player->isReady());

    if(player->isReady() && isFull()){
        bool allReady = true;
        foreach(ServerPlayer *player, m_players){
            if(!player->isReady()){
                allReady = false;
                break;
            }
        }

        if(allReady){
            foreach(ServerPlayer *player, m_players)
                setPlayerProperty(player, "ready", false);

            start();
        }
    }
}

void Room::signup(ServerPlayer *player, const QString &screen_name, const QString &avatar, bool is_robot){
    player->setObjectName(generatePlayerName());
    player->setProperty("avatar", avatar);
    player->setScreenName(screen_name);

    if(Config.ContestMode)
        player->startRecord();

    if(!is_robot){
        notifyProperty(player, player, "objectName");

        ServerPlayer *owner = getOwner();
        if(owner == NULL){
            player->setOwner(true);
            notifyProperty(player, player, "owner");
        }
    }

    // introduce the new joined player to existing players except himself
    player->introduceTo(NULL);

    if(!is_robot){
        if(!mode.contains("_mini_")){
            QString greetingStr = tr("<font color=#EEB422>Player <b>%1</b> joined the game</font>")
                .arg(Config.ContestMode ? tr("Contestant") : screen_name);
            speakCommand(player, greetingStr.toUtf8().toBase64());
            player->startNetworkDelayTest();
        }

        // introduce all existing player to the new joined
        foreach(ServerPlayer *p, m_players){
            if(p != player)
                p->introduceTo(player);
        }
    }else
        toggleReadyCommand(player, QString());
}

void Room::assignGeneralsForPlayers(const QList<ServerPlayer *> &to_assign){

    QSet<QString> existed;
    foreach(ServerPlayer *player, m_players){
        if(player->getGeneral())
            existed << player->getGeneralName();

        if(player->getGeneral2())
            existed << player->getGeneral2Name();
    }

    const int max_choice = (Config.EnableHegemony && Config.Enable2ndGeneral) ? 5
        : Config.value("MaxChoice", 5).toInt();
    const int total = Sanguosha->getGeneralCount();
    const int max_available = (total-existed.size()) / to_assign.length();
    const int choice_count = qMin(max_choice, max_available);

    QStringList choices = Sanguosha->getRandomGenerals(total-existed.size(), existed);

    if(Config.EnableHegemony)
    {
        if(to_assign.first()->getGeneral())
        {
            foreach(ServerPlayer *sp,m_players)
            {
                QStringList old_list = sp->getSelected();
                sp->clearSelected();
                QString choice;

                //keep legal generals
                foreach(QString name, old_list){
                    if(Sanguosha->getGeneral(name)->getKingdom() != sp->getGeneral()->getKingdom()
                       || sp->findReasonable(old_list, true) == name){
                        sp->addToSelected(name);
                        old_list.removeOne(name);
                    }
                }

                //drop the rest and add new generals
                while(old_list.length())
                {
                    choice = sp->findReasonable(choices);
                    sp->addToSelected(choice);
                    old_list.pop_front();
                    choices.removeOne(choice);
                }

            }
            return;
        }
    }


    foreach(ServerPlayer *player, to_assign){
        player->clearSelected();

        for(int i=0; i<choice_count; i++){
            QString choice = player->findReasonable(choices);
            player->addToSelected(choice);
            choices.removeOne(choice);
        }
    }
}

time_t Room::getCommandTimeout(QSanProtocol::CommandType command)
{
    if (Config.OperationNoLimit) return UINT_MAX;
    else if (command == S_COMMAND_CHOOSE_GENERAL)
    {
        return (Config.S_CHOOSE_GENERAL_TIMEOUT + 1) * 1000;
    }
    else if (command == S_COMMAND_SKILL_GUANXING)
    {
        return (Config.S_GUANXING_TIMEOUT + 1) * 1000;
    }
    else
    {
        return (Config.OperationTimeout + 1) * 1000;
    }
}

void Room::chooseGenerals(){

    // for lord.
    const int nonlord_prob = 5;
    if(!Config.EnableHegemony){
        QStringList lord_list;
        ServerPlayer *the_lord = getLord();
        if(Config.EnableSame){
            lord_list = Sanguosha->getRandomGenerals(Config.value("MaxChoice", 5).toInt());
            the_lord = getOwner();
        }
        else if(the_lord && the_lord->getState() == "robot")
            if(qrand()%100 < nonlord_prob)
                lord_list = Sanguosha->getRandomGenerals(1);
            else
                lord_list = Sanguosha->getLords();
        else
            lord_list = Sanguosha->getRandomLords();
        if(Config.EnableAnzhan)
            lord_list = Sanguosha->getRandomGenerals(Config.value("MaxChoice", 3).toInt());

        if(scenario){
            int sc = scenario->lordGeneralCount();
            if(sc > 0)
                lord_list = Sanguosha->getRandomGenerals(sc);
        }

        QString general = askForGeneral(the_lord, lord_list);
        the_lord->setGeneralName(general);

        if(!Config.EnableBasara)
            broadcastProperty(the_lord, "general", general);
        if(Config.EnableSame){ //@todo: crash and the lord different
            foreach(ServerPlayer *p, m_players)
                if(p != the_lord)
                    p->setGeneralName(general);
            if(Config.Enable2ndGeneral){
                QStringList bans;
                bans << general;
                QSet<QString> banset = bans.toSet();
                lord_list = Sanguosha->getRandomGenerals(Config.value("MaxChoice", 5).toInt(), banset);
                general = askForGeneral(the_lord, lord_list);
                foreach(ServerPlayer *p, m_players)
                    p->setGeneral2Name(general);
            }
            //getLord()->setGeneralName(getOwner()->getGeneralName());
            return;
        }
    }
    QList<ServerPlayer *> to_assign = m_players;
    if(!Config.EnableHegemony)to_assign.removeOne(getLord());
    assignGeneralsForPlayers(to_assign);
    foreach(ServerPlayer *player, to_assign){
        _setupChooseGeneralRequestArgs(player);
    }
    doBroadcastRequest(to_assign, S_COMMAND_CHOOSE_GENERAL);
    foreach (ServerPlayer *player, to_assign)
    {
        if (player->getGeneral() != NULL) continue;
        Json::Value generalName = player->getClientReply();
        if (!player->m_isClientResponseReady || !generalName.isString()
            || !_setPlayerGeneral(player, toQString(generalName), true))
            _setPlayerGeneral(player, _chooseDefaultGeneral(player), true);
    }

    if(Config.Enable2ndGeneral){
        QList<ServerPlayer *> to_assign = m_players;
        assignGeneralsForPlayers(to_assign);
        foreach(ServerPlayer *player, to_assign){
            _setupChooseGeneralRequestArgs(player);
        }
        doBroadcastRequest(to_assign, S_COMMAND_CHOOSE_GENERAL);
        foreach(ServerPlayer *player, to_assign){
            if (player->getGeneral2() != NULL) continue;
            Json::Value generalName = player->getClientReply();
            if (!player->m_isClientResponseReady || !generalName.isString()
                || !_setPlayerGeneral(player, toQString(generalName), false))
                _setPlayerGeneral(player, _chooseDefaultGeneral(player), false);
        }
    }


    if(Config.EnableBasara)
    {
        foreach(ServerPlayer *player, m_players)
        {
            QStringList names;
            if(player->getGeneral()){
                QString name = player->getGeneralName();
                names.append(name);
                player->setGeneralName("anjiang");
                notifyProperty(player, player, "general");
            }
            if(player->getGeneral2() && Config.Enable2ndGeneral){
                QString name = player->getGeneral2Name();
                names.append(name);
                player->setGeneral2Name("anjiang");
                notifyProperty(player, player, "general2");
            }
            this->setTag(player->objectName(),QVariant::fromValue(names));
        }
    }
}

void Room::run(){
    // initialize random seed for later use
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    foreach (ServerPlayer *player, m_players){
        //Ensure that the game starts with all player's mutex locked
        player->drainAllLocks();
        player->releaseLock(ServerPlayer::SEMA_MUTEX);
    }

    prepareForStart();

    bool using_countdown = true;
    if(_virtual || !property("to_test").toString().isEmpty())
        using_countdown = false;

#ifndef QT_NO_DEBUG
    using_countdown = false;
#endif

    if(using_countdown){
        for(int i = Config.CountDownSeconds; i>=0; i--){
            broadcastInvoke("startInXs", QString::number(i));
            sleep(1);
        }
    }else
        broadcastInvoke("startInXs", "0");

    if(scenario){
        if(scenario->generalSelection(this))
            chooseGenerals();
        startGame();
    }
    else if(mode == "06_3v3"){
        thread_3v3 = new RoomThread3v3(this);
        thread_3v3->start();

        connect(thread_3v3, SIGNAL(finished()), this, SLOT(startGame()));
        connect(thread_3v3, SIGNAL(finished()), thread_3v3, SLOT(deleteLater()));
    }else if(mode == "02_1v1"){
        thread_1v1 = new RoomThread1v1(this);
        thread_1v1->start();

        connect(thread_1v1, SIGNAL(finished()), this, SLOT(startGame()));
        connect(thread_1v1, SIGNAL(finished()), thread_1v1, SLOT(deleteLater()));
    }else{
        chooseGenerals();
        startGame();
    }
}

void Room::assignRoles(){
    int n = m_players.count(), i;

    QStringList roles = Sanguosha->getRoleList(mode);
    qShuffle(roles);

    for(i = 0; i < n; i++){
        ServerPlayer *player = m_players[i];
        QString role = roles.at(i);

        player->setRole(role);

        if(role == "lord")
            broadcastProperty(player, "role", "lord");
        else
            player->sendProperty("role");
    }
}

void Room::doSwap(){
    QStringList player_circle;
    foreach(ServerPlayer *player, m_players)
        player_circle << player->objectName();
    broadcastInvoke("arrangeSeats", player_circle.join("+"));

    m_alivePlayers.clear();
    for (int i = 0; i < m_players.length(); i++) {
        ServerPlayer *player = m_players.at(i);
        if(player->isAlive()){
            m_alivePlayers << player;
            player->setSeat(m_alivePlayers.length());
        }else{
            player->setSeat(0);
        }

        broadcastProperty(player, "seat");

        player->setNext(m_players.at((i+1) % m_players.length()));
    }
}

void Room::swapSeat(ServerPlayer *a, ServerPlayer *b){
    int seat1 = m_players.indexOf(a);
    int seat2 = m_players.indexOf(b);

    m_players.swap(seat1, seat2);
    doSwap();
}

void Room::jumpSeat(ServerPlayer *a, ServerPlayer *b, int flag){
    //flag = 0: before; flag = 1: next
    int seat1 = m_players.indexOf(a);
    int seat2 = m_players.indexOf(b);
    if(seat1 > seat2){
        int tmp = seat1;
        seat1 = seat2;
        seat2 = tmp;
    }
    m_players.move(seat1, seat2 + (flag-1));
    doSwap();
}

void Room::swapHandcards(ServerPlayer *source, ServerPlayer *target){
    DummyCard *card1 = source->wholeHandCards();
    DummyCard *card2 = target->wholeHandCards();
    if(card1){
        obtainCard(target, card1, false);
        delete card1;
    }
    thread->delay();
    if(card2){
        obtainCard(source, card2, false);
        delete card2;
    }
}

void Room::adjustSeats(){
    int i;
    for(i=0; i<m_players.length(); i++){
        if(m_players.at(i)->getRoleEnum() == Player::Lord){
            m_players.swap(0, i);
            break;
        }
    }

    if(mode == "changban"){
        for(i = 0; i < m_players.length(); i++){
            if(m_players.at(i)->getRoleEnum() == Player::Loyalist){
                m_players.swap(1, i);
                break;
            }
        }
    }

    for(i=0; i<m_players.length(); i++)
        m_players.at(i)->setSeat(i+1);

    // tell the players about the seat, and the first is always the lord
    QStringList player_circle;
    foreach(ServerPlayer *player, m_players)
        player_circle << player->objectName();

    broadcastInvoke("arrangeSeats", player_circle.join("+"));
}

int Room::getCardFromPile(const QString &card_pattern){
    if(draw_pile->isEmpty())
        swapPile();

    if(card_pattern.startsWith("@")){
        return -1;
    }else{
        QString card_name = card_pattern;
        foreach(int card_id, *draw_pile){
            const Card *card = Sanguosha->getCard(card_id);
            if(card->objectName() == card_name)
                return card_id;
        }
    }

    return -1;
}

QString Room::_chooseDefaultGeneral(ServerPlayer *player) const{
    Q_ASSERT(!player->getSelected().isEmpty());
    if (Config.EnableHegemony && Config.Enable2ndGeneral) {
        foreach (QString name, player->getSelected()) {
            Q_ASSERT(!name.isEmpty());
            if (player->getGeneral() != NULL) { // choosing first general
                if (name == player->getGeneralName()) continue;
                if (Sanguosha->getGeneral(name)->getKingdom()
                    == player->getGeneral()->getKingdom())
                    return name;
            } else {
                foreach(QString other,player->getSelected()) { // choosing second general
                    if (name == other) continue;
                    if(Sanguosha->getGeneral(name)->getKingdom()
                        == Sanguosha->getGeneral(other)->getKingdom())
                        return name;
                }
            }
        }
        Q_ASSERT(false);
        return QString();
    } else {
        GeneralSelector *selector = GeneralSelector::GetInstance();
        QString choice = selector->selectFirst(player, player->getSelected());
        return choice;
    }
}

bool Room::_setPlayerGeneral(ServerPlayer *player, const QString &generalName, bool isFirst){
    const General *general = Sanguosha->getGeneral(generalName);
    if(general == NULL)
        return false;
    else if(!Config.FreeChooseGenerals && !player->getSelected().contains(generalName))
        return false;
    if(isFirst){
        player->setGeneralName(general->objectName());
        notifyProperty(player, player, "general");
    }else{
        player->setGeneral2Name(general->objectName());
        notifyProperty(player, player, "general2");
    }
    return true;
}

void Room::speakCommand(ServerPlayer *player, const QString &arg){
    broadcastInvoke("speak", QString("%1:%2").arg(player->objectName()).arg(arg));
}

void Room::processResponse(ServerPlayer *player, const QSanGeneralPacket *packet){
    player->acquireLock(ServerPlayer::SEMA_MUTEX);
    bool success = false;
    if (player == NULL)
    {
        emit room_message(tr("Unable to parse player"));
    }
    else if (!player->m_isWaitingReply || player->m_isClientResponseReady)
    {
        emit room_message(tr("Server is not waiting for reply from %1").arg(player->objectName()));
    }
    else if (packet->getCommandType() != player->m_expectedReplyCommand)
    {
        emit room_message(tr("Reply command should be %1 instead of %2")
            .arg(player->m_expectedReplyCommand).arg(packet->getCommandType()));
    }
    else if ((int)packet->m_localSerial != player->m_expectedReplySerial)
    {
        emit room_message(tr("Reply serial should be %1 instead of %2")
            .arg(player->m_expectedReplySerial).arg((int)packet->m_localSerial));
    }
    else success = true;

    if (!success)
    {
        player->releaseLock(ServerPlayer::SEMA_MUTEX);
        return;
    }
    else
    {
        _m_semRoomMutex.acquire();
        if (_m_raceStarted)
        {

            player->setClientReply(packet->getMessageBody());
            player->m_isClientResponseReady = true;
            // Warning: the statement below must be the last one before releasing the lock!!!
            // Any statement after this statement will totally compromise the synchronization
            // because getRaceResult will then be able to acquire the lock, reading a non-null
            // raceWinner and proceed with partial data. The current implementation is based on
            // the assumption that the following line is ATOMIC!!!
            // @todo: Find a Qt atomic semantic or use _asm to ensure the following line is atomic
            // on a multi-core machine. This is the core to the whole synchornization mechanism for
            // broadcastRaceRequest.
            _m_raceWinner = player;
            // the _m_semRoomMutex.release() signal is in getRaceResult();
            _m_semRaceRequest.release();
        }
        else
        {
            _m_semRoomMutex.release();
            player->setClientReply(packet->getMessageBody());
            player->m_isClientResponseReady = true;
            player->releaseLock(ServerPlayer::SEMA_COMMAND_INTERACTIVE);
        }

        player->releaseLock(ServerPlayer::SEMA_MUTEX);
    }
}

void Room::useCard(const CardUseStruct &use, bool add_history){
    CardUseStruct card_use = use;
    const Card *card = card_use.card;
    if(card_use.from->getPhase() == Player::Play && add_history){
        QString key;
        if(card->inherits("LuaSkillCard"))
            key = "#" + card->objectName();
        else
            key = card->getClassName();

        bool slash_record =
            key.contains("Slash") &&
            card_use.from->getSlashCount() > 0 &&
            card_use.from->hasWeapon("crossbow");

        if(!slash_record){
            card_use.from->addHistory(key);
            card_use.from->invoke("addHistory", key);
        }

        broadcastInvoke("addHistory","pushPile");
    }

    card = card_use.card->validate(&card_use);
    if(card == card_use.card)
        card_use.card->onUse(this, card_use);
    else if(card){
        CardUseStruct new_use = card_use;
        new_use.card = card;
        useCard(new_use);
    }

    /*
    if(card->isVirtualCard())
    delete card;
    */
}

void Room::addHpSlot(ServerPlayer *victim, int number){
    setPlayerProperty(victim, "maxhp", victim->getMaxHp() + number);
    setPlayerProperty(victim, "hp", victim->getHp() + number);
}

void Room::loseHp(ServerPlayer *victim, int lose){
    if (victim->isDead())
        return;
    QVariant data = lose;
    thread->trigger(HpLost, this, victim, data);
}

void Room::loseMaxHp(ServerPlayer *victim, int lose){
    int hp = victim->getHp();
    int maxhp = qMax(victim->getMaxHp() - lose, 0);
    victim->setMaxHp(maxhp);

    bool hp_changed = hp - victim->getHp() != 0;

    //broadcastInvoke("playAudio", "damage/maxhplost");

    setPlayerProperty(victim, "maxhp", maxhp);

    if(hp_changed)
        setPlayerProperty(victim, "hp", victim->getHp());

    LogMessage log;
    log.type = !hp_changed ? "#LoseMaxHp" : "#LostMaxHpPlus";
    log.from = victim;
    log.arg = QString::number(lose);
    log.arg2 = QString::number(hp - victim->getHp());
    sendLog(log);

    broadcastInvoke("maxhpChange", QString("%1:%2").arg(victim->objectName()).arg(-lose));

    if(victim->getMaxHp() == 0)
        killPlayer(victim);
}

void Room::applyDamage(ServerPlayer *victim, const DamageStruct &damage){
    int new_hp = victim->getHp() - damage.damage;

    setPlayerProperty(victim, "hp", new_hp);
    QString change_str = QString("%1:%2").arg(victim->objectName()).arg(-damage.damage);
    switch(damage.nature){
    case DamageStruct::Fire: change_str.append("F"); break;
    case DamageStruct::Thunder: change_str.append("T"); break;
    default: break;
    }

    broadcastInvoke("hpChange", change_str);
}

void Room::recover(ServerPlayer *player, const RecoverStruct &recover, bool set_emotion){
    if(player->getLostHp() == 0 || player->isDead())
        return;

    QVariant data = QVariant::fromValue(recover);
    thread->trigger(HpRecover, this, player, data);

    if(set_emotion)
        setEmotion(player, "recover");

    thread->trigger(HpRecovered, this, player, data);
}

bool Room::cardEffect(const Card *card, ServerPlayer *from, ServerPlayer *to){
    CardEffectStruct effect;

    effect.card = card;
    effect.from = from;
    effect.to = to;

    return cardEffect(effect);
}

bool Room::cardEffect(const CardEffectStruct &effect){
    if(effect.to->isDead())
        return false;

    QVariant data = QVariant::fromValue(effect);
    bool broken = false;
    if(effect.from)
        broken = thread->trigger(CardEffect, this, effect.from, data);

    if(broken)
        return false;

    return !thread->trigger(CardEffected, this, effect.to, data);
}

void Room::damage(const DamageStruct &damage_data){
    if(damage_data.to == NULL)
        return;

    if(damage_data.to->isDead())
        return;

    QVariant data = QVariant::fromValue(damage_data);

    if(!damage_data.chain && damage_data.from){
        // predamage
        if(thread->trigger(Predamage, this, damage_data.from, data))
            return;
    }

    do{
        // DamagedProceed
        bool prevent = thread->trigger(DamagedProceed, this, damage_data.to, data);
        if(prevent)
            break;

        // DamageProceed
        if(damage_data.from){
            if(thread->trigger(DamageProceed, this, damage_data.from, data))
                break;
        }

        // predamaged
        bool broken = thread->trigger(Predamaged, this, damage_data.to, data);
        if(broken)
            break;

        // damage done, should not cause damage process broken
        thread->trigger(DamageDone, this, damage_data.to, data);

        // damage
        if(damage_data.from)
            thread->trigger(Damage, this, damage_data.from, data);

        // damaged
        thread->trigger(Damaged, this, damage_data.to, data);
    }while(false);

    if(damage_data.from)
        thread->trigger(DamageConclude, this, damage_data.from, data);
    thread->trigger(DamageComplete, this, damage_data.to, data);
}

void Room::sendDamageLog(const DamageStruct &data){
    LogMessage log;

    if(data.from){
        log.type = "#Damage";
        log.from = data.from;
    }else{
        log.type = "#DamageNoSource";
    }

    log.to << data.to;
    log.arg = QString::number(data.damage);

    switch(data.nature){
    case DamageStruct::Normal: log.arg2 = "normal_nature"; break;
    case DamageStruct::Fire: log.arg2 = "fire_nature"; break;
    case DamageStruct::Thunder: log.arg2 = "thunder_nature"; break;
    }

    sendLog(log);
}

bool Room::hasWelfare(const ServerPlayer *player) const{
    if(scenario)
        return scenario->lordWelfare(player);
    else if(mode == "06_3v3")
        return player->isLord() || player->getRole() == "renegade";
    else if(ServerInfo.EnableHegemony)
        return false;
    else if(ServerInfo.EnableAnzhan){
        return false;
    }
    else
        return player->isLord() && player_count > 4;
}

ServerPlayer *Room::getFront(ServerPlayer *a, ServerPlayer *b) const{
    ServerPlayer *p;

    for(p=current; true; p=p->getNext()){
        if(p == a)
            return a;
        else if(p == b)
            return b;
    }

    return a;
}

void Room::reconnect(ServerPlayer *player, ClientSocket *socket){
    player->setSocket(socket);
    player->setState("online");

    marshal(player);

    broadcastProperty(player, "state");
}

void Room::marshal(ServerPlayer *player){
    player->sendProperty("objectName");
    player->sendProperty("role");
    player->unicast(".flags marshalling");

    foreach(ServerPlayer *p, m_players){
        if(p != player)
            p->introduceTo(player);
    }

    QStringList player_circle;
    foreach(ServerPlayer *player, m_players)
        player_circle << player->objectName();

    player->invoke("arrangeSeats", player_circle.join("+"));
    player->invoke("startInXs", "0");

    foreach(ServerPlayer *p, m_players){
        player->sendProperty("general", p);

        if(p->getGeneral2())
            player->sendProperty("general2", p);
    }

    player->invoke("startGame");

    foreach(ServerPlayer *p, m_players){
        p->marshal(player);
    }

    player->unicast(".flags -marshalling");
    player->invoke("setPileNumber", QString::number(draw_pile->length()));
}

void Room::startGame(){
    if(Config.ContestMode)
        tag.insert("StartTime", QDateTime::currentDateTime());

    QString to_test = property("to_test").toString();
    if(!to_test.isEmpty()){
        bool found = false;

        foreach(ServerPlayer *p, m_players){
            if(p->getGeneralName() == to_test){
                found = true;
                break;
            }
        }

        if(!found){
            int r = qrand() % m_players.length();
            m_players.at(r)->setGeneralName(to_test);
        }
    }

    int i;
    if(true){
        int start_index = 1;
        if(mode == "06_3v3" || mode == "02_1v1")
            start_index = 0;

        if(!Config.EnableBasara)for(i = start_index; i < m_players.count(); i++){
            broadcastProperty(m_players.at(i), "general");
        }

        if(mode == "02_1v1"){
            foreach(ServerPlayer *player, m_players){
                broadcastInvoke("revealGeneral",
                    QString("%1:%2").arg(player->objectName()).arg(player->getGeneralName()),
                    player);
            }
        }
    }

    if(Config.Enable2ndGeneral)
        if(mode != "02_1v1" && mode != "06_3v3" && !scenario && !Config.EnableBasara){
            foreach(ServerPlayer *player, m_players)
                broadcastProperty(player, "general2");
    }

    m_alivePlayers = m_players;
    for(i=0; i<player_count-1; i++)
        m_players.at(i)->setNext(m_players.at(i+1));
    m_players.last()->setNext(m_players.first());

    foreach(ServerPlayer *player, m_players){
        player->setMaxHP(player->getGeneralMaxHP());
        player->setHp(player->getMaxHP());

        broadcastProperty(player, "maxhp");
        broadcastProperty(player, "hp");

        if(mode == "06_3v3")
            broadcastProperty(player, "role");

        // setup AI
        AI *ai = cloneAI(player);
        ais << ai;
        player->setAI(ai);
    }

    broadcastInvoke("startGame");
    game_started = true;

    Server *server = qobject_cast<Server *>(parent());
    foreach(ServerPlayer *player, m_players){
        if(player->getState() == "online")
            server->signupPlayer(player);
    }

    current = m_players.first();

    // initialize the place_map and owner_map;
    foreach(int card_id, *draw_pile){
        setCardMapping(card_id, NULL, Player::DrawPile);
    }

    broadcastInvoke("setPileNumber", QString::number(draw_pile->length()));

    thread = new RoomThread(this);
    connect(thread, SIGNAL(started()), this, SIGNAL(game_start()));

    GameRule *game_rule;
    if(Config.EnableScene)	//changjing
        game_rule = new SceneRule(this);
    else
        game_rule = new GameRule(this);

    thread->constructTriggerTable(game_rule);
    thread->addTriggerSkill(new ConjuringRule(this));
    if(!Config.BanPackages.contains("events"))
        thread->addTriggerSkill(new EventsRule(this));
    if(Config.EnableReincarnation)
        thread->addTriggerSkill(new ReincarnationRule(this));
    if(Config.EnableBasara)
        thread->addTriggerSkill(new BasaraMode(this));

    if(scenario){
        const ScenarioRule *rule = scenario->getRule();
        if(rule)
            thread->addTriggerSkill(rule);
    }

    Config.setValue("PCConsole", isPCConsole());
    if(!_virtual)
        thread->start();
}

bool Room::notifyProperty(ServerPlayer* playerToNotify, const ServerPlayer* propertyOwner, const char *propertyName, const QString &value)
{
    if (propertyOwner == NULL) return false;
    QString real_value = value;
    if (real_value.isNull()) real_value = propertyOwner->property(propertyName).toString();
    Json::Value arg(Json::arrayValue);
    if (propertyOwner == playerToNotify)
        arg[0] = toJsonString(QString(QSanProtocol::S_PLAYER_SELF_REFERENCE_ID));
    else
        arg[0] = toJsonString(propertyOwner->objectName());
    arg[1] = propertyName;
    arg[2] = toJsonString(real_value);
    return doNotify(playerToNotify, S_COMMAND_SET_PROPERTY, arg);
}

bool Room::broadcastProperty(ServerPlayer *player, const char *property_name, const QString &value){
    if (player == NULL) return false;
    QString real_value = value;
    if (real_value.isNull()) real_value = player->property(property_name).toString();
    Json::Value arg(Json::arrayValue);
    arg[0] = toJsonString(player->objectName());
    arg[1] = property_name;
    arg[2] = toJsonString(real_value);
    return doBroadcastNotify(S_COMMAND_SET_PROPERTY, arg);
}
/*
void Room::broadcastProperty(ServerPlayer *player, const char *property_name, const QString &value){
    if(value.isNull()){
        QString real_value = player->property(property_name).toString();
        broadcast(QString("#%1 %2 %3").arg(player->objectName()).arg(property_name).arg(real_value));
    }else
        broadcast(QString("#%1 %2 %3").arg(player->objectName()).arg(property_name).arg(value));
}
*/
void Room::drawCards(ServerPlayer *player, int n, const QString &reason){
    if(n <= 0)
        return;

    QList<int> card_ids;
    QStringList cards_str;

    int i;
    for(i=0; i<n; i++){
        int card_id = drawCard();
        card_ids << card_id;
        const Card *card = Sanguosha->getCard(card_id);
        player->getRoom()->setCardFlag(card, reason);

        QVariant data = QVariant::fromValue(card_id);
        if(thread->trigger(CardDrawing, this, player, data))
            continue;

        player->drawCard(card);

        cards_str << QString::number(card_id);

        // update place_map & owner_map
        setCardMapping(card_id, player, Player::Hand);
    }
    if(cards_str.isEmpty())
        return;

    player->invoke("drawCards", cards_str.join("+"));

    QString draw_str = QString("%1:%2").arg(player->objectName()).arg(n);

    QString dongchaee = tag.value("Dongchaee").toString();
    if(player->objectName() == dongchaee){
        QString dongchaer_name = tag.value("Dongchaer").toString();
        ServerPlayer *dongchaer = findChild<ServerPlayer *>(dongchaer_name);

        CardMoveStruct move;
        foreach(int card_id, card_ids){
            move.card_id = card_id;
            move.from = NULL;
            move.from_place = Player::DrawPile;
            move.to = player;
            move.to_place = Player::Hand;

            dongchaer->invoke("moveCard", move.toString());
        }

        foreach(ServerPlayer *p, m_players){
            if(p != player && p != dongchaer)
                p->invoke("drawNCards", draw_str);
        }
    }else
        broadcastInvoke("drawNCards", draw_str, player);

    QVariant data = QVariant::fromValue(n);
    thread->trigger(CardDrawnDone, this, player, data);
}

void Room::throwCard(const Card *card, ServerPlayer *who, ServerPlayer *thrower){
    if(card == NULL)
        return;

    LogMessage log;
    if(who){
        if(thrower){
            log.type = "$DiscardCardByOther"; //thrower throw who's card
            log.from = thrower;
            log.to << who;
        }
        else{
            log.type = "$DiscardCard";  //who throw the card
            log.from = who;
        }
    }
    else
        log.type = "$EnterDiscardPile";

    QList<int> to_discard;
    if(card->isVirtualCard())
        to_discard.append(card->getSubcards());
    else
        to_discard << card->getEffectiveId();

    foreach(int card_id, to_discard){
        setCardFlag(card_id, "visible");
        if(log.card_str.isEmpty())
            log.card_str = QString::number(card_id);
        else
            log.card_str += "+" + QString::number(card_id);
    }
    if(who && who->hasFlag("mute_throw"))
        who->hasFlag("-mute_throw");
    else
        sendLog(log);

    moveCardTo(card, NULL, Player::DiscardedPile);

    if(who){
        CardStar card_ptr = card;
        QVariant data = QVariant::fromValue(card_ptr);
        thread->trigger(CardDiscarded, this, who, data);
    }
}

void Room::throwCard(int card_id, ServerPlayer *who, ServerPlayer *thrower){
    throwCard(Sanguosha->getCard(card_id), who, thrower);
}

RoomThread *Room::getThread() const{
    return thread;
}

void Room::moveCardTo(const Card *card, ServerPlayer *to, Player::Place place, bool open){
    QSet<ServerPlayer *> scope;
    int eid = card->getEffectiveId();
    ServerPlayer *from = getCardOwner(eid);
    Player::Place from_place= getCardPlace(eid);

    if(!open){
        scope.insert(from);
        scope.insert(to);

        QString dongchaee_name = tag.value("Dongchaee").toString();
        if(!dongchaee_name.isEmpty()){
            ServerPlayer *dongchaee = findChild<ServerPlayer *>(dongchaee_name);
            if(dongchaee){
                bool invoke_dongcha = false;
                if(dongchaee == from)
                    invoke_dongcha = (from_place == Player::Hand);
                else if(dongchaee == to)
                    invoke_dongcha = (place == Player::Hand);

                if(invoke_dongcha){
                    QString dongchaer_name = tag.value("Dongchaer").toString();
                    ServerPlayer *dongchaer = findChild<ServerPlayer *>(dongchaer_name);
                    scope.insert(dongchaer);
                }
            }
        }

        QString from_str = from->objectName();
        if(from_place == Player::Special)
            from_str.append("@special");

        QString to_str = to->objectName();
        if(place == Player::Special)
            to_str.append("@special");

        int n = card->isVirtualCard() ? card->subcardsLength() : 1;
        QString private_move = QString("%1:%2->%3")
            .arg(n)
            .arg(from_str)
            .arg(to_str);

        foreach(ServerPlayer *player, m_players){
            if(!scope.contains(player))
                player->invoke("moveNCards", private_move);
        }
    }

    CardMoveStruct move;
    move.to = to;
    move.to_place = place;
    move.open = open;

    from = NULL;
    QVariant data;

    if(card->isVirtualCard()){
        QList<int> subcards = card->getSubcards();
        foreach(int subcard, subcards){
            move.card_id = subcard;
            move.from = getCardOwner(subcard);
            move.from_place = getCardPlace(subcard);

            if(to){
                data = QVariant::fromValue(move);
                thread->trigger(CardMoving, this, move.to, data);
            }
            doMove(move, scope);

            if(move.from)
                from = move.from;
        }
    }else{
        move.card_id = card->getId();
        move.from = getCardOwner(move.card_id);
        move.from_place = getCardPlace(move.card_id);

        if(to){
            data = QVariant::fromValue(move);
            thread->trigger(CardMoving, this, move.to, data);
        }
        doMove(move, scope);

        if(move.from)
            from = move.from;
    }

    if(from)
        thread->trigger(CardLostDone, this, from);
    if(to)
        thread->trigger(CardGotDone, this, to);

    if(card->inherits("Analeptic") && place == Player::DiscardedPile && !Config.BanPackages.contains("events")){
        ServerPlayer *sour = findPlayerWhohasCard("jiangjieshi");
        if(sour && sour != getCurrent()){
            const Card *fight = askForCard(sour, "Jiangjieshi", "@jiangshi", data, CardDiscarded);
            if(fight){
                sour->playCardEffect("@jiangjieshi2");
                LogMessage log;
                log.type = "#Jiangjs";
                log.from = sour;
                log.arg = "jiangjieshi";
                log.arg2 = card->objectName();
                sendLog(log);
                sour->obtainCard(card);
            }
        }
    }

    if(from_place == Player::Equip || place == Player::Equip){
        QList<ServerPlayer *> tzww = findPlayersBySkillName("qiaogong");
        QiaogongStruct qgdt;
        qgdt.equip = card;
        qgdt.wear = place == Player::Equip;
        qgdt.target = place == Player::Equip ? to : from;
        QVariant qg_data = QVariant::fromValue(qgdt);
        foreach(ServerPlayer *tzw, tzww)
            thread->trigger(QiaogongTrigger, this, tzw, qg_data);
    }
}

void Room::doMove(const CardMoveStruct &move, const QSet<ServerPlayer *> &scope){
    // avoid useless operation
    if(move.from == move.to && move.from_place == move.to_place)
        return;

    const Card *card = Sanguosha->getCard(move.card_id);
    if(move.from){
        if(move.from_place == Player::Special){
            QString pile_name = move.from->getPileName(move.card_id);

            //@todo: if (pile_name.isEmpty());

            QString pile_str = QString("%1:%2-%3")
                .arg(move.from->objectName()).arg(pile_name).arg(move.card_id);

            if(move.open)
                broadcastInvoke("pile", pile_str);
            else{
                foreach(ServerPlayer *p, scope)
                    p->invoke("pile", pile_str);
            }
        }

        move.from->removeCard(card, move.from_place);
    }else{
        switch(move.from_place){
        case Player::DiscardedPile: discard_pile->removeOne(move.card_id); break;
        case Player::DrawPile: draw_pile->removeOne(move.card_id); break;
        case Player::Special: table_cards.removeOne(move.card_id); break;
        default:
            break;
        }
    }

    if(move.to){
        move.to->addCard(card, move.to_place);

        if(move.to_place == Player::Special){
            QString pile_name = move.to->getPileName(move.card_id);

            //@todo: if (pile_name.isEmpty());

            QString pile_str = QString("%1:%2+%3")
                .arg(move.to->objectName()).arg(pile_name).arg(move.card_id);

            if(move.open)
                broadcastInvoke("pile", pile_str);
            else{
                foreach(ServerPlayer *p, scope)
                    p->invoke("pile", pile_str);
            }
        }
    }else{
        switch(move.to_place){
        case Player::DiscardedPile: discard_pile->prepend(move.card_id); break;
        case Player::DrawPile: draw_pile->prepend(move.card_id); break;
        case Player::Special: table_cards.append(move.card_id); break;
        default:
            break;
        }
    }

    setCardMapping(move.card_id, move.to, move.to_place);

    QString move_str = move.toString();
    if(!move.open){
        foreach(ServerPlayer *player, scope){
            player->invoke("moveCard", move_str);
        }
    }else{
        broadcastInvoke("moveCard", move_str);
    }

    if(move.from){
        CardMoveStar move_star = &move;
        QVariant data = QVariant::fromValue(move_star);
        thread->trigger(CardLost, this, move.from, data);
    }
    if(move.to && move.to!=move.from){
        CardMoveStar move_star = &move;
        QVariant data = QVariant::fromValue(move_star);
        thread->trigger(CardGot, this, move.to, data);
    }
    Sanguosha->getCard(move.card_id)->onMove(move);
}

QString CardMoveStruct::toString() const{
    static QMap<Player::Place, QString> place2str;
    if(place2str.isEmpty()){
        place2str.insert(Player::Hand, "hand");
        place2str.insert(Player::Equip, "equip");
        place2str.insert(Player::Judging, "judging");
        place2str.insert(Player::Special, "special");
        place2str.insert(Player::DiscardedPile, "_");
        place2str.insert(Player::DrawPile, "=");
    }

    QString from_str = from ? from->objectName() : "_";
    QString to_str = to ? to->objectName() : "_";

    return QString("%1:%2@%3->%4@%5")
        .arg(card_id)
        .arg(from_str).arg(place2str.value(from_place, "_"))
        .arg(to_str).arg(place2str.value(to_place, "_"));
}

void Room::playSkillEffect(const QString &skill_name, int index){
    broadcastInvoke("playSkillEffect", QString("%1:%2").arg(skill_name).arg(index));
}

void Room::startTest(const QString &to_test){
    fillRobotsCommand(NULL, ".");
    setProperty("to_test", to_test);
}

void Room::acquireSkill(ServerPlayer *player, const Skill *skill, bool open){
    QString skill_name = skill->objectName();
    if(player->getAcquiredSkills().contains(skill_name))
        return;

    player->acquireSkill(skill_name);

    if(skill->inherits("TriggerSkill")){
        const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
        thread->addTriggerSkill(trigger_skill);
    }

    if(skill->isVisible()){
        if(open){
            QString acquire_str = QString("%1:%2").arg(player->objectName()).arg(skill_name);
            broadcastInvoke("acquireSkill", acquire_str);
        }

        foreach(const Skill *related_skill, Sanguosha->getRelatedSkills(skill_name)){
            if(!related_skill->isVisible())
                acquireSkill(player, related_skill);
        }
    }
}

void Room::acquireSkill(ServerPlayer *player, const QString &skill_name, bool open){
    const Skill *skill = Sanguosha->getSkill(skill_name);

    if(skill){
        if(mode == "wheel_fight" && player->getGeneralName() != "ubuntenkei"){
            if(!player->getGeneral()->hasSkill(skill_name))
                return;
        }
        acquireSkill(player, skill, open);
    }
}

void Room::setTag(const QString &key, const QVariant &value){
    tag.insert(key, value);
    if(scenario)
        scenario->onTagSet(this, key);
}

QVariant Room::getTag(const QString &key) const{
    return tag.value(key);
}

void Room::removeTag(const QString &key){
    tag.remove(key);
}

void Room::setEmotion(ServerPlayer *target, const QString &emotion){
    broadcastInvoke("setEmotion",
        QString("%1:%2").arg(target->objectName()).arg(emotion.isEmpty() ? "." : emotion));
}

void Room::setEmotion(QList<ServerPlayer *> targets, const QString &emotion){
    foreach(ServerPlayer *target, targets)
        broadcastInvoke("setEmotion", QString("%1:%2").arg(target->objectName()).arg(emotion.isEmpty() ? "." : emotion));
}

#include <QElapsedTimer>

void Room::activate(ServerPlayer *player, CardUseStruct &card_use){
    if(player->hasFlag("ShutUp"))
        return;
    while(isPCConsole() && Config.Pause)
        wait(5000);
    AI *ai = player->getAI();
    if(ai){
        QElapsedTimer timer;
        timer.start();

        card_use.from = player;
        ai->activate(card_use);

        if(Config.value("AlterAIDelayAD", false).toBool()){
            bool ad = true;
            foreach(ServerPlayer *p, getOtherPlayers(player)){
                if(p->getState() != "robot"){
                    ad = false;
                    break;
                }
            }
            if(ad)
                Config.AIDelay = Config.AIDelayAD;
            else
                Config.AIDelay = Config.value("AIDelay", 1500).toInt();
        }

        qint64 diff = Config.AIDelay - timer.elapsed();
        if(diff > 0)
            thread->delay(diff);
    }else{
        bool success = doRequest(player, S_COMMAND_PLAY_CARD, toJsonString(player->objectName()), true);
        Json::Value clientReply = player->getClientReply();

        if (m_surrenderRequestReceived)
        {
            makeSurrender(player);
            if (!game_finished)
                return activate(player, card_use);
        }
        else
        {
            //@todo: change FreeChoose to EnableCheat
            if (Config.value("Cheat/EnableCheatMenu", false).toBool()) {
                if(makeCheat(player)){
                    if(player->isAlive())
                        return activate(player, card_use);
                    return;
                }
            }
        }

        if (!success || clientReply.isNull()) return;

        card_use.from = player;
        if (!card_use.tryParse(clientReply, this) || !card_use.isValid()){
            emit room_message(tr("Card can not parse:\n %1").arg(toQString(clientReply[0])));
            return;
        }
    }

    QVariant data = QVariant::fromValue(card_use);
    thread->trigger(ChoiceMade, this, player, data);
}

Card::Suit Room::askForSuit(ServerPlayer *player, const QString& reason){
    AI *ai = player->getAI();
    if(ai)
        return ai->askForSuit(reason);

    bool success = doRequest(player, S_COMMAND_CHOOSE_SUIT, Json::Value::null);

    Card::Suit suit = Card::AllSuits[qrand() % 4];
    if (success)
    {
        Json::Value clientReply = player->getClientReply();
        QString suitStr = toQString(clientReply);
        if(suitStr == "spade")
            suit = Card::Spade;
        else if(suitStr == "club")
            suit = Card::Club;
        else if(suitStr == "heart")
            suit = Card::Heart;
        else if (suitStr == "diamond")
            suit = Card::Diamond;
    }

    return suit;
}

QString Room::askForKingdom(ServerPlayer *player){
    AI *ai = player->getAI();
    if(ai)
        return ai->askForKingdom();

    bool success = doRequest(player, S_COMMAND_CHOOSE_KINGDOM, Json::Value::null);

    //@todo: check if the result is valid before return!!
    //@todo: make kingdom a enum or static const instead of variable QString
    Json::Value clientReply = player->getClientReply();
    if (success && clientReply.isString())
    {
        QString kingdom = toQString(clientReply.asCString());
        if (kingdom == "guan" || kingdom == "jiang" || kingdom == "min" || kingdom == "kou")
            return kingdom;
    }
    return "god";
}

bool Room::askForDiscard(ServerPlayer *player, const QString &reason, int discard_num, bool optional, bool include_equip){
    AI *ai = player->getAI();
    QList<int> to_discard;
    if (ai) {
        to_discard = ai->askForDiscard(reason, discard_num, optional, include_equip);
    }else{
        Json::Value ask_str(Json::arrayValue);
        ask_str[0] = discard_num;
        ask_str[1] = optional;
        ask_str[2] = include_equip;
        bool success = doRequest(player, S_COMMAND_DISCARD_CARD, ask_str);

        //@todo: also check if the player does have that card!!!
        Json::Value clientReply = player->getClientReply();
        if(!success || !clientReply.isArray() || (int)clientReply.size() != discard_num
            || !tryParse(clientReply, to_discard))
        {
            if(optional) return false;
            // time is up, and the server choose the cards to discard
            to_discard = player->forceToDiscard(discard_num, include_equip);
        }
    }

    if(to_discard.isEmpty())
        return false;

    DummyCard *dummy_card = new DummyCard;
    foreach(int card_id, to_discard)
        dummy_card->addSubcard(card_id);

    throwCard(dummy_card, player);

    QVariant data;
    data = QString("%1:%2").arg("cardDiscard").arg(dummy_card->toString());
    thread->trigger(ChoiceMade, this, player, data);

    dummy_card->deleteLater();

    return true;
}

bool Room::askForDiscard(ServerPlayer *player, const QString &reason, int discard_num, int min_num, bool optional, bool include_equip){
    return askForDiscard(player, reason, (discard_num + min_num) / 2, optional, include_equip);
}

const Card *Room::askForExchange(ServerPlayer *player, const QString &reason, int discard_num){
    AI *ai = player->getAI();
    QList<int> to_exchange;
    if(ai){
        // share the same callback interface
        to_exchange = ai->askForDiscard(reason, discard_num, false, false);
    }else{
        bool success = doRequest(player, S_COMMAND_EXCHANGE_CARD, discard_num);
        //@todo: also check if the player does have that card!!!
        Json::Value clientReply = player->getClientReply();
        if(!success || !clientReply.isArray() || (int)clientReply.size() != discard_num
            || !tryParse(clientReply, to_exchange))
        {
            to_exchange = player->forceToDiscard(discard_num, false);
        }

    }

    DummyCard *card = new DummyCard;
    foreach(int card_id, to_exchange)
        card->addSubcard(card_id);

    return card;
}

void Room::setCardMapping(int card_id, ServerPlayer *owner, Player::Place place){
    owner_map.insert(card_id, owner);
    place_map.insert(card_id, place);
}

ServerPlayer *Room::getCardOwner(int card_id) const{
    return owner_map.value(card_id);
}

Player::Place Room::getCardPlace(int card_id) const{
    return place_map.value(card_id);
}

ServerPlayer *Room::getLord() const{
    ServerPlayer *the_lord = m_players.first();
    if(the_lord->getRole() == "lord")
        return the_lord;

    foreach(ServerPlayer *player, m_players){
        if(player->getRole() == "lord")
            return player;
    }

    return NULL;
}

void Room::askForGuanxing(ServerPlayer *zhuge, const QList<int> &cards, bool up_only){
    QList<int> top_cards, bottom_cards;

    AI *ai = zhuge->getAI();
    if(ai){
        ai->askForGuanxing(cards, top_cards, bottom_cards, up_only);
    }else if(up_only && cards.length() == 1){
        top_cards = cards;
    }else{
        Json::Value guanxingArgs(Json::arrayValue);
        guanxingArgs[0] = toJsonArray(cards);
        guanxingArgs[1] = up_only;
        bool success = doRequest(zhuge, S_COMMAND_SKILL_GUANXING, guanxingArgs);

        //@todo: sanity check if this logic is correct
        if(!success){
            // the method "askForGuanxing" without any arguments
            // means to clear all the guanxing items
            //zhuge->invoke("doGuanxing");
            foreach (int card_id, cards)
                draw_pile->prepend(card_id);
            return;
        }
        Json::Value clientReply = zhuge->getClientReply();
        if (clientReply.isArray() && clientReply.size() == 2)
        {
            success &= tryParse(clientReply[0], top_cards);
            success &= tryParse(clientReply[1], bottom_cards);
        }
    }


    bool length_equal = top_cards.length() + bottom_cards.length() == cards.length();
    bool result_equal = top_cards.toSet() + bottom_cards.toSet() == cards.toSet();
    if(!length_equal || !result_equal){
        top_cards = cards;
        bottom_cards.clear();
    }

    LogMessage log;
    log.type = up_only ? "#GuanxingUpOnly" : "#GuanxingResult";
    log.from = zhuge;
    log.arg = QString::number(top_cards.length());
    log.arg2 = QString::number(bottom_cards.length());
    sendLog(log);

    QListIterator<int> i(top_cards);
    i.toBack();
    while(i.hasPrevious())
        draw_pile->prepend(i.previous());

    i = bottom_cards;
    while(i.hasNext())
        draw_pile->append(i.next());
}

void Room::doGongxin(ServerPlayer *shenlvmeng, ServerPlayer *target){
    //@todo: this thing should be put in AI!!!!!!!!!!
    if(!shenlvmeng->isOnline()){
        // throw the first card whose suit is Heart
        QList<const Card *> cards = target->getHandcards();
        foreach(const Card *card, cards){
            if(card->getSuit() == Card::Heart && !card->inherits("Shit")){
                showCard(target, card->getEffectiveId());
                thread->delay();
                throwCard(card, target);
                return;
            }
        }
        return;
    }

    Json::Value gongxinArgs(Json::arrayValue);
    gongxinArgs[0] = toJsonString(target->objectName());
    gongxinArgs[1] = true;
    gongxinArgs[2] = toJsonArray(target->handCards());
    bool success = doRequest(shenlvmeng, S_COMMAND_SKILL_GONGXIN, gongxinArgs);
    Json::Value clientReply = shenlvmeng->getClientReply();
    if (!success || !clientReply.isInt()
        || !target->handCards().contains(clientReply.asInt()))
        return;

    int card_id = clientReply.asInt();
    showCard(target, card_id);

    QString result = askForChoice(shenlvmeng, "gongxin", "discard+put");
    if(result == "discard")
        throwCard(card_id, target, shenlvmeng);
    else
        moveCardTo(Sanguosha->getCard(card_id), NULL, Player::DrawPile, true);
}

void Room::awake(ServerPlayer *player, const QString &skill_name, const QString &broad, int delay){
    QString skillname = skill_name;
    if(skillname.at(0).isUpper())
        skillname[0] = skillname.at(0).toLower();

    LogMessage log;
    log.type = "#WakeUp";
    log.from = player;
    log.arg = skillname;
    sendLog(log);
    playSkillEffect(skillname);

    if(!Config.DisableLightbox){
        QString bro = broad == "" ? "" : ":" + broad;
        broadcastInvoke("animate", "lightbox:$" + skill_name + bro);
        thread->delay(delay);
    }
    else
        thread->delay(qMin(delay / 2, 1000));
    setPlayerMark(player, skillname + "_wake", 1);
    detachSkillFromPlayer(player, skillname, false);
    if(!Config.EnableSkillEmotion)
        setEmotion(player, "awake");
    else
        setEmotion(player, "skill/" + skillname);
    broadcastInvoke("playAudio", "skill/awake");
}

void Room::playLightbox(ServerPlayer *player, const QString &skill_name, const QString &broad, int delay){
    QString skillname = skill_name;
    if(skillname.at(0).isUpper())
        skillname[0] = skillname.at(0).toLower();
    playSkillEffect(skillname);

    if(!Config.DisableLightbox){
        QString bro = broad == "" ? "" : ":" + broad;
        broadcastInvoke("animate", "lightbox:$" + skill_name + bro);
        thread->delay(delay);
    }
    else
        thread->delay(qMin(delay / 2, 1000));

    if(!Config.EnableSkillEmotion)
        setEmotion(player, "limited");
    else
        setEmotion(player, "skill/" + skillname);
    broadcastInvoke("playAudio", "skill/limited");
}

const Card *Room::askForPindian(ServerPlayer *player, ServerPlayer *from, ServerPlayer *to, const QString &reason)
{
    if(player->getHandcardNum() == 1){
        return player->getHandcards().first();
    }

    AI *ai = player->getAI();
    if(ai){
        thread->delay(Config.AIDelay);
        return ai->askForPindian(from, reason);
    }

    bool success = doRequest(player, S_COMMAND_PINDIAN, toJsonArray(from->objectName(), to->objectName()));

    Json::Value clientReply = player->getClientReply();
    if(!success || !clientReply.isString()){
        int card_id = player->getRandomHandCardId();
        return Sanguosha->getCard(card_id);
    }else{
        const Card *card = Card::Parse(toQString(clientReply));
        if(card->isVirtualCard()){
            const Card *real_card = Sanguosha->getCard(card->getEffectiveId());
            delete card;
            return real_card;
        }else
            return card;
    }
}

ServerPlayer *Room::askForPlayerChosen(ServerPlayer *player, const QList<ServerPlayer *> &targets, const QString &skillName){
    if(targets.isEmpty())
        return NULL;
    else if(targets.length() == 1)
        return targets.first();

    AI *ai = player->getAI();
    ServerPlayer* choice;
    if(ai)
        choice = ai->askForPlayerChosen(targets, skillName);
    else{
        Json::Value req;
        req[0] = Json::Value(Json::arrayValue);
        req[1] = toJsonString(skillName);
        foreach(ServerPlayer *target, targets)
            req[0].append(toJsonString(target->objectName()));
        bool success = doRequest(player, S_COMMAND_CHOOSE_PLAYER, req);

        //executeCommand(player, "askForPlayerChosen", "choosePlayerCommand", ask_str, ".");
        choice = NULL;
        Json::Value clientReply = player->getClientReply();
        if (success && clientReply.isString())
        {
            choice = findChild<ServerPlayer *>(clientReply.asCString());
        }
    }
    if(choice){
        QVariant data=QString("%1:%2:%3").arg("playerChosen").arg(skillName).arg(choice->objectName());
        thread->trigger(ChoiceMade, this, player, data);
    }
    return choice;
}

void Room::_setupChooseGeneralRequestArgs(ServerPlayer *player){
    Json::Value options = toJsonArray(player->getSelected());
    if(!Config.EnableBasara && getLord())
        options.append(toJsonString(QString("%1(lord)").arg(getLord()->getGeneralName())));
    else
        options.append("anjiang(lord)");
    player->m_commandArgs = options;
}

QString Room::askForGeneral(ServerPlayer *player, const QStringList &generals, QString default_choice){
    if(default_choice.isEmpty())
        default_choice = !generals.isEmpty() ? generals.at(qrand() % generals.length()) : "sujiang";

    if(player->isOnline())
    {
        Json::Value options = toJsonArray(generals);
        bool success = doRequest(player, S_COMMAND_CHOOSE_GENERAL, options);
        //executeCommand(player, "askForGeneral", "chooseGeneralCommand", generals.join("+"), ".");

        Json::Value clientResponse = player->getClientReply();
        if(!success || !clientResponse.isString() 
            || (!Config.FreeChooseGenerals && !generals.contains(clientResponse.asCString())))
            return default_choice;
        else
            return toQString(clientResponse);
    }

    return default_choice;
}

void Room::kickCommand(ServerPlayer *player, const QString &arg){
    // kicking is not allowed at contest mode
    if(Config.ContestMode)
        return;

    // only the lord can kick others
    if(player != getLord())
        return;

    ServerPlayer *to_kick = findChild<ServerPlayer *>(arg);
    if(to_kick == NULL)
        return;

    to_kick->kick();
}

bool Room::makeCheat(ServerPlayer* player){
    Json::Value& arg = player->m_cheatArgs;
    if (!arg.isArray() || !arg[0].isInt()) return false;
    CheatCode code = (CheatCode)arg[0].asInt();
    if (code == S_CHEAT_KILL_PLAYER)
    {
        if (!isStringArray(arg[1], 0, 1)) return false;
        makeKilling(toQString(arg[1][0]), toQString(arg[1][1]), true);
    }
    else if (code == S_CHEAT_MAKE_DAMAGE)
    {
        if (arg[1].size() != 5 || !isStringArray(arg[1], 0, 1)
            || !arg[1][2].isInt() || !arg[1][3].isInt() || !arg[1][4].asInt())
            return false;
        makeDamage(toQString(arg[1][0]), toQString(arg[1][1]),
            (QSanProtocol::CheatCategory)arg[1][2].asInt(), arg[1][3].asInt(), arg[1][4].asInt());
    }
    else if (code == S_CHEAT_REVIVE_PLAYER)
    {
        if (!arg[1].isString() || !arg[2].isString())
            return false;
        makeReviving(toQString(arg[1]), toQString(arg[2]));
    }
    else if (code == S_CHEAT_SET_STATE){
        if (!arg[1].isString()) return false;
        makeState(toQString(arg[1]), toQString(arg[2]));
    }
    else if (code == S_CHEAT_RUN_SCRIPT)
    {
        if (!arg[1].isString()) return false;
        QByteArray data = QByteArray::fromBase64(arg[1].asCString());
        data = qUncompress(data);
        doScript(data);
    }
    else if (code == S_CHEAT_GET_ONE_CARD)
    {
        if (!arg[1].isInt()) return false;
        int card_id = arg[1].asInt();

        LogMessage log;
        log.type = "$CheatCard";
        log.from = player;
        log.card_str = QString::number(card_id);
        sendLog(log);

        obtainCard(player, card_id);
    }
    else if (code == S_CHEAT_CHANGE_GENERAL)
    {
        if (!arg[1].isString()) return false;
        QString generalName = toQString(arg[1]);
        transfigure(player, generalName, false, true);
    }
    arg = Json::Value::null;
    return true;
}

void Room::makeDamage(const QString& source, const QString& target, QSanProtocol::CheatCategory nature, int point, int card_id){
    ServerPlayer* sourcePlayer = findChild<ServerPlayer *>(source);
    ServerPlayer* targetPlayer = findChild<ServerPlayer *>(target);
    if (targetPlayer == NULL) return;
    // damage
    if (nature == S_CHEAT_HP_LOSE){
        loseHp(targetPlayer, point);
        return;
    }
    else if (nature == S_CHEAT_MAX_HP_LOSE){
        loseMaxHp(targetPlayer, point);
        return;
    }
    else if (nature == S_CHEAT_HP_RECOVER){
        RecoverStruct recover;
        recover.who = sourcePlayer;
        recover.recover = point;
        if(card_id > -1)
            recover.card = Sanguosha->getCard(card_id);
        this->recover(targetPlayer, recover);
        return;
    }
    else if (nature == S_CHEAT_MAX_HP_RESET){
        setPlayerProperty(targetPlayer, "maxhp", point);
        return;
    }

    static QMap<QSanProtocol::CheatCategory, DamageStruct::Nature> nature_map;
    if(nature_map.isEmpty()){
        nature_map[S_CHEAT_NORMAL_DAMAGE] = DamageStruct::Normal;
        nature_map[S_CHEAT_THUNDER_DAMAGE] = DamageStruct::Thunder;
        nature_map[S_CHEAT_FIRE_DAMAGE] = DamageStruct::Fire;
    }

    if (targetPlayer == NULL) return;
    DamageStruct damage;
    damage.from = sourcePlayer;
    damage.to = targetPlayer;
    damage.damage = point;
    damage.nature = nature_map[nature];
    if(card_id > -1)
        damage.card = Sanguosha->getCard(card_id);
    this->damage(damage);
}

void Room::makeKilling(const QString& killerName, const QString& victimName, bool force){
    ServerPlayer *killer = NULL, *victim = NULL;

    killer = findChild<ServerPlayer *>(killerName);
    victim = findChild<ServerPlayer *>(victimName);

    if (victim == NULL) return;

    if (killer == NULL)
        return killPlayer(victim, NULL, force);

    DamageStruct damage;
    damage.from = killer;
    damage.to = victim;
    killPlayer(victim, &damage, force);
}

void Room::makeReviving(const QString &name, const QString &flag){
    ServerPlayer *player = findChild<ServerPlayer *>(name);
    Q_ASSERT(player);
    revivePlayer(player, flag.contains("I"));
    if(flag.contains("F")){
        setPlayerProperty(player, "maxhp", player->getGeneralMaxHP());
        setPlayerProperty(player, "hp", player->getMaxHP());
    }
    else{
        if(player->getMaxHp() < 1)
            setPlayerProperty(player, "maxhp", 1);
        if(player->getHp() < 1)
            setPlayerProperty(player, "hp", 1);
    }
}

void Room::makeState(const QString &name, const QString &str){
    ServerPlayer *player = findChild<ServerPlayer *>(name);
    Q_ASSERT(player);
    //general:songjiang|linchong,kindom:god,chained:true
    QStringList items = str.split(",");
    foreach(QString item, items){
        QString key = item.split(":").first();
        QString value = item.split(":").last();
        if(value == "" || value.isNull())
            continue;

        if(key == "general"){
            QString gen1 = value.split("|").first();
            QString gen2 = value.split("|").last();
            transfigure(player, ":" + gen1, false, true);
            if(value.contains("|"))
                transfigure(player, "%" + gen2, false, true);
        }
        else if(key == "kingdom")
            setPlayerProperty(player, "kingdom", value);
        else if(key == "role"){
            player->setRole(value);
            broadcastProperty(player, "role");
        }
        else if(key == "sex"){
            General *general = (General *)Sanguosha->getGeneral(player->getGeneralName());
            general->setGenderString(value);
        }
        else if(key == "dod"){
            if(value == "1")
                player->drawCards(1);
            else if(!player->isKongcheng())
                throwCard(player->getRandomHandCardId(), player);
        }
        else if(key == "turned"){
            if((value == "1" && player->faceUp()) ||
               (value == "0" && !player->faceUp()))
                player->turnOver();
        }
        else if(key == "chained"){
            if(value == "1" && !player->isChained())
                player->setChained(true);
            else if(value == "0" && player->isChained())
                player->setChained(false);
            broadcastProperty(player, "chained");
        }
        else if(key == "ecst")
            setPlayerFlag(player, value == "1" ? "ecst" : "-ecst");
        else if(key == "drank")
            setPlayerFlag(player, value == "1" ? "drank" : "-drank");
        else if(key == "tie")
            setPlayerFlag(player, value == "1" ? "ShutUp" : "-ShutUp");
        else if(key == "skill"){
            if(value.startsWith("-")){
                value.remove("-");
                detachSkillFromPlayer(player, value);
            }
        }
        else if(key == "history")
            player->clearHistory();
        else if(key.endsWith("_jur"))
            player->gainJur(key, value.toInt());
        else if(key == "flag")
            setPlayerFlag(player, value);
        else if(key == "mark"){
            QStringList m = value.split("=");
            key = m.first();
            value = m.last().isNull() ? "1" : m.last();
            QVariant va = QVariant::fromValue(value);
            setPlayerMark(player, key, va.toInt());
        }
        else if(key == "propty"){
            QStringList p = value.split("=");
            value = p.last();
            if(value == "true")
                value = "1";
            else if(value == "false")
                value = "0";
            setPlayerProperty(player, QString(p.first()).toLocal8Bit().data(), QVariant::fromValue(value));
        }
        else if(key == "tag"){
            QStringList t = value.split("=");
            value = t.last();
            if(value == "true")
                value = "1";
            else if(value == "false")
                value = "0";
            if(t.first().startsWith("-")){
                QString key2 = t.first().remove("-");
                player->tag.remove(key2);
            }
            else
                player->tag.insert(t.first(), QVariant::fromValue(value));
        }
    }
}

void Room::fillAG(const QList<int> &card_ids, ServerPlayer *who){
    QStringList card_str;
    foreach(int card_id, card_ids)
        card_str << QString::number(card_id);

    if(who)
        who->invoke("fillAG", card_str.join("+"));
    else{
        broadcastInvoke("fillAG", card_str.join("+"));
        //@todo: move this to the client side!!!
        broadcastInvoke("disableAG", "true");
    }
}

void Room::takeAG(ServerPlayer *player, int card_id){
    if(player){
        player->addCard(Sanguosha->getCard(card_id), Player::Hand);
        setCardMapping(card_id, player, Player::Hand);
        broadcastInvoke("takeAG", QString("%1:%2").arg(player->objectName()).arg(card_id));
        //@todo: move this to the client side!!!
        player->invoke("disableAG", "true");
        CardMoveStruct move;

        move.from = NULL;
        move.from_place = Player::DrawPile;
        move.to = player;
        move.to_place = Player::Hand;
        move.card_id = card_id;
        CardMoveStar move_star = &move;
        QVariant data = QVariant::fromValue(move_star);
        thread->trigger(CardGot, this, player, data);
        thread->trigger(CardGotDone, this, player);
    }else{
        discard_pile->prepend(card_id);
        setCardMapping(card_id, NULL, Player::DiscardedPile);
        broadcastInvoke("takeAG", QString(".:%1").arg(card_id));
    }
}

void Room::provide(const Card *card){
    Q_ASSERT(provided == NULL);
    Q_ASSERT(!has_provided);

    provided = card;
    has_provided = true;
}

QList<ServerPlayer *> Room::getLieges(const QString &kingdom, ServerPlayer *lord) const{
    QList<ServerPlayer *> lieges;
    foreach(ServerPlayer *player, m_alivePlayers){
        if(player != lord && player->getKingdom() == kingdom)
            lieges << player;
    }

    return lieges;
}

QList<ServerPlayer *> Room::getMenorWomen(const QString &gender, ServerPlayer *except) const{
    QList<ServerPlayer *> targets;
    foreach(ServerPlayer *player, m_alivePlayers){
        if(except && player == except)
            continue;
        if(player->getGenderString() == gender)
            targets << player;
    }
    return targets;
}

int Room::getKingdoms() const{
    QSet<QString> kingdom_set;
    foreach(ServerPlayer *tmp, m_alivePlayers)
        kingdom_set << tmp->getKingdom();
    return kingdom_set.size();
}

void Room::sendLog(const LogMessage &log){
    if(log.type.isEmpty())
        return;

    broadcastInvoke("log", log.toString());
}

void Room::showCard(ServerPlayer *player, int card_id, ServerPlayer *only_viewer){
    Json::Value show_str;
    show_str[0] = toJsonString(player->objectName());
    show_str[1] = card_id;
    if(only_viewer)
        doNotify(player, S_COMMAND_SHOW_CARD, show_str);
    else{
        if(card_id>0)
            setCardFlag(card_id, "visible");
        doBroadcastNotify(S_COMMAND_SHOW_CARD, show_str);
    }
}

void Room::showAllCards(ServerPlayer *player, ServerPlayer *to){
    broadcastInvoke("clearAG");
    Json::Value gongxinArgs(Json::arrayValue);
    gongxinArgs[0] = toJsonString(player->objectName());
    gongxinArgs[1] = false;
    gongxinArgs[2] = toJsonArray(player->handCards());
    bool isUnicast = (to != NULL);
    if (isUnicast)
        doNotify(player, S_COMMAND_SHOW_ALL_CARDS, gongxinArgs);
    else{
        foreach(int card_id, player->handCards())
            setCardFlag(card_id, "visible");
        doBroadcastNotify(S_COMMAND_SHOW_ALL_CARDS, gongxinArgs);
    }
}

bool Room::askForYiji(ServerPlayer *guojia, QList<int> &cards){
    if(cards.isEmpty())
        return false;

    AI *ai = guojia->getAI();
    if(ai){
        int card_id;
        ServerPlayer *who = ai->askForYiji(cards, card_id);
        if(who){
            cards.removeOne(card_id);
            moveCardTo(Sanguosha->getCard(card_id), who, Player::Hand, false);
            return true;
        }else
            return false;
    }else{

        bool success = doRequest(guojia, S_COMMAND_SKILL_YIJI, toJsonArray(cards));

        //Validate client response
        Json::Value clientReply = guojia->getClientReply();
        if(!success || !clientReply.isArray()
            || clientReply.size() != 2)
            return false;

        QList<int> ids;
        if (!tryParse(clientReply[0], ids)
            || !clientReply[1].isString()) return false;

        foreach (int id, ids)
            if (!cards.contains(id)) return false;

        ServerPlayer *who = findChild<ServerPlayer *>(toQString(clientReply[1]));

        if (!who) return false;

        DummyCard *dummy_card = new DummyCard;
        foreach(int card_id, ids){
            cards.removeOne(card_id);
            dummy_card->addSubcard(card_id);
        }

        moveCardTo(dummy_card, who, Player::Hand, false);
        delete dummy_card;

        //setEmotion(who, "draw-card");

        return true;

    }
}

bool Room::isNoLordSkill(){
    return Config.NoLordSkill;
}

QString Room::generatePlayerName(){
    static unsigned int id = 0;
    id++;
    return QString("sgs%1").arg(id);
}

void Room::arrangeCommand(ServerPlayer *player, const QString &arg){
    if(mode == "06_3v3")
        thread_3v3->arrange(player, arg.split("+"));
    else if(mode == "02_1v1")
        thread_1v1->arrange(player, arg.split("+"));
}

void Room::takeGeneralCommand(ServerPlayer *player, const QString &arg){
    if(mode == "06_3v3")
        thread_3v3->takeGeneral(player, arg);
    else if(mode == "02_1v1")
        thread_1v1->takeGeneral(player, arg);
}

QString Room::askForOrder(ServerPlayer *player){

    bool success = doRequest(player, S_COMMAND_CHOOSE_ORDER, (int)S_REASON_CHOOSE_ORDER_TURN, false);

    Game3v3Camp result = qrand() % 2 == 0 ? S_CAMP_WARM : S_CAMP_COOL;
    Json::Value clientReply = player->getClientReply();
    if (success && clientReply.isInt())
    {
        result = (Game3v3Camp)clientReply.asInt();
    }
    if (result == S_CAMP_WARM) return "warm";
    else return "cool";
}

QString Room::askForRole(ServerPlayer *player, const QStringList &roles, const QString &scheme){
    QStringList squeezed = roles.toSet().toList();
    Json::Value arg(Json::arrayValue);
    arg[0] = toJsonString(scheme);
    arg[1] = toJsonArray(squeezed);
    bool success = doRequest(player, S_COMMAND_CHOOSE_ROLE_3V3, arg, false);
    Json::Value clientReply = player->getClientReply();
    QString result = "abstain";
    if (success && clientReply.isString())
    {
        result = clientReply.asCString();
    }
    return result;
}

void Room::networkDelayTestCommand(ServerPlayer *player, const QString &){
    qint64 delay = player->endNetworkDelayTest();
    QString reportStr = tr("<font color=#EEB422>The network delay of player <b>%1</b> is %2 milliseconds.</font>")
        .arg(Config.ContestMode ? tr("Contestant") : player->screenName()).arg(QString::number(delay));
    speakCommand(player, reportStr.toUtf8().toBase64());
}

bool Room::isVirtual()
{
    return _virtual;
}

void Room::setVirtual()
{
    _virtual = true;
}

void Room::copyFrom(Room* rRoom)
{
    QMap<ServerPlayer*, ServerPlayer*> player_map;

    for(int i=0; i<m_players.length(); i++)
    {

        ServerPlayer* a = rRoom->m_players.at(i);
        ServerPlayer* b = m_players.at(i);
        player_map.insert(a, b);

        transfigure(b, a->getGeneralName(), false);

        b->copyFrom(a);
    }
    for(int i=0; i<m_players.length(); i++)
    {
        ServerPlayer* a = rRoom->m_players.at(i);
        ServerPlayer* b = m_players.at(i);
        b->setNext(player_map.value(a->getNext()));
    }

    foreach(ServerPlayer* a,m_alivePlayers)
    {
        if(!a->isAlive())m_alivePlayers.removeOne(a);
    }
    current = player_map.value(rRoom->getCurrent());

    pile1 = QList<int> (rRoom->pile1);
    pile2 = QList<int> (rRoom->pile2);
    table_cards = QList<int> (rRoom->table_cards);
    draw_pile = &pile1;
    discard_pile = &pile2;

    place_map = QMap<int, Player::Place> (rRoom->place_map);
    owner_map = QMap<int, ServerPlayer*>();

    QList<int> keys = rRoom->owner_map.keys();

    foreach(int i, keys)
        owner_map.insert(i, rRoom->owner_map.value(i));

    provided = rRoom->provided;
    has_provided = rRoom->has_provided;

    tag = QVariantMap(rRoom->tag);

}

Room* Room::duplicate()
{
    Server* svr = qobject_cast<Server *> (parent());
    Room* room = svr->createNewRoom();
    room->setVirtual();
    room->fillRobotsCommand(NULL, 0);
    room->copyFrom(this);
    return room;
}

void Room::setGerenalGender(const QString &name, const QString &gender)
{
    General *general = (General *)Sanguosha->getGeneral(name);

    if(general == NULL)
        return;

    if (gender == "M")
        general->setGender(General::Male);
    else if (gender == "F")
        general->setGender(General::Female);
    else
        general->setGender(General::Neuter);

    QString pattern = name + ":" + gender;

    broadcastInvoke("setGerenalGender", pattern);
}

void Room::playExtra(TriggerEvent event, const QVariant &data){
    if(event == CardUsed){
        CardUseStruct card_use = data.value<CardUseStruct>();
        PlayerStar player = card_use.from;
        bool mute = card_use.mute;
        if(card_use.card->inherits("Slash") && Config.EnableEquipEffects){
            if(player->hasSkill("shuangzhan") && card_use.to.count() == 2){
                playSkillEffect("shuangzhan", qrand() % 2 + 1);
                mute = true;
            }
            if(player->hasSkill("douzhan") && card_use.to.count() == 2){
                playSkillEffect("douzhan");
                mute = true;
            }
            if(player->hasSkill("bizhai") && card_use.to.count() == 2){
                int index = player->getGender() == General::Male ? qrand() % 2 + 1 : qrand() % 2 + 3;
                playSkillEffect("bizhai", index);
                mute = true;
            }
            if(player->hasWeapon("crossbow") && player->getPhase() == Player::Play && player->getMark("SlashCount") > 0)
                mute = true;
            if(card_use.card->getSkillName() == "spear"){
                player->playCardEffect("Espear", "weapon");
                mute = true;
            }
            else if(player->hasWeapon("fan") &&
                    player->isLastHandCard(card_use.card) && card_use.to.count() > 1){
                player->playCardEffect("Efan2", "weapon");
                mute = true;
            }
            else if(player->hasWeapon("sun_bow") && card_use.card->objectName() == "slash" && card_use.to.count() > 1){
                player->playCardEffect("Esun_bow", "weapon");
                mute = true;
            }
        }
        player->playCardEffect(card_use.card, mute);
        if(card_use.card->isKindOf("Duel"))
            broadcastInvoke("playAudio", "card/duel");
        if(card_use.card->isKindOf("Assassinate")){
            broadcastInvoke("playAudio", "card/assassinate");
            setEmotion(card_use.to, "assassinate");
        }
        if(card_use.card->isKindOf("GodSalvation")){
            broadcastInvoke("playAudio", "card/god_salvation");
            setEmotion(card_use.to, "god_salvation");
        }
        if(card_use.card->isKindOf("AmazingGrace")){
            broadcastInvoke("playAudio", "card/amazing_grace");
            setEmotion(card_use.to, "amazing_grace");
        }
        if(card_use.card->isKindOf("SavageAssault")){
            broadcastInvoke("playAudio", "card/savage_assault");
            setEmotion(card_use.to, "savage_assault");
        }
        if(card_use.card->isKindOf("ArcheryAttack")){
            broadcastInvoke("playAudio", "card/archery_attack");
            setEmotion(card_use.to, "archery_attack");
        }
        if(card_use.card->isKindOf("Inspiration")){
            broadcastInvoke("playAudio", "card/inspiration");
            setEmotion(card_use.to, "inspiration");
        }
        if(!card_use.card->isVirtualCard())
            setEmotion(card_use.from, "cards/" + card_use.card->objectName());
    }
    if(event == SlashEffect){
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.from->hasWeapon("crossbow")){
            int slash = effect.from->getMark("SlashCount");
            if(slash % 2 == 0)
                effect.from->playCardEffect("Ecrossbow1", "weapon");
            else if(slash % 2 == 1 && slash > 1)
                effect.from->playCardEffect("Ecrossbow2", "weapon");
        }

        if(effect.from->hasSkill("qinlong") && effect.from->getMark("SlashCount") > 1)
            playSkillEffect("qinlong");
        if(effect.from->hasSkill("yinyu") && effect.slash->getSkillName() != "yuanpei"){
            if(effect.from->hasMark("@stones") && effect.from->getMark("SlashCount") > 1){
                int index = effect.from->hasMark("mengshi_wake") ? qrand() % 2 + 9: qrand() % 2 + 3;
                playSkillEffect("yinyu", index);
            }
            else if(effect.from->hasMark("@stoneh")){
                if(effect.from->distanceTo(effect.to) > getAlivePlayers().count() / 4){
                    int index = effect.from->hasMark("mengshi_wake") ? 11: 5;
                    playSkillEffect("yinyu", index);
                }
            }
            else if(effect.from->hasMark("@stonec")){
                if(effect.to->getArmor() || (!effect.to->getArmor() && effect.to->hasSkill("jinjia"))){
                    int index = effect.from->hasMark("mengshi_wake") ? 12: 6;
                    playSkillEffect("yinyu", index);
                }
            }
        }
        if(effect.slash->inherits("ThunderSlash") && effect.slash->getSkillName() != "paohong" && effect.from->hasSkill("paohong"))
            playSkillEffect("paohong");
    }
}
