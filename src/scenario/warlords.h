#ifndef WARLORDS_MODE_H
#define WARLORDS_MODE_H

#include "scenario.h"
#include "maneuvering.h"

class WarlordsScenario : public Scenario{
    Q_OBJECT

public:
    explicit WarlordsScenario();

    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual QString getRoles() const;
    virtual void onTagSet(Room *room, const QString &key) const;
    virtual bool lordWelfare(const ServerPlayer *player) const;
};

#endif // WARLORDS_MODE_H
