#ifndef ZCYNPACKAGE_H
#define ZCYNPACKAGE_H

#include "package.h"
#include "card.h"

class ShouwangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShouwangCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // ZCYNPACKAGE_H
