#include "pixmap.h"
#include "settings.h"

#include <QPainter>
#include <QGraphicsColorizeEffect>
#include <QMessageBox>
#include <QImageReader>

Pixmap::Pixmap(const QString &filename, bool center_as_origin)
    :pixmap(filename), markable(false), marked(false)
{

#ifndef QT_NO_DEBUG
//only complains about pixmap loading errors under debug mode
    if(pixmap.isNull()){
        QImageReader reader(filename);
        QString error_string = reader.errorString();

        QString warning = tr("Can not load image %1[%2], error string is %3")
                          .arg(filename).arg(metaObject()->className()).arg(error_string);
        QMessageBox::warning(NULL, tr("Warning"), warning);
    }
#endif
    if(center_as_origin)
        setTransformOriginPoint(pixmap.width()/2, pixmap.height()/2);
}

Pixmap::Pixmap()
    :markable(false), marked(false)
{
}

QRectF Pixmap::boundingRect() const{
    return QRectF(0, 0, pixmap.width(), pixmap.height());
}

bool Pixmap::changePixmap(const QString &filename){
    bool success = pixmap.load(filename);
    if(success)
        prepareGeometryChange();

    return success;
}

void Pixmap::setPixmap(const QPixmap &pixmap){
    this->pixmap = pixmap;
    prepareGeometryChange();
}

void Pixmap::shift(){
    moveBy(-pixmap.width()/2, -pixmap.height()/2);
}

void Pixmap::MakeGray(QPixmap &pixmap){
    QImage img = pixmap.toImage();

    int i,j;
    for(i=0; i<img.width(); i++){
        for(j=0; j<img.height(); j++){
            QRgb color = img.pixel(i, j);
            int gray = qGray(color);
            color = qRgba(gray, gray, gray, qAlpha(color));
            img.setPixel(i, j, color);
        }
    }

    pixmap = QPixmap::fromImage(img);
}

void Pixmap::makeGray(){
    MakeGray(pixmap);
}

void Pixmap::scaleSmoothly(qreal ratio){
    qreal width = pixmap.width() * ratio;
    qreal height = pixmap.height() * ratio;
    pixmap = pixmap.scaled(width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    prepareGeometryChange();
}

void Pixmap::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->drawPixmap(0, 0, pixmap);
}

QVariant Pixmap::itemChange(GraphicsItemChange change, const QVariant &value){
    switch(change){
    case ItemSelectedHasChanged:{
        if(value.toBool()){
            QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect(this);
            effect->setColor(QColor(0xCC, 0x00, 0x00));
            setGraphicsEffect(effect);
        }else
            setGraphicsEffect(NULL);

        emit selected_changed();
        break;
    }
    case ItemEnabledHasChanged:{
        if(this->inherits("CardItem"))
        {
            if(value.toBool()){
                setOpacity(1.0);
            }else{
                setOpacity(0.7);
            }
        }
        else emit enable_changed();
        break;
    }
    /*
    case ItemPositionHasChanged:{
        qDebug("logo: %s", qPrintable(this->objectName()));
        if(this->objectName() == "logo"){
            Config.setValue("UI/LogoPosition", this->pos());
        }
        break;
    }*/
    default:
        break;
    }

    return QGraphicsObject::itemChange(change, value);
}

void Pixmap::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if(this->objectName() == "logo")
        Config.setValue("UI/LogoPosition", this->pos());
    QGraphicsItem::mouseReleaseEvent(event);
}

bool Pixmap::isMarked() const{
    return markable && marked;
}

bool Pixmap::isMarkable() const{
    return markable;
}

void Pixmap::mark(bool marked){
    if(markable){
        if(this->marked != marked){
            this->marked = marked;
            emit mark_changed();
        }
    }
}

void Pixmap::setMarkable(bool markable){
    this->markable = markable;
}

PixmapItem::PixmapItem(const QPixmap &pixmap)
    :QGraphicsPixmapItem(pixmap)
{
}

void PixmapItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    //if(this->objectName() == "plate")
    //    Config.setValue("UI/PlatePosition", this->pos());
    PixmapItem::mouseReleaseEvent(event);
}

void PixmapItem::setObjectName(const QString &name){
    this->objectname = name;
}
