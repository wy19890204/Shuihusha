#ifndef EVENTS_H
#define EVENTS_H

#include "package.h"
#include "card.h"

class EventsPackage: public Package{
    Q_OBJECT

public:
    EventsPackage();
};

class LuanwuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LuanwuCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class FangzhuCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FangzhuCard();
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class EventsCard:public Card{
    Q_OBJECT

public:
    EventsCard(Suit suit, int number):Card(suit, number){}
    virtual QString getType() const;
    virtual CardType getTypeId() const;
    virtual QString getSubtype() const;
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

#endif // EVENTS_H
