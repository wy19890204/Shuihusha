#ifndef MONKEYPACKAGE_H
#define MONKEYPACKAGE_H

#include "package.h"

class ShensuanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShensuanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

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

class MonkeyPackage: public GeneralPackage{
    Q_OBJECT

public:
    MonkeyPackage();
};

#endif // MONKEYPACKAGE_H
