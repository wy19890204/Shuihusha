#include "cardoverview.h"
#include "ui_cardoverview.h"
#include "engine.h"
#include "clientstruct.h"
#include "client.h"
#include "settings.h"

#ifdef USE_RCC
#include <QResource>
#endif
static CardOverview *Overview;

CardOverview *CardOverview::GetInstance(QWidget *main_window){
    if(Overview == NULL)
        Overview = new CardOverview(main_window);

#ifdef USE_RCC
    QResource::registerResource("image/big-card.rcc");
#endif
    return Overview;
}

CardOverview::CardOverview(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CardOverview)
{
    ui->setupUi(this);

    ui->tableWidget->setColumnWidth(0, 110);
    ui->tableWidget->setColumnWidth(1, 60);
    ui->tableWidget->setColumnWidth(2, 30);
    ui->tableWidget->setColumnWidth(3, 60);
    ui->tableWidget->setColumnWidth(4, 70);

    if(ServerInfo.isPlay && Config.FreeChooseCards){
        ui->getCardButton->show();
        connect(ui->getCardButton, SIGNAL(clicked()), this, SLOT(askCard()));
    }else
        ui->getCardButton->hide();

    ui->cardDescriptionBox->setProperty("description", true);
}

void CardOverview::loadFromAll(){
    int i, n = Sanguosha->getCardCount();
    ui->tableWidget->setRowCount(n);
    for(i=0; i<n ;i++){
        const Card *card = Sanguosha->getCard(i);
        addCard(i, card);
    }

    if(n>0)
        ui->tableWidget->setCurrentItem(ui->tableWidget->item(0,0));
}

void CardOverview::loadFromList(const QList<const Card*> &list){
    int i, n = list.length();
    ui->tableWidget->setRowCount(n);
    for(i=0; i<n; i++)
        addCard(i, list.at(i));

    if(n>0)
        ui->tableWidget->setCurrentItem(ui->tableWidget->item(0,0));
}

void CardOverview::addCard(int i, const Card *card){
    QString name = Sanguosha->translate(card->objectName());
    QIcon suit_icon = QIcon(QString("image/system/suit/%1.png").arg(card->getSuitString()));
    QString suit_str = Sanguosha->translate(card->getSuitString());
    QString point = card->getNumberString();
    QString type = Sanguosha->translate(card->getType());
    QString subtype = Sanguosha->translate(card->getSubtype());
    QString package = Sanguosha->translate(card->getPackage());

    QTableWidgetItem *name_item = new QTableWidgetItem(name);
    name_item->setData(Qt::UserRole, card->getId());

    ui->tableWidget->setItem(i, 0, name_item);
    ui->tableWidget->setItem(i, 1, new QTableWidgetItem(suit_icon, suit_str));
    ui->tableWidget->setItem(i, 2, new QTableWidgetItem(point));
    ui->tableWidget->setItem(i, 3, new QTableWidgetItem(type));
    ui->tableWidget->setItem(i, 4, new QTableWidgetItem(subtype));
    ui->tableWidget->setItem(i, 5, new QTableWidgetItem(package));
}

CardOverview::~CardOverview()
{
#ifdef USE_RCC
    QResource::unregisterResource("image/big-card.rcc");
#endif
    delete ui;
}

void CardOverview::on_tableWidget_itemSelectionChanged()
{
    int row = ui->tableWidget->currentRow();
    int card_id = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toInt();
    const Card *card = Sanguosha->getCard(card_id);
#ifdef USE_RCC
    QString pixmap_path = QString(":big-card/%1.png").arg(card->objectName());
#else
    QString pixmap_path = QString("image/big-card/%1.png").arg(card->objectName());
#endif

    ui->cardLabel->setPixmap(pixmap_path);

    ui->cardDescriptionBox->setText(card->getDescription());
}

void CardOverview::askCard(){
    if(!Config.FreeChooseCards)
        return;

    int row = ui->tableWidget->currentRow();
    if(row >= 0){
        int card_id = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toInt();
        ClientInstance->requestCheatGetOneCard(card_id);
    }
}

void CardOverview::on_tableWidget_itemDoubleClicked(QTableWidgetItem* item)
{
    if(Self)
        askCard();
}

void playEffect(int card_id, bool ismale){
    const Card *card = Sanguosha->getCard(card_id);
    if(card->inherits("Weapon") || card->inherits("Armor")){
        QString src = "E" + card->objectName();
        Sanguosha->playCardEffect(src, ismale);
    }
    else if(card->inherits("EventsCard") || card->inherits("IronChain")){
        QString src = "@";
        if(card->inherits("EventsCard")){
            src = src + card->objectName();
            src = src + QString::number(qrand() % 2 + 1);
        }
        else{
            QString iron = qrand() % 2 == 0 ? "tiesuo" : "recast";
            src = src + iron;
        }
        Sanguosha->playCardEffect(src, ismale);
    }
    else
        Sanguosha->playCardEffect(card->objectName(), ismale);
}

void CardOverview::on_malePlayButton_clicked()
{
    int row = ui->tableWidget->currentRow();
    if(row >= 0){
        int card_id = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toInt();
        playEffect(card_id, true);
    }
}

void CardOverview::on_femalePlayButton_clicked()
{
    int row = ui->tableWidget->currentRow();
    if(row >= 0){
        int card_id = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toInt();
        playEffect(card_id, false);
    }
}
