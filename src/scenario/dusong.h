#ifndef DUSONGSCENARIO_H
#define DUSONGSCENARIO_H

#include "scenario.h"
#include "card.h"

class DusongScenario: public Scenario{
    Q_OBJECT

public:
    DusongScenario();

    virtual void run(Room *room) const;
    virtual bool exposeRoles() const;
    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual bool lordWelfare(const ServerPlayer *player) const;
    virtual bool generalSelection(Room *room) const;
    virtual bool setCardPiles(const Card *card) const;
    virtual int swapCount() const;
};

#endif // DUSONGSCENARIO_H
