#ifndef EXPPATTERN_H
#define EXPPATTERN_H

#include <package.h>
#include <card.h>
#include <player.h>

class ExpPattern : public CardPattern
{
public:
    ExpPattern(const QString &exp);
    virtual bool match(const Player *player, const Card *card) const;
private:
    QString exp;
    bool matchOne(const Player *player,const Card *card, QString exp) const;
    bool willThrow() const;
};

#endif // EXPPATTERN_H
