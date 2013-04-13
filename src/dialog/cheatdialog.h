#ifndef CHEATDIALOG_H
#define CHEATDIALOG_H

#include "photo.h"
#include "dashboard.h"
#include <QDialog>
#include <QLabel>
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
    QLabel *target_label, *point_label;
    QButtonGroup *damage_nature;
    QAbstractButton *normal, *fire, *thunder, *rec, *lh, *lmh, *rmh;
    QSpinBox *damage_point;
    QComboBox *killer, *victim;
    QButtonGroup *killtype;

    QComboBox *target;
    QLineEdit *general, *kingdom, *role, *sex;
    QCheckBox *turn, *chain, *ecst, *drank;
    QPushButton *extra_button;
    QLineEdit *poison, *sleep, *dizzy, *petro;
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
