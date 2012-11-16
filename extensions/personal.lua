module("extensions.personal", package.seeall)
extension = sgs.Package("personal")

tianqi = sgs.General(extension, "tianqi", "god", 5, false)
tianyin = sgs.General(extension, "tianyin", "god", 3)

eatdeath=sgs.CreateTriggerSkill{
	name="eatdeath",
	frequency = sgs.Skill_NotFrequent,
	events={sgs.Death},

	can_trigger = function(self, player)
		return true
	end,

	on_trigger = function(self,event,player,data)
		local room = player:getRoom()
		local tenkei = room:findPlayerBySkillName(self:objectName())
		if not tenkei then return false end

		local skillslist = tenkei:getTag("EatDeath"):toString()
		local eatdeath_skills = skillslist:split("+")
		if eatdeath_skills[1] == "" then table.remove(eatdeath_skills, 1) end

		if room:askForSkillInvoke(tenkei, self:objectName(), data) then
			if #eatdeath_skills > 0 and sgs.Sanguosha:getSkill(eatdeath_skills[1]) then
				local choice = room:askForChoice(tenkei, self:objectName(), table.concat(eatdeath_skills, "+"))
				room:detachSkillFromPlayer(tenkei, choice)
				for i = #eatdeath_skills, 1, -1 do
					if eatdeath_skills[i] == choice then
						table.remove(eatdeath_skills, i)
					end
				end
			end
			room:loseMaxHp(tenkei)
			local skills = player:getVisibleSkillList()
			for _, skill in sgs.qlist(skills) do
				if skill:getLocation() == sgs.Skill_Right then
					if skill:getFrequency() ~= sgs.Skill_Limited and
						skill:getFrequency() ~= sgs.Skill_Wake then
						local sk = skill:objectName()
						room:acquireSkill(tenkei, sk)
						table.insert(eatdeath_skills, sk)
					end
				end
			end
			tenkei:setTag("EatDeath", sgs.QVariant(table.concat(eatdeath_skills, "+")))
		end
		return false
	end
}

skydao=sgs.CreateTriggerSkill
{
	name="skydao",
	frequency = sgs.Skill_Compulsory,
	events={sgs.Damaged},

	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		if player:getPhase() == sgs.Player_NotActive then
			local log = sgs.LogMessage()
			log.type = "#SkydaoMAXHP"
			log.from = player
			log.arg = tonumber(player:getMaxHp())
			log.arg2 = self:objectName()
			room:setPlayerProperty(player, "maxhp", sgs.QVariant(player:getMaxHp() + 1))
			room:sendLog(log)
		end
	end
}

noqing=sgs.CreateTriggerSkill{
	name="noqing",
	frequency = sgs.Skill_Compulsory,
	events={sgs.Damaged},
	priority = -1,

	default_choice = function(player)
		if player:getMaxHp() >= player:getHp() + 2 then
			return "maxhp"
		else
			return "hp"
		end
		end,

	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		for _, tmp in sgs.qlist(room:getOtherPlayers(player)) do
			if tmp:getHp() < player:getHp() then
				return false
			end
		end
		for _, tmp in sgs.qlist(room:getAllPlayers()) do
			local choice = room:askForChoice(tmp, self:objectName(), "hp+max_hp")
			local log = sgs.LogMessage()
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
		return false
	end
}

tianqi:addSkill(eatdeath)
tianyin:addSkill(skydao)
tianyin:addSkill(noqing)

sgs.LoadTranslationTable{
	["personal"] = "Pesonal",

	["#tianyin"] = "天道之化身",
	["tianyin"] = "天音",
	["designer:tianyin"] = "鎏铄天音",
	["cv:tianyin"] = "",
	["illustrator:tianyin"] = "帕秋丽同人",
	["skydao"] = "天道",
	[":skydao"] = "锁定技，你的回合外，你每受到一次伤害，增加1点体力上限",
	["#SkydaoMAXHP"] = "%from 的锁定技【%arg2】被触发，增加了一点体力上限，目前体力上限是 %arg",
	["noqing"] = "无情",
	[":noqing"] = "锁定技，你受到伤害时，若你的体力是全场最少或同时为最少，则所有人必须减少1点体力或1点体力上限",
	["noqing:hp"] = "体力",
	["noqing:max_hp"] = "体力上限",
	["#NoqingLoseHp"] = "受到 %from 【%arg】锁定技的影响，%to 流失了一点体力",
	["#NoqingLoseMaxHp"] = "受到 %from 【%arg】锁定技的影响，%to 流失了一点体力上限",

	["#tianqi"] = "食死徒",
	["tianqi"] = "天启",
	["designer:tianqi"] = "宇文天启",
	["cv:tianqi"] = "",
	["illustrator:tianqi"] = "火影忍者",
	["eatdeath"] = "拾尸",
	[":eatdeath"] = "当有角色死亡时，你可以失去一个因“拾尸”获得的技能(如果有的话)，然后失去一点体力上限并获得该角色当前的所有武将技(限定技、觉醒技除外)",
}
