#ifndef MUSTANGPACKAGE_H
#define MUSTANGPACKAGE_H

#include "package.h"
#include "card.h"

class MustangPackage : public Package{
    Q_OBJECT

public:
    MustangPackage();
};

class CihuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CihuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // MUSTANGPACKAGE_H
