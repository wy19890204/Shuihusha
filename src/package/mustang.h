#ifndef MUSTANGPACKAGE_H
#define MUSTANGPACKAGE_H

#include "package.h"
#include "card.h"

class MustangPackage : public Package{
    Q_OBJECT

public:
    MustangPackage();
};

class HuazhuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuazhuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class BingjiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BingjiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class MaiyiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MaiyiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class HunjiuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HunjiuCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class ZishiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZishiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // MUSTANGPACKAGE_H
