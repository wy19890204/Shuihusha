#ifndef EVENTS_H
#define EVENTS_H

#include "package.h"
#include "card.h"

class EventsPackage: public CardPackage{
    Q_OBJECT

public:
    EventsPackage();
};

class EventsCard:public Card{
    Q_OBJECT

public:
    EventsCard(Suit suit, int number):Card(suit, number){}
    virtual QString getType() const;
    virtual CardType getTypeId() const;
    virtual QString getSubtype() const;
    virtual QString getEffectPath(bool is_male) const;
};

class Jiefachang:public EventsCard{
    Q_OBJECT

public:
    Q_INVOKABLE Jiefachang(Card::Suit suit, int number);

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class Daojia:public EventsCard{
    Q_OBJECT

public:
    Q_INVOKABLE Daojia(Card::Suit suit, int number);

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
};

class Tifanshi:public EventsCard{
    Q_OBJECT

public:
    Q_INVOKABLE Tifanshi(Card::Suit suit, int number);

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
};

class NinedayGirl:public EventsCard{
    Q_OBJECT

public:
    Q_INVOKABLE NinedayGirl(Card::Suit suit, int number);

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
};

class FuckGaolian:public EventsCard{
    Q_OBJECT

public:
    Q_INVOKABLE FuckGaolian(Card::Suit suit, int number);

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool isAvailable(const Player *player) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
};

class Jiangjieshi:public EventsCard{
    Q_OBJECT

public:
    Q_INVOKABLE Jiangjieshi(Card::Suit suit, int number);

    virtual bool isAvailable(const Player *player) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class NanaStars:public EventsCard{
    Q_OBJECT

public:
    Q_INVOKABLE NanaStars(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool isAvailable(const Player *player) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class Xiaobawang:public EventsCard{
    Q_OBJECT

public:
    Q_INVOKABLE Xiaobawang(Card::Suit suit, int number);

    virtual bool isAvailable(const Player *p) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // EVENTS_H
