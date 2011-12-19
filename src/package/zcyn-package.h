#ifndef ZCYNPACKAGE_H
#define ZCYNPACKAGE_H

#include "package.h"
#include "card.h"

class ZCYNPackage : public Package{
    Q_OBJECT

public:
    ZCYNPackage();
};

class QuhuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QuhuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class JiemingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JiemingCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QiangxiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QiangxiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // ZCYNPACKAGE_H
