#ifndef YI_H
#define YI_H

#include "package.h"
#include "card.h"

class YiPackage: public Package{
    Q_OBJECT

public:
    YiPackage();
};

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

#endif // YI_H
