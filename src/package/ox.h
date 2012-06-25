#ifndef OXPACKAGE_H
#define OXPACKAGE_H

#include "package.h"
#include "card.h"

class OxPackage : public Package{
    Q_OBJECT

public:
    OxPackage();
};

class LianmaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LianmaCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class SheruCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SheruCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XunlieCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XunlieCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LianzhuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LianzhuCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class HuazhuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuazhuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // OXPACKAGE_H
