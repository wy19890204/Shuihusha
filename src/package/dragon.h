#ifndef DRAGONPACKAGE_H
#define DRAGONPACKAGE_H

#include "package.h"
#include "card.h"

class TaolueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TaolueCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ShexinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShexinCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QianxianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QianxianCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DragonPackage: public Package{
    Q_OBJECT

public:
    DragonPackage();
};

#endif // DRAGONPACKAGE_H
