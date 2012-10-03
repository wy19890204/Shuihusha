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

class ShangtongCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShangtongCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


#endif // STANDARDGENERALS_H
