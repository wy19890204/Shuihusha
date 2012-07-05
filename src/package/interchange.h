#ifndef INTERCHANGEPACKAGE_H
#define INTERCHANGEPACKAGE_H

#include "package.h"
#include "card.h"

class JingtianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JingtianCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class XianhaiCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XianhaiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class InterChangePackage: public Package{
    Q_OBJECT

public:
    InterChangePackage();
};

#endif // SPPACKAGE_H
