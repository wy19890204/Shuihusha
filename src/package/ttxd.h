#ifndef TTXDPACKAGE_H
#define TTXDPACKAGE_H

#include "package.h"
#include "card.h"

class HuanshuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuanshuCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class TTXDPackage: public Package{
    Q_OBJECT

public:
    TTXDPackage();
};

#endif // TTXDPACKAGE_H
