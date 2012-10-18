module("extensions.sanguosha", package.seeall)
extension = sgs.Package("sanguosha")

xiahoudun = sgs.General(extension, "xiahoudun", "god", 10)

fenyong=sgs.CreateTriggerSkill{
	name="fenyong",
	frequency = sgs.Skill_Compulsory,
	events={sgs.GameStart},
	priority = -3,

	on_trigger = function(self,event,player,data)
		local room = player:getRoom()
		local log = sgs.LogMessage()
		log.type = "#TriggerSkill"
		log.from = player
		log.arg = self:objectName()
		room:sendLog(log)

		local damage=sgs.DamageStruct()
		damage.from = player
		damage.to = player
		room:damage(damage)
		return false
	end
}

xuehen=sgs.CreateTriggerSkill{
	name="xuehen",
	frequency = sgs.Skill_Compulsory,
	events={sgs.Damaged},

	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local damage = data:toDamage()
		if damage.from then
			local log = sgs.LogMessage()
			log.type = "#Xuehen"
			log.from = player
			log.to:append(damage.from)
			log.arg = self:objectName()
			room:sendLog(log)
			if not room:askForDiscard(damage.from, "xuehen", 5, 5, true) then
				local dama=sgs.DamageStruct()
				dama.from = player
				dama.to = damage.from
				room:damage(dama)
				return true
			end
		end
		return false
	end
}

xiahoudun:addSkill(fenyong)
xiahoudun:addSkill(xuehen)

sgs.LoadTranslationTable{
	["sanguosha"] = "伞国沙",

	["$xiahoudun"] = "SP04",
	["#xiahoudun"] = "我勒个去",
	["xiahoudun"] = "夏侯惇",
	["designer:xiahoudun"] = "舟亢",
	["cv:xiahoudun"] = "",
	["illustrator:xiahoudun"] = "",
	["fenyong"] = "奋勇",
	[":fenyong"] = "锁定技，游戏开始时，你须对自己造成1点伤害。",
	["xuehen"] = "雪恨",
	[":xuehen"] = "锁定技，每当你受到一次伤害，伤害来源须弃置五张手牌，否则受到你对其造成的1点伤害。",
	["#Xuehen"] = "%from 的锁定技【%arg】被触发，%to 须弃置5张手牌",
}
