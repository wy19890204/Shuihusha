#ifndef GIFTPACKAGE_H
#define GIFTPACKAGE_H

#include "package.h"
#include "standard.h"

class GiftPackage : public Package{
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

#endif // GIFTPACKAGE_H
