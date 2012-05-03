#include "general.h"
#include "standard.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "carditem.h"
#include "serverplayer.h"
#include "room.h"

StandardPackage::StandardPackage()
    :Package("standard")
{
}

ADD_PACKAGE(Standard)
