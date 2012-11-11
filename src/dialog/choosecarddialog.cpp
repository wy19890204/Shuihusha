#include "card.h"
#include "engine.h"
#include "client.h"
#include "settings.h"
#include "choosecarddialog.h"

#include <QSignalMapper>
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QTabWidget>

QWidget *createTab(const QList<const Card *> &cards, QButtonGroup *group){
    QWidget *tab = new QWidget;

    QGridLayout *layout = new QGridLayout;
    layout->setOriginCorner(Qt::TopLeftCorner);

    const int columns = 4;

    QStringList carded;
    int i = 0;
    foreach(const Card *card, cards){
        QString card_name = card->objectName();
        if(card->inherits("Horse"))
            card_name = card->getSubtype();
        if(carded.contains(card_name))
            continue;

        QString text = QString("%1[%2]")
                       .arg(Sanguosha->translate(card_name))
                       .arg(Sanguosha->translate(card->getType()));

        QAbstractButton *button;
        button = new QCheckBox(text);
        button->setObjectName(card_name);
        button->setToolTip(card->getDescription());

        group->addButton(button);
        carded << card_name;

        int row = i / columns;
        int column = i % columns;
        i++;
        layout->addWidget(button, row, column);
    }

    tab->setLayout(layout);

    return tab;
}

CardKindDialog::CardKindDialog(QWidget *parent)
    :QDialog(parent)
{
    setWindowTitle(tr("Card kind choose"));

    QTabWidget *tab_widget = new QTabWidget;

    group = new QButtonGroup(this);
    group->setExclusive(false);

    QList<const Card *> all_cards = Sanguosha->findChildren<const Card *>();
    QMap<QString, QList<const Card*> > map;
    foreach(const Card *card, all_cards)
        map[card->getType()] << card;

    QStringList types;
    types << "basic" << "trick" << "equip" << "events";

    foreach(QString type, types){
        QList<const Card*> cards = map[type];

        if(!cards.isEmpty()){
            QWidget *tab = createTab(cards, group);
            tab_widget->addTab(tab, Sanguosha->translate(type));
        }
    }

    QPushButton *ok_button = new QPushButton(tr("OK"));
    connect(ok_button, SIGNAL(clicked()), this, SLOT(chooseCard()));

    QPushButton *cancel_button = new QPushButton(tr("Cancel"));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

    QHBoxLayout *button_layout = new QHBoxLayout;
    button_layout->addStretch();
    button_layout->addWidget(ok_button);
    button_layout->addWidget(cancel_button);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tab_widget);
    layout->addLayout(button_layout);

    setLayout(layout);
}

void CardKindDialog::chooseCard(){
    QList<QAbstractButton *> buttons = group->buttons();
    QStringList firsts;
    foreach(QAbstractButton *button, buttons){
        if(!button->isChecked())
            continue;
        firsts << button->objectName();
    }
    emit card_kind_chosen(firsts.join("+"));

    accept();
}

