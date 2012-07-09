#ifndef CRYPTO_H
#define CRYPTO_H
#include <QtCore/QCoreApplication>
#include <crypto++/des.h>
#include <stdio.h>
#include <QFile>

class Crypto{
public:
    enum CryType {Jiami, Jiemi};

    static char *doCrypto(CryType type, const QString &input, const QString &output = "none", const char *key = "shuihusha");

//private:
//    static QString key;
};

#endif // CRYPTO_H
