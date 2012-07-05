#include "chem-cards.h"

Acid::Acid(Suit suit, int number): BasicCard(suit, number)
{
    setObjectName("acid");
}

bool Acid::IsAvailable(const Player *player){
    return player->hasEquip("acid_buret")/* || player->canSlashWithoutCrossbow()*/;
}

bool Acid::isAvailable(const Player *player) const{
    return IsAvailable(player);
}

QString Acid::getSubtype() const{
    return "attack_card";
}

void Acid::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    BasicCard::use(room, source, targets);

    if(source->hasFlag("drank")){
        LogMessage log;
        log.type = "#UnsetDrank";
        log.from = source;
        room->sendLog(log);
    }
}

void Acid::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if(effect.from->hasFlag("drank")){
        room->setCardFlag(this, "drank");
        room->setPlayerFlag(effect.from, "-drank");
    }

    bool mi = false;
    if(effect.from->hasEquip("ph_meter")){
        JudgeStruct judge;
        judge.pattern = QRegExp("(.*):(.*):([1-6])");
        judge.reason = "ph_meter";
        judge.who = effect.to;
        judge.good = false;

        room->judge(judge);
        if(judge.isBad())
            mi = true;
    }

    const Card *base = !mi ? room->askForCard(effect.to, "Base", "ask-for-base:" + effect.from->objectName()) :NULL;
    if(base){
        room->setEmotion(effect.to, "base");
        if(effect.from->hasEquip("conical_flask") && effect.from->askForSkillInvoke("conical_flask"))
            effect.from->drawCards(1);
        if(effect.from->hasEquip("thermograph")){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club|spade):(.*)");
            judge.reason = "thermograph";
            judge.who = effect.from;
            judge.good = true;

            room->judge(judge);
            if(judge.isGood() && effect.to->hasEquip() && effect.from->askForSkillInvoke("thermograph")){
                room->throwCard(room->askForCardChosen(effect.to, effect.from, "e", "thermograph"));
            }
        }
        if(effect.from->hasEquip("test_tube")){
            if(effect.from->askForSkillInvoke("test_tube")){
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart):(.*)");
                judge.reason = "test_tube";
                judge.who = effect.from;
                judge.good = true;

                room->judge(judge);
                if(judge.isGood()){
                    RecoverStruct recover;
                    room->recover(effect.from, recover, true);
                }
            }
        }
    }
    else{
        DamageStruct damage;
        damage.card = this;
        damage.damage = 1;
        damage.to = effect.to;

        if(effect.from->isAlive())
            damage.from = effect.from;
        else
            damage.from = NULL;

        room->damage(damage);
    }
}

bool Acid::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return !targets.isEmpty();
}

bool Acid::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int slash_targets = 1;
    if(Self->hasEquip("dropper") && Self->isLastHandCard(this)){
        slash_targets = 3;
    }

    bool distance_limit = true;

    if(Self->hasWeapon("sun_bow") && isRed() && objectName() == "slash"){
        slash_targets ++;
    }

    if(targets.length() >= slash_targets)
        return false;

    return Self->canSlash(to_select, distance_limit);
}


Base::Base(Suit suit, int number): BasicCard(suit, number)
{
    setObjectName("base");
}

bool Base::IsAvailable(const Player *player){
    return player->hasWeapon("crossbow") || player->canSlashWithoutCrossbow();
}

bool Base::isAvailable(const Player *player) const{
    return IsAvailable(player);
}

QString Base::getSubtype() const{
    return "attack_card";
}

void Base::use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const{
    BasicCard::use(room, source, targets);

    if(source->hasFlag("drank")){
        LogMessage log;
        log.type = "#UnsetDrank";
        log.from = source;
        room->sendLog(log);
    }
}

void Base::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    if(effect.from->hasFlag("drank")){
        room->setCardFlag(this, "drank");
        room->setPlayerFlag(effect.from, "-drank");
    }

    bool mi = false;
    if(effect.from->hasEquip("ph_meter")){
        JudgeStruct judge;
        judge.pattern = QRegExp("(.*):(.*):([8-K])");
        judge.reason = "ph_meter";
        judge.who = effect.to;
        judge.good = false;

        room->judge(judge);
        if(judge.isBad())
            mi = true;
    }

    const Card *acid = !mi ? room->askForCard(effect.to, "Acid", "ask-for-acid:" + effect.from->objectName()) :NULL;
    if(acid){
        room->setEmotion(effect.to, "acid");
        if(effect.from->hasEquip("conical_flask") && effect.from->askForSkillInvoke("conical_flask"))
            effect.from->drawCards(1);
        if(effect.from->hasEquip("thermograph")){
            JudgeStruct judge;
            judge.pattern = QRegExp("(.*):(club|spade):(.*)");
            judge.reason = "thermograph";
            judge.who = effect.from;
            judge.good = true;

            room->judge(judge);
            if(judge.isGood() && effect.to->hasEquip() && effect.from->askForSkillInvoke("thermograph")){
                room->throwCard(room->askForCardChosen(effect.to, effect.from, "e", "thermograph"));
            }
        }
        if(effect.from->hasEquip("test_tube")){
            if(effect.from->askForSkillInvoke("test_tube")){
                JudgeStruct judge;
                judge.pattern = QRegExp("(.*):(heart):(.*)");
                judge.reason = "test_tube";
                judge.who = effect.from;
                judge.good = true;

                room->judge(judge);
                if(judge.isGood()){
                    RecoverStruct recover;
                    room->recover(effect.from, recover, true);
                }
            }
        }
    }
    else{
        DamageStruct damage;
        damage.card = this;
        damage.damage = 1;
        damage.to = effect.to;

        if(effect.from->isAlive())
            damage.from = effect.from;
        else
            damage.from = NULL;

        room->damage(damage);
    }
}

bool Base::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    return !targets.isEmpty();
}

bool Base::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    int slash_targets = 1;
    if(Self->hasEquip("dropper") && Self->isLastHandCard(this)){
        slash_targets = 3;
    }

    bool distance_limit = true;

    if(Self->hasWeapon("sun_bow") && isRed() && objectName() == "slash"){
        slash_targets ++;
    }

    if(targets.length() >= slash_targets)
        return false;

    return Self->canSlash(to_select, distance_limit);
}

Safflower::Safflower(Suit suit, int number)
    :Peach(suit, number){
    setObjectName("safflower");
    target_fixed = true;
}

PotassiumDichromate::PotassiumDichromate(Suit suit, int number)
    :Analeptic(suit, number){
    setObjectName("potassium_dichromate");
    target_fixed = true;
}

AquaRegia::AquaRegia(Suit suit, int number)
    :Dismantlement(suit, number){
    setObjectName("aqua_regia");
}

EDTA::EDTA(Suit suit, int number)
    :Nullification(suit, number){
    setObjectName("edta");
}

RefluxCondenser::RefluxCondenser(Suit suit, int number)
    :Snatch(suit, number){
    setObjectName("reflux_condenser");
}

Diatomite::Diatomite(Suit suit, int number)
    :SingleTargetTrick(suit, number, true){
    setObjectName("diatomite");
}

void Diatomite::onEffect(const CardEffectStruct &effect) const{
    ServerPlayer *first = effect.to;
    ServerPlayer *second = effect.from;
    Room *room = first->getRoom();

    //room->setEmotion(first, "diatomite-a");
    //room->setEmotion(second, "diatomite-b");

    QString flag = "none";
    forever{
        const Card *slash;
        if(second->hasSkill("wushuang")){
            room->playSkillEffect("wushuang");
            slash = room->askForCard(first, "slash", "@wushuang-slash-1:" + second->objectName());
            if(slash == NULL)
                break;

            slash = room->askForCard(first, "slash", "@wushuang-slash-2:" + second->objectName());
            if(slash == NULL)
                break;

        }else{
            if(flag == "none")
                slash = room->askForCard(first, "Acid,Base", "diatomite-first:" + second->objectName());
            else if(flag == "acid")
                slash = room->askForCard(first, "Acid", "diatomite-acid:" + second->objectName());
            else
                slash = room->askForCard(first, "Base", "diatomite-base:" + second->objectName());
            if(slash == NULL)
                break;
        }
        if(flag == "none")
            flag = slash->objectName();

        qSwap(first, second);
    }

    DamageStruct damage;
    damage.card = this;
    if(second->isAlive())
        damage.from = second;
    else
        damage.from = NULL;
    damage.to = first;

    room->damage(damage);
}

Acids::Acids(Suit suit, int number)
    :AOE(suit, number)
{
    setObjectName("acids");
}

void Acids::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *base = room->askForCard(effect.to, "Base", "acids-base:" + effect.from->objectName());
    if(base)
        room->setEmotion(effect.to, "base");
    else{
        DamageStruct damage;
        damage.card = this;
        damage.damage = 1;
        damage.to = effect.to;

        if(effect.from->isAlive())
            damage.from = effect.from;
        else
            damage.from = NULL;

        room->damage(damage);
    }
}

Bases::Bases(Suit suit, int number)
    :AOE(suit, number)
{
    setObjectName("bases");
}

void Bases::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.to->getRoom();
    const Card *acid = room->askForCard(effect.to, "Acid", "bases-acid:" + effect.from->objectName());
    if(acid)
        room->setEmotion(effect.to, "acid");
    else{
        DamageStruct damage;
        damage.card = this;
        damage.damage = 1;
        damage.to = effect.to;

        if(effect.from->isAlive())
            damage.from = effect.from;
        else
            damage.from = NULL;

        room->damage(damage);
    }
}

NegativeCatalyst::NegativeCatalyst(Suit suit, int number)
    :Indulgence(suit, number)
{
    setObjectName("negative_catalyst");
    target_fixed = false;
}

TestTube::TestTube(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("test_tube");
}

PHMeter::PHMeter(Suit suit, int number)
    :Weapon(suit, number, 2)
{
    setObjectName("ph_meter");
}

AcidBuret::AcidBuret(Suit suit, int number)
    :Weapon(suit, number, 1)
{
    setObjectName("acid_buret");
}

ConicalFlask::ConicalFlask(Suit suit, int number)
    :Weapon(suit, number, 3)
{
    setObjectName("conical_flask");
}

Dropper::Dropper(Suit suit, int number)
    :Weapon(suit, number, 4)
{
    setObjectName("dropper");
}

Thermograph::Thermograph(Suit suit, int number)
    :Weapon(suit, number, 5)
{
    setObjectName("thermograph");
}

ChemCardsPackage::ChemCardsPackage()
    :Package("chem_cards")
{
    QList<Card *> cards;
    cards
            << new Acid(Card::Spade, 2)
            << new Base(Card::Club, 2)
            << new Safflower(Card::Heart, 12)
            << new Acids(Card::Diamond, 12)
            << new Bases(Card::Diamond, 1)
            << new NegativeCatalyst(Card::Spade, 4)
            << new Diatomite(Card::Club, 5)
            << new RefluxCondenser(Card::Diamond, 12)
            << new EDTA(Card::Diamond, 1)
            << new AquaRegia(Card::Spade, 3)
            << new PotassiumDichromate(Card::Club, 7)

            << new TestTube(Card::Diamond, 11)
            << new PHMeter(Card::Spade, 11)
            << new AcidBuret(Card::Club, 11)
            << new ConicalFlask(Card::Heart, 11)
            << new Dropper(Card::Heart, 1)
            << new Thermograph(Card::Heart, 2);

    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
}

ADD_PACKAGE(ChemCards)
