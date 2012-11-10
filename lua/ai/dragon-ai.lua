-- AI for dragon package

-- taolue
local taolue_skill={}
taolue_skill.name = "taolue"
table.insert(sgs.ai_skills, taolue_skill)
taolue_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("TaolueCard") and not self.player:isKongcheng() then
		local max_card
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		for _, card in ipairs(cards) do
			if card:getSuit() == sgs.Card_Spade then
				max_card = card
				break
			end
		end
		if not max_card then max_card = self:getMaxCard() end
		return sgs.Card_Parse("@TaolueCard=" .. max_card:getEffectiveId())
	end
end
sgs.ai_skill_use_func["TaolueCard"]=function(card,use,self)
	self:sort(self.enemies, "handcard")
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isKongcheng() and not enemy:getEquips():isEmpty() then
			if use.to then use.to:append(enemy) end
			use.card=card
			return
		end
	end
end
sgs.ai_skill_playerchosen["taolue"] = sgs.ai_skill_playerchosen["lihun"]

-- shexin
local shexin_skill={}
shexin_skill.name = "shexin"
table.insert(sgs.ai_skills, shexin_skill)
shexin_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("ShexinCard") and not self.player:isNude() then
		self:sort(self.enemies, "handcard2")
		if self.enemies[1]:getHandcardNum() <= 3 then return end
		local cards = self.player:getCards("he")
		cards = sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		for _, card in ipairs(cards) do
			if card:isNDTrick() or card:inherits("EquipCard") then
				return sgs.Card_Parse("@ShexinCard=" .. card:getEffectiveId())
			end
		end
	end
end
sgs.ai_skill_use_func["ShexinCard"] = function(card,use,self)
	self:sort(self.enemies, "handcard2")
	if use.to then
		use.to:append(self.enemies[1])
	end
	use.card=card
end

-- wugou
local wugou_skill={}
wugou_skill.name = "wugou"
table.insert(sgs.ai_skills,wugou_skill)
wugou_skill.getTurnUseCard = function(self)
	local first_found, second_found = false, false
	local first_card, second_card
	if self.player:getHandcardNum() >= 2 then
		local cards = self.player:getHandcards()
		local same_suit=false
		cards = sgs.QList2Table(cards)
		for _, fcard in ipairs(cards) do
			if not fcard:inherits("Peach") and fcard:inherits("BasicCard") then
				first_card = fcard
				first_found = true
				for _, scard in ipairs(cards) do
					if first_card ~= scard and scard:inherits("BasicCard") and
						(scard:isRed() and first_card:isRed()) and 
						not scard:inherits("Peach") then
						second_card = scard
						second_found = true
						break
					end
				end
				if second_card then break end
			end
		end
	end
	if first_found and second_found then
		local wugou_card = {}
		local first_suit, first_number, first_id = first_card:getSuitString(), first_card:getNumberString(), first_card:getId()
		local second_suit, second_number, second_id = second_card:getSuitString(), second_card:getNumberString(), second_card:getId()
		local card_str = ("assassinate:wugou[%s:%s]=%d+%d"):format(first_suit, first_number, first_id, second_id)
		local assassinate = sgs.Card_Parse(card_str)
		assert(assassinate)
		return assassinate
	end
end

-- qiaojiang
local qiaojiang_skill={}
qiaojiang_skill.name = "qiaojiang"
table.insert(sgs.ai_skills, qiaojiang_skill)
qiaojiang_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local jink_card
	self:sortByUseValue(cards, true)
	for _,card in ipairs(cards)  do
		if card:isBlack() and card:inherits("TrickCard") then
			jink_card = card
			break
		end
	end
	if not jink_card then return nil end
	local suit = jink_card:getSuitString()
	local number = jink_card:getNumberString()
	local card_id = jink_card:getEffectiveId()
	local card_str = ("slash:qiaojiang[%s:%s]=%d"):format(suit, number, card_id)
	local slash = sgs.Card_Parse(card_str)
	assert(slash)
	return slash
end
sgs.ai_view_as["qiaojiang"] = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()

	if card:isBlack() and card:inherits("TrickCard") then
		return ("slash:qiaojiang[%s:%s]=%d"):format(suit, number, card_id)
	elseif card:isRed() and card:inherits("TrickCard") then
		return ("jink:qiaojiang[%s:%s]=%d"):format(suit, number, card_id)
	end
end

-- qianxian
local qianxian_skill={}
qianxian_skill.name = "qianxian"
table.insert(sgs.ai_skills, qianxian_skill)
qianxian_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("QianxianCard") then return end
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	for _, acard in ipairs(cards) do
		if acard:isNDTrick() and acard:isBlack() then
			return sgs.Card_Parse("@QianxianCard=" .. acard:getEffectiveId())
		end
	end
end
sgs.ai_skill_use_func["QianxianCard"] = function(card,use,self)
	self:sort(self.enemies, "handcard")
	local first, second
	for _, tmp in ipairs(self.enemies) do
		if not tmp:isChained() or tmp:faceUp() then
			if not first then
				first = tmp
			elseif tmp:getMaxHP() ~= first:getMaxHP() then
				second = tmp
			end
			if first and second then break end
		end
	end
	if not first then
		for _, tmp in ipairs(self.friends_noself) do
			if tmp:getHandcardNum() > 2 and (not tmp:faceUp() or tmp:isChained()) then
				first = tmp
			elseif tmp:getMaxHP() ~= first:getMaxHP() then
				second = tmp
			end
			if first and second then break end
		end
	elseif not second then
		for _, tmp in ipairs(self.friends_noself) do
			if tmp:getHandcardNum() > 2 and (not tmp:faceUp() or tmp:isChained()) then
				second = tmp
			end
			if first and second then break end
		end
	end
	if first and second and use.to then
		use.card = card
		use.to:append(first)
		use.to:append(second)
	end
end

-- meicha
meicha_skill={}
meicha_skill.name = "meicha"
table.insert(sgs.ai_skills, meicha_skill)
meicha_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards,true)
	for _,acard in ipairs(cards)  do
		if (acard:getSuit() == sgs.Card_Club) then
			card = acard
			break
		end
	end
	if not card then return nil end
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("analeptic:meicha[club:%s]=%d"):format(number, card_id)
	local analeptic = sgs.Card_Parse(card_str)
	assert(analeptic)
	return analeptic
end
sgs.ai_view_as["meicha"] = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()

	if card_place ~= sgs.Player_Equip and card:getSuit() == sgs.Card_Club then
		return ("analeptic:meicha[%s:%s]=%d"):format(suit, number, card_id)
	end
end

-- zaochuan
local zaochuan_skill = {}
zaochuan_skill.name = "zaochuan"
table.insert(sgs.ai_skills, zaochuan_skill)
zaochuan_skill.getTurnUseCard = function(self)
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
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("iron_chain:zaochuan[club:%s]=%d"):format(number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end

-- nusha
nusha_skill={}
nusha_skill.name = "nusha"
table.insert(sgs.ai_skills, nusha_skill)
nusha_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("NushaCard") then return end
	local enum = 0
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHandcardNum() > enum then
			enum = enemy:getHandcardNum()
		end
	end
	local fnum = 0
	for _, friend in ipairs(self.friends_noself) do
		if friend:getHandcardNum() > fnum then
			fnum = friend:getHandcardNum()
		end
	end
	if enum >= fnum then
		local slash = self:getCardId("Slash")
		if slash then
			return sgs.Card_Parse("@NushaCard=" .. slash)
		end
	end
	return
end
sgs.ai_skill_use_func["NushaCard"] = function(card, use, self)
	local enum = self.enemies[1]
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHandcardNum() > enum:getHandcardNum() then
			enum = enemy
		end
	end
	use.card = card
	if use.to then
		use.to:append(enum)
	end
end

-- kongying
sgs.ai_skill_invoke["kongying"] = true
sgs.ai_skill_playerchosen["kongying"] = function(self, targets)
	self:sort(self.enemies, "hp")
	return self.enemies[1]
end
function sgs.ai_cardneed.kongying(to, card, self)
	return card:inherits("Jink")
end
