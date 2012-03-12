#include "standard.h"
#include "skill.h"
#include "guben.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"

class Jielue: public TriggerSkill{
public:
    Jielue():TriggerSkill("jielue"){
        events << SlashEffect << Pindian;
        frequency = Frequent;
    }

    virtual int getPriority() const{
        return -1;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        if(event == Pindian){
            PindianStar pindian = data.value<PindianStar>();
            if(pindian->reason == objectName() && pindian->isSuccess())
                pindian->from->obtainCard(pindian->to_card);
            return false;
        }
        if(player->getPhase() != Player::Play)
            return false;
        SlashEffectStruct effect = data.value<SlashEffectStruct>();
        if(effect.slash->getNumber() == 0)
            return false;
        if(effect.slash && !effect.to->isKongcheng() && effect.from->askForSkillInvoke(objectName(), data))
            effect.from->pindian(effect.to, objectName(), effect.slash);
        return false;
    }
};

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
    General *cuimeng = new General(this, "cuimeng", "guan");
    cuimeng->addSkill("paoxiao");

    General *gaochonghan = new General(this, "gaochonghan", "jiang", 7);
    gaochonghan->addSkill(new Pishan);
    gaochonghan->addSkill(new Shixie);

    General *zhangkui = new General(this, "zhangkui", "kou");
    zhangkui->addSkill(new Jielue);

    //addMetaObject<XianhaiCard>();
}

ADD_PACKAGE(Guben);
