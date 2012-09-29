#ifndef DUSONGSCENARIO_H
#define DUSONGSCENARIO_H

#include "scenario.h"
#include "card.h"

class DusongScenario: public Scenario{
    Q_OBJECT

public:
    DusongScenario();

    virtual bool exposeRoles() const;
    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual void onTagSet(Room *room, const QString &key) const;
    virtual bool lordWelfare(const ServerPlayer *player) const;
    virtual bool generalSelection() const;
    virtual bool setCardPiles(const Card *card) const;
};

#endif // DUSONGSCENARIO_H
