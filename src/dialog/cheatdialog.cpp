#include "cheatdialog.h"
#include "mainwindow.h"
#include "roomscene.h"
#include "client.h"

using namespace QSanProtocol;

ScriptExecutor::ScriptExecutor(QWidget *parent)
    :QDialog(parent)
{
    setWindowTitle(tr("Script execution"));

    QVBoxLayout *vlayout = new QVBoxLayout;

    vlayout->addWidget(new QLabel(tr("Please input the script that should be executed at server side:\n P = you, R = your room")));

    QTextEdit *box = new QTextEdit;
    box->setObjectName("scriptBox");
    vlayout->addWidget(box);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();

    QPushButton *ok_button = new QPushButton(tr("OK"));
    hlayout->addWidget(ok_button);

    vlayout->addLayout(hlayout);

    connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(this, SIGNAL(accepted()), this, SLOT(doScript()));

    setLayout(vlayout);
}

void ScriptExecutor::doScript(){
    QTextEdit *box = findChild<QTextEdit *>("scriptBox");
    if(box == NULL)
        return;

    QString script = box->toPlainText();
    QByteArray data = script.toAscii();
    data = qCompress(data);
    script = data.toBase64();

    ClientInstance->requestCheatRunScript(script);
}

CheatDialog::CheatDialog(QWidget *parent)
    :QDialog(parent)
{
    setWindowTitle(tr("Cheat dialog"));

    QFormLayout *layout = new QFormLayout;

    tab_widget = new QTabWidget;
    tab_widget->addTab(createDamageMakeTab(), tr("Damage maker"));
    tab_widget->addTab(createDeathNoteTab(), tr("Death note"));
    tab_widget->addTab(createSetStateTab(), tr("Set state"));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    QPushButton *ok_button = new QPushButton(tr("OK"));
    QPushButton *cancel_button = new QPushButton(tr("Cancel"));
    QPushButton *apply_button = new QPushButton(tr("Apply"));
    connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    connect(apply_button, SIGNAL(clicked()), this, SLOT(doApply()));
    hlayout->addWidget(ok_button);
    hlayout->addWidget(cancel_button);
    hlayout->addWidget(apply_button);

    layout->addWidget(tab_widget);
    layout->addRow(hlayout);
    setLayout(layout);
}

QWidget *CheatDialog::createDamageMakeTab(){
    QWidget *widget = new QWidget;

    damage_source = new QComboBox;
    RoomScene::FillPlayerNames(damage_source, true);

    damage_target = new QComboBox;
    RoomScene::FillPlayerNames(damage_target, false);

    damage_nature = new QComboBox;
    damage_nature->addItem(tr("Normal"), S_CHEAT_NORMAL_DAMAGE);
    damage_nature->addItem(tr("Thunder"), S_CHEAT_THUNDER_DAMAGE);
    damage_nature->addItem(tr("Fire"), S_CHEAT_FIRE_DAMAGE);
    damage_nature->addItem(tr("Recover HP"), S_CHEAT_HP_RECOVER);
    damage_nature->addItem(tr("Lose HP"), S_CHEAT_HP_LOSE);
    damage_nature->addItem(tr("Lose Max HP"), S_CHEAT_MAX_HP_LOSE);
    damage_nature->addItem(tr("Reset Max HP"), S_CHEAT_MAX_HP_RESET);

    damage_point = new QSpinBox;
    damage_point->setRange(1, 1000);
    damage_point->setValue(1);

    QFormLayout *layout = new QFormLayout;

    layout->addRow(tr("Damage source"), damage_source);
    layout->addRow(tr("Damage target"), damage_target);
    layout->addRow(tr("Damage nature"), damage_nature);
    layout->addRow(tr("Damage point"), damage_point);

    connect(damage_nature, SIGNAL(currentIndexChanged(int)), this, SLOT(disableSource()));

    widget->setLayout(layout);

    return widget;
}

QWidget *CheatDialog::createDeathNoteTab(){
    QWidget *widget = new QWidget;

    killer = new QComboBox;
    RoomScene::FillPlayerNames(killer, true);

    victim = new QComboBox;
    RoomScene::FillPlayerNames(victim, false);

    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Killer"), killer);
    layout->addRow(tr("Victim"), victim);

    killtype = new QComboBox;
    killtype->addItem(tr("Kill"));
    killtype->addItem(tr("Revive"));
    layout->addRow(tr("Type"), killtype);

    widget->setLayout(layout);
    return widget;
}

void CheatDialog::doApply(){
    switch(tab_widget->currentIndex()){
    case 0:
        ClientInstance->requestCheatDamage(damage_source->itemData(damage_source->currentIndex()).toString(),
                                damage_target->itemData(damage_target->currentIndex()).toString(),
                                (DamageStruct::Nature)damage_nature->itemData(damage_nature->currentIndex()).toInt(),
                                damage_point->value());
        break;
    case 1:{
        int type = killtype->currentIndex();
        if(type == 0)
            ClientInstance->requestCheatKill(killer->itemData(killer->currentIndex()).toString(),
                                victim->itemData(victim->currentIndex()).toString());
        else if(type == 1)
            ClientInstance->requestCheatRevive(victim->itemData(victim->currentIndex()).toString());
        else if(type == 2){
            QString data = makeData();
            ClientInstance->requestCheatState(target->itemData(target->currentIndex()).toString(), data);
        }
        break;
    }
    default:
        break;
    }
}

const QString CheatDialog::makeData(){
    QStringList strs;
    QString str = QString("general:%1").arg(general->text());
    strs << str;
    str = QString("kingdom:%1").arg(kingdom->text());
    strs << str;
    str = QString("role:%1").arg(role->text());
    strs << str;
    str = QString("sex:%1").arg(sex->text());
    strs << str;
    return strs.join(",");
}

void CheatDialog::accept(){
    QDialog::accept();
    doApply();
}

void CheatDialog::disableSource(){
    int nature = damage_nature->itemData(damage_nature->currentIndex()).toInt();
    damage_source->setEnabled(nature < 5);
}

static QLayout *HLay(QWidget *left, QWidget *right){
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(left);
    layout->addWidget(right);

    return layout;
}

QWidget *CheatDialog::createSetStateTab(){
    QWidget *widget = new QWidget;

    target = new QComboBox;
    RoomScene::FillPlayerNames(target, false);
    connect(target, SIGNAL(currentIndexChanged(int)), this, SLOT(loadState(int)));

    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Target"), target);

    QTabWidget *tab_state = new QTabWidget;
    QWidget *base = new QWidget;
    QFormLayout *base_layout = new QFormLayout;
    general = new QLineEdit("songjiang|lujunyi");
    kingdom = new QLineEdit("guan");
    role = new QLineEdit("lord");
    sex = new QLineEdit("male");
    base_layout->addRow(HLay(new QLabel(tr("General")), general));
    base_layout->addRow(HLay(new QLabel(tr("Kingdom")), kingdom));
    base_layout->addRow(HLay(new QLabel(tr("Role")), role));
    base_layout->addRow(HLay(new QLabel(tr("Sex")), sex));
    base->setLayout(base_layout);

    QWidget *adhere = new QWidget;
    QFormLayout *adhere_layout = new QFormLayout;
    QCheckBox *turn = new QCheckBox(tr("FaceUp"));
    QCheckBox *chain = new QCheckBox(tr("Chained"));
    QCheckBox *ecst = new QCheckBox(tr("Ecst"));
    QCheckBox *drank = new QCheckBox(tr("Drank"));
    QLineEdit *mark = new QLineEdit("@skull");
    adhere_layout->addRow(HLay(turn, chain));
    adhere_layout->addRow(HLay(ecst, drank));
    adhere_layout->addRow(HLay(new QLabel(tr("Set Mark")), mark));
    adhere->setLayout(adhere_layout);

    QWidget *conjur = new QWidget;
    QFormLayout *conjur_layout = new QFormLayout;
    QCheckBox *poison = new QCheckBox(tr("Poison"));
    QCheckBox *sleep = new QCheckBox(tr("Sleep"));
    conjur_layout->addRow(HLay(poison, sleep));
    conjur->setLayout(conjur_layout);

    tab_state->addTab(base, tr("Base"));
    tab_state->addTab(adhere, tr("Adhere"));
    tab_state->addTab(conjur, tr("Conjur"));

    layout->addRow(tab_state);

    widget->setLayout(layout);
    return widget;
}

void CheatDialog::loadState(int index){
    /*
    QString player_obj = target->itemData(index).toString();
    const Player *player = ClientInstance->getPlayer(player_obj);
    if(player){
        general->setText(QString("%1|%2").arg(player->getGeneralName()).arg(player->getGeneral2Name()));
        kingdom->setText(player->getKingdom());
        role->setText(player->getRole());
        sex->setText(player->getGenderString());
    }
    */
}
