-- yushui
yushui_skill={}
yushui_skill.name = "yushui"
table.insert(sgs.ai_skills, yushui_skill)
yushui_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("YushuiCard") or not self.player:isWounded() then return end
	local cards = self.player:getCards("he")
    cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards)  do
		if card:getSuit() == sgs.Card_Heart then
		    return sgs.Card_Parse("@YushuiCard=" .. card:getEffectiveId())
		end
	end
	return
end
sgs.ai_skill_use_func["YushuiCard"] = function(card, use, self)
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() and friend:getGeneral():isMale() then
			use.card = card
			if use.to then use.to:append(friend) end
			return
		end
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

-- fanwu
sgs.ai_skill_use["@@fanwu"] = function(self, prompt)
	if self.player:isKongcheng() then return "." end
	local card = self.player:getRandomHandCard()
	local damage = self.player:getTag("FanwuStruct"):toDamage()
	local dmg = damage.damage
	if damage.to:getArmor() and damage.to:getArmor():objectName() == "vine" and damage.nature == sgs.DamageStruct_Fire then dmg = dmg + 1 end
	if damage.to:getArmor() and damage.to:getArmor():objectName() == "silver_lion" then dmg = 1 end
	if damage.to:getRole() == "loyal" and self:isEnemy(damage.to) and damage.to:getHp() - dmg < 1
		and self.room:getLord() and self.room:getLord():getGeneral():isMale() then
		return "@FanwuCard=" .. card:getEffectiveId() .. "->" .. self.room:getLord():objectName()
	end
	if damage.to:getRole() == "rebel" and self:isEnemy(damage.from) and damage.to:getHp() - dmg < 1 then
		self:sort(self.friends, "handcard")
		for _, t in ipairs(self.friends) do
			if t:getGeneral():isMale() then
				return "@FanwuCard=" .. card:getEffectiveId() .. "->" .. t:objectName()
			end
		end
	end
	return "."
end

-- panxin
sgs.ai_skill_invoke["panxin"] = sgs.ai_skill_invoke["dujian"]

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

-- shouwang
shouwang_skill={}
shouwang_skill.name = "shouwang"
table.insert(sgs.ai_skills, shouwang_skill)
shouwang_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("ShouwangCard") then return end
	local slash = self:getCard("Slash")
	if not slash then return end
	return sgs.Card_Parse("@ShouwangCard=" .. slash:getEffectiveId())
end
sgs.ai_skill_use_func["ShouwangCard"] = function(card, use, self)
	self:sort(self.friends,"threat")
	for _, friend in ipairs(self.friends) do
		if friend:getGeneral():isMale() then
			use.card = card
			if use.to then use.to:append(friend) end
			return
		end
	end
end

-- zhongzhen
sgs.ai_skill_invoke["zhongzhen"] = function(self, data)
	local damage = data:toDamage()
	local max_card = self:getMaxCard()
	if max_card and max_card:getNumber() > 11 and self:isEnemy(damage.from) then
		return true
	else
		return false
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
	local slash = self:getCard("Slash")
	if slash then return false end
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

-- huakui
sgs.ai_skill_invoke["huakui"] = true

-- zhiyu
sgs.ai_skill_invoke["zhiyu"] = sgs.ai_skill_invoke["qiongtu"]
sgs.ai_skill_askforag["zhiyu"] = function(self, card_ids)
	local cards = {}
	for _, card_id in ipairs(card_ids)  do
		table.insert(cards, sgs.Sanguosha:getCard(card_id))
	end
	self:sortByUseValue(cards)
	return cards[1]:getId()
end
sgs.ai_cardshow["zhiyu"] = function(self, requestor)
	local card = self:getUnuseCard()
	return card
end
