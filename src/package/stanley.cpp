#include "standard.h"
#include "stanley.h"

StanleyPackage::StanleyPackage()
    :Package("stanley")
{
    General *chenliqing = new General(this, "chenliqing", "min", 4, false, true);
    chenliqing->addSkill(new Skill("dangkou"));
}

ADD_PACKAGE(Stanley);
