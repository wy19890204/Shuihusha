#ifndef SCENARIO_H
#define SCENARIO_H

#include "engine.h"
#include "settings.h"
#include "package.h"
#include "ai.h"

class Room;
class ScenarioRule;

#include <QMap>

class Scenario : public Package
{
    Q_OBJECT

public:
    explicit Scenario(const QString &name);    
    ScenarioRule *getRule() const;

    virtual void run(Room *room) const;
    virtual bool exposeRoles() const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;
    virtual void onTagSet(Room *room, const QString &key) const;
    virtual bool lordWelfare(const ServerPlayer *player) const;
    virtual int lordGeneralCount() const;
    virtual bool unloadLordSkill() const;
    virtual bool generalSelection(Room *room) const;
    virtual bool setCardPiles(const Card *card) const;
    virtual int swapCount() const;
    virtual QString setBackgroundMusic() const;

protected:
    QString lord;
    QStringList loyalists, rebels, renegades;
    ScenarioRule *rule;
};

typedef QHash<QString, Scenario *> ScenarioHash;

class ScenarioAdder{
public:
    ScenarioAdder(const QString &name, Scenario *scenario){
        scenarios()[name] = scenario;
    }

    static ScenarioHash& scenarios(void);
};

#define ADD_SCENARIO(name) static ScenarioAdder name##ScenarioAdder(#name, new name##Scenario);

#endif // SCENARIO_H
