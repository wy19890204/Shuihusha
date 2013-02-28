#ifndef WHEELFIGHTSCENARIO_H
#define WHEELFIGHTSCENARIO_H

#include "scenario.h"

class WheelFightScenario : public Scenario{
    Q_OBJECT

public:
    explicit WheelFightScenario();

    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual int lordGeneralCount() const;
    virtual int swapCount() const;
};

#endif // WHEELFIGHTSCENARIO_H
