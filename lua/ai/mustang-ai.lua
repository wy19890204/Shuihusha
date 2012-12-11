-- AI for mustang package

-- yueli
function sgs.ai_slash_prohibit.yueli(self, to)
	if self:isEquip("EightDiagram", to) then return true end
end

-- taohui
sgs.ai_skill_playerchosen["taohui"] = function(self, targets)
	self:sort(self.friends, "handcard")
	return self.friends[1]
end

-- manli
sgs.ai_skill_invoke["manli"] = sgs.ai_skill_invoke["liba"]

-- qiaogong

-- tianyan
sgs.ai_skill_invoke["tianyan"] = true
sgs.ai_skill_askforag["tianyan"] = function(self, card_ids)
	local player = self.player:getTag("TianyanTarget"):toPlayer()
	local cards = {}
	for _, card_id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(card_id)
		table.insert(cards, card)
	end
	self:sortByUseValue(cards, true)
	if self:isFriend(player) then
		for _, card in ipairs(cards) do
			if self:getUseValue(card) < 2.7 then
				return card:getEffectiveId()
			end
		end
	else
		self:sortByUseValue(cards)
		for _, card in ipairs(cards) do
			if self:getUseValue(card) > 5.4 then
				return card:getEffectiveId()
			end
		end
	end
	return -1
end

-- hunjiu-jiu
hunjiu_skill={}
hunjiu_skill.name = "hunjiu"
table.insert(sgs.ai_skills, hunjiu_skill)
hunjiu_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards,true)
	for _,acard in ipairs(cards)  do
		if (acard:inherits("Ecstasy") or
			(not self.player:isWounded() and acard:inherits("Peach"))) then
			card = acard
			break
		end
	end
	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("analeptic:hunjiu[%s:%s]=%d"):format(suit, number, card_id)
	local caard = sgs.Card_Parse(card_str)
	assert(caard)
	return caard
end
-- hunjiu-mi
hunjiu2_skill={}
hunjiu2_skill.name = "hunjiu"
table.insert(sgs.ai_skills, hunjiu2_skill)
hunjiu2_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards,true)
	for _,acard in ipairs(cards)  do
		if (acard:inherits("Analeptic") or
			(not self.player:isWounded() and acard:inherits("Peach"))) then
			card = acard
			break
		end
	end
	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("ecstasy:hunjiu[%s:%s]=%d"):format(suit, number, card_id)
	local caard = sgs.Card_Parse(card_str)
	assert(caard)
	return caard
end
sgs.ai_view_as["hunjiu"] = function(card, player, card_place, class_name)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()

	if card:inherits("Peach") or card:inherits("Analeptic") or card:inherits("Ecstasy") then
		if class_name == "Analeptic" then
			return ("analeptic:hunjiu[%s:%s]=%d"):format(suit, number, card_id)
		else
			return ("ecstasy:hunjiu[%s:%s]=%d"):format(suit, number, card_id)
		end
	end
end

-- guitai
sgs.ai_skill_cardask["@guitai"] = function(self, data)
	local ard
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		if card:getSuit() == sgs.Card_Heart then
			ard = card
			break
		end
	end
	if not ard then return "." end
	local effect = data:toCardEffect()
	if self:isEnemy(effect.to) or
		(self:isFriend(effect.to) and self:isWeak() and not self:isWeak(effect.to)) then
		return ard:getEffectiveId()
	else
		return "."
	end
end

-- tuzai&longjiao
sgs.ai_skill_invoke["tuzai"] = true
sgs.ai_skill_invoke["longjiao"] = true

