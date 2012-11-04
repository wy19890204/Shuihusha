-- AI for sp package

-- luda
-- baoquan
sgs.ai_skill_invoke["baoquan"] = true
sgs.ai_skill_playerchosen["baoquan"] = function(self, targets)
	if self:isWeak() or self.player:getHandcardNum() < self.player:getMaxCards() then
		return self.player
	else
		self:sort(self.friends, "handcard")
		return self.friends[1]
	end
end
sgs.ai_skill_use["@@baoquan"] = function(self, prompt)
	local cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByUseValue(cards, true)
	local card_ids = {}
	for _, card in ipairs(cards) do
		if card:inherits("EquipCard") then
			table.insert(card_ids, card:getEffectiveId())
		end
	end
	if #card_ids < 1 then return "." end
	self:sort(self.enemies)
	local target = self.enemies[1]
	if target then
		return "@BaoquanCard=" .. table.concat(card_ids, "+") .. "->" .. target:objectName()
	else
		return "."
	end
end

-- tora
-- pu
strike_skill = {}
strike_skill.name = "strike"
table.insert(sgs.ai_skills, strike_skill)
strike_skill.getTurnUseCard = function(self, inclusive)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	if #cards<(self.player:getHp()+1) then return nil end
	if #cards<2 then return nil end

	self:sortByUseValue(cards,true)
	local suit1 = cards[1]:getSuitString()
	local card_id1 = cards[1]:getEffectiveId()

	local suit2 = cards[2]:getSuitString()
	local card_id2 = cards[2]:getEffectiveId()

	local suit = "no_suit"
	if cards[1]:isBlack() == cards[2]:isBlack() then suit = suit1 end
	local card_str = ("slash:strike[%s:%s]=%d+%d"):format(suit, 0, card_id1, card_id2)

	local slash = sgs.Card_Parse(card_str)
	return slash
end

-- xian
sgs.ai_skill_invoke["lift"] = function(self, data)
	if not self.player:faceUp() then return true end
	if self:isWeak() then return false end
	local effect = data:toSlashEffect()
	if self:isFriend(effect.to) then return false end
	return self:isWeak(effect.to) or self.player:getHandcardNum() >= 3
end

-- jian
sgs.ai_skill_invoke["exterminate"] = function(self, data)
	local damage = data:toDamage()
	local notarget = true
	for _, aplayer in sgs.qlist(global_room:getAllPlayers()) do
		if damage.to:distanceTo(aplayer) == 1 then
			notarget = false
			if self:isFriend(aplayer) then return false end
		end
	end
	return not notarget
end

-- keyin
-- luanjun
sgs.ai_skill_invoke["luanjun"] = function(self, data)
	local player = data:toPlayer()
	return self:isFriend(player)
end
sgs.ai_skill_use["@@luanjun"] = function(self, prompt)
	self:sort(self.enemies)
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isNude() then
			return "@LuanjunCard=.->" .. enemy:objectName()
		end
	end
	return "."
end

-- qingshang
sgs.ai_skill_invoke["qingshang"] = true
sgs.ai_skill_playerchosen["qingshang"] = sgs.ai_skill_playerchosen["shunshui"]

-- yuzhong
sgs.ai_skill_choice["yuzhong"] = function(self, choice)
	if choice == "hp+card+cancel" then
		local lord = self.room:getLord()
		if self:isFriend(lord) then
			if lord:getLostHp() > 1 then
				return "hp"
			else
				return "card"
			end
		else
			return "cancel"
		end
	elseif choice == "all+me+cancel" then
		local king = self.room:getKingdoms()
		if #self.friends >= king then
			return "all"
		else
			return "me"
		end
	end
end
sgs.ai_skill_use["@@yuzhong"] = function(self, prompt)
	local king = self.room:getKingdoms()
	local friends = {}
	self:sort(self.friends, "handcard")
	for _, friend in ipairs(self.friends) do
		table.insert(friends, friend:objectName())
		if #friends >= king then break end
	end
	if #friends > 0 then
		return "@YuzhongCard=.->" .. table.concat(friends, "+")
	else
		return "."
	end
end

-- jiebao
sgs.ai_skill_use["@@jiebao"] = function(self, prompt)
	local enemies = {}
	local i = 0
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isNude() and not enemy == self.player then
			table.insert(enemies, enemy:objectName())
			i = i + 1
		end
		if i == 2 then break end
	end
	if #enemies <= 2 then
		return "@JiebaoCard=.->" .. table.concat(enemies, "+")
	else
		return "."
	end
end

-- dushi
sgs.ai_skill_invoke["dushi"] = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.from)
end
sgs.ai_skill_playerchosen["dushi"] = function(self, targets)
	local mx = 1
	local mxp = self.player:getNextAlive()
	for _, player in sgs.qlist(targets) do
		local kuro = 0
		for _, hcard in sgs.qlist(target:getHandcards()) do
			if hcard:isBlack() then kuro = kuro + 1 end
		end
		if kuro > mx then
			mx = kuro
			mxp = player
		end
	end
	return mxp
end

-- xiaduo
sgs.ai_skill_cardask["@xiaduo"] = function(self, data)
	local caninvoke = false
	local damage = data:toDamage()
	for _, enemy in sgs.qlist(self.room:getOtherPlayers(damage.to)) do
		if self:isEnemy(enemy) and self.player:distanceTo(enemy) == 1 then
			self.xiaduotarget = enemy
			caninvoke = true
			break
		end
	end
	if not caninvoke then return end
	local cards = self.player:getCards("he")
	for _, card in sgs.qlist(cards) do
		if card:inherits("EquipCard") then
			return card:getEffectiveId()
		end
	end
	return "."
end
sgs.ai_skill_playerchosen["xiaduo"] = function(self, targets)
	local target = self.xiaduotarget
	return target
end

