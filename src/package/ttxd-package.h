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

class HaoshenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HaoshenCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class HuatianAiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuatianAiCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class HuatianCuoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuatianCuoCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

struct QimenStruct{
    QimenStruct();
    QString kingdom;
    QString generalA;
    QString generalB;
    int maxhp;
    QStringList skills;
};

Q_DECLARE_METATYPE(QimenStruct);

class CujuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE CujuCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HuanshuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuanshuCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class WujiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE WujiCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class YanshouCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YanshouCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

typedef Skill SkillClass;
class TTXDPackage: public Package{
    Q_OBJECT

public:
    TTXDPackage();
};

#endif // TTXDPACKAGE_H
