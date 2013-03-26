#ifndef STARTSCENE_H
#define STARTSCENE_H

#include "button.h"
#include "pixmap.h"
#include "server.h"

class IrregularButton;
#include <QGraphicsScene>
#include <QAction>
#include <QTextEdit>

class StartScene: public QGraphicsScene{
    Q_OBJECT

public:
    StartScene();
    void addButton(QAction *action);
    void addMainButton(QList<QAction *> actions);
    void setServerLogBackground();
    void switchToServer(Server *server);

private:
    void printServerInfo();

    Pixmap *logo;
    Pixmap *button_group;
    PixmapItem *button_widget;
    QTextEdit *server_log;
    QList<Button*> buttons;
};

#endif // STARTSCENE_H
