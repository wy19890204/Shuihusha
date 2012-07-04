#ifndef XZDDPACKAGE_H
#define XZDDPACKAGE_H

#include "package.h"
#include "card.h"

class XZDDPackage: public Package{
    Q_OBJECT

public:
    XZDDPackage();
};

class BinggongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BinggongCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class FeiqiangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FeiqiangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // XZDDPACKAGE_H
