#ifndef HAREPACKAGE_H
#define HAREPACKAGE_H

#include "package.h"
#include "card.h"

class HarePackage: public Package{
    Q_OBJECT

public:
    HarePackage();
};

class SixiangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SixiangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LinmoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LinmoCard();

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class ZhaixingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhaixingCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class BinggongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BinggongCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class SheyanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SheyanCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class YijieCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YijieCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class HuatianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuatianCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ShemiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShemiCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // HAREPACKAGE_H
