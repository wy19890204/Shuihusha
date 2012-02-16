#include "standard.h"
#include "skill.h"
#include "guben.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"

class Pishan: public SlashBuffSkill{
public:
    Pishan():SlashBuffSkill("pishan"){
        frequency = Compulsory;
    }

    virtual bool buff(const SlashEffectStruct &effect) const{
        ServerPlayer *gao = effect.from;
        Room *room = gao->getRoom();
        room->playSkillEffect(objectName());
        LogMessage log;
        log.type = "#TriggerSkill";
        log.from = gao;
        log.arg = objectName();
        room->sendLog(log);

        room->slashResult(effect, NULL);
        return true;
    }
};

class Shixie: public PhaseChangeSkill{
public:
    Shixie():PhaseChangeSkill("shixie"){
        frequency = Compulsory;
    }

    virtual bool onPhaseChange(ServerPlayer *han) const{
        if(han->getPhase() == Player::Finish){
            Room *room = han->getRoom();
            if(han->getSlashCount() == 0){
                room->playSkillEffect(objectName());
                LogMessage log;
                log.type = "#TriggerSkill";
                log.from = han;
                log.arg = objectName();
                room->sendLog(log);

                room->loseHp(han);
            }
        }
        return false;
    }
};

GubenPackage::GubenPackage()
    :Package("guben")
{
    General *gaochonghan = new General(this, "gaochonghan", "jiang", 7);
    gaochonghan->addSkill(new Pishan);
    gaochonghan->addSkill(new Shixie);

    //addMetaObject<XianhaiCard>();
}

ADD_PACKAGE(Guben);
