#ifndef FCDCPACKAGE_H
#define FCDCPACKAGE_H

#include "package.h"
#include "card.h"

class FCDCPackage : public Package{
    Q_OBJECT

public:
    FCDCPackage();
};

class XunlieCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XunlieCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LianzhuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LianzhuCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

#endif // FCDCPACKAGE_H
