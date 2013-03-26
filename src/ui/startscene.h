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

    Pixmap *logo;
    QGraphicsItem *button_plate;
private:
    void printServerInfo();

    Pixmap *button_group;
    QTextEdit *server_log;
    QList<Button*> buttons;
};

#endif // STARTSCENE_H
