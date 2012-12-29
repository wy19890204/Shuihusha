#ifndef LANDLORDSCENARIO_H
#define LANDLORDSCENARIO_H

#include "scenario.h"

class LandlordScenario : public Scenario{
    Q_OBJECT

public:
    explicit LandlordScenario();

    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual bool lordWelfare(const ServerPlayer *player) const;
    virtual bool unloadLordSkill() const;
    virtual bool generalSelection(Room *room) const;
};

class LesuoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LesuoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class FangdaiCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FangdaiCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // LANDLORDSCENARIO_H
