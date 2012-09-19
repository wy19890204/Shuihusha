#ifndef TIGERPACKAGE_H
#define TIGERPACKAGE_H

#include "package.h"
#include "card.h"

class TigerPackage: public Package{
    Q_OBJECT

public:
    TigerPackage();
};

class JintangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JintangCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class LiejiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LiejiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class HuweiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuweiCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
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
