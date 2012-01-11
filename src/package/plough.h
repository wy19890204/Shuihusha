#ifndef PLOUGHPACKAGE_H
#define PLOUGHPACKAGE_H

#include "package.h"
#include "standard.h"

class Ecstasy: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Ecstasy(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool isAvailable(const Player *player) const;
    static bool IsAvailable(const Player *player);
};

class Drivolt:public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Drivolt(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Wiretap: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Wiretap(Card::Suit suit, int number);

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Assassinate: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Assassinate(Card::Suit suit, int number);

    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Counterplot:public Nullification{
    Q_OBJECT

public:
    Q_INVOKABLE Counterplot(Card::Suit suit, int number);
};

class Provistore:public DelayedTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Provistore(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void takeEffect(ServerPlayer *target, bool good = false) const;
};

class Treasury: public Disaster{
    Q_OBJECT

public:
    Q_INVOKABLE Treasury(Card::Suit suit, int number);
    virtual void takeEffect(ServerPlayer *target, bool good = false) const;
};

class Tsunami: public Disaster{
    Q_OBJECT

public:
    Q_INVOKABLE Tsunami(Card::Suit suit, int number);
    virtual void takeEffect(ServerPlayer *target, bool good = false) const;
};

class DoubleWhip:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE DoubleWhip(Card::Suit suit, int number);
};

class MeteorSword:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE MeteorSword(Card::Suit suit, int number);
};

class SunBow:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE SunBow(Card::Suit suit, int number);
};

class GoldArmor:public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE GoldArmor(Card::Suit suit, int number);
};

class PloughPackage: public Package{
    Q_OBJECT

public:
    PloughPackage();
};

#endif // PLOUGHPACKAGE_H
