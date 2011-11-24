#ifndef BWQZPACKAGE_H
#define BWQZPACKAGE_H

#include "package.h"
#include "card.h"

class YuanyinCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YuanyinCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ShougeCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShougeCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class NushaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE NushaCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class QiaogongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QiaogongCard();
    void swapEquip(ServerPlayer *first, ServerPlayer *second, int index) const;

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class BWQZPackage: public Package{
    Q_OBJECT

public:
    BWQZPackage();
};

#endif // BWQZPACKAGE_H
