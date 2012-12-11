#ifndef RATPACKAGE_H
#define RATPACKAGE_H

#include "package.h"
#include "card.h"

class BuzhenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BuzhenCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ShougeCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShougeCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class HuanshuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuanshuCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class YuanpeiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YuanpeiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class RatPackage: public GeneralPackage{
    Q_OBJECT

public:
    RatPackage();
};

#endif // RATPACKAGE_H
