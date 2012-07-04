#ifndef CHEMCARDSPACKAGE_H
#define CHEMCARDSPACKAGE_H

#include "package.h"
#include "card.h"

class ChemCardsPackage: public Package{
    Q_OBJECT

public:
    ChemCardsPackage();
};

class GuiouCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuiouCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ZhonglianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhonglianCard();

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class MingwangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MingwangCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // CHEMCARDSPACKAGE_H
