#ifndef CONTRACTSCENARIO_H
#define CONTRACTSCENARIO_H

#include "scenario.h"
#include "roomthread.h"

//class ServerPlayer;

class ContractScenario : public Scenario{
    Q_OBJECT

public:
    explicit ContractScenario();

    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual bool lordWelfare(const ServerPlayer *player) const;
    virtual AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;

    static ServerPlayer *getComrade(const ServerPlayer *player);
    QList<ServerPlayer *> getComraded(Room *room) const;
    void setComrade(ServerPlayer *player, ServerPlayer *comrade = NULL) const;
    void annex(ServerPlayer *seme, ServerPlayer *uke) const;
    void rupture(ServerPlayer *seme, ServerPlayer *uke = NULL) const;
};

#endif // CONTRACTSCENARIO_H
