#ifndef CGDKPACKAGE_H
#define CGDKPACKAGE_H

#include "package.h"
#include "card.h"

class BingjiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BingjiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class LingdiCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LingdiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class CGDKPackage : public Package
{
    Q_OBJECT

public:
    CGDKPackage();
};

#endif // CGDKPACKAGE_H
