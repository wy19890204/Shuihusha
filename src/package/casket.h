#ifndef CASKETPACKAGE_H
#define CASKETPACKAGE_H

#include "package.h"
#include "engine.h"

class TumiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TumiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class CasketPackage : public GeneralPackage
{
    Q_OBJECT

public:
    CasketPackage();
};

#endif // CASKETPACKAGE_H
