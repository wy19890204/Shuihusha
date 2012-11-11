#ifndef CHOOSECARDDIALOG_H
#define CHOOSECARDDIALOG_H

#include <QDialog>
#include <QGroupBox>
#include <QButtonGroup>

class CardKindDialog: public QDialog{
    Q_OBJECT

public:
    explicit CardKindDialog(QWidget *parent);

private:
    QButtonGroup *group;

private slots:
    void chooseCard();

signals:
    void card_kind_chosen(const QString &name);
};

#endif // CHOOSECARDDIALOG_H
