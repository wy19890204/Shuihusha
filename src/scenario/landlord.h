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
    virtual bool generalSelection(Room *room) const;
    virtual AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;

    static ServerPlayer *getComrade(const ServerPlayer *player);
    QList<ServerPlayer *> getComraded(Room *room) const;
    void setComrade(ServerPlayer *player, ServerPlayer *comrade = NULL) const;
    void annex(ServerPlayer *seme, ServerPlayer *uke) const;
    void rupture(ServerPlayer *seme, ServerPlayer *uke = NULL) const;
};

#endif // LANDLORDSCENARIO_H
