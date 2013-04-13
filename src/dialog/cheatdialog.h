#ifndef CHEATDIALOG_H
#define CHEATDIALOG_H

#include "photo.h"
#include "dashboard.h"
#include <QDialog>
#include <QSpinBox>
#include <QListWidget>
#include <QCheckBox>
#include <QRadioButton>
#include <QMenu>

class Window;

class ScriptExecutor: public QDialog{
    Q_OBJECT

public:
    ScriptExecutor(QWidget *parent);

public slots:
    void doScript();
};

class CheatDialog: public QDialog{
    Q_OBJECT

public:
    CheatDialog(QWidget *parent, ClientPlayer *Self);
    QTabWidget *tab_widget;
    QAbstractButton *kill, *unkill;

private:
    ClientPlayer *Self;
    QWidget *createDamageMakeTab();
    QWidget *createDeathNoteTab();
    QWidget *createSetStateTab();
    QPushButton *ok_button;
    QPushButton *cancel_button;
    QPushButton *apply_button;

    QComboBox *damage_source;
    QComboBox *damage_target;
    QButtonGroup *damage_nature;
    QAbstractButton *normal, *fire, *thunder, *rec, *lh, *lmh, *rmh;
    QSpinBox *damage_point, *damage_card;
    QComboBox *killer, *victim;
    QButtonGroup *killtype;
    QAbstractButton *revive1, *revive2;

    QComboBox *target;
    QLineEdit *general, *kingdom, *role, *sex;
    QCheckBox *turn, *chain, *ecst, *drank, *shutup;
    QPushButton *extra_button;
    QButtonGroup *conjur_group;
    QAbstractButton *poison, *sleep, *dizzy, *petro;
    QLineEdit *conjur_text;
    QLineEdit *flags, *marks, *propty, *tag;
    QPushButton *flag_option;
    QPushButton *mark_option;

protected:
    virtual QString getPlayerString();
    virtual const Player *getPlayer();
    virtual const QString makeData();
    virtual const QStringList getExtraSkills();
    virtual void accept();

private slots:
    void doApply();
    void doApplyExpert();
    void doClearExpert();
    void disableSource(QAbstractButton* but);
    void loadState(int index);
    void loadBase();
    void clearBase();
    void fillBase();
    void loseSkill();
    void setGray(int index);
};

#endif // CHEATDIALOG_H
