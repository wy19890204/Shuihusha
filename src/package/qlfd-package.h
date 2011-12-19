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

class FanwuCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FanwuCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QianxianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QianxianCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ShouwangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShouwangCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ZiyiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZiyiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ZishiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZishiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class QLFDPackage: public Package{
    Q_OBJECT

public:
    QLFDPackage();
};

#endif // QLFDPACKAGE_H
