-- AI for tiger package

-- leiheng
-- guzong
sgs.ai_skill_invoke["guzong"] = function(self, data)
	local player = data:toPlayer()
	self.guzongtarget = player
--	local cards = self.player:getTag("Guzong")
	if self:isEnemy(player) then
		return math.random(1, 3) == 2
	else
		return math.random(2, 3) == 2
	end
--[[for _, card_id in sgs.qlist(cards) do
		local card = sgs.Sanguosha:getCard(card_id:toInt())
		if card:inherits("Peach") or card:inherits("Analeptic") then
			return true
		end
	end]]
end
sgs.ai_skill_askforag["guzong"] = function(self, card_ids)
	if self.player:getHandcardNum() < 2 then return -1 end
	if self:isEnemy(self.guzongtarget) then return 9999 end
	for _, card_id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(card_id)
		if card:inherits("Peach") or card:inherits("Analeptic") then
			return card_id
		end
	end
	return -1
end
sgs.ai_skill_cardchosen["guzong"] = function(self, who)
	local cards = sgs.QList2Table(who:getCards("he"))
	self:sortByUseValue(cards, self:isEnemy(who))
	return cards[1]
end

-- sunli
-- neiying
sgs.ai_view_as["neiying"] = function(card, player, card_place)
	if player:getCardCount(true) < 3 then return end	
	local first_found, second_found = false, false
	local first_card, second_card
	local cards = player:getCards("he")
	cards=sgs.QList2Table(cards)
--	self:sortByUseValue(cards, true)
	for _, fcard in ipairs(cards) do
		if not (fcard:inherits("Peach") or fcard:inherits("ExNihilo")) then
			first_card = fcard
			first_found = true
			for _, scard in ipairs(cards) do
				if first_card ~= scard and scard:getColor() == first_card:getColor() and 
					not (scard:inherits("Peach") or scard:inherits("ExNihilo")) then
					second_card = scard
					second_found = true
					break
				end
			end
			if second_card then break end
		end
	end
	if first_found and second_found then
		local first_suit, first_number, first_id = first_card:getSuitString(), first_card:getNumberString(), first_card:getId()
		local second_suit, second_number, second_id = second_card:getSuitString(), second_card:getNumberString(), second_card:getId()
		local card_str = ("counterplot:neiying[%s:%s]=%d+%d"):format(first_suit, first_number, first_id, second_id)
		return card_str
	end
end

-- wuyanguang
-- jintang
sgs.ai_skill_use["@@jintang!"] = function(self, prompt)
--	local count = self.player:getEquips():length()
	local equip = self.player:getTag("Jintg"):toCard()
	self:sort(self.friends_noself, "equip")
	for _, friend in ipairs(self.friends_noself) do
		local cards = sgs.QList2Table(friend:getCards("e"))
		for _, card in ipairs(cards) do
			if equip:getSubtype() ~= card:getSubtype() then
				return "@JintangCard=.->" .. friend:objectName()
			end
		end
	end
	return "@JintangCard=.->" .. self.friends_noself[1]:objectName()
end

-- shixiu
-- pinming
sgs.ai_skill_invoke["pinming"] = function(self, data)
	local damage = data:toDamage()
	if (damage.from and self:isFriend(damage.from)) or damage.damage < 1 then return false end
	if not self.player:hasFlag("PinmingDie") then
		if self.player:getMaxHp() > 4 then
			if damage.damage > 1 then
				return true
			else
				return math.random(0, 2) == 1
			end
		elseif self.player:getMaxHp() > 3 then
			return damage.damage > 1
		end
	else
		if #self:getEnemies() == 1 then
			return true
		end
		if self.player:isLord() then return false end
		if #self:getEnemies() == 2 and self.player:getRole() == "loyalist" then
			return true
		end
	end
	return false
end

-- lvfang
-- lieji
sgs.ai_skill_use["@@lieji"] = function(self, prompt)
	local basiccard = self:getCard("BasicCard")
	if not basiccard or self.player:getHandcardNum() - 1 > self.player:getMaxCards() then return "." end
	enemies = self:getEnemies()
	if #enemies < 2 then return "." end
	self:sort(enemies)

	local cards = sgs.QList2Table(self.player:getCards("h"))
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		if not card:inherits("Peach") or (self:isWeak() and not card:inherits("Analeptic")) then
			return "@LiejiCard=" .. card:getEffectiveId() .. "->"
					.. enemies[1]:objectName() .. "+" .. enemies[2]:objectName()
		end
	end
end

-- tianhu
-- wuzhou
sgs.ai_skill_invoke["wuzhou"] = true

-- huwei
huwei_skill={}
huwei_skill.name = "huweiv"
table.insert(sgs.ai_skills, huwei_skill)
huwei_skill.getTurnUseCard = function(self)
	local lord = self.room:getLord()
	if not lord or self.player:hasLordSkill("huwei") or self.player:getKingdom() ~= "jiang" then return end
	if self:isFriend(lord) and lord:hasLordSkill("huwei") and not lord:hasEquip() then
		local slash = self:getCard("EquipCard")
		if slash then
			return sgs.Card_Parse("@HuweiCard=" .. slash:getEffectiveId())
		end
	end
end
sgs.ai_skill_use_func["HuweiCard"] = function(card, use, self)
	use.card = card
	if use.to then use.to:append(self.room:getLord()) end
end

-- zhangheng
-- jielue
sgs.ai_skill_invoke["jielue"] = sgs.ai_skill_invoke["lihun"]

function SmartAI:getZhangheng(player) -- enemy zhangheng's threat
	player = player or self.player
	if player:isKongcheng() then return false end
	local room = player:getRoom()
	for _, zhangheng in sgs.qlist(room:findPlayersBySkillName("jielue")) do
		if zhangheng ~= player then
			if self:isEnemy(player, zhangheng) and zhangheng:getMark("fuhun") == 0 then
				return true
			end
		end
	end
	return false
end

function SmartAI:getZhanghengf(target, player) -- has friend zhangheng (for qimen)
	player = player or self.player
	if target:isKongcheng() then return false end
	local room = player:getRoom()
	for _, zhangheng in sgs.qlist(room:findPlayersBySkillName("jielue")) do
		if self:isEnemy(target, player) and zhangheng:getMark("fuhun") == 0 then
			return true
		end
	end
	return false
end

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
		if effect.from:hasEquip() then
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
			table.insert(slashs, card)
		end
	end
	if #slashs < 2 then return end

	local anal = self:getCard("Analeptic")
	if anal and self:isNoZhenshaMark() and not hasSilverLion(target) and not self:isWeak() then
		return sgs.Card_Parse(anal:getEffectiveId())
	end

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
