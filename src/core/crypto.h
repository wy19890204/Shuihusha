#ifndef CRYPTO_H
#define CRYPTO_H
#include "fmod.h"
#include <QtCore/QCoreApplication>
#include <crypto++/des.h>
#include <stdio.h>
#include <QFileInfo>

class Crypto{
public:
    bool encryptMusicFile(const QString &filename, const QString &key = "DefaultKey");
    bool decryptMusicFile(const QString &filename, const QString &GlobalKey);
    FMOD_SOUND *initEncryptedFile(FMOD_SYSTEM *System, const QString &filename, const QString &key = "DefaultKey");
    const uchar *getEncryptedFile(const QString &filename, const QString &key = "DefaultKey");
private:
    qint64 size;
};

#endif // CRYPTO_H
