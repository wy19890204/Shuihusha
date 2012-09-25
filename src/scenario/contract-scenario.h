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
    virtual void onTagSet(Room *room, const QString &key) const;
    virtual bool generalSelection() const;
    virtual AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;

    static ServerPlayer *getSpouse(const ServerPlayer *player);
    QList<ServerPlayer *> getMarried(Room *room) const;
    void setSpouse(ServerPlayer *player, ServerPlayer *spouse = NULL) const;
    void marry(ServerPlayer *husband, ServerPlayer *wife) const;
    void divorce(ServerPlayer *enkemann, ServerPlayer *widow = NULL) const;
};

#endif // CONTRACTSCENARIO_H
