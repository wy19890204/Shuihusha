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

    if(ServerInfo.isPlay && Config.value("Cheat/FreeChange", false).toBool()){
        addChangeAction(ui->changeGeneralButton);
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

        QString nickname, name, kingdom, gender, max_hp, package, id;

        nickname = general->getNickname();
        name = Sanguosha->translate(general->objectName());
        kingdom = Sanguosha->translate(general->getKingdom());
        gender = Sanguosha->translate(general->getGenderString());
        max_hp = general->getShowHp();
        package = Sanguosha->translate(general->getPackage());
        id = general->getId();

        if(nickname == "")
            nickname = Sanguosha->translate("UnknowNick");
        QTableWidgetItem *nickname_item = new QTableWidgetItem(nickname);
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

        QTableWidgetItem *id_item = new QTableWidgetItem(id);
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

bool GeneralOverview::isInvisibleSkill(const QString &skill_name, int index){
    //effect line cut down begin the index
    if(skill_name == "yinyu") // for mengshi_wake
        return index > 7;
    if(index > 2){  // butian&qimen for wudao_wake; other for landlord mode
        QStringList skills;
        skills << "butian" << "qimen" << "linse" << "duoming" << "shemi";
        return skills.contains(skill_name);
    }
    return false;
}

bool GeneralOverview::singleSkillFineTuning(const QString &general_name, const QString &skill_name, int index){
    if(skill_name == "zhengfa"){
        if(general_name == "tongguan") //tongguan show zhengfa1,3,5  pass 2,4,6
            return index % 2 == 0;
        else if(general_name == "tongguanf") //tongguanf show zhengfa2,4,6  pass 1,3,5
            return index % 2 == 1;
    }
    return false;
}

void GeneralOverview::addLines(const Skill *skill, int wake_index){
    QString skill_name = Sanguosha->translate(skill->objectName());
    QStringList sources = skill->getSources();

    if(sources.isEmpty()){
        QCommandLinkButton *button = new QCommandLinkButton(skill_name);

        button->setEnabled(false);
        button_layout->addWidget(button);
    }else{
        QRegExp rx(".+/(\\w+\\d?).(\\w+)");
        for(int i = 0; i < sources.length(); i++){
            if(isInvisibleSkill(skill->objectName(), i + 1))
                break;
            QString general_name = ui->tableWidget->item(ui->tableWidget->currentRow(), 0)->data(Qt::UserRole).toString();
            if(singleSkillFineTuning(general_name, skill->objectName(), i + 1))
                continue;
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

void GeneralOverview::addWakeLines(const QString &general_name){
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

    QString resum = Sanguosha->translate("resume:" + general->objectName());
    if(!resum.startsWith("resume:")){
        QString resume = Sanguosha->translate("resume::");
        if(general_name == "tora")
            resume = Sanguosha->translate("resume:;");
        for(int i=0; i < resum.length();i ++){
            resume.append(resum.at(i));
            if((i + 1) % 25 == 0)
                resume.append("<br />");
        }
        ui->generalPhoto->setToolTip(resume);
    }
    else
        ui->generalPhoto->setToolTip(QString());
    ui->generalPhoto->setWhatsThis("FAQ:"); //@todo
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

    foreach(const Skill *skill, skills)
        addLines(skill);

    addWakeLines(general_name);

    QString last_word = Sanguosha->translate("~" + general->objectName());
    if(!last_word.startsWith("~")){
        QCommandLinkButton *death_button = new QCommandLinkButton(tr("Death"), last_word);
        button_layout->addWidget(death_button);
        connect(death_button, SIGNAL(clicked()), general, SLOT(lastWord()));
        addCopyAction(death_button);
    }

    QString win_word = general->getWinword();
    if(!win_word.startsWith("`")){
        QCommandLinkButton *win_button = new QCommandLinkButton(tr("Victory"), win_word);
        button_layout->addWidget(win_button);
        connect(win_button, SIGNAL(clicked()), general, SLOT(winWord()));
        addCopyAction(win_button);
    }

    QString designer_text = Sanguosha->translate("designer:" + general->objectName());
    if(designer_text.startsWith("designer:"))
        designer_text = Sanguosha->translate("designer:" + general->getPackage());
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
    if(coder_text.startsWith("coder:"))
        coder_text = Sanguosha->translate("coder:" + general->getPackage());
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

void GeneralOverview::addChangeAction(QPushButton *button){
    button->setContextMenuPolicy(Qt::ActionsContextMenu);

    QAction *action = new QAction(button);
    action->setData("general");
    action->setText(tr("Change general"));
    button->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(askChange()));

    QAction *action2 = new QAction(button);
    action2->setData("general2");
    action2->setText(tr("Change general2"));
    button->addAction(action2);
    connect(action2, SIGNAL(triggered()), this, SLOT(askChange()));
}

void GeneralOverview::askChange(){
    if(!Config.value("Cheat/FreeChange", false).toBool())
        return;

    QString change2 = "";
    QAction *action = qobject_cast<QAction *>(sender());
    if(action && action->data().toString() == "general2")
        change2 = "%";

    int row = ui->tableWidget->currentRow();
    QString general_name = ui->tableWidget->item(row, 0)->data(Qt::UserRole).toString();
    if(Self && general_name != Self->getGeneralName()){
        ClientInstance->requestCheatChangeGeneral(change2 + general_name);
        if(change2 != "%")
            ui->changeGeneralButton->setEnabled(false);
    }
}

void GeneralOverview::on_tableWidget_itemDoubleClicked(QTableWidgetItem* item)
{
    if(Self)
        askChange();
}
