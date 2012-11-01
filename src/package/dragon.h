#ifndef DRAGONPACKAGE_H
#define DRAGONPACKAGE_H

#include "package.h"
#include "card.h"

class YuanyinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YuanyinCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class NushaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE NushaCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class DragonPackage: public Package{
    Q_OBJECT

public:
    DragonPackage();
};

#endif // DRAGONPACKAGE_H
