-- AI for mini-generals

-- zhangbao
-- fangdiao
fangdiao_skill = {}
fangdiao_skill.name = "fangdiao"
table.insert(sgs.ai_skills, fangdiao_skill)
fangdiao_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("FangdiaoCard") then return end
	if self.player:isKongcheng() then return end
	local cards = sgs.QList2Table(self.player:getCards("h"))
	self:sortByUseValue(cards, true)
	self:sort(self.enemies)
	for _, enemy in ipairs(self.enemies) do
		if self.player:inMyAttackRange(enemy) and self.player:getHandcardNum() - 1 < enemy:getHandcardNum()  then
			self.fangdiaotarget = enemy
			return sgs.Card_Parse("@FangdiaoCard=" .. cards[1]:getEffectiveId())
		end
	end
end
sgs.ai_skill_use_func["FangdiaoCard"] = function(card, use, self)
	use.card = card
	if use.to then
		use.to:append(self.fangdiaotarget)
	end
end
sgs.ai_skill_choice["fangdiao"] = function(self, choice, data)
	local target = data:toPlayer()
	if target:getHandcardNum() - self.player:getHandcardNum() < 2 then
		local n = 0
		for _, t in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if self.player:distanceTo(t) < 2 and self:isFriend(t) then
				n = n + 1
			end
		end
		if n >= 2 then
			return "diao"
		end
	end
	local value1, value2 = 0, 0
	for _, hcard in sgs.qlist(self.player:getHandcards()) do
		value1 = value1 + self:getUseValue(hcard)
	end
	for _, hcard in sgs.qlist(target:getHandcards()) do
		value2 = value2 + self:getUseValue(hcard)
	end
	if value2 > value1 then
		return "fang"
	else
		return "diao"
	end
end
	
-- liruilan
-- chumai
sgs.ai_skill_cardask["@chumai"] = function(self, data)
	local target = data:toPlayer()
	if self:isEnemy(target) then
		local cards = self.player:getHandcards()
		for _, card in sgs.qlist(cards) do
			if card:isBlack() then return card:getEffectiveId() end
		end
	end
	return "."
end

-- yinlang
local yinlang_skill={}
yinlang_skill.name = "yinlang"
table.insert(sgs.ai_skills, yinlang_skill)
yinlang_skill.getTurnUseCard = function(self)
	if self.player:isKongcheng() then return end
	for _, player in ipairs(self.friends_noself) do
		if player:faceUp() then
			return sgs.Card_Parse("@YinlangCard=.")
		end
	end
	if (self.player:usedTimes("YinlangCard") < 2 or self:getOverflow() > 0) then
		return sgs.Card_Parse("@YinlangCard=.")
	end
	if self.player:getLostHp() < 2 then
		return sgs.Card_Parse("@YinlangCard=.")
	end
end
sgs.ai_skill_use_func["YinlangCard"] = function(card, use, self)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards, true)
	local name = self.player:objectName()
	if #self.friends > 1 then
		for _, hcard in ipairs(cards) do
			if hcard:inherits("EquipCard") then
				self:sort(self.friends_noself, "handcard")
				for _, player in ipairs(self.friends_noself) do
					if player:getGeneral():isMale() then
						use.card = sgs.Card_Parse("@YinlangCard=" .. hcard:getEffectiveId())
						if use.to then use.to:append(player) end
						return
					end
				end
			end
		end
	end
end

-- fangjie
-- beishui
sgs.ai_skill_choice["beishui"] = function(self, choice, data)
	local n = self.player:getMark("BeishuiNum")
	local e = #self.enemies
	if n > e then return "bei" end
	if math.random(1, 3) == 2 then
		return "bei"
	end
	return "shui"
end
sgs.ai_skill_playerchosen["beishui"] = function(self, targets)
	return self.player
end
sgs.ai_skill_use["@@beishui"] = function(self, prompt)
	local n = self.player:getMark("BeishuiNum")
	local targets = {}
	self:sort(self.enemies)
	for _, enemy in ipairs(self.enemies) do
		table.insert(targets, enemy:objectName())
		if #targets >= n then break end
	end
	if #targets > 0 then
		return "@BeishuiCard=.->" .. table.concat(targets, "+")
	else
		return "."
	end
end

-- renyuan
-- pushou
sgs.ai_skill_use["@@pushou"] = function(self, prompt)
	local target
	self:sort(self.enemies, "handcard")
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isKongcheng() then
			target = enemy
			break
		end
	end
	local card = self:getMaxCard()
	if card:getNumber() > 11 and target then
		return "@PushouCard=" .. card:getEffectiveId() .. "->" .. target:objectName()
	else
		return "."
	end
end

-- xisheng
-- zhengbing
sgs.ai_view_as["zhengbing"] = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place ~= sgs.Player_Equip then
		if card:isBlack() then
			return ("nullification:zhengbing[%s:%s]=%d"):format(suit, number, card_id)
		end
	end
end

-- qi6ing
sgs.ai_skill_use["@@qi6ing"] = function(self, prompt)
	local use = self.player:getTag("Qi6ingData"):toCardUse()
	if self.player:isNude() or self:isFriend(use.from) then return "." end
	if use.card:inherits("Wiretap") or use.card:inherits("Collateral") then return "." end
	if self.player:getHandcardNum() <= 2 and use.card:inherits("FireAttack") then return "." end
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByUseValue(cards, true)
	return "@Qi6ingCard=" .. cards[1]:getEffectiveId() .. "->."
end
