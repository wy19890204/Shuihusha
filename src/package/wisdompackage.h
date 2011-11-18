#ifndef WISDOM_H
#define WISDOM_H

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
/*
class WeidaiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE WeidaiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class HouyuanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HouyuanCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

*/
class WisdomPackage: public Package{
    Q_OBJECT

public:
    WisdomPackage();
};

#endif // WISDOM_H
