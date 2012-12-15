#ifndef WHEELFIGHTSCENARIO_H
#define WHEELFIGHTSCENARIO_H

#include "scenario.h"
#include "roomthread.h"

class WheelFightScenario : public Scenario{
    Q_OBJECT

public:
    explicit WheelFightScenario();

    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
};

#endif // WHEELFIGHTSCENARIO_H
