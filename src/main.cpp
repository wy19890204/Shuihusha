#include <QtGui/QApplication>

#include <QCoreApplication>
#include <QTranslator>
#include <QDir>
#include <cstring>
#include <QDateTime>
#include <QResource>

#include "mainwindow.h"
#include "settings.h"
#include "banpair.h"
#include "server.h"
#include "audio.h"

int main(int argc, char *argv[])
{    
    if(argc > 1 && strcmp(argv[1], "-server") == 0)
        new QCoreApplication(argc, argv);
    else
        new QApplication(argc, argv);

#ifdef Q_OS_MAC
#ifdef QT_NO_DEBUG

    QDir::setCurrent(qApp->applicationDirPath());

#endif
#endif

#ifdef Q_OS_LINUX
    QDir dir(QString("lua"));
    if (dir.exists() && (dir.exists(QString("config.lua")))) {
        // things look good and use current dir
    } else {
        QDir::setCurrent(qApp->applicationFilePath().replace("games","share"));
    }
#endif

    // initialize random seed for later use
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    QTranslator qt_translator, translator;
    qt_translator.load("qt_zh_CN.qm");
    translator.load("shuihusha.qm");
    if(Config.value("Language", "zh_cn").toString() == "zh_cn"){
        qApp->installTranslator(&qt_translator);
        qApp->installTranslator(&translator);
    }

    Sanguosha = new Engine;
    Config.init();
    BanPair::loadBanPairs();

    if(qApp->arguments().contains("-server")){
        Server *server = new Server(qApp);
        printf("Server is starting on port %u\n", Config.ServerPort);

        if(server->listen())
            printf("Starting successfully\n");
        else
            printf("Starting failed!\n");

        return qApp->exec();
    }

    QFile file("shuihusha.qss");
    if(file.open(QIODevice::ReadOnly)){
        QTextStream stream(&file);
#ifdef USE_RCC
        QString content = stream.readAll();
#else
        QString content = stream.readAll();
        content.replace(":system", "image/system");
#endif
        qApp->setStyleSheet(content);
    }

#ifdef AUDIO_SUPPORT

    Audio::init();

#endif

    MainWindow *main_window = new MainWindow;

    Sanguosha->setParent(main_window);
    main_window->show();

    foreach(QString arg, qApp->arguments()){
        if(arg.startsWith("-connect:")){
            arg.remove("-connect:");
            Config.HostAddress = arg;
            Config.setValue("HostAddress", arg);

            main_window->on_actionRestart_game_triggered();

            break;
        }
    }
    QResource::registerResource("backdrop/shuihu.rcc");

    return qApp->exec();
}
