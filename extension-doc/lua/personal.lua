module("extensions.personal", package.seeall)
extension = sgs.Package("personal")

tianqi = sgs.General(extension, "tianqi", "god", 5, false)
tianyin = sgs.General(extension, "tianyin", "god", 3)

eatdeath=sgs.CreateTriggerSkill{
	name="eatdeath",
	frequency = sgs.Skill_NotFrequent,
	events={sgs.Death},

	on_trigger=function(self,event,player,data)
        local room = player:getRoom()
		local tenkei = room:findPlayerBySkillName(self:objectName())
		if not tenkei then return false end

        local eatdeath_skills = tenkei:getTag("EatDeath"):toList()
		if(room:askForSkillInvoke(tenkei, self:objectName(), data)) then
			local eatdeaths = {}

			for _, tmp in sgs.qlist(eatdeath_skills) do
				table.insert(eatdeaths, tmp:toString())
			    if not eatdeaths.isEmpty() then
					local choice = room:askForChoice(tenkei, self:objectName(), eatdeaths.join("+"))
					room:detachSkillFromPlayer(tenkei, choice)
					for i = #eatdeath_skills, 1, -1 do
						if eatdeath_skills[i]:objectName() == choice then
							table.remove(eatdeath_skills, i)
						end
					end
				end
				room:loseMaxHp(tenkei)
				local skills = player:getVisibleSkillList()
				for _, skill in sgs.qlist(skills) do
					if not (skill:isLordSkill() or player:isLord()) and skill:parent() then
						local sk = skill:objectName()
						room:acquireSkill(tenkei, sk)
						table.insert(eatdeaths, sk)
					end
				end
				local datatmp=sgs.QVariant(0)
				datatmp:setValue(eatdeath_skills)
				tenkei:setTag("EatDeath",datatmp)
			end
		end
        return false
	end,
}

skydao=sgs.CreateTriggerSkill
{
	name="skydao",
	frequency =sgs.Skill_Compulsory,
	events={sgs.Damaged},

	on_trigger=function(self,event,player,data)
		local damage = data:toDamage()
        local room = player:getRoom()
		if damage.to and damage.to == player and player:getPhase() == sgs.Player_NotActive then
            local log=sgs.LogMessage()
			log.type = "#SkydaoMAXHP"
			log.from = player
			log.arg = player:getMaxHP():toString()
			log.arg2 = self:objectName()
			room:setPlayerProperty(player, "maxhp", player:getMaxHP() + 1)
			room:sendLog(log)
        end
        return false
	end,
}

noqing=sgs.CreateTriggerSkill{
	name="noqing",
	frequency = sgs.Skill_Compulsory,
	events={sgs.Damaged},
	priority = -1,

	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local damage = data:toDamage()
		if damage.to and damage.to == player then
			for _, tmp in sgs.qlist(room:getOtherPlayers(player)) do
				if tmp:getHp() < player:getHp() then
					return false
				end
			end
			for _, tmp in sgs.qlist(room:getAllPlayers()) do
				local choice = room:askForChoice(tmp, objectName(), "hp+max_hp")
				local log=sgs.LogMessage()
				log.from = player
				log.arg = self:objectName()
				log.to:append(tmp)
				if(choice == "hp") then
					log.type = "#NoqingLoseHp"
					room:sendLog(log)
					room:loseHp(tmp)
				else
					log.type = "#NoqingLoseMaxHp"
					room:sendLog(log)
					room:loseMaxHp(tmp)
				end
			end
		end
	end,
}

tianqi:addSkill(eatdeath)
tianyin:addSkill(skydao)
tianyin:addSkill(noqing)

sgs.LoadTranslationTable{
	["personal"] = "Pesonal",

}