#include "generaloverview.h"
#include "ui_generaloverview.h"
#include "engine.h"
#include "settings.h"
#include "clientstruct.h"
#include "client.h"

#include <QMessageBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QCommandLinkButton>
#include <QClipboard>

GeneralOverview::GeneralOverview(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneralOverview)
{
    ui->setupUi(this);

    button_layout = new QVBoxLayout;

    QGroupBox *group_box = new QGroupBox;
    group_box->setTitle(tr("Effects"));
    group_box->setLayout(button_layout);
    ui->scrollArea->setWidget(group_box);
    ui->skillTextEdit->setProperty("description", true);

    if(ServerInfo.isPlay && Config.value("FreeChange", false).toBool()){
        ui->changeGeneralButton->show();
        connect(ui->changeGeneralButton, SIGNAL(clicked()), this, SLOT(askChange()));
    }
    else
        ui->changeGeneralButton->hide();
}

void GeneralOverview::fillGenerals(const QList<const General *> &generals){
    QList<const General *> copy_generals = generals;
    QMutableListIterator<const General *> itor = copy_generals;
    while(itor.hasNext()){
        if(itor.next()->isTotallyHidden())
            itor.remove();
    }

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(copy_generals.length());
    ui->tableWidget->setIconSize(QSize(20,20));
    QIcon lord_icon("image/system/roles/lord.png");

    int i;
    for(i=0; i<copy_generals.length(); i++){
        const General *general = copy_generals[i];

        QString name, kingdom, gender, max_hp, package;

        name = Sanguosha->translate(general->objectName());
        kingdom = Sanguosha->translate(general->getKingdom());
        gender = general->isMale() ? tr("Male") : tr("Female");
        max_hp = QString::number(general->getMaxHp());
        for(int n = 1; n <= 3; n++){
            if(general->hasSkill("#hp-" + QString::number(n))){
                max_hp = QString::number(general->getMaxHp() - n) + "/" + QString::number(general->getMaxHp());
                break;
            }
        }
        package = Sanguosha->translate(general->getPackage());

        QString nickname = Sanguosha->translate("#" + general->objectName());
        QTableWidgetItem *nickname_item;
        if(!nickname.startsWith("#"))
            nickname_item = new QTableWidgetItem(nickname);
        else
            nickname_item = new QTableWidgetItem(Sanguosha->translate("UnknowNick"));
        nickname_item->setData(Qt::UserRole, general->objectName());
        nickname_item->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *name_item = new QTableWidgetItem(name);
        name_item->setTextAlignment(Qt::AlignCenter);
        if(general->isLord()){
            name_item->setIcon(lord_icon);
            name_item->setTextAlignment(Qt::AlignVCenter);
        }

        if(general->isHidden()){
            nickname_item->setBackgroundColor(Qt::gray);
            nickname_item->setTextColor(Qt::white);
            //nickname_item->setToolTip("hidden");
            name_item->setBackgroundColor(Qt::gray);
            name_item->setTextColor(Qt::white);
        }
        QTableWidgetItem *kingdom_item = new QTableWidgetItem(kingdom);
        kingdom_item->setTextAlignment(Qt::AlignCenter);
        kingdom_item->setBackgroundColor(Sanguosha->getKingdomColor(general->getKingdom()));

        QTableWidgetItem *gender_item = new QTableWidgetItem(gender);
        gender_item->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *max_hp_item = new QTableWidgetItem(max_hp);
        max_hp_item->setTextAlignment(Qt::AlignCenter);

        //if(package.length() > 3)
        //    package.chop(2);
        QTableWidgetItem *package_item = new QTableWidgetItem(package);
        package_item->setTextAlignment(Qt::AlignCenter);

        QString id = Sanguosha->translate("$" + general->objectName());
        QTableWidgetItem *id_item;
        if(!id.startsWith("$"))
            id_item = new QTableWidgetItem(id);
        else
            id_item = new QTableWidgetItem("-1");
        id_item->setTextAlignment(Qt::AlignCenter);

        ui->tableWidget->setItem(i, 0, nickname_item);
        ui->tableWidget->setItem(i, 1, name_item);
        ui->tableWidget->setItem(i, 2, kingdom_item);
        ui->tableWidget->setItem(i, 3, gender_item);
        ui->tableWidget->setItem(i, 4, max_hp_item);
        ui->tableWidget->setItem(i, 5, package_item);
        ui->tableWidget->setItem(i, 6, id_item);
    }

    ui->tableWidget->setColumnWidth(0, 70);
    ui->tableWidget->setColumnWidth(1, 70);
    ui->tableWidget->setColumnWidth(2, 40);
    ui->tableWidget->setColumnWidth(3, 50);
    ui->tableWidget->setColumnWidth(4, 45);
    ui->tableWidget->setColumnWidth(5, 60);
    ui->tableWidget->setColumnWidth(6, 40);

    ui->tableWidget->setCurrentItem(ui->tableWidget->item(0,0));
}

void GeneralOverview::resetButtons(){
    QLayoutItem *child;
    while((child = button_layout->takeAt(0))){
        QWidget *widget = child->widget();
        if(widget)
            delete widget;
    }
}

GeneralOverview::~GeneralOverview()
{
    delete ui;
}

void GeneralOverview::addLines(const Skill *skill, int wake_index){
    QString skill_name = Sanguosha->translate(skill->objectName());
    QStringList sources = skill->getSources();

    if(sources.isEmpty()){
        QCommandLinkButton *button = new QCommandLinkButton(skill_name);

        button->setEnabled(false);
        button_layout->addWidget(button);
    }else{
        QRegExp rx(".+/(\\w+\\d?).ogg");
        for(int i = 0; i < sources.length(); i++){
            if(skill->objectName() == "yinyu" && i > 6) // wake skills
                break;
            if((skill->objectName() == "butian" || skill->objectName() == "qimen") && i > 1)
                break;
            QString general_name = ui->tableWidget->item(ui->tableWidget->currentRow(), 0)->data(Qt::UserRole).toString();
            if(skill->objectName() == "zhengfa"){
                if(general_name == "tongguan" && (i == 1 || i == 3 || i == 5))
                    continue;
                else if(general_name == "tongguanf" && (i == 0 || i == 2 || i == 4))
                    continue;
            }
            QString source = wake_index == 0 ? sources.at(i) : sources.at(wake_index - 1);
            if(!rx.exactMatch(source))
                continue;

            QString button_text = skill_name;
            if(sources.length() != 1){
                if(wake_index > 0)
                    button_text.append(tr(" (%1) [Wake]").arg(wake_index));
                else
                    button_text.append(QString(" (%1)").arg(i+1));
            }

            QCommandLinkButton *button = new QCommandLinkButton(button_text);
            button->setObjectName(source);
            button_layout->addWidget(button);

            QString filename = rx.capturedTexts().at(1);
            QString skill_line = Sanguosha->translate("$" + filename);
            button->setDescription(skill_line);

            connect(button, SIGNAL(clicked()), this, SLOT(playEffect()));

            addCopyAction(button);
            if(wake_index != 0)
                break;
        }
    }
}

void GeneralOverview::addCopyAction(QCommandLinkButton *button){
    QAction *action = new QAction(button);
    action->setData(button->description());
    button->addAction(action);
    action->setText(tr("Copy lines"));
    button->setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(action, SIGNAL(triggered()), this, SLOT(copyLines()));
}

void GeneralOverview::copyLines(){
    QAction *action = qobject_cast<QAction *>(sender());
    if(action){
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(action->data().toString());
    }
}

void GeneralOverview::on_tableWidget_itemSelectionChanged()
{
    int row = ui->tableWidget->currentRow();
    QString general_name = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toString();
    const General *general = Sanguosha->getGeneral(general_name);
    ui->generalPhoto->setPixmap(QPixmap(general->getPixmapPath("card")));

    QString resume = Sanguosha->translate("resume:" + general->objectName());
    if(!resume.startsWith("resume:"))
        ui->generalPhoto->setToolTip(resume);
    else
        ui->generalPhoto->setToolTip(Sanguosha->translate("DefaultResume"));
    if(Self && general->objectName() == Self->getGeneralName())
        ui->changeGeneralButton->setEnabled(false);
    else
        ui->changeGeneralButton->setEnabled(true);

    QList<const Skill *> skills = general->getVisibleSkillList();

    foreach(QString skill_name, general->getRelatedSkillNames()){
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if(skill)
            skills << skill;
    }

    ui->skillTextEdit->clear();

    resetButtons();

    foreach(const Skill *skill, skills){
        addLines(skill);
    }

    //QGroupBox *wake = new QGroupBox(tr("Wake Skills"));
    const Skill *wake_skill;
    if(general_name == "qiongying"){
        wake_skill = Sanguosha->getSkill("yinyu");
        for(int i = 8; i <= 13; i ++)
            addLines(wake_skill, i);
    }
    if(general_name == "fanrui"){
        wake_skill = Sanguosha->getSkill("butian");
        addLines(wake_skill, 3);
        addLines(wake_skill, 4);
        wake_skill = Sanguosha->getSkill("qimen");
        addLines(wake_skill, 3);
        addLines(wake_skill, 4);
    }

    QString last_word = Sanguosha->translate("~" + general->objectName());
    if(!last_word.startsWith("~")){
        QCommandLinkButton *death_button = new QCommandLinkButton(tr("Death"), last_word);
        button_layout->addWidget(death_button);

        connect(death_button, SIGNAL(clicked()), general, SLOT(lastWord()));

        addCopyAction(death_button);
    }
/*
    if(general_name == "caocao" || general_name == "shencc" || general_name == "shencaocao"){
        QCommandLinkButton *win_button = new QCommandLinkButton(tr("Victory"), tr(
                "Six dragons lead my chariot, "
                "I will ride the wind with the greatest speed."
                "With all of the feudal lords under my command,"
                "to rule the world with one name!"));

        button_layout->addWidget(win_button);
        addCopyAction(win_button);

        win_button->setObjectName("audio/system/win-cc.ogg");
        connect(win_button, SIGNAL(clicked()), this, SLOT(playEffect()));
    }
*/
    QString designer_text = Sanguosha->translate("designer:" + general->objectName());
    if(!designer_text.startsWith("designer:"))
        ui->designerLineEdit->setText(designer_text);
    else
        ui->designerLineEdit->setText(Sanguosha->translate("DefaultDesigner"));

    QString cv_text = Sanguosha->translate("cv:" + general->objectName());
    if(!cv_text.startsWith("cv:"))
        ui->cvLineEdit->setText(cv_text);
    else
        ui->cvLineEdit->setText(Sanguosha->translate("DefaultCV"));

    QString coder_text = Sanguosha->translate("coder:" + general->objectName());
    if(!coder_text.startsWith("coder:"))
        ui->coderLineEdit->setText(coder_text);
    else
        ui->coderLineEdit->setText(Sanguosha->translate("DefaultCoder"));

    QString illustrator_text = Sanguosha->translate("illustrator:" + general->objectName());
    if(!illustrator_text.startsWith("illustrator:"))
        ui->illustratorLineEdit->setText(illustrator_text);
    else
        ui->illustratorLineEdit->setText(Sanguosha->translate("DefaultIllustrator"));

    button_layout->addStretch();
    ui->skillTextEdit->append(general->getSkillDescription());
}

void GeneralOverview::playEffect()
{
    QObject *button = sender();
    if(button){
        QString source = button->objectName();
        if(!source.isEmpty())
            Sanguosha->playEffect(source);
    }
}

void GeneralOverview::askChange(){
    if(!Config.value("FreeChange", false).toBool())
        return;

    int row = ui->tableWidget->currentRow();
    QString general_name = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toString();
    if(general_name != Self->getGeneralName()){
        ClientInstance->requestCheatChangeGeneral(general_name);
        ui->changeGeneralButton->setEnabled(false);
    }
}

void GeneralOverview::on_tableWidget_itemDoubleClicked(QTableWidgetItem* item)
{
    if(Self)
        askChange();
}
