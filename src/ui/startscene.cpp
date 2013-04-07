#include "startscene.h"
#include "engine.h"
#include "audio.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QNetworkInterface>
#include <QGraphicsDropShadowEffect>

StartScene::StartScene()
{
    // game logo
    logo = new Pixmap("image/logo/logo.png");
    logo->shift();
    if(Config.value("UI/ButtonStyle", true).toBool()){
        logo->moveBy(Config.Rect.width()/3+20, -Config.Rect.height()/4-50);
        logo->setPos(Config.value("UI/LogoPosition", logo->pos()).toPoint());
        logo->setFlags(QGraphicsItem::ItemIsMovable);
        logo->setObjectName("logo");
    }
    else
        logo->moveBy(0, -Config.Rect.height()/4);
    addItem(logo);
/*
    //the website URL
    QFont website_font(Config.SmallFont);
    website_font.setStyle(QFont::StyleItalic);
    QGraphicsSimpleTextItem *website_text = addSimpleText("http://qsanguosha.org", website_font);
    website_text->setBrush(Qt::white);
    website_text->setPos(Config.Rect.width()/2 - website_text->boundingRect().width(),
                       Config.Rect.height()/2 - website_text->boundingRect().height());
*/
    server_log = NULL;

    if(Config.value("UI/ButtonStyle", false).toBool()){
        //Provide coordinates for the button
#ifdef USE_RCC
        button_group = new Pixmap(":plate/background.png");
#else
        button_group = new Pixmap("image/system/button/plate/background.png");
#endif
        button_group->shift();
        //button_group->moveBy(0, -Config.Rect.height()/10);
        button_group->hide();
    }
}

void StartScene::addButton(QAction *action){
    QString text = action->text();
    if(action->objectName() == "actionPackaging")
        text = tr("Lua Manager");
    else if(action->objectName() == "actionReplay")
        text = tr("Replay");
    Button *button = new Button(text);
    button->setMute(false);

    connect(button, SIGNAL(clicked()), action, SLOT(trigger()));
    addItem(button);

    QRectF rect = button->boundingRect();
    int n = buttons.length();
    if(n < 5)
        button->setPos(- rect.width() - 5, (n - 1) * (rect.height() * 1.2));
    else
        button->setPos(5, (n - 6) * (rect.height() * 1.2));

    buttons << button;
}

#include "irregularbutton.h"
void StartScene::addMainButton(QList<QAction *> actions){
    static QString tianjis_name[9] = {
        "start", "join", "replay", "lua", "config", "general", "card", "mode", "thanks"
    };
    static int tianjipos[9][2] = {
        {220, 235}, //0.start
        {175, 190}, //1.join
        {250, 480}, //2.replay
        {145, 260}, //3.lua
        {175, 430}, //4.config
        {245, 160}, //5.general
        {415, 190}, //6.card
        {415, 360}, //7.mode
        {145, 360}, //8.thanks
    };

#ifdef USE_RCC
    QString path = ":plate/background.png";
#else
    QString path = "image/system/button/plate/background.png";
#endif
    button_plate = new QGraphicsPixmapItem(QPixmap(path));
    button_plate->setPos(Config.value("UI/PlatePosition", button_group->pos()).toPoint());
    button_plate->setFlags(QGraphicsItem::ItemIsMovable);

    int count = 0;
    foreach(QAction *action, actions){
        IrregularButton *tianji = new IrregularButton(tianjis_name[count], "plate");
        tianji->setMute(false);
        tianji->setParentItem(button_plate);
        //tianji->setPos(button_plate->pos());
        int *froups = tianjipos[count];
        tianji->moveBy(froups[0]+330, froups[1]+200);
        connect(tianji, SIGNAL(clicked()), action, SLOT(trigger()));
        count ++;
    }

    addItem(button_plate);
}

void StartScene::setServerLogBackground(){
    if(server_log){
        // make its background the same as background, looks transparent
        QPalette palette;
        palette.setBrush(QPalette::Base, backgroundBrush());
        server_log->setPalette(palette);
    }
}

void StartScene::switchToServer(Server *server){    
#ifdef AUDIO_SUPPORT

    Audio::quit();

#endif

    // performs leaving animation
    QPropertyAnimation *logo_shift = new QPropertyAnimation(logo, "pos");
    logo_shift->setEndValue(Config.Rect.topLeft());

    QPropertyAnimation *logo_shrink = new QPropertyAnimation(logo, "scale");
    logo_shrink->setEndValue(0.5);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(logo_shift);
    group->addAnimation(logo_shrink);
    group->start(QAbstractAnimation::DeleteWhenStopped);

    foreach(Button *button, buttons)
        delete button;
    buttons.clear();

    server_log = new QTextEdit();
    server_log->setReadOnly(true);
    server_log->resize(700, 420);
    server_log->move(-400, -180);
    server_log->setFrameShape(QFrame::NoFrame);
    server_log->setFont(QFont("Verdana", 12));
    server_log->setTextColor(Config.TextEditColor);
    server_log->setProperty("type", "border");
    setServerLogBackground();

    addWidget(server_log);

    printServerInfo();

    connect(server, SIGNAL(server_message(QString)), server_log, SLOT(append(QString)));

    update();
}

void StartScene::printServerInfo(){
    QStringList items;
    QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    foreach(QHostAddress address, addresses){
        quint32 ipv4 = address.toIPv4Address();
        if(ipv4)
            items << address.toString();
    }

    items.sort();

    foreach(QString item, items){
        if(item.startsWith("192.168.") || item.startsWith("10."))
            server_log->append(tr("Your LAN address: %1, this address is available only for hosts that in the same LAN").arg(item));
        else if(item == "127.0.0.1")
            server_log->append(tr("Your loopback address %1, this address is available only for your host").arg(item));
        else if(item.startsWith("5."))
            server_log->append(tr("Your Hamachi address: %1, the address is available for users that joined the same Hamachi network").arg(item));
        else if(!item.startsWith("169.254."))
            server_log->append(tr("Your other address: %1, if this is a public IP, that will be available for all cases").arg(item));
    }

    server_log->append(tr("Binding port number is %1").arg(Config.ServerPort));
    server_log->append(tr("Game mode is %1").arg(Sanguosha->getModeName(Config.GameMode)));
    server_log->append(tr("Player count is %1").arg(Sanguosha->getPlayerCount(Config.GameMode)));

    server_log->append( Config.OperationNoLimit ?
                        tr("There is no time limit") :
                        tr("Operation timeout is %1 seconds").arg(Config.OperationTimeout));

    if(Config.ContestMode)
        server_log->append(tr("The contest mode is enabled"));

    QStringList cheats;
    Config.beginGroup("Cheat");
    if(Config.value("EnableCheatMenu", false).toBool())
        cheats << Sanguosha->translate("cheat0");
    if(Config.FreeChooseGenerals)
        cheats << Sanguosha->translate("cheat1");
    if(Config.FreeChooseCards)
        cheats << Sanguosha->translate("cheat2");
    if(Config.value("FreeAssign", false).toBool())
        cheats << Sanguosha->translate("cheat3");
    if(Config.value("FreeRegulate", false).toBool())
        cheats << Sanguosha->translate("cheat4");
    if(Config.value("FreeChange", false).toBool())
        cheats << Sanguosha->translate("cheat5");
    if(Config.value("FreeShowRole", false).toBool())
        cheats << Sanguosha->translate("cheat6");
    if(Config.value("FreeUnDead", false).toBool())
        cheats << Sanguosha->translate("cheat7");
    Config.endGroup();
    QString cheating = cheats.isEmpty() ? tr("Disabled") : cheats.join(" , ");
    server_log->append(tr("Cheat choose is") + cheating);

    if(Config.Enable2ndGeneral){
        QString scheme_str;
        switch(Config.MaxHpScheme){
        case 0: scheme_str = tr("sum - 3"); break;
        case 1: scheme_str = tr("minimum"); break;
        case 2: scheme_str = tr("average"); break;
        }

        server_log->append(tr("Secondary general is enabled, max hp scheme is %1").arg(scheme_str));
    }else
        server_log->append(tr("Seconardary general is disabled"));

    server_log->append( Config.EnableReincarnation ?
                        tr("Reincarnation Rule is enabled") :
                        tr("Reincarnation Rule is disabled"));

    server_log->append( Config.EnableAnzhan ?
                        tr("Anzhan Mode is enabled") :
                        tr("Anzhan Mode is disabled"));

    server_log->append( Config.EnableScene ?
                        tr("Scene Mode is enabled") :
                        tr("Scene Mode is disabled"));

    server_log->append( Config.EnableBasara ?
                        tr("Basara Mode is enabled") :
                        tr("Basara Mode is disabled"));

    server_log->append( Config.EnableHegemony ?
                        tr("Hegemony Mode is enabled") :
                        tr("Hegemony Mode is disabled"));

    server_log->append( Config.EnableSame ?
                        tr("Same Mode is enabled") :
                        tr("Same Mode is disabled"));

    server_log->append( Config.EnableEndless ?
                        tr("Endless Mode is enabled, time: %1").arg(Config.value("EndlessTimes").toString()) :
                        tr("Endless Mode is disabled"));

    if(Config.EnableAI){
        server_log->append(tr("This server is AI enabled, AI delay is %1 milliseconds").arg(Config.AIDelay));
        server_log->append( Config.value("AIChat", true).toBool() ?
                            tr("This server is AI chat enabled") :
                            tr("This server is AI chat disabled"));
    }
    else
        server_log->append(tr("This server is AI disabled"));

}
