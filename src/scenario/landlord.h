#ifndef LANDLORDSCENARIO_H
#define LANDLORDSCENARIO_H

#include "scenario.h"
#include "roomthread.h"

class LandlordScenario : public Scenario{
    Q_OBJECT

public:
    explicit LandlordScenario();

    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual void onTagSet(Room *room, const QString &key) const;
    virtual bool lordWelfare(const ServerPlayer *player) const;
    virtual bool setCardPiles(const Card *card) const;
    virtual bool generalSelection(Room *room) const;
};

class FangdaiCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FangdaiCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // LANDLORDSCENARIO_H
