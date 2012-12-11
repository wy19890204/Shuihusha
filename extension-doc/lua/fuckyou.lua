module("extensions.fuckyou", package.seeall)
extension = sgs.Package("fuckyou")

ooki = sgs.General(extension, "ooki", "wei")

tuiche=sgs.CreateTriggerSkill
{
	name = "tuiche",
	frequency = sgs.Skill_Compulsory,
	events={sgs.Damage},

	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local damage = data:toDamage()
		if damage.card and damage.card:inherits("Slash") and not damage.to:getGeneral():isMale() and
			damage.to:faceUp() then
			damage.to:turnOver()
		end
	end
}

huanzicard=sgs.CreateSkillCard{
	name = "huanzi",
	target_fixed=false,
	filter = function(self, targets, to_select, player)
		return #targets==0 and not to_select:getGeneral():isMale() and not to_select:faceUp()
	end,
	on_effect=function(self, effect)
		effect.to:turnOver()
	end,
}

huanzi=sgs.CreateViewAsSkill{
	name = "huanzi",
	n = 0,

	view_as = function(self, cards)
		local card = huanzicard:clone()
		card:setSkillName(self:objectName())
		return card
	end,
}

baoju=sgs.CreateTriggerSkill
{
	name = "baoju",
	frequency = sgs.Skill_Compulsory,
	events={sgs.Predamage},

	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local damage = data:toDamage()
		if damage.card and damage.card:inherits("Slash") and damage.to:getGeneral():isMale() then
			damage.damage = damage.damage + 1
			damage.to:gainMark("@chrysa")
			data:setValue(damage)
		end
	end
}

fanbao=sgs.CreateTriggerSkill
{
	name = "fanbao",
	frequency = sgs.Skill_NotFrequent,
	events={sgs.Predamage},

	can_trigger = function(self, player)
		return true
	end,

	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local damage = data:toDamage()
		if damage.from:getMark("@chrysa") < 1 then return end
		if damage.to:hasSkill("baoju") and damage.card:inherits("Slash") and
			room:askForSkillInvoke(damage.from, self:objectName()) then
			damage.damage = damage.damage + 1
			data:setValue(damage)
		end
	end
}

ooki:addSkill(tuiche)
ooki:addSkill(huanzi)
ooki:addSkill(baoju)
ooki:addSkill(fanbao)

sgs.LoadTranslationTable{
	["fuckyou"] = "实干家",

	["#ooki"] = "猥琐男",
	["ooki"] = "东尼大木",
	["designer:ooki"] = "玮哥投胎了",
	["illustrator:ooki"] = "",
	["cv:ooki"] = "",
	["tuiche"] = "推车",
	[":tuiche"] = "锁定技，你使用的【杀】对女性角色造成伤害时，若目标角色武将牌正面朝上，将其武将牌翻面。",
	["huanzi"] = "换姿",
	[":huanzi"] = "出牌阶段，你可以将武将牌背面朝上的女性角色的武将牌翻回正面。",
	["baoju"] = "爆菊",
	[":baoju"] = "锁定技，你使用的【杀】对男性角色造成的伤害+1。",
	["@chrysa"] = "菊花",
	["fanbao"] = "反爆",
	[":fanbao"] = "被你爆过菊的男性角色获得1枚“菊花”标记，有菊花标记的角色可以对你使用“爆菊”。",
}
