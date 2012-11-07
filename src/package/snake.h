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

class SnakePackage: public Package{
    Q_OBJECT

public:
    SnakePackage();
};

#endif // SNAKEPACKAGE_H
