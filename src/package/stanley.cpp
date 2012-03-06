#include "standard.h"
#include "skill.h"
#include "stanley.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"

class Youxia: public TriggerSkill{
public:
    Youxia():TriggerSkill("youxia"){
        events << CardLost;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent , ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        ServerPlayer *jinge = room->findPlayerBySkillName(objectName());
        if(player->isKongcheng() && jinge && !jinge->isKongcheng() && jinge->isWounded()){
            CardMoveStar move = data.value<CardMoveStar>();
            if(player->isAlive() && move->from_place == Player::Hand && jinge->askForSkillInvoke(objectName(), data)){
                const Card *card = room->askForCardShow(jinge, player, "youxia");
                player->obtainCard(card, false);
                RecoverStruct o;
                o.card = card;
                room->recover(jinge, o);
            }
        }
        return false;
    }
};

StanleyPackage::StanleyPackage()
    :Package("stanley")
{
    General *ximenjinge = new General(this, "ximenjinge", "jiang");
    ximenjinge->addSkill(new Youxia);

    //addMetaObject<XianhaiCard>();
}

ADD_PACKAGE(Stanley);
