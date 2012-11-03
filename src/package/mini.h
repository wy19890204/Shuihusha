#ifndef MINIGENERALS_H
#define MINIGENERALS_H

#include "package.h"
#include "card.h"

class FangdiaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FangdiaoCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &list) const;
};

class YinlangCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YinlangCard();

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

#endif // MINIGENERALS_H
