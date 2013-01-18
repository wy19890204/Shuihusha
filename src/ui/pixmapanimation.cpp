#include "pixmapanimation.h"
#include <QPainter>
#include <QPixmapCache>
#include <QDir>
#include <QResource>

PixmapAnimation::PixmapAnimation(QGraphicsScene *scene) :
    QGraphicsItem(0,scene)
{
}

void PixmapAnimation::advance(int phase)
{
    if(phase)current++;
    if(current>=frames.size())
    {
        current = 0;
        emit finished();
    }
    update();
}

void PixmapAnimation::setPath(const QString &path)
{
    frames.clear();

    int i = 0;
    QString pic_path = QString("%1%2%3").arg(path).arg(i++).arg(".png");
    do{
        frames << GetFrameFromCache(pic_path);
        pic_path = QString("%1%2%3").arg(path).arg(i++).arg(".png");
    }while(!GetFrameFromCache(pic_path).isNull());

    current = 0;
}

void PixmapAnimation::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->drawPixmap(0,0,frames.at(current));
}

QRectF PixmapAnimation::boundingRect() const
{
    return frames.at(current).rect();
}

bool PixmapAnimation::valid()
{
    return !frames.isEmpty();
}

void PixmapAnimation::timerEvent(QTimerEvent *)
{
    advance(1);
}

void PixmapAnimation::start(bool permanent,int interval)
{
    startTimer(interval);
    if(!permanent)connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
}

PixmapAnimation* PixmapAnimation::GetPixmapAnimation(QGraphicsObject *parent, const QString &emotion)
{
    PixmapAnimation *pma = new PixmapAnimation();
#ifdef USE_RCC
    QResource::registerResource(QString("image/system/emotion/%1.rcc").arg(emotion));
    pma->setPath(QString(":/%1/").arg(emotion));
#else
    pma->setPath(QString("image/system/emotion/%1/").arg(emotion));
#endif
    bool returnpma = false;
    if(pma->valid()){
        QStringList emotions;
        emotions //<< "slash_red" << "slash_black" << "thunder_slash" << "fire_slash"
                //<< "peach" << "analeptic"
                //<< "chain" << "recover"
                //<< "weapon" << "armor"
                //<< "no-success" << "success"
                ;
        if(emotions.contains(emotion)){
            pma->moveBy(pma->boundingRect().width()*0.15,
                        pma->boundingRect().height()*0.15);
            pma->setScale(0.7);
        }
        else if(emotion == "horse"){
            pma->setZValue(pma->zValue() + 0.5);
            pma->moveBy(90,0);
        }

        pma->moveBy((parent->boundingRect().width() - pma->boundingRect().width())/2,
                (parent->boundingRect().height() - pma->boundingRect().height())/2);

        pma->setParentItem(parent);
        pma->startTimer(70);
        connect(pma,SIGNAL(finished()),pma,SLOT(deleteLater()));
        returnpma = true;
    }
#ifdef USE_RCC
    QResource::unregisterResource(QString("image/system/emotion/%1.rcc").arg(emotion));
#endif
    if(returnpma)
        return pma;
    else{
        delete pma;
        return NULL;
    }
}

QPixmap PixmapAnimation::GetFrameFromCache(const QString &filename){
    QPixmap pixmap;
    if(!QPixmapCache::find(filename, &pixmap)){
        pixmap.load(filename);
        if(!pixmap.isNull())
            QPixmapCache::insert(filename, pixmap);
    }

    return pixmap;
}

int PixmapAnimation::GetFrameCount(const QString &emotion){
    QString path = QString("image/system/emotion/%1/").arg(emotion);
    QDir dir(path);
    return dir.entryList(QDir::Files | QDir::NoDotAndDotDot).count();
}
