#ifndef QLFDPACKAGE_H
#define QLFDPACKAGE_H

#include "package.h"
#include "card.h"

class YushuiCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YushuiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QLFDPackage: public Package{
    Q_OBJECT

public:
    QLFDPackage();
};

#endif // QLFDPACKAGE_H
