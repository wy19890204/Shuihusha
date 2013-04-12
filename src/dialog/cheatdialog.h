#ifndef CHEATDIALOG_H
#define CHEATDIALOG_H

#include "photo.h"
#include "dashboard.h"
#include <QDialog>
#include <QSpinBox>
#include <QListWidget>
#include <QCheckBox>

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
    QComboBox *damage_source;
    QComboBox *damage_target;
    QComboBox *damage_nature;
    QSpinBox *damage_point;
    QComboBox *killer, *victim;

    QComboBox *target;
    QLineEdit *general, *kingdom, *role, *sex;
    QCheckBox *turn, *chain, *ecst, *drank;
    QLineEdit *mark;
    QLineEdit *poison, *sleep;

protected:
    virtual const QString makeData();
    virtual void accept();

private slots:
    void doApply();
    void disableSource();
    void loadState(int index);
};

#endif // CHEATDIALOG_H
