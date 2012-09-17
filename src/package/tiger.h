#ifndef TIGERPACKAGE_H
#define TIGERPACKAGE_H

#include "package.h"
#include "card.h"

class TigerPackage: public Package{
    Q_OBJECT

public:
    TigerPackage();
};

class XiaozaiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XiaozaiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};
/*
class TaolueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TaolueCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class HuazhuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuazhuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};
*/
#endif // TIGERPACKAGE_H
