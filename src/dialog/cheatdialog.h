#ifndef CHEATDIALOG_H
#define CHEATDIALOG_H

#include "photo.h"
#include "dashboard.h"
#include <QDialog>
#include <QSpinBox>
#include <QListWidget>
#include <QCheckBox>
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
    QComboBox *killtype;

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
    QComboBox *damage_nature;
    QSpinBox *damage_point;
    QComboBox *killer, *victim;

    QComboBox *target;
    QLineEdit *general, *kingdom, *role, *sex;
    QCheckBox *turn, *chain, *ecst, *drank;
    QLineEdit *mark;
    QLineEdit *poison, *sleep, *dizzy, *petro;
    QLineEdit *flags, *marks, *propty, *tag;

protected:
    virtual const QString makeData();
    virtual void accept();

private slots:
    void doApply();
    void doApplyExpert();
    void doClearExpert();
    void disableSource();
    void loadState(int index);
    void loadBase();
    void clearBase();
    void fillBase();
    void setGray(int index);
};

#endif // CHEATDIALOG_H
