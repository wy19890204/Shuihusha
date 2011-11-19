#ifndef TTXDPACKAGE_H
#define TTXDPACKAGE_H

#include "package.h"
#include "card.h"
#include "standard.h"

class GanlinCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GanlinCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class JuyiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JuyiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class CujuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CujuCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};
/*
class HouyuanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HouyuanCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

*/
class TTXDPackage: public Package{
    Q_OBJECT

public:
    TTXDPackage();
};

#endif // TTXDPACKAGE_H
