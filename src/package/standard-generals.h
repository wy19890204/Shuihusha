#ifndef STANDARDGENERALS_H
#define STANDARDGENERALS_H

#include "package.h"
#include "card.h"
#include "standard.h"
#include "common-skillcards.h"

class StandardPackage : public Package{
    Q_OBJECT

public:
    StandardPackage();
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
};

#include <QGroupBox>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>

class HuaceCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuaceCard();

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(const CardUseStruct *card_use) const;
};

class HuaceDialog: public QDialog{
    Q_OBJECT

public:
    static HuaceDialog *GetInstance();

public slots:
    void popup();
    void selectCard(QAbstractButton *button);

private:
    HuaceDialog();

    QAbstractButton *createButton(const Card *card);
    QButtonGroup *group;
    QHash<QString, const Card *> map;
};

typedef Skill SkillClass;
class YixingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YixingCard();

    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

struct QimenStruct{
    QimenStruct();
    QString kingdom;
    QString generalA;
    QString generalB;
    int maxhp;
    QStringList skills;
};

Q_DECLARE_METATYPE(QimenStruct);

class DuijueCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DuijueCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HaoshenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HaoshenCard();

    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class SijiuCard: public QingnangCard{
    Q_OBJECT

public:
    Q_INVOKABLE SijiuCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
};

class MaidaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MaidaoCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class MAIdaoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE MAIdaoCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class FengmangCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FengmangCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class DaleiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE DaleiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

#endif // STANDARDGENERALS_H
