#ifndef PACKAGE_H
#define PACKAGE_H

class Skill;
class Card;
class Player;

#include "card.h"
#include <QObject>
#include <QHash>
#include <QStringList>
#include <QMap>

class CardPattern{
public:
    virtual bool match(const Player *player, const Card *card) const = 0;
    virtual bool willThrow() const{
        return true;
    }
};

class Package: public QObject{
    Q_OBJECT

    Q_ENUMS(Type)
    Q_ENUMS(Genre)

public:
    enum Type{
        GeneralPack = 1,
        CardPack = 2,
        MixedPack = 3,
        SpecialPack = 13
    };

    enum Genre{
        LUA = 0,
        CPP = 1
    };

    Package(const QString &name){
        setObjectName(name);
        type = GeneralPack;
        genre = LUA;
    }

    QList<const QMetaObject *> getMetaObjects() const{
        return metaobjects;
    }

    QList<const Skill *> getSkills() const{
        return skills;
    }

    QMap<QString, const CardPattern *> getPatterns() const{
        return patterns;
    }

    QMultiMap<QString, QString> getRelatedSkills() const{
        return related_skills;
    }

    Type getType() const{
        return type;
    }

    Genre getGenre() const{
        return genre;
    }

    template<typename T>
    void addMetaObject(){
        metaobjects << &T::staticMetaObject;
    }

protected:
    QList<const QMetaObject *> metaobjects;
    QList<const Skill *> skills;
    QMap<QString, const CardPattern *> patterns;
    QMultiMap<QString, QString> related_skills;
    Type type;
    Genre genre;
};

class GeneralPackage : public Package{
    Q_OBJECT
public:
    explicit GeneralPackage(const QString &name);
};

class CardPackage : public Package{
    Q_OBJECT
public:
    explicit CardPackage(const QString &name);
};

typedef QHash<QString, Package *> PackageHash;
class PackageAdder{

public:
    PackageAdder(const QString &name, Package *pack){
        packages()[name] = pack;
    }
    
    static PackageHash& packages(void);
};

#define ADD_PACKAGE(name) static PackageAdder name##PackageAdder(#name, new name##Package);

#endif // PACKAGE_H
