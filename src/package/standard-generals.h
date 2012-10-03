#ifndef STANDARDGENERALS_H
#define STANDARDGENERALS_H

#include "package.h"
#include "card.h"
#include "standard.h"
#include "common-skillcards.h"

class StandardPackage : public Package{
    Q_OBJECT

public:
    StandardPackage();
};

class JianaiCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE JianaiCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class FeigongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FeigongCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ShengxueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShengxueCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YoulanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YoulanCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class SuoshaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE SuoshaCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HuomeiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuomeiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ShangtongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShangtongCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class PofuCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE PofuCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ShenwuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShenwuCard();

    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class DushaCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DushaCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class LuoshengCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LuoshengCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class EnchouCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE EnchouCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ShouyaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShouyaoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class GuirouCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE GuirouCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // STANDARDGENERALS_H
