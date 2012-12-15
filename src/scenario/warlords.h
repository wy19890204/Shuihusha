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
    virtual bool lordWelfare(const ServerPlayer *player) const;
    virtual AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;
};

#endif // WARLORDS_MODE_H
#ifndef ARTHURFERRIS_MODE_H
#define ARTHURFERRIS_MODE_H

class ArthurFerrisScenario : public Scenario{
    Q_OBJECT

public:
    explicit ArthurFerrisScenario();

    virtual bool setCardPiles(const Card *card) const;
    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;
};

#endif // ARTHURFERRIS_MODE_H
