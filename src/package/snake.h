#ifndef SNAKEPACKAGE_H
#define SNAKEPACKAGE_H

#include "package.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"

class SinueCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SinueCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FangzaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FangzaoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FeizhenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FeizhenCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class FeizhenResponseCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FeizhenResponseCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class JiejiuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JiejiuCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ZhaoanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhaoanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XiangmaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XiangmaCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class SouguaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SouguaCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class SnakePackage: public GeneralPackage{
    Q_OBJECT

public:
    SnakePackage();
};

#endif // SNAKEPACKAGE_H
