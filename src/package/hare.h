#ifndef HAREPACKAGE_H
#define HAREPACKAGE_H

#include "package.h"
#include "card.h"

class HarePackage: public Package{
    Q_OBJECT

public:
    HarePackage();
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

#endif // HAREPACKAGE_H
