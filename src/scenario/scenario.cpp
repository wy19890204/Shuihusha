#include "scenario.h"
#include "engine.h"

Scenario::Scenario(const QString &name)
    :Package(name), rule(NULL)
{
    type = SpecialPack;
}

int Scenario::getPlayerCount() const{
    return 1 + loyalists.length() + rebels.length() + renegades.length();
}

ScenarioRule *Scenario::getRule() const{
    return rule;
}

bool Scenario::exposeRoles() const{
    return true;
}

void Scenario::getRoles(char *roles) const{
    qstrcpy(roles, "Z");

    int i;
    for(i=0; i<loyalists.length(); i++)
        strcat(roles, "C");

    for(i=0; i<rebels.length(); i++)
        strcat(roles, "N");

    for(i=0; i<rebels.length(); i++)
        strcat(roles, "F");
}

void Scenario::assign(QStringList &generals, QStringList &roles) const{
    generals << lord << loyalists << rebels << renegades;
    qShuffle(generals);

    foreach(QString general, generals){
        if(general == lord)
            roles << "lord";
        else if(loyalists.contains(general))
            roles << "loyalist";
        else if(rebels.contains(general))
            roles << "rebel";
        else
            roles << "renegade";
    }
}

bool Scenario::lordWelfare(const ServerPlayer *player) const{ // if player maxhp +1 on game start, return true
    return player->isLord() && player->getRoom()->getPlayerCount() > 4;
}

bool Scenario::generalSelection() const{ // if need choose general freely, return true
    return false;
}

bool Scenario::setCardPiles(const Card *) const{ // if the unuse this card, return true
    return false;
}

void Scenario::Prerun(Room *, QList<ServerPlayer *>) const{ // if generalSelection == true then do
    return;
}

AI::Relation Scenario::relationTo(const ServerPlayer *a, const ServerPlayer *b) const{
    return AI::GetRelation(a, b);
}

Q_GLOBAL_STATIC(ScenarioHash, Scenarios)
ScenarioHash& ScenarioAdder::scenarios(){
    return *(::Scenarios());
}
