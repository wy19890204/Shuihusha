#ifndef IMPASSEFIGHTSCENARIO_H
#define IMPASSEFIGHTSCENARIO_H

#include "scenario.h"

class ImpasseScenario : public Scenario{
    Q_OBJECT

public:
    explicit ImpasseScenario();

    virtual bool exposeRoles() const;
    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual bool unloadLordSkill() const;
};

#endif // IMPASSEFIGHTSCENARIO_H
