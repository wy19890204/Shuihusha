#ifndef JOYPACKAGE_H
#define JOYPACKAGE_H

#include "package.h"
#include "standard.h"

class KusoPackage: public Package{
    Q_OBJECT

public:
    KusoPackage();
};

class JoyPackage: public Package{
    Q_OBJECT

public:
    JoyPackage();
};

class Shit:public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Shit(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual void onMove(const CardMoveStruct &move) const;

    static bool HasShit(const Card *card);
};

class Stink: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Stink(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual QString getEffectPath(bool is_male) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class Poison: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Poison(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual QString getEffectPath(bool is_male) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class JoyGeneralPackage: public Package{
    Q_OBJECT

public:
    JoyGeneralPackage();
};

class YuluCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YuluCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ViewMyWordsCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ViewMyWordsCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ZhuangcheCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhuangcheCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class ChuiniuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ChuiniuCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // JOYPACKAGE_H
