-- AI for tiger package

-- leiheng
-- guzong

-- sunli
-- neiying

-- wuyanguang
-- jintang

-- shixiu
-- pinming

-- lvfang
-- lieji

-- tianhu
-- wuzhou
-- huwei

-- zhangheng
-- jielue
-- fuhun

-- xiebao
-- liehuo
sgs.ai_skill_invoke["liehuo"] = sgs.ai_skill_invoke["lihun"]

-- shien
-- longluo
sgs.ai_skill_playerchosen["longluo"] = function(self, targets)
	local card = self.player:getTag("LongluoCard"):toCard()
	if(self:getCardsNum("Jink") < 1 and card:inherits("Jink")) then
		return self.player
	elseif(self:isWeak() and (card:inherits("Peach") or card:inherits("Analeptic"))) then
		return self.player
	elseif(card:inherits("Shit")) then
		self:sort(self.enemies, "hp")
		return self.enemies[1]
	end
	self:sort(self.friends, "handcard")
	return self.friends[1]
end

-- xiaozai
sgs.ai_skill_use["@@xiaozai"] = function(self, prompt)
	if self.player:getHandcardNum() >= 4 then
		local players = {}
		for _, t in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if not t:hasFlag("Xiaozai") then
				table.insert(players, t)
			end
		end
		if #players == 0 then return "." end
		local r = math.random(1, #players)
		local target = players[r]
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		local card_ids = {}
		for i = 1, 2 do
			if self:getUseValue(cards[i]) > 5 then return "." end
			table.insert(card_ids, cards[i]:getEffectiveId())
		end
		return "@XiaozaiCard=" .. table.concat(card_ids, "+") .. "->" .. target:objectName()
	else
		return "."
	end
end

-- yanshun
-- huxiao
local huxiao_skill={}
huxiao_skill.name = "huxiao"
table.insert(sgs.ai_skills, huxiao_skill)
huxiao_skill.getTurnUseCard = function(self)
	if not self.player:isNude() then
		local cards = self.player:getCards("he")
		cards = sgs.QList2Table(cards)
		for _, fcard in ipairs(cards) do
			if fcard:inherits("EquipCard") then
				local suit, number, id = fcard:getSuitString(), fcard:getNumberString(), fcard:getId()
				local card_str = ("savage_assault:huxiao[%s:%s]=%d"):format(suit, number, id)
				local savage = sgs.Card_Parse(card_str)
				assert(savage)
				return savage
			end
		end
	end
end

-- wangying
-- tanse
sgs.ai_skill_invoke["tanse"] = function(self, data)
	local effect = data:toCardEffect()
	if self:isFriend(effect.from) then
		if self:getCard("EquipCard") then
			return true
		end
	else
		if not effect.from:getEquips():isEmpty() then
			return true
		end
	end
	return false
end
sgs.ai_skill_cardask["@tanse"] = function(self, data)
	local effect = data:toCardEffect()
	if self:isFriend(effect.from) then
		return self:getCardId("EquipCard")
	end
	return "."
end

-- houfa
sgs.ai_skill_invoke["houfa"] = true
houfa_skill = {}
houfa_skill.name = "houfa"
table.insert(sgs.ai_skills, houfa_skill)
houfa_skill.getTurnUseCard = function(self, inclusive)
	local cards = self.player:getCards("h")
	local slashs = {}
	cards=sgs.QList2Table(cards)
	for _, card in ipairs(cards)  do
		if card:inherits("Slash") then
			table.insert(slashs, card:getId())
		end
	end
	if #slashs < 2 then return end

	local suit1 = slashs[1]:getSuitString()
	local card_id1 = slashs[1]:getEffectiveId()
	local suit2 = slashs[2]:getSuitString()
	local card_id2 = slashs[2]:getEffectiveId()

	local suit = "no_suit"
	if slashs[1]:isBlack() == slashs[2]:isBlack() then suit = suit1 end

	local card_str = ("slash:houfa[%s:%s]=%d+%d"):format(suit, 0, card_id1, card_id2)
	local slash = sgs.Card_Parse(card_str)
	return slash
end

-- lizhong
-- linse
function sgs.ai_trick_prohibit.linse(card)
	return card:inherits("Dismantlement") or card:inherits("Snatch")
end
