#ifndef STANDARDSKILLCARDS_H
#define STANDARDSKILLCARDS_H

#include "skill.h"
#include "card.h"

class CheatCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CheatCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ChangeCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChangeCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class UbunaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE UbunaCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class UbuncCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE UbuncCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class UbundCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE UbundCard();
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class UbuneCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE UbuneCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QiapaiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QiapaiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // STANDARDSKILLCARDS_H
