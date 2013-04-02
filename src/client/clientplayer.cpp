#include "clientplayer.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "standard.h"

#include <QTextDocument>
#include <QTextOption>
#include <QFile>

ClientPlayer *Self = NULL;

ClientPlayer::ClientPlayer(Client *client)
    :Player(client), handcard_num(0)
{
    mark_doc = new QTextDocument(this);
    mark_doc->setTextWidth(128);
    mark_doc->setDefaultTextOption(QTextOption(Qt::AlignRight));
    mark_doc_small = new QTextDocument(this);
    mark_doc_small->setTextWidth(128);
    mark_doc_small->setDefaultTextOption(QTextOption(Qt::AlignLeft));
}

void ClientPlayer::handCardChange(int delta){
    handcard_num += delta;
}

int ClientPlayer::aliveCount() const{
    return ClientInstance->alivePlayerCount();
}

int ClientPlayer::getHandcardNum() const{
    return handcard_num;
}

void ClientPlayer::addCard(const Card *card, Place place){
    switch(place){
    case Hand: {
            if(card)
                known_cards << card;
            handcard_num++;
            break;
        }
    case Equip: {
            const EquipCard *equip = qobject_cast<const EquipCard*>(card);
            setEquip(equip);
            break;
        }
    case Judging:{
            addDelayedTrick(card);
            break;
        }
    default:
        // FIXME
        ;
    }
}

void ClientPlayer::addKnownHandCard(const Card *card){
    if(!known_cards.contains(card))
        known_cards << card;
}

bool ClientPlayer::isLastHandCard(const Card *card) const{
    if(known_cards.length() != 1)
        return false;

    return known_cards.first()->getEffectiveId() == card->getEffectiveId();
}

void ClientPlayer::removeCard(const Card *card, Place place){
    switch(place){
    case Hand: {
            handcard_num--;
            if(card)
                known_cards.removeOne(card);
            break;
        }
    case Equip:{
            const EquipCard *equip = qobject_cast<const EquipCard*>(card);
            removeEquip(equip);
            break;
        }
    case Judging:{
            removeDelayedTrick(card);
            break;
        }

    default:
        // FIXME
        ;
    }
}

QList<const Card *> ClientPlayer::getCards() const{
    return known_cards;
}

void ClientPlayer::setCards(const QList<int> &card_ids){
    known_cards.clear();

    foreach(int card_id, card_ids){
        known_cards << Sanguosha->getCard(card_id);
    }
}

QTextDocument *ClientPlayer::getMarkDoc(bool dashboard) const{
    return dashboard ? mark_doc: mark_doc_small;
}

void ClientPlayer::changePile(const QString &name, bool add, int card_id){
    if(add)
        piles[name] << card_id;
    else
        piles[name].removeOne(card_id);

    if(!name.startsWith("#"))
        emit pile_changed(name);
}

QString ClientPlayer::getDeathPixmapPath(bool isdash) const{
    QString basename = "unknown";
    if(ServerInfo.GameMode == "06_3v3" ||
       ServerInfo.GameMode == "warlords" ||
       ServerInfo.GameMode == "landlord")
        basename = getScreenRole();
    else if(ServerInfo.EnableHegemony)
        basename.clear();
    else
        basename = getRole();

    if(property("panxin").toBool())
        basename = "unknown";

    QString dash = isdash ? "dashboard" : "photo";
    return QString("image/system/death/%1/%2.png").arg(dash).arg(basename);
}

void ClientPlayer::setHandcardNum(int n){
    handcard_num = n;
}

QString ClientPlayer::getGameMode() const{
    return ServerInfo.GameMode;
}

void ClientPlayer::setFlags(const QString &flag){
    Player::setFlags(flag);

    if(flag.endsWith("drank"))
        emit drank_changed();
    //else if(flag.endsWith("ecst"))
    //    emit ecst_changed();
    else if(flag.endsWith("actioned"))
        emit action_taken();
}

void ClientPlayer::setMark(const QString &mark, int value){
    if(marks[mark] == value)
        return;

    marks[mark] = value;

    if(mark.endsWith("poison"))
        emit poison_changed();
    if(mark.endsWith("_wake"))
        emit waked();
    if(!mark.startsWith("@"))
        return;

    // set mark doc
    QString text = "";
    QString text_small = "";
    QMapIterator<QString, int> itor(marks);
    itor.toBack();
    while(itor.hasPrevious()){
        itor.previous();

        if(itor.key().startsWith("@") && itor.value() > 0){
            QString path = QString("image/mark/%1.png").arg(itor.key());
            if(!QFile::exists(path))
                path = QString("extensions/generals/mark/%1.png").arg(itor.key());
            QString mark_text = QString("<img src='%1' />").arg(path);
            QString mark_text_small = QString("<img src='%1' />").arg(path);
            if(itor.value() != 1){
                mark_text.append(QString("x%1").arg(itor.value()));
                mark_text_small.append(QString("x%1").arg(itor.value()));
            }
            text.append(mark_text);
            text_small.append(mark_text_small);
        }
    }

    mark_doc->setHtml(text);
    mark_doc_small->setHtml(text_small);
}
