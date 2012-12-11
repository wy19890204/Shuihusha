#ifndef CRYPTO_H
#define CRYPTO_H
#include "fmod.h"
#include <QtCore/QCoreApplication>
#include <crypto++/des.h>
#include <stdio.h>
#include <QFileInfo>

class Crypto{
public:

    void DES_Process(const char *keyString, byte *block, size_t length, CryptoPP::CipherDir direction);
    QString chooseMusicFile();
    bool encryptMusicFile(const QString &filename, const char *GlobalKey = "shui____hu____sha");
    FMOD_SOUND *initEncryptedFile(FMOD_SYSTEM *System, const QString &filename, const char *GlobalKey = "shui____hu____sha");
    void playEncryptedFile(FMOD_SYSTEM *System, FMOD_SOUND *sound);

//private:
//    static QString key;
};

#endif // CRYPTO_H
