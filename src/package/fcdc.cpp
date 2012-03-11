#include "fcdc.h"
#include "general.h"
#include "skill.h"
#include "standard.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"


FCDCPackage::FCDCPackage()
    :Package("FCDC")
{
    General *xiezhen = new General(this, "xiezhen", "min");
    xiezhen->addSkill(new Xunlie);

}

ADD_PACKAGE(FCDC);
