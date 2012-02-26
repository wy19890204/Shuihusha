#include "standard.h"
#include "skill.h"
#include "peasa.h"
#include "client.h"
#include "carditem.h"
#include "engine.h"
#include "ai.h"

class Piaoyong: public PhaseChangeSkill{
public:
    Piaoyong():PhaseChangeSkill("piaoyong"){
    }

    virtual bool onPhaseChange(ServerPlayer *player) const{
        if(player->getPhase() != Player::Start || !player->askForSkillInvoke(objectName()))
            return false;
        Room *room = player->getRoom();
        player->skip(Player::Judge);
        if(room->askForChoice(player, objectName(), "first+second") == "first")
            player->skip(Player::Draw);
        else
            player->skip(Player::Play);
        foreach(const Card *card, player->getJudgingArea())
            room->throwCard(card);
        ServerPlayer *target = room->askForPlayerChosen(player, room->getAllPlayers(), objectName());
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName(objectName());
        CardUseStruct use;
        use.card = slash;
        use.from = player;
        use.to << target;
        room->useCard(use, false);
        return false;
    }
};

PeasaPackage::PeasaPackage()
    :Package("peasa")
{
    General *guanzhang = new General(this, "guanzhang", "jiang");
    guanzhang->addSkill(new Piaoyong);
}

ADD_PACKAGE(Peasa);
