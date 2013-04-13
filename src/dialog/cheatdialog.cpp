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

static QLayout *HLay(QWidget *left, QWidget *right, QWidget *other = NULL, QWidget *other2 = NULL){
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(left);
    layout->addWidget(right);
    if(other)
        layout->addWidget(other);
    if(other2)
        layout->addWidget(other2);

    return layout;
}

CheatDialog::CheatDialog(QWidget *parent, ClientPlayer *Self)
    :QDialog(parent)
{
    setWindowTitle(tr("Cheat dialog"));

    this->Self = Self;
    setMaximumWidth(300);
    QFormLayout *layout = new QFormLayout;

    tab_widget = new QTabWidget;
    tab_widget->addTab(createDamageMakeTab(), tr("Damage maker"));
    tab_widget->addTab(createDeathNoteTab(), tr("Death note"));
    tab_widget->addTab(createSetStateTab(), tr("Set state"));
    connect(tab_widget, SIGNAL(currentChanged(int)), this, SLOT(setGray(int)));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    ok_button = new QPushButton(tr("OK"));
    cancel_button = new QPushButton(tr("Cancel"));
    apply_button = new QPushButton(tr("Apply"));
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

QString CheatDialog::getPlayerString(){
    return target->itemData(target->currentIndex()).toString();
}

const Player *CheatDialog::getPlayer(){
    QString player_obj = getPlayerString();
    const Player *player = Self->findPlayer(player_obj);
    if(player)
        return player;
    else
        return Self;
}

QWidget *CheatDialog::createDamageMakeTab(){
    QWidget *widget = new QWidget;

    damage_source = new QComboBox;
    RoomScene::FillPlayerNames(damage_source, true);

    damage_target = new QComboBox;
    RoomScene::FillPlayerNames(damage_target, false);

    damage_point = new QSpinBox;
    damage_point->setRange(1, 1000);
    damage_point->setValue(1);

    damage_card = new QSpinBox;
    damage_card->setRange(-1, Sanguosha->getCardCount());
    damage_card->setValue(-1);

    QPushButton *select_card = new QPushButton(tr("Short Cut"));
    QMenu *select_menu = new QMenu(select_card);
    select_card->setMenu(select_menu);
    QStringList items;
    items << "slash_red" << "slash_black"
         << "fire_slash" << "thunder_slash"
         << "red_trick" << "black_trick";
    foreach(QString item, items){
        QAction *action = new QAction(select_menu);
        action->setText(Sanguosha->translate(item));
        action->setData(QString("card#%1").arg(item));
        select_menu->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(fillBase()));
    }

    damage_nature = new QButtonGroup();
    normal = new QRadioButton(tr("Normal"));
    damage_nature->addButton(normal);
    fire = new QRadioButton(tr("Fire"));
    damage_nature->addButton(fire);
    thunder = new QRadioButton(tr("Thunder"));
    damage_nature->addButton(thunder);

    rec = new QRadioButton(tr("Recover HP"));
    damage_nature->addButton(rec);
    lh = new QRadioButton(tr("Lose HP"));
    damage_nature->addButton(lh);
    lmh = new QRadioButton(tr("Lose Max HP"));
    damage_nature->addButton(lmh);
    rmh = new QRadioButton(tr("Reset Max HP"));
    damage_nature->addButton(rmh);
    normal->setChecked(true);

    QFormLayout *layout = new QFormLayout;

    layout->addRow(tr("Source"), damage_source);
    layout->addRow(tr("Target"), damage_target);
    layout->addRow(tr("Point"), damage_point);
    layout->addRow(tr("CardID"), HLay(damage_card, select_card));
    layout->addRow(tr("Nature"), HLay(normal, fire, thunder));
    layout->addRow(QString(), HLay(rec, lh));
    layout->addRow(QString(), HLay(lmh, rmh));

    connect(damage_nature, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(disableSource(QAbstractButton*)));

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

    killtype = new QButtonGroup();
    kill = new QRadioButton(tr("Kill"));
    kill->setObjectName("kill");
    kill->setChecked(true);
    killtype->addButton(kill);
    unkill = new QRadioButton(tr("Revive"));
    unkill->setObjectName("revive");
    killtype->addButton(unkill);
    layout->addRow(tr("Type"), HLay(kill, unkill));

    QButtonGroup *revivetype = new QButtonGroup();
    revivetype->setExclusive(false);
    revive1 = new QCheckBox(tr("Full State"));
    revive1->setChecked(true);
    revivetype->addButton(revive1);
    revive2 = new QCheckBox(tr("Start Init"));
    revivetype->addButton(revive2);
    revive1->hide();
    revive2->hide();
    connect(unkill, SIGNAL(toggled(bool)), revive1, SLOT(setVisible(bool)));
    connect(unkill, SIGNAL(toggled(bool)), revive2, SLOT(setVisible(bool)));
    layout->addRow(QString(), HLay(revive1, revive2));

    widget->setLayout(layout);
    return widget;
}

void CheatDialog::doApply(){
    switch(tab_widget->currentIndex()){
    case 0:{
        ClientInstance->requestCheatDamage(damage_source->itemData(damage_source->currentIndex()).toString(),
                                damage_target->itemData(damage_target->currentIndex()).toString(),
                                damage_nature->buttons().indexOf(damage_nature->checkedButton()) + 1,
                                damage_point->value(), damage_card->value());
        break;
    }
    case 1:{
        if(killtype->checkedButton()->objectName() != "kill")
            ClientInstance->requestCheatRevive(victim->itemData(victim->currentIndex()).toString(),
                                               revive1->isChecked(), revive2->isChecked());
        else
            ClientInstance->requestCheatKill(killer->itemData(killer->currentIndex()).toString(),
                            victim->itemData(victim->currentIndex()).toString());
        break;
    }
    case 2:
        ClientInstance->requestCheatState(getPlayerString(), makeData());
        break;
    default:
        break;
    }
}

const QString CheatDialog::makeData(){
    //general:songjiang|linchong,kindom:god,chained:true,poison_jur:4
    QStringList strs;
    QString str = QString("general:%1").arg(general->text());
    strs << str;
    str = QString("kingdom:%1").arg(kingdom->text());
    strs << str;
    str = QString("role:%1").arg(role->text());
    strs << str;
    str = QString("sex:%1").arg(sex->text());
    strs << str;

    str = QString("turned:%1").arg(turn->isChecked());
    strs << str;
    str = QString("chained:%1").arg(chain->isChecked());
    strs << str;
    str = QString("ecst:%1").arg(ecst->isChecked());
    strs << str;
    str = QString("drank:%1").arg(drank->isChecked());
    strs << str;
    str = QString("tie:%1").arg(shutup->isChecked());
    strs << str;

    if(conjur_group->checkedButton()){
        str = QString("%1:%2").arg(conjur_group->checkedButton()->objectName())
                .arg(conjur_text->text());
        strs << str;
    }

    //qDebug("str: %s", qPrintable(strs.join(",")));
    return strs.join(",");
}

void CheatDialog::accept(){
    QDialog::accept();
    doApply();
}

void CheatDialog::disableSource(QAbstractButton* but){
    int nature = damage_nature->buttons().indexOf(but) + 1;
    damage_source->setEnabled(nature < 5);
}

QWidget *CheatDialog::createSetStateTab(){
    QWidget *widget = new QWidget;

    target = new QComboBox;
    RoomScene::FillPlayerNames(target, false);
    connect(target, SIGNAL(currentIndexChanged(int)), this, SLOT(loadState(int)));

    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Target"), target);

    tab_state = new QTabWidget;
    QWidget *base = new QWidget;
    QFormLayout *base_layout = new QFormLayout;
    general = new QLineEdit();
    general->setPlaceholderText("songjiang|lujunyi");
    kingdom = new QLineEdit();
    kingdom->setPlaceholderText("guan");
    role = new QLineEdit();
    role->setPlaceholderText("lord");
    sex = new QLineEdit();
    sex->setPlaceholderText("male");

    QPushButton *kingdom_option = new QPushButton(tr("Option"));
    QMenu *kingdom_menu = new QMenu(kingdom_option);
    kingdom_option->setMenu(kingdom_menu);
    QStringList kingdoms = Sanguosha->getKingdoms();
    foreach(QString ki, kingdoms){
        QAction *action = new QAction(kingdom_menu);
        action->setIcon(QIcon(QString("image/kingdom/icon/%1.png").arg(ki)));
        action->setText(ki);
        action->setData("kingdom");
        kingdom_menu->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(fillBase()));
    }
    QPushButton *role_option = new QPushButton(tr("Option"));
    QMenu *role_menu = new QMenu(role_option);
    role_option->setMenu(role_menu);
    QStringList roles;
    roles << "lord" << "loyalist" << "rebel" << "renegade";
    foreach(QString ro, roles){
        QAction *action = new QAction(role_menu);
        action->setIcon(QIcon(QString("image/system/roles/%1.png").arg(ro)));
        action->setText(ro);
        action->setData("role");
        role_menu->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(fillBase()));
    }
    QPushButton *sex_option = new QPushButton(tr("Option"));
    QMenu *sex_menu = new QMenu(sex_option);
    sex_option->setMenu(sex_menu);
    QStringList sexs;
    sexs << "male" << "female" << "neuter";
    foreach(QString se, sexs){
        QAction *action = new QAction(sex_menu);
        action->setText(se);
        action->setData("sex");
        sex_menu->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(fillBase()));
    }

    QPushButton *draw_one = new QPushButton(tr("Draw One"));
    QPushButton *discard_one = new QPushButton(tr("Discard One"));
    connect(draw_one, SIGNAL(clicked()), this, SLOT(drawOne()));
    connect(discard_one, SIGNAL(clicked()), this, SLOT(discardOne()));
    QPushButton *load_base = new QPushButton(tr("Load Base"));
    QPushButton *clear_base = new QPushButton(tr("Clear Base"));
    connect(load_base, SIGNAL(clicked()), this, SLOT(loadBase()));
    connect(clear_base, SIGNAL(clicked()), this, SLOT(clearBase()));
    base_layout->addRow(tr("General"), general);
    base_layout->addRow(tr("Kingdom"), HLay(kingdom, kingdom_option));
    base_layout->addRow(tr("Role"), HLay(role, role_option));
    base_layout->addRow(tr("Sex"), HLay(sex, sex_option));
    base_layout->addRow(QString(), HLay(draw_one, discard_one));
    base_layout->addRow(QString(), HLay(load_base, clear_base));
    base->setLayout(base_layout);

    QWidget *adhere = new QWidget;
    QFormLayout *adhere_layout = new QFormLayout;
    turn = new QCheckBox(tr("Turned"));
    chain = new QCheckBox(tr("Chained"));
    ecst = new QCheckBox(tr("Ecst"));
    drank = new QCheckBox(tr("Drank"));
    shutup = new QCheckBox(tr("Tie"));
    extra_button = new QPushButton(tr("Remove extra skills"));
    QPushButton *clear_button = new QPushButton(tr("Clear History"));
    connect(clear_button, SIGNAL(clicked()), this, SLOT(clearHistory()));
    adhere_layout->addRow(HLay(turn, chain));
    adhere_layout->addRow(HLay(ecst, drank));
    adhere_layout->addRow(HLay(shutup, new QLabel));
    adhere_layout->addRow(HLay(extra_button, clear_button));
    adhere->setLayout(adhere_layout);

    QWidget *conjur = new QWidget;
    QFormLayout *conjur_layout = new QFormLayout;

    conjur_group = new QButtonGroup();
    poison = new QRadioButton(tr("Poison"));
    poison->setObjectName("poison_jur");
    conjur_group->addButton(poison);
    sleep = new QRadioButton(tr("Sleep"));
    sleep->setObjectName("sleep_jur");
    conjur_group->addButton(sleep);
    dizzy = new QRadioButton(tr("Dizzy"));
    dizzy->setObjectName("dizzy_jur");
    conjur_group->addButton(dizzy);
    petro = new QRadioButton(tr("Petro"));
    petro->setObjectName("petro_jur");
    conjur_group->addButton(petro);
    conjur_layout->addRow(HLay(poison, sleep));
    conjur_layout->addRow(HLay(dizzy, petro));

    conjur_text = new QLineEdit();
    conjur_text->setValidator(new QIntValidator(0, 99, conjur_text));
    conjur_text->setPlaceholderText("0");
    conjur_layout->addRow(tr("Surplus"), conjur_text);
    connect(conjur_group, SIGNAL(buttonClicked(QAbstractButton*)), conjur_text, SLOT(clear()));
    conjur->setLayout(conjur_layout);

    QWidget *expert = new QWidget;
    QVBoxLayout *expert_layout = new QVBoxLayout;
    flags = new QLineEdit();
    flags->setPlaceholderText("key");
    marks = new QLineEdit();
    marks->setPlaceholderText("key=value");
    propty = new QLineEdit();
    propty->setPlaceholderText("key=value");
    tag = new QLineEdit();
    tag->setPlaceholderText("key=value");
    QPushButton *clear = new QPushButton(tr("Clear"));
    QPushButton *apply = new QPushButton(tr("Apply"));
    connect(clear, SIGNAL(clicked()), this, SLOT(doClearExpert()));
    connect(apply, SIGNAL(clicked()), this, SLOT(doApplyExpert()));
    expert_layout->addWidget(new QLabel(tr("Expert Warning")));

    flag_option = new QPushButton(tr("Option"));
    mark_option = new QPushButton(tr("Option"));
    expert_layout->addLayout(HLay(new QLabel("Flag"), flags, flag_option));
    expert_layout->addLayout(HLay(new QLabel("Mark"), marks, mark_option));
    expert_layout->addLayout(HLay(new QLabel("Property"), propty));
    expert_layout->addLayout(HLay(new QLabel("Tag"), tag));
    expert_layout->addLayout(HLay(clear, apply));
    expert->setLayout(expert_layout);

    tab_state->addTab(base, tr("Base"));
    tab_state->addTab(adhere, tr("Adhere"));
    tab_state->addTab(conjur, tr("Conjur"));
    tab_state->addTab(expert, tr("Expert"));
    connect(tab_state, SIGNAL(currentChanged(int)), this, SLOT(setGray(int)));

    layout->addRow(tab_state);

    widget->setLayout(layout);
    loadState(0);
    return widget;
}

void CheatDialog::setGray(int){
    ok_button->setVisible(tab_state->currentIndex() != 3 || tab_widget->currentIndex() != 2);
    apply_button->setVisible(tab_state->currentIndex() != 3 || tab_widget->currentIndex() != 2);
}

void CheatDialog::drawOne(){
    //dod:1
    QString item = "dod:1";
    ClientInstance->requestCheatState(getPlayerString(), item);
}

void CheatDialog::discardOne(){
    //dod:0
    QString item = "dod:0";
    ClientInstance->requestCheatState(getPlayerString(), item);
}

const QStringList CheatDialog::getExtraSkills(){
    const Player *player = getPlayer();
    QStringList func;
    QSet<QString> skill_names = player->getAcquiredSkills();
    foreach(QString skill_name, skill_names){
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if(skill){
            if(skill->getFrequency() == Skill::NotSkill)
                continue;
            if(player->getGeneral()->hasSkill(skill_name) ||
               (player->getGeneral2() && player->getGeneral2()->hasSkill(skill_name)))
                continue;
            func << skill_name;
        }
    }
    return func;
}

void CheatDialog::loadState(int index){
    QString player_obj = target->itemData(index).toString();
    const Player *player = Self->findPlayer(player_obj);
    if(player){
        turn->setChecked(!player->faceUp());
        chain->setChecked(player->isChained());
        ecst->setChecked(player->hasFlag("ecst"));
        drank->setChecked(player->hasFlag("drank"));
        shutup->setChecked(player->hasFlag("ShutUp"));
        QStringList jurlist = player->getAllMarkName(3, "_jur");
        if(!jurlist.isEmpty()){
            QString jur = jurlist.first();
            foreach(QAbstractButton *b, conjur_group->buttons()){
                if(b->objectName() == jur){
                    b->setChecked(true);
                    conjur_text->setText(QString::number(player->getMark(jur)));
                    break;
                }
            }
        }

        QMenu *skill_menu = new QMenu(extra_button);
        extra_button->setMenu(skill_menu);
        QStringList extra_skills = getExtraSkills();
        foreach(QString skill, extra_skills){
            QAction *action = new QAction(skill_menu);
            action->setText(Sanguosha->translate(skill));
            action->setData(skill);
            skill_menu->addAction(action);
            connect(action, SIGNAL(triggered()), this, SLOT(loseSkill()));
        }
        QMenu *flag_menu = new QMenu(flag_option);
        flag_option->setMenu(flag_menu);
        QStringList f1ags = player->getFlags().split("+");
        foreach(QString fla, f1ags){
            QAction *action = new QAction(flag_menu);
            action->setText(fla);
            action->setData("flag");
            flag_menu->addAction(action);
            connect(action, SIGNAL(triggered()), this, SLOT(fillBase()));
        }
        QMenu *mark_menu = new QMenu(mark_option);
        mark_option->setMenu(mark_menu);
        QStringList ma2ks = player->getAllMarkName(2);
        foreach(QString mrk, ma2ks){
            QAction *action = new QAction(mark_menu);
            action->setIcon(QIcon(QString("image/mark/%1.png").arg(mrk)));
            action->setText(mrk);
            action->setData(QString("mark#%1").arg(player->getMark(mrk)));
            mark_menu->addAction(action);
            connect(action, SIGNAL(triggered()), this, SLOT(fillBase()));
        }
    }
}

void CheatDialog::loadBase(){
    const Player *player = getPlayer();
    if(player->getGeneral2())
        general->setText(QString("%1|%2").arg(player->getGeneralName()).arg(player->getGeneral2Name()));
    else
        general->setText(player->getGeneralName());
    kingdom->setText(player->getKingdom());
    role->setText(player->getRole());
    sex->setText(player->getGenderString());
}

void CheatDialog::clearBase(){
    general->clear();
    kingdom->clear();
    role->clear();
    sex->clear();
}

void CheatDialog::fillBase(){
    QAction *action = qobject_cast<QAction *>(sender());
    if(action){
        QString item = action->data().toString();
        if(item == "kingdom")
            kingdom->setText(action->text());
        else if(item == "role")
            role->setText(action->text());
        else if(item == "sex")
            sex->setText(action->text());
        //not base
        else if(item == "flag")
            flags->setText(action->text());
        else if(item.startsWith("mark")){
            QString num = item.split("#").last();
            marks->setText(QString("%1=%2").arg(action->text()).arg(num));
        }
        else if(item.startsWith("card")){
            int id = -1;
            QString cardkind = item.split("#").last();
            foreach(Card *card, Sanguosha->getCards()){
                if(cardkind.contains("slash") && card->isKindOf("Slash")){
                    if(card->objectName() == cardkind)
                        id = card->getId();
                    else if((cardkind.contains("red") && card->isRed()) ||
                            (cardkind.contains("black") && card->isBlack()))
                        id = card->getId();
                }
                else if(cardkind.contains("trick") && card->isNDTrick()){
                    if((cardkind.contains("red") && card->isRed()) ||
                            (cardkind.contains("black") && card->isBlack()))
                        id = card->getId();
                }
                if(id > 0)
                    break;
            }
            damage_card->setValue(id);
        }
    }
}

void CheatDialog::loseSkill(){
    QAction *action = qobject_cast<QAction *>(sender());
    if(action){
        QString item = action->data().toString();
        //skill:-sheri
        item.prepend("skill:-");
        ClientInstance->requestCheatState(getPlayerString(), item);
        //loadState(target->currentIndex());
    }
}

void CheatDialog::clearHistory(){
    //history:1
    QString item = "history:1";
    ClientInstance->requestCheatState(getPlayerString(), item);
}

void CheatDialog::doClearExpert(){
    marks->clear();
    flags->clear();
    propty->clear();
    tag->clear();
}

void CheatDialog::doApplyExpert(){
    //mark:key=value,flag:flag,propty:hp=2,tag:Tag=0
    QStringList strs;
    QString str = QString("mark:%1").arg(marks->text());
    strs << str;
    str = QString("flag:%1").arg(flags->text());
    strs << str;
    str = QString("propty:%1").arg(propty->text());
    strs << str;
    str = QString("tag:%1").arg(tag->text());
    strs << str;

    QString data = strs.join(",");
    ClientInstance->requestCheatState(getPlayerString(), data);
}
