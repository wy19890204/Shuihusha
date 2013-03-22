#include "irregularbutton.h"
#include "clientplayer.h"
#include "engine.h"

#include <QBitmap>
#include <QPainter>

static inline QString MakePath(const QString &name, const QString &state, const QString &father){
    return QString("image/system/button/%1/%2-%3.png").arg(father).arg(name).arg(state);
}

IrregularButton::IrregularButton(const QString &name, const QString &father)
    :mute(true)
{
    state = Normal;

    normal.load(MakePath(name, "normal", father));
    hover.load(MakePath(name, "hover", father));
    down.load(MakePath(name, "down", father));
    disabled.load(MakePath(name, "disabled", father));

    QBitmap mask_bitmap(MakePath(name, "mask", father));
    mask = QRegion(mask_bitmap);

    setAcceptsHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
}

void IrregularButton::click(){
    if(isEnabled())
        emit clicked();
}

QRectF IrregularButton::boundingRect() const{
    return QRectF(normal.rect());
}

void IrregularButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    QPixmap *to_draw = NULL;
    if(!isEnabled())
        to_draw = &disabled;
    else{
        switch(state){
        case Normal: to_draw = &normal; break;
        case Hover: to_draw = &hover; break;
        case Down: to_draw = &down; break;
        }
    }

    if(to_draw)
        painter->drawPixmap(0, 0, *to_draw);
}

void IrregularButton::changeState(IrregularButton::State state){
    if(this->state != state){
        this->state = state;
        update();
    }
}

bool IrregularButton::inMask(const QPointF &pos) const{
    return mask.contains(QPoint(pos.x(), pos.y()));
}

void IrregularButton::setMute(bool mute){
    this->mute = mute;
}

#include <QGraphicsSceneHoverEvent>

void IrregularButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    QPointF point = mapToParent(event->pos());
    if(inMask(point)){
#ifdef AUDIO_SUPPORT
        if(!mute)
            Sanguosha->playAudio("button-hover");
#endif
        changeState(Hover);
    }
}

void IrregularButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    changeState(Normal);
}

void IrregularButton::hoverMoveEvent(QGraphicsSceneHoverEvent *event){
    QPointF point = mapToParent(event->pos());
    changeState(inMask(point) ? Hover : Normal);
}

void IrregularButton::mousePressEvent(QGraphicsSceneMouseEvent *event){
    QPointF point = mapToParent(event->pos());
    if(inMask(point)){
        changeState(Down);
    }
}

void IrregularButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    QPointF point = mapToParent(event->pos());
    if(inMask(point)){
#ifdef AUDIO_SUPPORT
        if(!mute)
            Sanguosha->playAudio("button-down");
#endif
        changeState(Normal);
        emit clicked();
    }
}

TrustButton::TrustButton(){
    trust.load("image/system/button/irregular/trust.png");
    untrust.load("image/system/button/irregular/untrust.png");
    mask = QRegion(QBitmap("image/system/button/irregular/trust-mask.png"));
}

QRectF TrustButton::boundingRect() const{
    return QRectF(trust.rect());
}

void TrustButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){
    if(Self->getState() == "trust")
        painter->drawPixmap(0, 0, untrust);
    else
        painter->drawPixmap(0, 0, trust);
}

void TrustButton::mousePressEvent(QGraphicsSceneMouseEvent *event){
    event->accept();
}

void TrustButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    QPointF pos = mapToParent(event->pos());
    if(mask.contains(QPoint(pos.x(), pos.y()))){
        emit clicked();
    }
}
