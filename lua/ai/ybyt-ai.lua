-- yuanpei
local yuanpei_skill={}
yuanpei_skill.name = "yuanpei"
table.insert(sgs.ai_skills, yuanpei_skill)
yuanpei_skill.getTurnUseCard = function(self)
    if self.player:hasUsed("YuanpeiCard") then
		if not self.player:hasFlag("yuanpei") or self.player:isKongcheng() then return end
		local cards = self.player:getCards("h")
		cards=sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		local suit = cards[1]:getSuitString()
		local number = cards[1]:getNumberString()
		local card_id = cards[1]:getEffectiveId()
		local card_str = ("slash:yuanpei[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)
		assert(slash)
		return slash
	end
	return sgs.Card_Parse("@YuanpeiCard=.")
end
sgs.ai_skill_use_func["YuanpeiCard"] = function(card,use,self)
	self:sort(self.friends, "defense")
	for _, enemy in ipairs(self.friends) do
		if enemy:getGeneral():isMale() then
            use.card = card
		    if use.to then use.to:append(enemy) end
            return
		end
	end
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

-- goulian
sgs.ai_skill_invoke["goulian"] = sgs.ai_skill_invoke["liba"]

-- jinjia
function sgs.ai_armor_value.jinjia(card)
	if not card then return 4 end
end

-- sinue
sgs.ai_skill_use["@@sinue"] = function(self, prompt)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	for _, enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy) == 1 then
			return "@SinueCard=" .. cards[1]:getEffectiveId()
		end
	end
	return "."
end

-- shexin
local shexin_skill={}
shexin_skill.name = "shexin"
table.insert(sgs.ai_skills, shexin_skill)
shexin_skill.getTurnUseCard = function(self)
    if not self.player:hasUsed("ShexinCard") and not self.player:isNude() then
		self:sort(self.enemies, "handcard")
		if self.enemies[#self.enemies]:getHandcardNum() <= 3 then return end
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
	self:sort(self.enemies, "handcard")
	if use.to then
		use.to:append(self.enemies[#self.enemies])
	end
    use.card=card
end

-- jiayao
sgs.ai_skill_invoke["jiayao"] = true

-- sheyan
local sheyan_skill = {}
sheyan_skill.name = "sheyan"
table.insert(sgs.ai_skills, sheyan_skill)
sheyan_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("SheyanCard") then return end
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards, true)
	for _, acard in ipairs(cards) do
		if acard:getSuit() == sgs.Card_Heart then
			card = acard
			break
		end
	end
	if card then
		return sgs.Card_Parse("@SheyanCard=" .. card:getEffectiveId())
	end
end
sgs.ai_skill_use_func["SheyanCard"] = function(card,use,self)
    use.card=card
end

-- guibing
local guibing_skill = {}
guibing_skill.name = "guibing"
table.insert(sgs.ai_skills, guibing_skill)
guibing_skill.getTurnUseCard = function(self)
	if self.player:hasFlag("Guibing") or not self:slashIsAvailable() then return end
	local card_str = "@GuibingCard=."
	local slash = sgs.Card_Parse(card_str)
	assert(slash)
	return slash
end
sgs.ai_skill_use_func["GuibingCard"] = sgs.ai_skill_use_func["ZhangshiCard"]
function sgs.ai_cardneed.guibing(to, card, self)
	return card:isBlack()
end
