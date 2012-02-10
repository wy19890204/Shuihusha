#ifndef YBYTPACKAGE_H
#define YBYTPACKAGE_H

#include "package.h"
#include "card.h"

class YuanpeiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YuanpeiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class GuibingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuibingCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class HeiwuCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HeiwuCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

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

class ShexinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShexinCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class MaiyiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MaiyiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LongaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LongaoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HunjiuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HunjiuCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class YBYTPackage: public Package{
    Q_OBJECT

public:
    YBYTPackage();
};

#endif // YBYTPACKAGE_H
