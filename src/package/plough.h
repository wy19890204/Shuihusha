#ifndef PLOUGHPACKAGE_H
#define PLOUGHPACKAGE_H

#include "package.h"
#include "standard.h"

class Discuss:public AOE{
    Q_OBJECT

public:
    Q_INVOKABLE Discuss(Card::Suit suit, int number);
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Burn: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE Burn(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

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

class SnowStop:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE SnowStop(Card::Suit suit, int number);
};

class Shark:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Shark(Card::Suit suit, int number);
};

class SpaceAsk:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE SpaceAsk(Card::Suit suit, int number);
    virtual void onMove(const CardMoveStruct &move) const;
};

class SevenStar:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE SevenStar(Card::Suit suit, int number);
    virtual void onInstall(ServerPlayer *player) const;
};

class Rainbow:public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Rainbow(Card::Suit suit, int number);
};

class Pendant:public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE Pendant(Card::Suit suit, int number);
    void onUninstall(ServerPlayer *player) const;
};

class Scroll:public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE Scroll(Card::Suit suit, int number);
};

class Square:public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE Square(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class Mirage: public DefensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE Mirage(Card::Suit suit, int number);
};

class Weasel: public OffensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE Weasel(Card::Suit suit, int number);

    virtual void onInstall(ServerPlayer *player) const;
    virtual void onUninstall(ServerPlayer *player) const;
    virtual QString getEffectPath(bool is_male) const;

private:
    TriggerSkill *skill;
};

class PloughPackage: public Package{
    Q_OBJECT

public:
    PloughPackage();
};

class Inspiration: public GlobalEffect{
    Q_OBJECT

public:
    Q_INVOKABLE Inspiration(Card::Suit suit, int number);

    virtual bool isCancelable(const CardEffectStruct &effect) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class Haiqiu: public OffensiveHorse{
    Q_OBJECT

public:
    Q_INVOKABLE Haiqiu(Card::Suit suit, int number);

    virtual QString getEffectPath(bool is_male) const;
};

#endif // PLOUGHPACKAGE_H
