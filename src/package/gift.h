#ifndef GIFTPACKAGE_H
#define GIFTPACKAGE_H

#include "package.h"
#include "standard.h"
#include "client.h"

class GiftPackage : public CardPackage{
    Q_OBJECT

public:
    GiftPackage();
};

class Zongzi: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Zongzi(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual QString getEffectPath(bool is_male) const;
    virtual bool isAvailable(const Player *quyuan) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class Moonpie: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE Moonpie(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual bool isAvailable(const Player *change) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *houyi, const Player *change) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class RiceBall: public BasicCard{
    Q_OBJECT

public:
    Q_INVOKABLE RiceBall(Card::Suit suit, int number);
    virtual QString getSubtype() const;
    virtual bool isAvailable(const Player *change) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

#endif // GIFTPACKAGE_H
