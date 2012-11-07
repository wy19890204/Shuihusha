#ifndef SHEEPPACKAGE_H
#define SHEEPPACKAGE_H

#include "package.h"
#include "card.h"

class NushaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE NushaCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class CihuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CihuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LingdiCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LingdiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FeiqiangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FeiqiangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class SheepPackage : public Package
{
    Q_OBJECT

public:
    SheepPackage();
};

#endif // SHEEPPACKAGE_H
