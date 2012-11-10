#ifndef SNAKEPACKAGE_H
#define SNAKEPACKAGE_H

#include "package.h"
#include "card.h"

class SinueCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SinueCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FangzaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FangzaoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FeizhenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FeizhenCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class SnakePackage: public GeneralPackage{
    Q_OBJECT

public:
    SnakePackage();
};

#endif // SNAKEPACKAGE_H
