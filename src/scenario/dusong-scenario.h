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
    virtual bool generalSelection() const;
};

class DouzhanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DouzhanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

#endif // DUSONGSCENARIO_H
