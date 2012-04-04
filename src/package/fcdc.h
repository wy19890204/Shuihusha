#ifndef FCDCPACKAGE_H
#define FCDCPACKAGE_H

#include "package.h"
#include "card.h"

class FCDCPackage : public Package{
    Q_OBJECT

public:
    FCDCPackage();
};

class XunlieCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XunlieCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HuazhuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuazhuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // FCDCPACKAGE_H
