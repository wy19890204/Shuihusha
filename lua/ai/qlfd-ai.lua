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

-- banzhuang
local banzhuang_skill={}
banzhuang_skill.name = "banzhuang"
table.insert(sgs.ai_skills, banzhuang_skill)
banzhuang_skill.getTurnUseCard = function(self,inclusive)
    local cards = self.player:getHandcards()
    cards = sgs.QList2Table(cards)
	for _,card in ipairs(cards)  do
		if card:getSuit() == sgs.Card_Heart or inclusive then
			local number = card:getNumberString()
			local card_id = card:getEffectiveId()
			local card_str = ("ex_nihilo:banzhuang[heart:%s]=%d"):format(number, card_id)
			local exnihilo = sgs.Card_Parse(card_str)
			assert(exnihilo)
			return exnihilo
		end
	end
end

-- zhuying
local zhuying_skill={}
zhuying_skill.name = "zhuying"
table.insert(sgs.ai_skills, zhuying_skill)
zhuying_skill.getTurnUseCard = function(self)
	if not self.player:isWounded() then return end
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local anal_card
	self:sortByUseValue(cards, true)
	for _,card in ipairs(cards)  do
		if card:inherits("Analeptic") then
			anal_card = card
			break
		end
	end
	if anal_card then
		local suit = anal_card:getSuitString()
		local number = anal_card:getNumberString()
		local card_id = anal_card:getEffectiveId()
		local card_str = ("peach:zhuying[%s:%s]=%d"):format(suit, number, card_id)
		local peach = sgs.Card_Parse(card_str)
		return peach
	end
end

-- zhangshi
local zhangshi_skill={}
zhangshi_skill.name="zhangshi"
table.insert(sgs.ai_skills,zhangshi_skill)
zhangshi_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("ZhangshiCard") or not self:slashIsAvailable() then return end
	local card_str = "@ZhangshiCard=."
	local slash = sgs.Card_Parse(card_str)
	assert(slash)
	return slash
end
sgs.ai_skill_use_func["ZhangshiCard"]=function(card,use,self)
	self:sort(self.enemies, "defense")
	local target_count=0
	for _, enemy in ipairs(self.enemies) do
		if ((self.player:canSlash(enemy, not no_distance)) or
			(use.isDummy and (self.player:distanceTo(enemy)<=self.predictedRange))) and
			self:objectiveLevel(enemy)>3 and
			self:slashIsEffective(card, enemy) then
			use.card=card
			if use.to then
				use.to:append(enemy)
			end
			target_count=target_count+1
			if self.slash_targets<=target_count then return end
		end
	end
end
sgs.ai_skill_invoke["zhangshi"] = function(self, data)
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:inherits("Slash") then
			return false
		end
	end
	if sgs.zhangshisource then return false else return true end
end

-- huoshui
local huoshui_skill={}
huoshui_skill.name = "huoshui"
table.insert(sgs.ai_skills, huoshui_skill)
huoshui_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local driver_card
	self:sortByUseValue(cards, true)
	for _,card in ipairs(cards)  do
		if card:inherits("Weapon") or card:inherits("Slash") then
			driver_card = card
			break
		end
	end
	if driver_card then
		local suit = driver_card:getSuitString()
		local number = driver_card:getNumberString()
		local card_id = driver_card:getEffectiveId()
		local card_str = ("drivolt:huoshui[%s:%s]=%d"):format(suit, number, card_id)
		local driver = sgs.Card_Parse(card_str)
		return driver
	end
end
