#include "package.h"

GeneralPackage::GeneralPackage(const QString &name)
    :Package(name)
{
    type = GeneralPack;
    genre = CPP;
}

CardPackage::CardPackage(const QString &name)
    :Package(name)
{
    type = CardPack;
    genre = CPP;
}

Q_GLOBAL_STATIC(PackageHash, Packages)
PackageHash& PackageAdder::packages(){
    return *(::Packages());
}
