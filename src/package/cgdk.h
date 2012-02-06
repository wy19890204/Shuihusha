#ifndef CGDKPACKAGE_H
#define CGDKPACKAGE_H

#include "package.h"
#include "card.h"

#include <QGroupBox>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QDialog>

class BingjiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE BingjiCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class YunchouCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE YunchouCard();

    virtual bool targetFixed() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual const Card *validate(const CardUseStruct *card_use) const;
};

class YunchouDialog: public QDialog{
    Q_OBJECT

public:
    static YunchouDialog *GetInstance();

public slots:
    void popup();
    void selectCard(QAbstractButton *button);

private:
    YunchouDialog();

    QAbstractButton *createButton(const Card *card);
    QButtonGroup *group;
    QHash<QString, const Card *> map;
};

class LingdiCard:public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LingdiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class LinmoCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE LinmoCard();

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
};

class ZhaixingCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ZhaixingCard();
    virtual void use(Room *room, ServerPlayer *source, const QList<ServerPlayer *> &targets) const;
};

class CGDKPackage : public Package
{
    Q_OBJECT

public:
    CGDKPackage();
};

#endif // CGDKPACKAGE_H
