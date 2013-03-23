#ifndef IRREGULARBUTTON_H
#define IRREGULARBUTTON_H

#include <QGraphicsObject>
#include <QPixmap>
#include <QRegion>

class IrregularButton : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit IrregularButton(const QString &name, const QString &father = "irregular");
    void click();

    virtual QRectF boundingRect() const;
    void setMute(bool mute);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    enum State { Normal, Hover, Down };
    State state;
    bool mute;

    QPixmap normal, hover, down, disabled;
    QRegion mask;

    void changeState(State state);
    bool inMask(const QPointF &pos) const;

signals:
    void clicked();
};

class TrustButton : public QGraphicsObject{
    Q_OBJECT

public:
    explicit TrustButton();

    virtual QRectF boundingRect() const;

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    QPixmap trust, untrust;
    QRegion mask;

signals:
    void clicked();
};

#endif // IRREGULARBUTTON_H
