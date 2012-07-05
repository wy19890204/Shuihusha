#ifndef GOD_H
#define GOD_H

#include "package.h"
#include "card.h"
#include "skill.h"
#include "standard.h"

class GodPackage : public Package{
    Q_OBJECT

public:
    GodPackage();
};

class ZhushaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhushaCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class DuanbiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DuanbiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class FeihuangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FeihuangCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class MeiyuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MeiyuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class HuafoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuafoCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class ShensuanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShensuanCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // GOD_H
