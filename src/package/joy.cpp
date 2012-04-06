#include "joy.h"
#include "engine.h"

Shit::Shit(Suit suit, int number):BasicCard(suit, number){
    setObjectName("shit");

    target_fixed = true;
}

QString Shit::getSubtype() const{
    return "disgusting_card";
}

void Shit::onMove(const CardMoveStruct &move) const{
    ServerPlayer *from = move.from;
    if(from && move.from_place == Player::Hand &&
       from->getRoom()->getCurrent() == move.from
       && (move.to_place == Player::DiscardedPile || move.to_place == Player::Special)
       && move.to == NULL
       && from->isAlive()){

        LogMessage log;
        log.card_str = getEffectIdString();
        log.from = from;

        Room *room = from->getRoom();

        if(getSuit() == Spade){
            log.type = "$ShitLostHp";
            room->sendLog(log);

            room->loseHp(from);

            return;
        }

        DamageStruct damage;
        damage.from = damage.to = from;
        damage.card = this;

        switch(getSuit()){
        case Club: damage.nature = DamageStruct::Thunder; break;
        case Heart: damage.nature = DamageStruct::Fire; break;
        default:
            damage.nature = DamageStruct::Normal;
        }

        log.type = "$ShitDamage";
        room->sendLog(log);

        room->damage(damage);
    }
}

bool Shit::HasShit(const Card *card){
    if(card->isVirtualCard()){
        QList<int> card_ids = card->getSubcards();
        foreach(int card_id, card_ids){
            const Card *c = Sanguosha->getCard(card_id);
            if(c->objectName() == "shit")
                return true;
        }

        return false;
    }else
        return card->objectName() == "shit";
}

Stink::Stink(Suit suit, int number):BasicCard(suit, number){
    setObjectName("stink");
    target_fixed = true;
}

QString Stink::getSubtype() const{
    return "disgusting_card";
}

QString Stink::getEffectPath(bool is_male) const{
    return "audio/card/common/stink.ogg";
}

void Stink::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    room->throwCard(this);
    ServerPlayer *nextfriend = targets.isEmpty() ? source->getNextAlive() : targets.first();
    room->setEmotion(nextfriend, "bad");
    const Card *pipi = room->askForCard(nextfriend, "Jink,Assassinate", "@haochou:" + source->objectName(), QVariant::fromValue((PlayerStar)source));
    LogMessage log;
    log.from = nextfriend;

    if(!pipi){
        log.type = "#StinkSuccess";
        log.to << nextfriend->getNextAlive();
        room->sendLog(log);
        room->swapSeat(nextfriend, nextfriend->getNextAlive());
    }
    else if(!pipi->inherits("Jink")){
        DamageStruct damage;
        damage.from = nextfriend;
        damage.to = source;
        damage.card = pipi;
        room->setEmotion(nextfriend, "good");
        log.type = "#StinkHit";
        log.to << source;
        room->sendLog(log);
        room->damage(damage);
    }
    else{
        log.type = "#StinkJink";
        log.to << source;
        room->sendLog(log);
        room->setEmotion(nextfriend, "good");
    }
}


KusoPackage::KusoPackage()
    :Package("kuso"){
    QList<Card *> cards;

    cards << new Shit(Card::Club, 1)
            << new Shit(Card::Heart, 8)
            << new Shit(Card::Diamond, 13)
            << new Shit(Card::Spade, 10)
            << new Stink(Card::Diamond, 1);

    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

class GrabPeach: public TriggerSkill{
public:
    GrabPeach():TriggerSkill("grab_peach"){
        events << CardUsed;
    }

    virtual bool triggerable(const ServerPlayer *) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(use.card->inherits("Peach")){
            Room *room = player->getRoom();
            QList<ServerPlayer *> players = room->getOtherPlayers(player);

            foreach(ServerPlayer *p, players){
                if(p->getOffensiveHorse() == parent() &&
                   p->askForSkillInvoke("grab_peach", data))
                {
                    room->throwCard(p->getOffensiveHorse());
                    p->playCardEffect(objectName());
                    p->obtainCard(use.card);

                    return true;
                }
            }
        }

        return false;
    }
};

Monkey::Monkey(Card::Suit suit, int number)
    :OffensiveHorse(suit, number)
{
    setObjectName("monkey");

    grab_peach = new GrabPeach;
    grab_peach->setParent(this);
}

void Monkey::onInstall(ServerPlayer *player) const{
    player->getRoom()->getThread()->addTriggerSkill(grab_peach);
}

void Monkey::onUninstall(ServerPlayer *player) const{

}

QString Monkey::getEffectPath(bool ) const{
    return "audio/card/common/monkey.ogg";
}

class GaleShellSkill: public ArmorSkill{
public:
    GaleShellSkill():ArmorSkill("gale-shell"){
        events << Predamaged;
    }

    virtual bool trigger(TriggerEvent, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.nature == DamageStruct::Fire){
            LogMessage log;
            log.type = "#GaleShellDamage";
            log.from = player;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            player->getRoom()->sendLog(log);

            damage.damage ++;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

GaleShell::GaleShell(Suit suit, int number) :Armor(suit, number){
    setObjectName("gale-shell");
    skill = new GaleShellSkill;

    target_fixed = false;
}

bool GaleShell::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->distanceTo(to_select) <= 1;
}

void GaleShell::onUse(Room *room, const CardUseStruct &card_use) const{
    Card::onUse(room, card_use);
}

Poison::Poison(Suit suit, int number)
    : BasicCard(suit, number){
    setObjectName("poison");
}

QString Poison::getSubtype() const{
    return "attack_card";
}

QString Poison::getEffectPath(bool is_male) const{
    return "audio/card/common/poison.ogg";
}

bool Poison::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && Self->distanceTo(to_select) <= 1;
}

void Poison::onEffect(const CardEffectStruct &card_effect) const{
    Room *room = card_effect.from->getRoom();

    LogMessage log;
    log.from = card_effect.to;
    if(card_effect.to->getMark("poison") == 0){
        room->setPlayerMark(card_effect.to, "poison", 1);
        room->setEmotion(card_effect.to, "bad");

        log.type = "#Poison_in";
        room->sendLog(log);
    }
    else{
        room->setPlayerMark(card_effect.to, "poison", 0);
        room->setEmotion(card_effect.to, "good");

        log.type = "#Poison_out";
        room->sendLog(log);
    }
}

JoyPackage::JoyPackage()
    :Package("joy")
{
    QList<Card *> cards;
    cards
                << new Monkey(Card::Diamond, 5)
                << new GaleShell(Card::Heart, 1)
                << new Poison(Card::Heart, 7)
                << new Poison(Card::Club, 9)
                << new Poison(Card::Diamond, 11)
                << new Poison(Card::Spade, 13);

    foreach(Card *card, cards)
            card->setParent(this);

    type = CardPack;
}

//joy generals : miheng
#include "maneuvering.h"
#include "settings.h"
#include "carditem.h"
class Jieao: public PhaseChangeSkill{
public:
    Jieao():PhaseChangeSkill("jieao"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *miheng) const{
        if(miheng->getPhase() == Player::Start && miheng->getHp() > miheng->getHandcardNum()){
            LogMessage log;
            log.type = "#TriggerSkill";
            log.from = miheng;
            log.arg = objectName();
            miheng->getRoom()->sendLog(log);
            miheng->drawCards(2);
        }
        return false;
    }
};

YuluCard::YuluCard(){
    target_fixed = true;
    will_throw = false;
}

void YuluCard::use(Room *, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    foreach(int word_id, this->getSubcards()){
        source->addToPile("word", word_id);
    }
}

class Yulu: public ViewAsSkill{
public:
    Yulu():ViewAsSkill("yulu"){
    }

    virtual bool viewFilter(const QList<CardItem *> &, const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.isEmpty())
            return NULL;
        YuluCard *yulu_card = new YuluCard;
        yulu_card->addSubcards(cards);
        return yulu_card;
    }
};

ViewMyWordsCard::ViewMyWordsCard(){
    target_fixed = true;
}

void ViewMyWordsCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    QList<int> words = source->getPile("word");
    if(words.isEmpty())
        return;
    room->fillAG(words, source);
    int card_id = room->askForAG(source, words, true, "viewmywords");
    if(card_id != -1){
        words.removeOne(card_id);
        room->moveCardTo(Sanguosha->getCard(card_id), source, Player::Hand, false);
    }
    source->invoke("clearAG");
    words.clear();
}

class ViewMyWords: public ZeroCardViewAsSkill{
public:
    ViewMyWords():ZeroCardViewAsSkill("numa"){
    }
    virtual const Card *viewAs() const{
        return new ViewMyWordsCard;
    }
};

class Numa: public PhaseChangeSkill{
public:
    Numa():PhaseChangeSkill("numa"){
        view_as_skill = new ViewMyWords;
    }

    virtual bool onPhaseChange(ServerPlayer *miheng) const{
        Room *room = miheng->getRoom();
        if(miheng->getPhase() == Player::Finish &&
           !miheng->getPile("word").isEmpty() &&
           room->askForSkillInvoke(miheng, objectName())){
            Room *room = miheng->getRoom();
            QString c,word;
            foreach(int i, miheng->getPile("word")){
                c = Sanguosha->getCard(i)->getSuitString().left(1);

                LogMessage log;
                log.type = "#Numasingle";
                log.from = miheng;
                log.arg = objectName() + c;
                room->sendLog(log);

                word = word + c;
            }

            LogMessage gitlog;
            gitlog.type = "#Numa_" + word;
            gitlog.from = miheng;
            if(word == "hc"){
                room->sendLog(gitlog);
                //womei:recover self
                RecoverStruct womei;
                womei.who = miheng;
                room->recover(miheng, womei);
            }
            else if(word == "dc"){
                room->sendLog(gitlog);
                //nimei:throw single player 2 cards
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getAlivePlayers()){
                    if(tmp->getHandcardNum() >= 2)
                        players << tmp;
                }
                room->askForDiscard(room->askForPlayerChosen(miheng, players, objectName()), objectName(), 2);
            }
            else if(word == "cc"){
                room->sendLog(gitlog);
                //meimei:clear single player's all judge_area
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getAlivePlayers()){
                    if(!tmp->getJudgingArea().isEmpty())
                        players << tmp;
                }
                if(!players.isEmpty()){
                    ServerPlayer *target = room->askForPlayerChosen(miheng, players, objectName());
                    foreach(const Card *c, target->getJudgingArea()){
                        room->throwCard(c);
                    }
                }
            }
            else if(word == "sd"){
                room->sendLog(gitlog);
                //rini:let single player tribute a card and recover 1 hp
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getOtherPlayers(miheng)){
                    if(tmp->isWounded() && !tmp->isKongcheng())
                        players << tmp;
                }
                if(!players.isEmpty()){
                    ServerPlayer *target = room->askForPlayerChosen(miheng, players, objectName());
                    const Card *card = room->askForCardShow(target, miheng, objectName());
                    miheng->obtainCard(card, false);
                    RecoverStruct rini;
                    rini.card = card;
                    rini.who = miheng;
                    room->recover(target, rini);
                }
            }
            else if(word == "hs"){
                room->sendLog(gitlog);
                //wori:get skill fanchun
                JudgeStruct judge;
                judge.pattern = QRegExp("(Peach|GodSalvation):(.*):(.*)");
                judge.good = true;
                judge.reason = objectName();
                judge.who = miheng;
                room->judge(judge);
                if(judge.isGood())
                    room->acquireSkill(miheng, "fanchun");
            }
            else if(word == "hsc" || word == "hsd"){
                room->sendLog(gitlog);
                //worimei&worini:recover hp with a girl or a boy
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getOtherPlayers(miheng)){
                    if(((word == "hsc" && tmp->getGeneral()->isFemale()) ||
                       (word == "hsd" && tmp->getGeneral()->isMale())) && tmp->isWounded())
                        players << tmp;
                }
                if(!players.isEmpty()){
                    ServerPlayer *target = room->askForPlayerChosen(miheng, players, objectName());
                    RecoverStruct worimei;
                    worimei.who = miheng;
                    room->recover(target, worimei);
                    room->recover(miheng, worimei);
                }
            }
            else if(word == "dsh"){
                room->sendLog(gitlog);
                //niriwo:call slash me! or taking away all his cards
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getAlivePlayers()){
                    if(tmp->canSlash(miheng))
                        players << tmp;
                }
                ServerPlayer *target = room->askForPlayerChosen(miheng, players, objectName());
                const Card *slash = room->askForCard(target, "slash", objectName());
                if(slash){
                    CardUseStruct niriwo;
                    niriwo.card = slash;
                    niriwo.to << miheng;
                    niriwo.from = target;
                    room->useCard(niriwo);
                }else if(!target->isNude()){
                    QList<const Card *> cards = target->getCards("hej");
                    foreach(const Card *tmp, cards)
                        miheng->obtainCard(tmp, false);
                }
            }
            else if(word == "shc"){
                room->sendLog(gitlog);
                //riwomei:let single player damage myself and recover himself
                DamageStruct riwmei;
                ServerPlayer *target = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());
                riwmei.from = target;
                riwmei.to = miheng;
                room->damage(riwmei);

                RecoverStruct riwomei;
                riwomei.who = miheng;
                room->recover(target, riwomei);
            }
            else if(word == "hhh"){
                room->sendLog(gitlog);
                //wowowo:the same to Jushou
                miheng->turnOver();
                miheng->drawCards(3);
            }
            else if(word == "sss"){
                room->sendLog(gitlog);
                //ririri:the same to Fangzhu
                ServerPlayer *target = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());
                target->turnOver();
                target->drawCards(miheng->getMaxHP() - miheng->getHp());
            }
            else if(word == "ddd"){
                room->sendLog(gitlog);
                //ninini:let a player obtain word-card
                ServerPlayer *target = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());
                foreach(int i, miheng->getPile("word"))
                    room->obtainCard(target, i, false);
            }
            else if(word == "ccc"){
                room->sendLog(gitlog);
                //meimeimei:clear single player's all equip_area
                ServerPlayer *target = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());
                target->throwAllEquips();
            }
            else if(word == "dcdc"){
                room->sendLog(gitlog);
                //nimeinimei:make a extra turn
                ServerPlayer *target = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());
                foreach(int i, miheng->getPile("word"))
                    room->throwCard(i);
                target->gainAnExtraTurn(miheng);
            }
            else if(word == "sdc" || word == "hsdc"){
                room->sendLog(gitlog);
                //rinimei:slash
                //worinimei:drank and slash
                if(word == "hsdc")
                    room->setPlayerFlag(miheng, "drank");

                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getAlivePlayers()){
                    if(tmp->hasSkill("jueming") && tmp->getHp() == 1)
                        continue;
                    players << tmp;
                }
                ServerPlayer *target = room->askForPlayerChosen(miheng, players, objectName());

                int slashtype = Sanguosha->getCard(miheng->getPile("word").first())->getNumber();

                if(!players.isEmpty()){
                    CardUseStruct worinimei;
                    Card *card;
                    if(word == "sdc" && slashtype < 5)
                        card = new ThunderSlash(Card::NoSuit, 0);
                    else if(word == "sdc" && slashtype >9)
                        card = new FireSlash(Card::NoSuit, 0);
                    else
                        card = new Slash(Card::NoSuit, 0);
                    card->setSkillName(objectName());
                    worinimei.card = card;
                    worinimei.from = miheng;
                    worinimei.to << target;
                    room->useCard(worinimei);
                }
            }
            else if(word == "ccsh"){
                room->sendLog(gitlog);
                //nimeiriwo:hp full
                room->setPlayerProperty(miheng, "hp", miheng->getMaxHP());
            }
            else if(word == "dsdc"){
                room->sendLog(gitlog);
                //nimeiriwo:show one player's handcard to other one
                ServerPlayer *source = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());
                ServerPlayer *target = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());

                LogMessage log;
                log.type = "#Info_dsdc";
                log.from = source;
                log.to << target;
                room->sendLog(log);

                room->showAllCards(target, source);
            }
            else if(word == "dshc"){
                room->sendLog(gitlog);
                //niriwomei:kill-self
                if(Config.FreeChoose && room->askForChoice(miheng, "numat", "kno+kyes") == "kno"){
                    gitlog.type = "#Numa_tequan";
                    gitlog.from = miheng;
                    room->sendLog(gitlog);
                }
                else{
                    DamageStruct damage;
                    damage.from = miheng;
                    room->killPlayer(miheng, &damage);
                }
            }
            else if(word == "dshcc"){
                room->sendLog(gitlog);
                //niriwomeimei:throw other 4 card and make 2 damage to self
                ServerPlayer *target = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());
                for(int i = qMin(4, target->getCardCount(true)); i > 0; i--)
                    room->throwCard(room->askForCardChosen(miheng, target, "he", objectName()));
                DamageStruct niriwomm;
                niriwomm.from = miheng;
                niriwomm.to = miheng;
                niriwomm.damage = 2;
                room->damage(niriwomm);
            }
            else if(word == "hsdcc" && miheng->getMark("hsdcc") == 0){
                room->sendLog(gitlog);
                //worinimeimei:Limited-Skill, like GreatYeyan
                ServerPlayer *target = room->askForPlayerChosen(miheng, room->getAlivePlayers(), objectName());
                DamageStruct worinimm;
                worinimm.from = miheng;
                worinimm.to = target;
                worinimm.nature = DamageStruct::Thunder;
                room->damage(worinimm);
                worinimm.nature = DamageStruct::Fire;
                room->damage(worinimm);
                worinimm.nature = DamageStruct::Normal;
                room->damage(worinimm);
                room->loseHp(miheng, 2);
                miheng->addMark("hsdcc");
            }
            else if(word == "dcshc" && miheng->getMark("dcshc") == 0){
                room->sendLog(gitlog);
                //worinimeimei:Limited-Skill, like Guixin
                room->loseHp(miheng);
                foreach(ServerPlayer *player, room->getAllPlayers()){
                    if(!player->isKongcheng()){
                        int card_id = player->getRandomHandCardId();
                        room->obtainCard(miheng, card_id, false);
                    }
                }
                miheng->turnOver();
                miheng->addMark("dcshc");
            }
            else if(word == "ssdcc" && miheng->getMark("ssdcc") == 0){
                room->sendLog(gitlog);
                //ririnimeimei:lightning
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getAlivePlayers()){
                    foreach(const Card *lightning, tmp->getJudgingArea()){
                        if(lightning->objectName() == "lightning"){
                            players << tmp;
                            break;
                        }
                    }
                }
                if(!players.isEmpty()){
                    ServerPlayer *target = room->askForPlayerChosen(miheng, players, objectName());
                    foreach(const Card *lightning, target->getJudgingArea()){
                        if(lightning->objectName() == "lightning"){
                            room->throwCard(lightning);
                            break;
                        }
                    }
                    DamageStruct damage;
                    damage.to = target;
                    damage.nature = DamageStruct::Thunder;
                    damage.damage = 3;
                    room->damage(damage);

                    miheng->addMark("ssdcc");
                }
            }
            else if(word == "ssscc" && miheng->getMark("ssscc") == 0){
                room->sendLog(gitlog);
                //riririmeimei:let single player acquire fushang or dunwu
                QList<ServerPlayer *> players;
                foreach(ServerPlayer *tmp, room->getOtherPlayers(miheng)){
                    if(tmp->getMaxHP() > miheng->getMaxHP())
                        players << tmp;
                }
                if(!players.isEmpty()){
                    ServerPlayer *target = room->askForPlayerChosen(miheng, players, objectName());
                    QString choice = room->askForChoice(target, objectName(), "bthx+wump");
                    if(choice == "bthx"){
                        room->setPlayerProperty(target, "maxhp", target->getMaxHP() + 2);
                        room->acquireSkill(target, "fushang");
                    }
                    else{
                        room->setPlayerProperty(target, "maxhp", target->getMaxHP() + 1);
                        room->acquireSkill(target, "dunwu");
                    }
                    miheng->addMark("ssscc");
                }
            }
            else if(word.length() == 4){
                gitlog.type = "#Numa_4wd";
                gitlog.from = miheng;
                room->sendLog(gitlog);
                //worinimeimei:Wake-Skill, lost all skills
                if(Config.FreeChoose && room->askForChoice(miheng, "numat", "suno+suyes") == "suno"){
                    gitlog.type = "#Numa_tequan";
                    gitlog.from = miheng;
                    room->sendLog(gitlog);
                }
                else{
                    QList<const Skill *> skills = miheng->getVisibleSkillList();
                    foreach(const Skill *skill, skills)
                        room->detachSkillFromPlayer(miheng, skill->objectName());
                    room->setPlayerProperty(miheng, "general", "sujiang");
                    room->setPlayerProperty(miheng, "general2", "sujiangf");
                    room->setPlayerProperty(miheng, "maxhp", miheng->getMaxHP() + 2);
                }
            }
            else if(word.length() == 5 && miheng->getMark("fivewd") == 0){
                gitlog.type = "#Numa_5wd";
                gitlog.from = miheng;
                room->sendLog(gitlog);
                //worinimeimei:Wake-Skill
                if(Config.FreeChoose && room->askForChoice(miheng, "numat", "lhno+lhyes") == "lhno"){
                    gitlog.type = "#Numa_tequan";
                    gitlog.from = miheng;
                    room->sendLog(gitlog);
                }
                else{
                    room->loseMaxHp(miheng);
                    if(miheng->isAlive())
                        miheng->addMark("fivewd");
                }
            }
            else if(word.length() > 5 && miheng->getMark("othwd") == 0){
                gitlog.type = "#Numa_wds";
                gitlog.from = miheng;
                room->sendLog(gitlog);
                //worinimeimei:Wake-Skill
                room->loseMaxHp(miheng, 2);
                if(miheng->isAlive())
                    miheng->addMark("othwd");
            }
            else{
                gitlog.type = "#Numa_git";
                gitlog.from = miheng;
                room->sendLog(gitlog);
            }
            foreach(int i, miheng->getPile("word"))
                room->throwCard(i);
        }
        return false;
    }
};

class Fanchun:public MasochismSkill{
public:
    Fanchun():MasochismSkill("fanchun"){
    }
    virtual void onDamaged(ServerPlayer *mh, const DamageStruct &damage) const{
        Room *room = mh->getRoom();
        CardStar card = damage.card;
        if(!room->obtainable(card, mh))
            return;
        QVariant data = QVariant::fromValue(card);
        if(room->askForSkillInvoke(mh, objectName(), data)){
            if(!card->getSubcards().isEmpty())
                foreach(int cd, card->getSubcards())
                    mh->addToPile("word", cd);
            else
                mh->addToPile("word", card->getEffectiveId());
        }
    }
};

class Timer: public PhaseChangeSkill{
public:
    Timer():PhaseChangeSkill("timer"){
    }

    virtual int getPriority() const{
        return 2;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Start){
            Room *room = player->getRoom();
            room->loseMaxHp(player);
        }
        return false;
    }
};

class Lingyu: public TriggerSkill{
public:
    Lingyu():TriggerSkill("lingyu"){
        events << GameStart << CardLostDone << CardDrawnDone << CardGotDone;
    }

    static int getMaqueCardNum(ServerPlayer *me){
        int hand = me->getHandcardNum();
        int fu = 0;
        for(int i = 1; i <= 4; i ++)
            fu = fu + me->getPile("fu" + QString::number(i)).length();
        return hand + fu;
    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &) const{
        if(event == GameStart){
            int num = player->getHandcardNum();
            player->drawCards(13 - num);
        }
        else{
            Room *room = player->getRoom();
            int num = Lingyu::getMaqueCardNum(player);
            if(num > 13){
                if(!player->getPile("fu4").isEmpty() && player->getHandcardNum() == 2){
                    if(player->getHandcards().first()->getNumber() == player->getHandcards().last()->getNumber())
                        room->gameOver(player->getRole());
                }
                if(!player->getPhase() == Player::Draw)
                    room->askForDiscard(player, objectName(), num - 13);
            }
            else if(num < 13)
                room->drawCards(player, 13 - num);
        }


        return false;
    }
};

ZhuangcheCard::ZhuangcheCard(){
    target_fixed = true;
    will_throw = false;
}

void ZhuangcheCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    if(getSubcards().length() != 3)
        return;

    for(int i = 1; i <= 4; i ++){
        if(source->getPile("fu" + QString::number(i)).isEmpty()){
            foreach(int card_id, getSubcards())
                source->addToPile("fu" + QString::number(i), card_id);
            break;
        }
    }

    if(!source->getPile("fu4").isEmpty() && source->getHandcardNum() == 2){
        if(source->getHandcards().first()->getNumber() == source->getHandcards().last()->getNumber())
            room->gameOver(source->getRole());
    }
}

#include "clientplayer.h"
class ZhuangcheViewAsSkill: public ViewAsSkill{
public:
    ZhuangcheViewAsSkill():ViewAsSkill("zhuangche"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        int card_id = Self->property("peng").toInt();
        const Card *card = Sanguosha->getCard(card_id);
        if(selected.length() >= 3)
            return false;
        if(to_select->isEquipped())
            return false;
        return to_select->getCard()->getNumber() == card->getNumber();
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@zhuangche";
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 2)
            return NULL;
        ZhuangcheCard *card = new ZhuangcheCard;
        card->addSubcards(cards);
        card->addSubcard(Self->property("peng").toInt());
        return card;
    }
};

class Zhuangche: public TriggerSkill{
public:
    Zhuangche():TriggerSkill("zhuangche"){
        events << CardLost;
        view_as_skill = new ZhuangcheViewAsSkill;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual int getPriority() const{
        return -2;
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        if(player->getPhase() == Player::Discard)
            return false;
        QList<ServerPlayer *> birds = room->findPlayersBySkillName(objectName());
        CardMoveStar move = data.value<CardMoveStar>();
        foreach(ServerPlayer *bird, birds){
            if(move->from && move->to_place == Player::DiscardedPile){
                const Card *card = Sanguosha->getCard(move->card_id);
                if(!bird->getPile("fu4").isEmpty() && bird->getHandcardNum() == 1
                   && card->getNumber() == bird->getHandcards().first()->getNumber()){
                    room->gameOver(bird->getRole());
                }
                room->setPlayerProperty(bird, "peng", move->card_id);
                int number = card->getNumber();
                int i = 0;
                foreach(const Card *tmp, bird->getHandcards())
                    if(tmp->getNumber() == number)
                        i ++;
                if(i == 2){
                    QString prompt = QString("@zhuangche:%1:%2:%3").arg(move->from->objectName()).arg(card->getNumberString()).arg(card->objectName());
                    if(room->askForUseCard(bird, "@@zhuangche", prompt))
                        break;
                }
            }
        }
        return false;
    }
};

class ZoumaViewAsSkill: public ViewAsSkill{
public:
    ZoumaViewAsSkill():ViewAsSkill("zouma"){
    }

    virtual bool viewFilter(const QList<CardItem *> &selected, const CardItem *to_select) const{
        if(selected.length() >= 4)
            return false;
        if(to_select->isEquipped())
            return false;
        if(selected.length() == 1){
            int num1 = selected.last()->getCard()->getNumber();
            int num2 = to_select->getCard()->getNumber();
            return (num2 == num1 + 1) || (num1 == num2);
        }
        if(selected.length() == 2){
            int num1 = selected.first()->getCard()->getNumber();
            int num2 = selected.last()->getCard()->getNumber();
            int num3 = to_select->getCard()->getNumber();
            return (num2 == num1 + 1 && num3 == num2 + 1) ||
                    (num1 == num2 && num2 == num3);
        }
        return true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return pattern == "@@zouma";
    }

    virtual const Card *viewAs(const QList<CardItem *> &cards) const{
        if(cards.length() != 3)
            return NULL;
        ZhuangcheCard *card = new ZhuangcheCard;
        card->addSubcards(cards);
        return card;
    }
};

class Zouma: public PhaseChangeSkill{
public:
    Zouma():PhaseChangeSkill("zouma"){
        view_as_skill = new ZoumaViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() == Player::Start){
            player->skip(Player::Judge);
            player->skip(Player::Play);
            player->skip(Player::Discard);
        }
        else if(player->getPhase() == Player::Draw){
            Room *room = player->getRoom();
            player->drawCards(1);
            while(room->askForUseCard(player, "@@zouma", "@zouma"));

            if(!player->getPile("fu4").isEmpty() && player->getHandcardNum() == 2){
                if(player->getHandcards().first()->getNumber() == player->getHandcards().last()->getNumber())
                    room->gameOver(player->getRole());
            }
            if(Lingyu::getMaqueCardNum(player) > 13)
                room->askForDiscard(player, "lingyu", Lingyu::getMaqueCardNum(player) - 13);
            return true;
        }
        return false;
    }
};

ChuiniuCard::ChuiniuCard(){
    once = true;
}

bool ChuiniuCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select->getHandcardNum() < Self->getHandcardNum();
}

void ChuiniuCard::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    LogMessage log;
    ServerPlayer *target = targets.first();
    foreach(int hd, source->handCards())
        source->addToPile("chuiniu", hd, false);
    foreach(int hd, target->handCards())
        target->addToPile("chuiniu", hd, false);

    while(source->getHandcardNum() < 3){
        source->drawCards(1, false, false);
        if(source->getHandcards().last()->getNumber() > 6)
            room->throwCard(source->getHandcards().last());
    }
    while(target->getHandcardNum() < 3){
        target->drawCards(1, false, false);
        if(target->getHandcards().last()->getNumber() > 6)
            room->throwCard(target->getHandcards().last());
    }

    ServerPlayer *first = source;
    ServerPlayer *second = target;
    static QMap<ServerPlayer*, int> cmap;
    static QMap<ServerPlayer*, int> nmap;
    forever{
        QString qs = room->askForChoice(first, "chuiniu_num", "2+3+4+5+6+pass");
        if(qs == "pass")
            break;
        else{
            nmap[first] = qs.toInt();
        }

        qs = room->askForChoice(first, "chuiniu_count", "2+3+4+5+6");
        cmap[first] = qs.toInt();

        if(cmap[first] <= cmap[second] && nmap[first] <= nmap[second])
            continue;

        log.type = "#Chuiniuing";
        log.from = first;
        log.arg = QString::number(cmap.value(first));
        log.arg2 = QString::number(nmap.value(first));
        room->sendLog(log);

        qSwap(first, second);
    }

    QList<const Card *> allhands = source->getHandcards();
    allhands.append(target->getHandcards());
    int count = 0;
    foreach(const Card *card, allhands){
        if(card->getNumber() == nmap.value(first) || card->getNumber() == 1)
            count ++;
    }
    log.type = "#ChuiniuEnd";
    log.from = first;
    log.arg = QString::number(count);
    log.arg2 = QString::number(nmap.value(first));
    room->sendLog(log);

    bool win = (first == source && count >= cmap.value(first)) ||
               (first != source && count < cmap.value(first));

    room->getThread()->delay();
    QList<int> uvnn = source->getPile("chuiniu");
    uvnn.append(target->getPile("chuiniu"));
    log.type = "#ChuiniuWin";
    if(win){
        log.from = source;
        room->setEmotion(source, "good");
        foreach(int x, uvnn)
            room->throwCard(x);
        DummyCard *cards = target->wholeHandCards();
        room->obtainCard(source, cards, false);
        room->setEmotion(target, "bad");
    }
    else{
        log.from = target;
        room->setEmotion(target, "good");
        target->throwAllHandCards();
        foreach(int x, uvnn)
            room->obtainCard(target, x, false);
        room->setEmotion(source, "bad");
    }
    room->sendLog(log);
}

class ChuiniuViewAsSkill: public ZeroCardViewAsSkill{
public:
    ChuiniuViewAsSkill():ZeroCardViewAsSkill("chuiniu"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ChuiniuCard");
    }

    virtual const Card *viewAs() const{
        return new ChuiniuCard;
    }
};

class Chuiniu: public TriggerSkill{
public:
    Chuiniu():TriggerSkill("chuiniu"){
        view_as_skill = new ChuiniuViewAsSkill;
        events << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        return false;
    }
};

JoyGeneralPackage::JoyGeneralPackage()
    :Package("joyer")
{
    General *tiger = new General(this, "tiger", "god", 3, true, true);
    tiger->addSkill(new Skill("pu"));
    tiger->addSkill(new Skill("xian"));
    tiger->addSkill(new Skill("jian"));

    General *miheng = new General(this, "miheng", "god", 3);
    miheng->addSkill(new Yulu);
    miheng->addSkill(new Numa);
    miheng->addSkill(new Jieao);
    skills << new Fanchun;

    General *maque = new General(this, "maque", "god", 12);
    maque->addSkill(new Timer);
    maque->addSkill(new Lingyu);
    maque->addSkill(new Zhuangche);
    maque->addSkill(new Zouma);
    maque->addSkill(new Skill("jizha"));

    General *chuiniu = new General(this, "chuiniu", "god", 5);
    chuiniu->addSkill(new Chuiniu);

    addMetaObject<YuluCard>();
    addMetaObject<ViewMyWordsCard>();
    addMetaObject<ZhuangcheCard>();
    addMetaObject<ChuiniuCard>();

    type = GeneralPack;
}

ADD_PACKAGE(Kuso)
ADD_PACKAGE(Joy)
ADD_PACKAGE(JoyGeneral)
