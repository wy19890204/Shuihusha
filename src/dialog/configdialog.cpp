#include "configdialog.h"
#include "settings.h"

#include <QFileDialog>
#include <QDesktopServices>
#include <QFontDialog>
#include <QColorDialog>

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);

    // tab 1
    QString bg_path = Config.value("BackgroundBrush").toString();
    if(!bg_path.startsWith(":"))
        ui->bgPathLineEdit->setText(bg_path);
    ui->bgPathLineEdit->setEnabled(false);
    ui->browseBgButton->setEnabled(false);
    ui->resetBgButton->setEnabled(false);

    ui->bgMusicPathLineEdit->setText(Config.value("BackgroundMusic").toString());

    ui->enableCardEffectCheckBox->setChecked(Config.EnableCardEffects);
    ui->enableEquipEffectCheckBox->setChecked(Config.EnableEquipEffects);
    ui->enableSkillEffectCheckBox->setChecked(Config.EnableSkillEffects);
    ui->enableLastWordCheckBox->setChecked(Config.EnableLastWord);
    ui->enableCheatRingCheckBox->setChecked(Config.EnableCheatRing);
    ui->enableBgMusicCheckBox->setChecked(Config.EnableBgMusic);
    ui->fitInViewCheckBox->setChecked(Config.FitInView);
    ui->circularViewCheckBox->setChecked(Config.CircularView);
    ui->noIndicatorCheckBox->setChecked(Config.value("NoIndicator", false).toBool());
    ui->minimizecCheckBox->setChecked(Config.value("EnableMinimizeDialog", false).toBool());

    ui->bgmVolumeSlider->setValue(100 * Config.BGMVolume);
    ui->effectVolumeSlider->setValue(100 * Config.EffectVolume);

    // tab 2
    ui->nullificationSpinBox->setValue(Config.NullificationCountDown);
    ui->neverNullifyMyTrickCheckBox->setChecked(Config.NeverNullifyMyTrick);
    ui->autoTargetCheckBox->setChecked(Config.EnableAutoTarget);
    ui->minimizecCheckBox->setChecked(Config.value("EnableMinimizeDialog", false).toBool());
    ui->showAllName->setChecked(Config.ShowAllName);
    ui->spOpen->setChecked(Config.SPOpen);
    ui->enableLua->setChecked(Config.EnableLua);
    ui->muteLoad->setChecked(Config.value("MuteLoad", false).toBool());

    QString lang = "zh_cn+en";
    ui->langComboBox->addItems(lang.split("+"));
    ui->langComboBox->lineEdit()->setText(Config.value("Language", "zh_cn").toString());

    // tab 3
    ui->skillEmotionBox->setChecked(Config.EnableSkillEmotion);

    QSet<QString> ban_emotions = Config.BanEmotions.toSet();
    QGridLayout *layout = new QGridLayout;
    layout->setOriginCorner(Qt::TopLeftCorner);
    ui->emoGroupBox->setLayout(layout);
    extension_group = new QButtonGroup;
    extension_group->setExclusive(false);
    const int columns = 4;
    QStringList emotions;
    QDir dir("image/system/emotion");
    foreach(QFileInfo info, dir.entryInfoList(QStringList() << "*.rcc" << "*.png")){
        if(QFile::exists(info.filePath()))
            emotions << info.baseName();
    }
    foreach(QString emotion, emotions){
        QCheckBox *checkbox = new QCheckBox;
        checkbox->setObjectName(emotion);
        checkbox->setText(Config.translate(emotion));
        checkbox->setChecked(!ban_emotions.contains(emotion));

        extension_group->addButton(checkbox);

        int row = emotions.indexOf(emotion) / columns;
        int column = emotions.indexOf(emotion) % columns;
        layout->addWidget(checkbox, row, column);
    }

    // tab 4
    ui->smtpServerLineEdit->setText(Config.value("Contest/SMTPServer").toString());
    ui->senderLineEdit->setText(Config.value("Contest/Sender").toString());
    ui->passwordLineEdit->setText(Config.value("Contest/Password").toString());
    ui->receiverLineEdit->setText(Config.value("Contest/Receiver").toString());

    ui->onlySaveLordCheckBox->setChecked(Config.value("Contest/OnlySaveLordRecord", true).toBool());

    connect(this, SIGNAL(accepted()), this, SLOT(saveConfig()));

    QFont font = Config.AppFont;
    showFont(ui->appFontLineEdit, font);

    font = Config.UIFont;
    showFont(ui->textEditFontLineEdit, font);

    QPalette palette;
    palette.setColor(QPalette::Text, Config.TextEditColor);
    ui->textEditFontLineEdit->setPalette(palette);
}

void ConfigDialog::showFont(QLineEdit *lineedit, const QFont &font){
    lineedit->setFont(font);
    lineedit->setText(QString("%1 %2").arg(font.family()).arg(font.pointSize()));
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::on_browseBgButton_clicked()
{
    QString location = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation);
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Select a background image"),
                                                    location,
                                                    tr("Images (*.png *.bmp *.jpg)"));

    if(!filename.isEmpty()){
        ui->bgPathLineEdit->setText(filename);

        Config.BackgroundBrush = filename;
        Config.setValue("BackgroundBrush", filename);

        emit bg_changed();
    }
}

void ConfigDialog::on_resetBgButton_clicked()
{
    ui->bgPathLineEdit->clear();

    QString filename = ":shuihu.jpg";
    Config.BackgroundBrush = filename;
    Config.setValue("BackgroundBrush", filename);

    emit bg_changed();
}

void ConfigDialog::saveConfig()
{
    int count_down = ui->nullificationSpinBox->value();
    Config.NullificationCountDown = count_down;
    Config.setValue("NullificationCountDown", count_down);

    float volume = ui->bgmVolumeSlider->value() / 100.0;
    Config.BGMVolume = volume;
    Config.setValue("BGMVolume", volume);
    volume = ui->effectVolumeSlider->value() / 100.0;
    Config.EffectVolume = volume;
    Config.setValue("EffectVolume", volume);

    bool enabled = ui->enableCardEffectCheckBox->isChecked();
    Config.EnableCardEffects = enabled;
    Config.setValue("EnableCardEffects", enabled);

    enabled = ui->enableEquipEffectCheckBox->isChecked();
    Config.EnableEquipEffects = enabled;
    Config.setValue("EnableEquipEffects", enabled);

    enabled = ui->enableSkillEffectCheckBox->isChecked();
    Config.EnableSkillEffects = enabled;
    Config.setValue("EnableSkillEffects", enabled);

    enabled = ui->enableLastWordCheckBox->isChecked();
    Config.EnableLastWord = enabled;
    Config.setValue("EnabledLastWord", enabled);

    enabled = ui->enableCheatRingCheckBox->isChecked();
    Config.EnableCheatRing = enabled;
    Config.setValue("EnabledCheatRing", enabled);

    enabled = ui->enableBgMusicCheckBox->isChecked();
    Config.EnableBgMusic = enabled;
    Config.setValue("EnableBgMusic", enabled);

    Config.FitInView = ui->fitInViewCheckBox->isChecked();
    Config.setValue("FitInView", Config.FitInView);

    Config.CircularView = ui->circularViewCheckBox->isChecked();
    Config.setValue("CircularView", Config.CircularView);

    Config.setValue("NoIndicator", ui->noIndicatorCheckBox->isChecked());

    Config.NeverNullifyMyTrick = ui->neverNullifyMyTrickCheckBox->isChecked();
    Config.setValue("NeverNullifyMyTrick", Config.NeverNullifyMyTrick);

    Config.EnableAutoTarget = ui->autoTargetCheckBox->isChecked();
    Config.setValue("EnableAutoTarget", Config.EnableAutoTarget);

    Config.EnableMinimizeDialog = ui->minimizecCheckBox->isChecked();
    Config.setValue("EnableMinimizeDialog", Config.EnableMinimizeDialog);

    Config.ShowAllName = ui->showAllName->isChecked();
    Config.setValue("ShowAllName", Config.ShowAllName);

    Config.SPOpen = ui->spOpen->isChecked();
    Config.setValue("SPOpen", Config.SPOpen);

    Config.EnableLua = ui->enableLua->isChecked();
    Config.setValue("EnableLua", Config.EnableLua);

    Config.setValue("MuteLoad", ui->muteLoad->isChecked());

    Config.EnableSkillEmotion = ui->skillEmotionBox->isChecked();
    Config.setValue("EnableSkillEmotion", Config.EnableSkillEmotion);

    QSet<QString> ban_emotions;
    QList<QAbstractButton *> checkboxes = extension_group->buttons();
    foreach(QAbstractButton *checkbox, checkboxes){
        if(!checkbox->isChecked()){
            QString emotion = checkbox->objectName();
            ban_emotions.insert(emotion);
        }
    }

    Config.BanEmotions = ban_emotions.toList();
    Config.setValue("BanEmotions", Config.BanEmotions);

    Config.setValue("Language", ui->langComboBox->lineEdit()->text());
    Config.setValue("Contest/SMTPServer", ui->smtpServerLineEdit->text());
    Config.setValue("Contest/Sender", ui->senderLineEdit->text());
    Config.setValue("Contest/Password", ui->passwordLineEdit->text());
    Config.setValue("Contest/Receiver", ui->receiverLineEdit->text());
    Config.setValue("Contest/OnlySaveLordRecord", ui->onlySaveLordCheckBox->isChecked());
}

void ConfigDialog::on_browseBgMusicButton_clicked()
{
    QString location = QDesktopServices::storageLocation(QDesktopServices::MusicLocation);
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Select a background music"),
                                                    location,
                                                    tr("Audio files (*.wav *.mp3 *.ogg)"));
    if(!filename.isEmpty()){
        ui->bgMusicPathLineEdit->setText(filename);
        Config.setValue("BackgroundMusic", filename);
    }
}

void ConfigDialog::on_resetBgMusicButton_clicked()
{
    QString default_music = "audio/system/background.mp3";
    Config.setValue("BackgroundMusic", default_music);
    ui->bgMusicPathLineEdit->setText(default_music);
}

void ConfigDialog::on_changeAppFontButton_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, Config.AppFont, this);
    if(ok){
        Config.AppFont = font;
        showFont(ui->appFontLineEdit, font);

        Config.setValue("AppFont", font);
        QApplication::setFont(font);
    }
}


void ConfigDialog::on_setTextEditFontButton_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, Config.UIFont, this);
    if(ok){
        Config.UIFont = font;
        showFont(ui->textEditFontLineEdit, font);

        Config.setValue("UIFont", font);
        QApplication::setFont(font, "QTextEdit");
    }
}

void ConfigDialog::on_setTextEditColorButton_clicked()
{
    QColor color = QColorDialog::getColor(Config.TextEditColor, this);
    if(color.isValid()){
        Config.TextEditColor = color;
        Config.setValue("TextEditColor", color);
        QPalette palette;
        palette.setColor(QPalette::Text, color);
        ui->textEditFontLineEdit->setPalette(palette);
    }
}
