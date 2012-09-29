#ifndef CHANGBANSCENARIO_H
#define CHANGBANSCENARIO_H

#include "scenario.h"
#include "package.h"
#include "card.h"
#include "skill.h"
#include "player.h"

class ChangbanScenario: public Scenario{
    Q_OBJECT

public:
    ChangbanScenario();
    virtual bool exposeRoles() const;
    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual void onTagSet(Room *room, const QString &key) const;
    virtual bool lordWelfare(const ServerPlayer *player) const;
    virtual void generalSelection(Room *room) const;
};

class CBLongNuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CBLongNuCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class CBYuXueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CBYuXueCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class CBJuWuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CBJuWuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class CBChanSheCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CBChanSheCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class CBShiShenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CBShiShenCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // CHANGBANSCENARIO_H
