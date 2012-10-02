#ifndef ZCYNPACKAGE_H
#define ZCYNPACKAGE_H

#include "package.h"
#include "card.h"

class ZCYNPackage : public Package{
    Q_OBJECT

public:
    ZCYNPackage();
};

class CihuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CihuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // ZCYNPACKAGE_H
