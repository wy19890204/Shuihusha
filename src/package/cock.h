#ifndef COCKPACKAGE_H
#define COCKPACKAGE_H

#include "package.h"

class FanwuCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FanwuCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class EyanCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE EyanCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class EyanSlashCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE EyanSlashCard();
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class ZhangshiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhangshiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class CockPackage: public GeneralPackage{
    Q_OBJECT

public:
    CockPackage();
};

#endif // COCKPACKAGE_H
