#ifndef CRYPTO_H
#define CRYPTO_H
#include "fmod.h"
#include <QtCore/QCoreApplication>
#include <crypto++/des.h>
#include <stdio.h>
#include <QFileInfo>

class Crypto{
public:

    QString chooseMusicFile();
    bool encryptMusicFile(const QString &filename, const char *GlobalKey = "shuihusha");
    void playEncryptedFile(FMOD_SYSTEM *System, const QString &filename, const char *GlobalKey = "shuihusha");

//private:
//    static QString key;
};

#endif // CRYPTO_H
