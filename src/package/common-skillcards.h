#ifndef STANDARDSKILLCARDS_H
#define STANDARDSKILLCARDS_H

#include "skill.h"
#include "card.h"

class QingnangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QingnangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FreeRegulateCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FreeRegulateCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class SacrificeCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SacrificeCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class UbunbCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE UbunbCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class UbuncCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE UbuncCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
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

class FanduiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FanduiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ZhichiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhichiCard();
    QStringList allPiles() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // STANDARDSKILLCARDS_H
