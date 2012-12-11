#ifndef WARLORDS_MODE_H
#define WARLORDS_MODE_H

#include "scenario.h"

class WarlordsScenario : public Scenario{
    Q_OBJECT

public:
    explicit WarlordsScenario();

    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual void onTagSet(Room *room, const QString &key) const;
    virtual bool lordWelfare(const ServerPlayer *player) const;
    virtual AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;
};

#endif // WARLORDS_MODE_H
