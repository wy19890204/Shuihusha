#ifndef YBYTPACKAGE_H
#define YBYTPACKAGE_H

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

class YBYTPackage: public Package{
    Q_OBJECT

public:
    YBYTPackage();
};

#endif // YBYTPACKAGE_H
