#ifndef CRYPTO_H
#define CRYPTO_H
#include <QtCore/QCoreApplication>
#include <crypto++/des.h>
#include <stdio.h>
#include <QFile>

struct CryStruct{
    char *buffer;
    int size;
};

class Crypto{
public:
    enum CryType {Jiami, Jiemi};

    static CryStruct doCrypto(CryType type, const QString &input, const QString &output = "none", const char *key = "shuihusha");

//private:
//    static QString key;
};

#endif // CRYPTO_H
