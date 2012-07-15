#ifndef OXPACKAGE_H
#define OXPACKAGE_H

#include "package.h"
#include "card.h"
#include "structs.h"

class OxPackage : public Package{
    Q_OBJECT

public:
    OxPackage();
};

class GuibingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuibingCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class HeiwuCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HeiwuCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ZhengfaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhengfaCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LianmaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LianmaCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class SheruCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SheruCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LianzhuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LianzhuCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class ButianCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ButianCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class DuomingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DuomingCard();
    virtual PlayerStar findPlayerByFlag(Room *room, const QString &flag) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class XunlieCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XunlieCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ShouwangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShouwangCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ZiyiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZiyiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // OXPACKAGE_H
