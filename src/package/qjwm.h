#ifndef QJWMPACKAGE_H
#define QJWMPACKAGE_H

#include "package.h"
#include "card.h"

class QJWMPackage: public Package{
    Q_OBJECT

public:
    QJWMPackage();
};

class TaolueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE TaolueCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class XiaozaiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XiaozaiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ButianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ButianCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // QJWMPACKAGE_H
