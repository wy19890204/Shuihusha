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

#endif // YI_H
