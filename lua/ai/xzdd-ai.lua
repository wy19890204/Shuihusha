-- duijue
sgs.ai_skill_use["@@duijue"] = function(self, prompt)
	self:sort(self.enemies, "hp")
	local n1 = self:getCardsNum("Slash")
	local final
	for _, enemy in ipairs(self.enemies) do
		if n1 + 1 > self:getCardsNum("Slash", enemy) then
			final = enemy
			break
		end
	end
	if final then
		return "@DuijueCard=.->"..final:objectName()
	else
		return "."
	end
end

-- maidao
maidao_skill={}
maidao_skill.name = "maidao"
table.insert(sgs.ai_skills, maidao_skill)
maidao_skill.getTurnUseCard = function(self)
	if self.player:getWeapon() then
		local cards = self.player:getCards("h")
		cards = sgs.QList2Table(cards)
		for _, acard in ipairs(cards)  do
			if acard:inherits("Weapon") then
				return sgs.Card_Parse("@MaidaoCard=" .. acard:getId())
			end
		end
	end
	return
end
sgs.ai_skill_use_func["MaidaoCard"] = function(card, use, self)
	use.card = card
end

-- fengmang
sgs.ai_skill_invoke["fengmang"] = true
sgs.ai_skill_playerchosen["fengmang"] = function(self, targets)
	self:sort(self.enemies, "hp")
	return self.enemies[1]
end

-- shunshui&lihun
sgs.ai_skill_invoke["shunshui"] = true
sgs.ai_skill_invoke["lihun"] = function(self, data)
	local from = data:toPlayer()
	return self:isEnemy(from)
end
sgs.ai_skill_playerchosen["lihun"] = sgs.ai_skill_playerchosen["taolue"]

-- shenhuo
sgs.ai_skill_invoke["shenhuo"] = true
local shenhuo_skill={}
shenhuo_skill.name = "shenhuo"
table.insert(sgs.ai_skills, shenhuo_skill)
shenhuo_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards,true)
	for _,acard in ipairs(cards)  do
		if acard:isRed() and acard:inherits("TrickCard") then
			card = acard
			break
		end
	end
	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("fire_attack:shenhuo[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end

-- shenpan
sgs.ai_skill_invoke["shenpan"] = function(self, data)
	local judge = data:toJudge()
	return self:needRetrial(judge)
end

-- yixian
sgs.ai_skill_invoke["yixian"] = sgs.ai_skill_invoke["qiangqu"]

-- feiqiang
feiqiang_skill={}
feiqiang_skill.name = "feiqiang"
table.insert(sgs.ai_skills, feiqiang_skill)
feiqiang_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("FeiqiangCard") then
		local cards = self.player:getCards("he")
		cards = sgs.QList2Table(cards)
		for _, acard in ipairs(cards)  do
			if acard:inherits("Weapon") then
				return sgs.Card_Parse("@FeiqiangCard=" .. acard:getId())
			end
		end
	end
	return
end
sgs.ai_skill_use_func["FeiqiangCard"] = function(card, use, self)
	self:sort(self.enemies, "defense")
	use.card = card
	if use.to then
		use.to:append(self.enemies[1])
	end
end
sgs.ai_skill_choice["feiqiang"] = function(self, choices)
	return "gong"
end

-- shentou
shentou_skill={}
shentou_skill.name = "shentou"
table.insert(sgs.ai_skills, shentou_skill)
shentou_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards,true)
	for _,acard in ipairs(cards)  do
		if acard:getSuit() == sgs.Card_Club then
			card = acard
			break
		end
	end
	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("snatch:shentou[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end

--[[
-- maida0
maida0_skill={}
maida0_skill.name = "maida0"
table.insert(sgs.ai_skills, maida0_skill)
maida0_skill.getTurnUseCard = function(self)
	local yangzhi = self.room:findPlayerBySkillName("maidao")
	if yangzhi and not yangzhi:getPile("knife"):isEmpty() and self:isEnemy(yangzhi) then
		local cards = self.player:getCards("h")
		cards = sgs.QList2Table(cards)
		if #cards < 4 then return end
		self:sortByUseValue(cards, true)
		local card_ids = {}
		for i = 1, 2 do
			if self:getUseValue(cards[i]) > 4 then return end
			table.insert(card_ids, cards[i]:getEffectiveId())
		end
		return "@Maida0Card=" .. table.concat(card_ids, "+")
	end
	return
end
sgs.ai_skill_use_func["Maida0Card"] = function(card, use, self)
	local yangzhi = self.room:findPlayerBySkillName("maidao")
	use.card = card
	if yangzhi and not yangzhi:getPile("knife"):isEmpty() and use.to then
		use.to:append(yangzhi)
	end
end
--]]
