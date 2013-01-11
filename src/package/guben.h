#ifndef GUBENPACKAGE_H
#define GUBENPACKAGE_H

#include "package.h"

class GubenPackage: public GeneralPackage{
    Q_OBJECT

public:
    GubenPackage();
};
/*
class TuxiPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TuxiPassCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LuoyiPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LuoyiPassCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class TuodaoPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TuodaoPassCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LiegongPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LiegongPassCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class JianhunPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JianhunPassCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class FanjianPassCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FanjianPassCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};
*/
#endif // GUBENPACKAGE_H
