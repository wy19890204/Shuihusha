#include "packagingeditor.h"
#include "mainwindow.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QTabWidget>
#include <QProcess>
#include <QMessageBox>
#include <QFormLayout>
#include <QSettings>

#include "settings.h"

typedef const QSettings *SettingsStar;

Q_DECLARE_METATYPE(SettingsStar);

MetaInfoWidget::MetaInfoWidget(bool load_config){
    setTitle(tr("Meta information"));

    QFormLayout *layout = new QFormLayout;

    name_edit = new QLineEdit;
    designer_edit = new QLineEdit;
    programmer_edit = new QLineEdit;
    version_edit = new QLineEdit;
    description_edit = new QTextEdit;

    name_edit->setObjectName("Name");
    designer_edit->setObjectName("Designer");
    programmer_edit->setObjectName("Programmer");
    version_edit->setObjectName("Version");
    description_edit->setObjectName("Description");
/*
    if(load_config){
        Config.beginGroup("PackageManager");
        name_edit->setText(Config.value("Name", "My DIY").toString());
        designer_edit->setText(Config.value("Designer", tr("Designer")).toString());
        programmer_edit->setText(Config.value("Programmer", "Moligaloo").toString());
        version_edit->setText(Config.value("Version", "1.0").toString());
        description_edit->setText(Config.value("Description").toString());
        Config.endGroup();
    }
*/
    layout->addRow(tr("Name"), name_edit);
    layout->addRow(tr("Designer"), designer_edit);
    layout->addRow(tr("Programmer"), programmer_edit);
    layout->addRow(tr("Version"), version_edit);
    layout->addRow(tr("Description"), description_edit);

    setLayout(layout);
}

void MetaInfoWidget::saveToSettings(QSettings &settings){
    QList<const QLineEdit *> edits = findChildren<const QLineEdit *>();

    foreach(const QLineEdit *edit, edits){
        settings.setValue(edit->objectName(), edit->text());
    }

    settings.setValue("Description", description_edit->toPlainText());
}

void MetaInfoWidget::setName(const QString &name){
    name_edit->setText(name);
}

void MetaInfoWidget::setDesigner(const QString &designer){
    designer_edit->setText(designer);
}

void MetaInfoWidget::setCoder(const QString &coder){
    programmer_edit->setText(coder);
}

void MetaInfoWidget::showSettings(const QSettings *settings){
    QList<QLineEdit *> edits = findChildren<QLineEdit *>();

    foreach(QLineEdit *edit, edits){
        edit->setText(settings->value(edit->objectName()).toString());
    }

    description_edit->setText(settings->value("Description").toString());
}

PackagingEditor::PackagingEditor(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("DIY package manager"));
    if(!Config.value("EnableLua", false).toBool())
        QMessageBox::critical(this, tr("Warning"), tr("Lua extra is disabled!"));

    QString url = "http://www.7-zip.org";
    QLabel *label = new QLabel(tr("Package format is 7z, see its offcial site :<a href='%1' style = \"color:#0072c1; \">%1</a>").arg(url));

    tab_widget = new QTabWidget;
    tab_widget->addTab(createManagerTab(), tr("Package management"));
    tab_widget->addTab(createPackagingTab(), tr("Resource management"));
    tab_widget->addTab(createSniffTab(), tr("Sniff lua packages"));

    QVBoxLayout *layout = new QVBoxLayout;

    layout->addWidget(label);
    layout->addWidget(tab_widget);

    setLayout(layout);
}

void PackagingEditor::loadPackageList(){
    QDir dir("extensions");
    QIcon icon("image/system/ark.png");
    foreach(QFileInfo info, dir.entryInfoList(QStringList() << "*.ini")){
        const QSettings *settings = new QSettings(info.filePath(), QSettings::IniFormat, package_list);

        if(qgetenv("USERNAME") != "Tenkei" && settings->value("Hide", false).toBool())
            continue;
        QString name = settings->value("Name").toString();
        if(name.isEmpty())
            name = info.baseName();

        QListWidgetItem *item = !settings->value("Hide", false).toBool() ?
                                new QListWidgetItem(icon, name, package_list) :
                                new QListWidgetItem(name, package_list);
        QVariant data = QVariant::fromValue(settings);
        item->setData(Qt::UserRole, data);
    }
}

QWidget *PackagingEditor::createManagerTab(){
    QWidget *widget = new QWidget;

    package_list = new QListWidget;
    package_list->setViewMode(QListView::IconMode);
    package_list->setIconSize(QSize(64, 64));
    package_list->setMovement(QListView::Static);
    package_list->setWordWrap(true);
    package_list->setResizeMode(QListView::Adjust);

    loadPackageList();

    QVBoxLayout *vlayout = new QVBoxLayout;

    QCommandLinkButton *install_button = new QCommandLinkButton(tr("Install"));
    install_button->setDescription(tr("Install a DIY package"));

    QCommandLinkButton *modify_button = new QCommandLinkButton(tr("Modify"));
    modify_button->setDescription(tr("Modify a DIY package"));

    QCommandLinkButton *uninstall_button = new QCommandLinkButton(tr("Uninstall"));
    uninstall_button->setDescription(tr("Uninstall a DIY package"));

    QCommandLinkButton *hdsw_button = new QCommandLinkButton(tr("Hide/Show"));
    hdsw_button->setDescription(tr("Hide or show a DIY package"));

    QCommandLinkButton *rescan_button = new QCommandLinkButton(tr("Rescan"));
    rescan_button->setDescription(tr("Rescan existing packages"));

    vlayout->addWidget(install_button);
    vlayout->addWidget(modify_button);
    vlayout->addWidget(uninstall_button);
    vlayout->addWidget(hdsw_button);
    vlayout->addWidget(rescan_button);
    vlayout->addStretch();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addLayout(vlayout);
    layout->addWidget(package_list);
    layout->addWidget(package_list_meta = new MetaInfoWidget(false));
    widget->setLayout(layout);

    connect(install_button, SIGNAL(clicked()), this, SLOT(installPackage()));
    connect(modify_button, SIGNAL(clicked()), this, SLOT(modifyPackage()));
    connect(uninstall_button, SIGNAL(clicked()), this, SLOT(uninstallPackage()));
    connect(hdsw_button, SIGNAL(clicked()), this, SLOT(hideorshowPackage()));
    connect(rescan_button, SIGNAL(clicked()), this, SLOT(rescanPackage()));
    connect(package_list, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(updateMetaInfo(QListWidgetItem*)));
    connect(package_list, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(updateMetaInfo(QListWidgetItem*)));
    connect(package_list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(modifyPackage()));

    return widget;
}

void PackagingEditor::updateMetaInfo(QListWidgetItem *item){
    if(item == NULL)
        return;
    SettingsStar settings = item->data(Qt::UserRole).value<SettingsStar>();
    if(settings){
        package_list_meta->showSettings(settings);
    }
}

void PackagingEditor::hideorshowPackage(){
    if(qgetenv("USERNAME") != "Tenkei"){
        QMessageBox::critical(this, tr("Warning"), tr("Insufficient permissions"));
        return;
    }

    QListWidgetItem *item = package_list->currentItem();
    if(item == NULL)
        return;

    SettingsStar settings = item->data(Qt::UserRole).value<SettingsStar>();
    if(settings == NULL)
        return;

    QSettings setting(settings->fileName(), QSettings::IniFormat);
    bool is_hide = settings->value("Hide", false).toBool();
    setting.setValue("Hide", !is_hide);
    rescanPackage();
    QMessageBox::information(this, tr("Hide or Show"), tr("%1 is set to %2").arg(settings->value("Name").toString()).arg(is_hide ? tr("Show") : tr("Hide")));
}

void PackagingEditor::rescanPackage(){
    QList<QSettings *> settings_list = findChildren<QSettings *>();
    foreach(QSettings *settings, settings_list)
        settings->deleteLater();

    package_list->clear();

    loadPackageList();
}

QWidget *PackagingEditor::createPackagingTab(){
    QWidget *widget = new QWidget;

    file_list = new QListWidget;

    QVBoxLayout *vlayout = new QVBoxLayout;

    QCommandLinkButton *browse_button = new QCommandLinkButton(tr("Add files"));
    browse_button->setDescription(tr("Select files to package"));

    QCommandLinkButton *remove_button = new QCommandLinkButton(tr("Remove files"));
    remove_button->setDescription(tr("Remove files to package"));

    QCommandLinkButton *edit_button = new QCommandLinkButton(tr("Edit file"));
    edit_button->setDescription(tr("Edit file to package"));

    QCommandLinkButton *package_button = new QCommandLinkButton(tr("Make package"));
    package_button->setDescription(tr("Export files to a single package"));

    QCommandLinkButton *migration_button = new QCommandLinkButton(tr("Resource migration"));
    migration_button->setDescription(tr("Resource migration with single package"));

    vlayout->addWidget(browse_button);
    vlayout->addWidget(remove_button);
    vlayout->addWidget(edit_button);
    vlayout->addWidget(package_button);
    vlayout->addWidget(migration_button);
    vlayout->addStretch();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addLayout(vlayout);
    layout->addWidget(file_list);
    layout->addWidget(file_list_meta = new MetaInfoWidget(true));

    widget->setLayout(layout);

    connect(browse_button, SIGNAL(clicked()), this, SLOT(browseFiles()));
    connect(remove_button, SIGNAL(clicked()), this, SLOT(removeFile()));
    connect(edit_button, SIGNAL(clicked()), this, SLOT(editFile()));
    connect(package_button, SIGNAL(clicked()), this, SLOT(makePackage()));
    connect(migration_button, SIGNAL(clicked()), this, SLOT(migrationPackage()));
    connect(file_list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(removeFile(QListWidgetItem*)));

    return widget;
}

void PackagingEditor::installPackage(){
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Select a package to install"),
                                                    QString(),
                                                    tr("7z format (*.7z)")
                                                    );

    if(!filename.isEmpty()){
        QProcess *process = new QProcess(this);
        QStringList args;
        args << "x" << filename;
        process->start("7zr", args);

        connect(process, SIGNAL(finished(int)), this, SLOT(done7zProcess(int)));
        QMessageBox::information(this, tr("Notice"), tr("DIY package is loaded, please reset the game."));
    }
}

void PackagingEditor::modifyPackage(){
    QListWidgetItem *item = package_list->currentItem();
    if(!item)
        return;
    SettingsStar settings = item->data(Qt::UserRole).value<SettingsStar>();
    if(settings == NULL)
        return;

    tab_widget->setCurrentIndex(1);

    file_list->clear();
    QStringList filelist = settings->value("FileList").toStringList();
    foreach(QString file, filelist){
        if(!QFile::exists(file))
            QMessageBox::warning(this, tr("Warning"), tr("File %1 not found.").arg(file));
        else
            new QListWidgetItem(file, file_list);
    }

    file_list_meta->showSettings(settings);
}

void PackagingEditor::uninstallPackage(){
    QListWidgetItem *item = package_list->currentItem();
    if(item == NULL)
        return;

    SettingsStar settings = item->data(Qt::UserRole).value<SettingsStar>();
    if(settings == NULL)
        return;

    QMessageBox::StandardButton button = QMessageBox::question(this,
                                                               tr("Uninstall package"),
                                                               tr("Are you sure to remove %1 ?").arg(item->text()),
                                                               QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No), QMessageBox::Yes);
    if(button != QMessageBox::Yes)
        return;

    QStringList filelist = settings->value("FileList").toStringList();
    foreach(QString file, filelist)
        QFile::remove(file);

    QFile::remove(settings->fileName());
    delete item;
    QMessageBox::information(this, tr("Notice"), tr("DIY package is deleted."));
}

void PackagingEditor::browseFiles(){
    QStringList files = QFileDialog::getOpenFileNames(this,
                                                      tr("Select one or more files to package"),
                                                      "extensions",
                                                      tr("Any files (*.*)"));

    QDir dir;
    foreach(QString file, files)
        new QListWidgetItem(dir.relativeFilePath(file), file_list);
}

void PackagingEditor::removeFile(QListWidgetItem* item, bool mute){
    if(mute == true){
        delete(item);
        return;
    }
    QString file_name = item->text();

    QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Remove file"), tr("Are you sure to physical delete this file?"),
                                                              QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel), QMessageBox::Cancel);
    if(button == QMessageBox::Cancel)
        return;
    if(button == QMessageBox::Yes)
        QFile::remove(file_name);
    delete(item);
}

void PackagingEditor::removeFile(){
    QListWidgetItem *item = file_list->currentItem();
    if(item)
        removeFile(item, false);
    else{
        QMessageBox::StandardButton button = QMessageBox::question(this, tr("Clear file list"), tr("Are you sure to clear the file list?"),
                                                                   QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No), QMessageBox::Yes);
        if(button == QMessageBox::Yes)
            file_list->clear();
    }
}

void PackagingEditor::editFile(){
    QMessageBox::critical(this, tr("Warning"), tr("Insufficient permissions"));
}

void PackagingEditor::makePackage(){
    if(file_list->count() == 0)
        return;

    QList<const QLineEdit *> edits = file_list_meta->findChildren<const QLineEdit *>();
    foreach(const QLineEdit *edit, edits){
        if(edit->text().isEmpty()){
            QMessageBox::warning(this, tr("Warning"), tr("Please fill the meta information before making package"));
            return;
        }
    }
    /*
    Config.beginGroup("PackageManager");
    file_list_meta->saveToSettings(Config);
    Config.endGroup();
    */
    QString filepath = file_list->item(0)->text();
    QStringList split = filepath.split("/");
    if(split.count() == 2 && filepath.endsWith(".lua"))
        filepath = split.last().replace(QString(".lua"), QString(""));

    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Select a package name"),
                                                    filepath,
                                                    tr("7z format (*.7z)"));

    if(!filename.isEmpty()){
        QFileInfo info(filename);
        QString spec_name = QString("extensions/%1.ini").arg(info.baseName());
        QSettings settings(spec_name, QSettings::IniFormat);
        file_list_meta->saveToSettings(settings);
        QStringList filelist;
        for(int i=0; i<file_list->count(); i++)
            filelist << file_list->item(i)->text();
        settings.setValue("FileList", filelist);

        QProcess *process = new QProcess(this);
        QStringList args;
        args << "a" << filename << spec_name << filelist;
        process->start("7zr", args);

        connect(process, SIGNAL(finished(int)), this, SLOT(done7zProcess(int)));
        QMessageBox::information(this, tr("Notice"), tr("DIY package is finished."));

        tab_widget->setCurrentIndex(0);
    }
}

void PackagingEditor::migrationPackage(){
    QString indexame;
    for(int i=0; i< file_list->count(); i++){
        QString filepath = file_list->item(i)->text();
        if(!filepath.startsWith("extensions")){
            QStringList th = filepath.split("/");
            if(th.at(1) == "generals")
                th[0] = "extensions";
            else if(th.at(1) == "mark")
                th[0] = "extensions/generals";
            else if(th.at(1) == "ai")
                th[0] = "extensions";
            else if(th.at(0) == "audio")
                th[0] = "extensions/audio";
            else{
                QMessageBox::warning(this, tr("Warning"), tr("File %1 is unknown.").arg(filepath));
                continue;
            }

            QString newname = th.join("/");
            th.removeLast();

            QString newpath = th.join("/");
            QDir *newdir = new QDir;
            if(!newdir->exists(newpath))
                newdir->mkpath(newpath);

            if(!QFile::exists(filepath)){
                QMessageBox::warning(this, tr("Warning"), tr("File %1 not found.").arg(filepath));
                //delete(file_list->item(i));
                continue;
            }
            if(QFile::exists(newname)){
                QMessageBox::StandardButton button = QMessageBox::warning(this, tr("Warning"), tr("File %1 is exists.").arg(newname),
                                                                          QMessageBox::StandardButtons(QMessageBox::Ok | QMessageBox::Ignore), QMessageBox::Ignore);
                if(button == QMessageBox::Ignore){
                    file_list->item(i)->setText(newname);
                    continue;
                }
                else
                    QFile::remove(newname);
            }

            QFile::copy(filepath, newname);
            QFile::remove(filepath);
            file_list->item(i)->setText(newname);

            if(!QFile::exists(newname))
                QMessageBox::warning(this, tr("Warning"), tr("File %1 not found.").arg(newname));
        }
        else{
            QStringList split = filepath.split("/");
            if(split.count() == 2 && filepath.endsWith(".lua") && indexame.isNull())
                indexame = split.last().replace(QString(".lua"), QString(""));
        }
    }

    QString spec_name = QString("extensions/%1.ini").arg(indexame);
    QSettings settings(spec_name, QSettings::IniFormat);
    file_list_meta->saveToSettings(settings);
    QStringList filelist;
    for(int i=0; i<file_list->count(); i++)
        filelist << file_list->item(i)->text();
    settings.setValue("FileList", filelist);

    QMessageBox::information(this, tr("Notice"), tr("Migration done."));
    rescanPackage();
}

void PackagingEditor::done7zProcess(int exit_code){
    if(exit_code != 0)
        QMessageBox::warning(this, tr("Warning"), tr("Package compress/decompress error!"));
    else
        rescanPackage();
}

void MainWindow::on_actionPackaging_triggered()
{
    PackagingEditor *editor = new PackagingEditor(this);
    editor->show();
}

QWidget *PackagingEditor::createSniffTab(){
    QWidget *widget = new QWidget;

    general_list = new QListWidget;
    lua_list = new QListWidget;

    QVBoxLayout *vlayout = new QVBoxLayout;

    QCommandLinkButton *sniff_button = new QCommandLinkButton(tr("Sniff it"));
    sniff_button->setDescription(tr("Sniff lua packages resource"));

    filtrate_button = new QCommandLinkButton();

    QCommandLinkButton *duplicated_button = new QCommandLinkButton(tr("Check Duplicated"));
    duplicated_button->setDescription(tr("Check duplicated words in lua packages"));

    showAll();
    vlayout->addWidget(sniff_button);
    vlayout->addWidget(filtrate_button);
    vlayout->addWidget(duplicated_button);
    vlayout->addStretch();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addLayout(vlayout);
    layout->addWidget(lua_list);
    layout->addWidget(general_list);

    widget->setLayout(layout);

    connect(sniff_button, SIGNAL(clicked()), this, SLOT(sniffLua()));
    connect(duplicated_button, SIGNAL(clicked()), this, SLOT(duplicateLua()));
    connect(lua_list, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(updateLuaGeneral(QListWidgetItem*)));
    connect(lua_list, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(updateLuaGeneral(QListWidgetItem*)));

    return widget;
}

void PackagingEditor::updateLuaGeneral(QListWidgetItem *item){
    general_list->clear();
    if(item == NULL)
        return;
    const Package *package = Sanguosha->findChild<const Package *>(item->text());
    QList<General *> all_generals = package->findChildren<General *>();
    foreach(General *general, all_generals){
        QString text = QString("%1[%2] %3 %4 %5").arg(Sanguosha->translate(general->objectName()))
                .arg(general->objectName())
                .arg(Sanguosha->translate(general->getGenderString()))
                .arg(QString::number(general->getMaxHp()) + "HP")
                .arg(Sanguosha->translate(general->getKingdom(true)));
        QListWidgetItem *item2 = new QListWidgetItem(text, general_list);
        item2->setToolTip(general->getSkillDescription());
    }
}

void PackagingEditor::showAll(){
    lua_list->clear();
    foreach(QString lua, Sanguosha->getLuaExtensions()){
        QListWidgetItem *item = new QListWidgetItem(lua, lua_list);
        item->setToolTip(Sanguosha->translate(lua));
    }

    filtrate_button->setText(tr("Filtrate it"));
    filtrate_button->setDescription(tr("Filtrate lua packages"));
    connect(filtrate_button, SIGNAL(clicked()), this, SLOT(filtRate()));
}

void PackagingEditor::filtRate(){
    QStringList luasinini;
    QDir dir("extensions");
    foreach(QFileInfo info, dir.entryInfoList(QStringList() << "*.ini")){
        const QSettings *settings = new QSettings(info.filePath(), QSettings::IniFormat, package_list);
        if(settings == NULL)
            continue;
        QStringList filelist = settings->value("FileList").toStringList();
        foreach(QString file, filelist){
            QStringList split = file.split("/");
            if(QFile::exists(file) && split.count() == 2 &&
               split.first() == "extensions" && file.endsWith(".lua"))
                luasinini << file;
        }
    }
    lua_list->clear();
    foreach(QFileInfo info, dir.entryInfoList(QStringList() << "*.lua")){
        QString luaname = info.fileName().replace(QString(".lua"), QString(""));
        if(!luasinini.contains(info.filePath()) && Sanguosha->getLuaExtensions().contains(luaname))
            new QListWidgetItem(luaname, lua_list);
    }

    filtrate_button->setText(tr("Show all"));
    filtrate_button->setDescription(tr("Show all lua packages"));
    connect(filtrate_button, SIGNAL(clicked()), this, SLOT(showAll()));
}

void PackagingEditor::sniffLua(){
    file_list->clear();
    QListWidgetItem *item = lua_list->currentItem();
    if(item == NULL)
        return;
    QString package_name = item->text();
    const Package *package = Sanguosha->findChild<const Package *>(package_name);
    if(!package)
        return;

    QString path = QString("extensions/%1.lua").arg(package_name);
    new QListWidgetItem(path, file_list);
    path = QString("lua/ai/%1-ai.lua").arg(package_name);
    if(QFile::exists(path))
        new QListWidgetItem(path, file_list);

    QStringList skills;
    foreach(const Skill *tmp, package->getSkills())
        skills << tmp->objectName();

    QList<General *> all_generals = package->findChildren<General *>();
    foreach(General *general, all_generals){
        QString general_name = general->objectName();
        path = QString("image/generals/card/%1.jpg").arg(general_name);
        if(QFile::exists(path))
            new QListWidgetItem(path, file_list);
        path = QString("image/generals/big/%1.png").arg(general_name);
        if(QFile::exists(path))
            new QListWidgetItem(path, file_list);
        path = QString("image/generals/small/%1.png").arg(general_name);
        if(QFile::exists(path))
            new QListWidgetItem(path, file_list);
        path = QString("image/generals/tiny/%1.png").arg(general_name);
        if(QFile::exists(path))
            new QListWidgetItem(path, file_list);
        path = QString("audio/death/%1.ogg").arg(general_name);
        if(QFile::exists(path))
            new QListWidgetItem(path, file_list);

        foreach(const Skill *tmp, general->findChildren<const Skill *>())
            skills << tmp->objectName();
    }

    foreach(QString skill, skills){
        QString effect_file = QString("audio/skill/%1.ogg").arg(skill);
        if(QFile::exists(effect_file))
            new QListWidgetItem(effect_file, file_list);
        for(int i=1; ;i++){
            effect_file = QString("audio/skill/%1%2.ogg").arg(skill).arg(i);
            if(QFile::exists(effect_file))
                new QListWidgetItem(effect_file, file_list);
            else
                break;
        }
    }

    sniffMarks(QString("extensions/%1.lua").arg(package_name));

    QMessageBox::information(this, tr("Notice"), tr("Sniff done, find %1 files").arg(file_list->count()));
    tab_widget->setCurrentIndex(1);

    file_list_meta->setName(Sanguosha->translate(package_name));
    QString designer = Sanguosha->translate("designer:" + package_name);
    if(designer.startsWith("designer:"))
        designer = Sanguosha->translate("designer:" + all_generals.first()->objectName());
    if(designer.startsWith("designer:"))
        designer = Sanguosha->translate("DefaultDesigner");
    file_list_meta->setDesigner(designer);
    QString coder = Sanguosha->translate("coder:" + package_name);
    if(coder.startsWith("coder:"))
        coder = Sanguosha->translate("coder:" + all_generals.first()->objectName());
    if(coder.startsWith("coder:"))
        coder = Sanguosha->translate("DefaultCoder");
    file_list_meta->setCoder(coder);

    for(int i=0; i<file_list->count(); i++){
        QString filename = file_list->item(i)->text();
        if(!QFile::exists(filename))
            QMessageBox::warning(this, tr("Warning"), tr("File %1 not found.").arg(filename));
    }
}

void PackagingEditor::sniffMarks(const QString &luapath){
    QDir dir("image/mark");
    foreach(QFileInfo info, dir.entryInfoList(QStringList() << "@*.png")){
        QString mark = info.fileName().replace(QString(".png"), QString(""));

        QFile file(luapath);
        QString check;
        if(!file.open(QFile::ReadOnly | QFile::Text))
            return;
        check = file.readAll();
        if(check.contains(mark))
            new QListWidgetItem(info.filePath(), file_list);
        file.close();
    }
}

#include <QInputDialog>
void PackagingEditor::duplicateLua(){
    QListWidgetItem *item = lua_list->currentItem();
    if(item == NULL)
        return;

    int word = 0, file = 0;
    QString package_name = item->text();
    const Package *package = Sanguosha->findChild<const Package *>(package_name);
    if(!package)
        return;
    QStringList duplis = Config.value("Duplicated", QVariant()).toStringList();

    QStringList generals, skills;
    foreach(const Skill *tmp, package->getSkills())
        skills << tmp->objectName();
    foreach(General *general, package->findChildren<General *>()){
        generals << general->objectName();
        foreach(const Skill *tmp, general->findChildren<const Skill *>())
            skills << tmp->objectName();
    }

    foreach(QString tmp, generals){
        if(!duplis.contains(tmp))
            continue;
        QString name;
        do{
            name = QInputDialog::getText(this, tr("Duplicated"),
                                         tr("%1 is duplicated, Please input new name :").arg(tmp),
                                         QLineEdit::Normal, "lua" + tmp);
            if(name.isEmpty() || !Sanguosha->isDuplicated(name, false))
                break;
        }while(1==1);
        if(!name.isEmpty()){
            word += doReplace(tmp, name);
            file += doRename(tmp, name, false);
        }
    }
    foreach(QString tmp, skills){
        if(!duplis.contains(tmp))
            continue;
        QString name;
        do{
            name = QInputDialog::getText(this, tr("Duplicated"),
                                         tr("%1 is duplicated, Please input new name :").arg(tmp),
                                         QLineEdit::Normal, "lua" + tmp);
            if(name.isEmpty() || !Sanguosha->isDuplicated(name, true))
                break;
        }while(1==1);
        if(!name.isEmpty()){
            word += doReplace(tmp, name);
            file += doRename(tmp, name, false);
        }
    }

    QMessageBox::information(this, tr("Notice"), tr("Execution completed, %1 words, %2 files").arg(word).arg(file));
}

int PackagingEditor::doReplace(const QString &old_word, const QString &new_word){
    QString package_name = lua_list->currentItem()->text();
    int count_words = 0;

    QString luapath = QString("extensions/%1.lua").arg(package_name);
    QFile file(luapath);
    QString str;
    if(file.open(QIODevice::ReadOnly)){
        str = QString::fromUtf8(file.readAll());
        count_words += str.count(old_word, Qt::CaseSensitive);
        str.replace(old_word, new_word, Qt::CaseSensitive);
        file.close();
    }
    QFile::remove(luapath);
    if(file.open(QIODevice::WriteOnly)){
        file.write(str.toUtf8());
        file.close();
    }

    QString aipath = QString("lua/ai/%1-ai.lua").arg(package_name);
    if(!QFile::exists(aipath))
        aipath = QString("extensions/ai/%1-ai.lua").arg(package_name);
    if(QFile::exists(aipath)){
        QFile file2(aipath);
        if(file2.open(QIODevice::ReadOnly)){
            str = QString::fromUtf8(file2.readAll());
            count_words += str.count(old_word, Qt::CaseSensitive);
            str.replace(old_word, new_word, Qt::CaseSensitive);
            file2.close();
        }
        QFile::remove(aipath);
        if(file2.open(QIODevice::WriteOnly)){
            file2.write(str.toUtf8());
            file2.close();
        }
    }

    return count_words;
}

int PackagingEditor::doRename(const QString &old_name, const QString &new_name, bool is_skill){
    int count_files = 0;
    QString tmp;
    if(!is_skill){
        tmp = "image/generals/card/";
        if(!QFile::exists(tmp + QString("%1.jpg").arg(old_name)))
            tmp.replace("image/", "extensions/");
        if(QFile::copy(tmp + QString("%1.jpg").arg(old_name), tmp + QString("%1.jpg").arg(new_name)))
            count_files ++;

        QStringList bst;
        bst << "big" << "small" << "tiny";
        foreach(QString bstn, bst){
            tmp = QString("image/generals/%1/").arg(bstn);
            if(!QFile::exists(tmp + QString("%1.png").arg(old_name)))
                tmp.replace("image/", "extensions/");
            if(QFile::copy(tmp + QString("%1.png").arg(old_name), tmp + QString("%1.png").arg(new_name)))
                count_files ++;
        }

        tmp = "audio/death/";
        if(!QFile::exists(tmp + QString("%1.ogg").arg(old_name)))
            tmp.replace("audio/", "extensions/audio/");
        if(QFile::copy(tmp + QString("%1.ogg").arg(old_name), tmp + QString("%1.ogg").arg(new_name)))
            count_files ++;
    }
    else{
        tmp = "audio/skill/";
        if(!QFile::exists(tmp + QString("%1.ogg").arg(old_name)))
            tmp.replace("audio/", "extensions/audio/");
        if(QFile::copy(tmp + QString("%1.ogg").arg(old_name), tmp + QString("%1.ogg").arg(new_name)))
            count_files ++;
        for(int i=1; ;i++){
            tmp = "audio/skill/";
            if(!QFile::exists(tmp + QString("%1%2.ogg").arg(old_name).arg(i)))
                tmp.replace("audio/", "extensions/audio/");
            if(!QFile::exists(tmp + QString("%1%2.ogg").arg(old_name).arg(i)))
                break;
            if(QFile::copy(tmp + QString("%1%2.ogg").arg(old_name).arg(i), tmp + QString("%1%2.ogg").arg(new_name).arg(i)))
                count_files ++;
        }
    }

    return count_files;
}
