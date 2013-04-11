#ifndef CHEATDIALOG_H
#define CHEATDIALOG_H

#include "photo.h"
#include "dashboard.h"
#include <QDialog>
#include <QSpinBox>
#include <QListWidget>

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
    CheatDialog(QWidget *parent);
    QTabWidget *tab_widget;
    QComboBox *killtype;

private:
    QWidget *createDamageMakeTab();
    QWidget *createDeathNoteTab();
    QWidget *createSetStateTab();
    QComboBox *damage_source;
    QComboBox *damage_target;
    QComboBox *damage_nature;
    QSpinBox *damage_point;
    QComboBox *killer, *victim;

    QComboBox *target;
    QLineEdit *general;
    QLineEdit *kingdom;
    QLineEdit *role;
    QLineEdit *sex;

protected:
    virtual const QString makeData();
    virtual void accept();

private slots:
    void doApply();
    void disableSource();
    void loadState(int index);
};

#endif // CHEATDIALOG_H
