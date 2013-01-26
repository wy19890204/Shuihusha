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
    FMOD_SOUND *initEncryptedFile(FMOD_SYSTEM *System, const QString &filename, const QString &key = "DefaultKey");
    //void playEncryptedFile(FMOD_SYSTEM *System, FMOD_SOUND *sound) = FMOD_System_PlaySound(System, FMOD_CHANNEL_FREE, sound, false, NULL);
};

#endif // CRYPTO_H
