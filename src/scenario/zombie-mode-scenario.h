#ifndef ZOMBIE_MODE_H
#define ZOMBIE_MODE_H

#include "scenario.h"
#include "standard-skillcards.h"
#include "tocheck.h"

class ZombieScenario : public Scenario{
    Q_OBJECT

public:
    explicit ZombieScenario();

    virtual bool exposeRoles() const;
    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual void onTagSet(Room *room, const QString &key) const;
    virtual bool generalSelection() const;
    virtual AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;

private:
    QStringList females;
};

class GanranEquip: public IronChain{
    Q_OBJECT

public:
    Q_INVOKABLE GanranEquip(Card::Suit suit, int number);
};

class QingnangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QingnangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class PeachingCard: public QingnangCard{
    Q_OBJECT

public:
    Q_INVOKABLE PeachingCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};


#endif // ZOMBIE_MODE_H
