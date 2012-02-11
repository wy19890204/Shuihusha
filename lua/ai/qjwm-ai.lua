-- kaixian
sgs.ai_skill_invoke["kaixian"] = true

--xiagu
sgs.ai_skill_cardask["@xiagu"] = function(self, data)
	local damage = data:toDamage()
	if self:isFriend(damage.to) then
		if damage.to:hasSkill("fushang") and
			damage.to:getMaxHP() > 3 and damage.damage < 2 then return "." end
		local allcards = self.player:getCards("he")
		for _, card in sgs.qlist(allcards) do
			if card:inherits("EquipCard") then
				return card:getEffectiveId()
			end
		end
	end
	return "."
end

-- kongliang
sgs.ai_skill_invoke["kong1iang"] = function(self, data)
	local showcardnum = self.player:getMaxHP() + self.player:getLostHp() + self.player:getHandcardNum()
	return showcardnum > 8
end
sgs.ai_skill_askforag["kong1iang"] = function(self, card_ids)
	local final = sgs.Sanguosha:getCard(card_ids[1])
	local suitnum = 100
	for _, card_id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(card_id)
		local suit = card:getSuitString()
		if final and final:getSuitString() ~= suit then
			local num = self:getSuitNum(suit, false)
			if num < suitnum then
				suitnum = num
				final = card
			end
		end
	end
	return final:getEffectiveId()
end

-- liba
sgs.ai_skill_invoke["liba"] = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.to)
end

-- fuhu
sgs.ai_skill_cardask["@fuhu"] = function(self, data)
	local damage = data:toDamage()
	if self:isEnemy(damage.from) then
		local cards = self.player:getHandcards()
		for _, card in sgs.qlist(cards) do
			if card:inherits("BasicCard") then
				return card:getEffectiveId()
			end
		end
	end
	return "."
end

-- zhanchi
sgs.ai_skill_invoke["zhanchi"] = function(self, data)
	if self.player:hasWeapon("crossbow") then
		return true
	else
		return false
	end
end

-- fuqin
sgs.ai_skill_choice["fuqin"] = function(self, choice)
	local source = self.player:getTag("FuqinSource"):toPlayer()
	if self:isFriend(source) then
		return "qing"
	else
		local rand = math.random(1, 2)
		if rand == 1 then
			return "yan"
		else
			return "qing"
		end
	end
end
sgs.ai_skill_playerchosen["fuqin"] = function(self, targets)
	self:sort(self.friends, "handcard")
	if self.friends[1]:getHandcardNum() > 2 then
		self:sort(self.friends, "hp")
		if self.friends[1]:getHp() > 2 then return self.player
		else return self.friends[1] end
	end
	return self.friends[1]
end

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
sgs.ai_skill_playerchosen["taolue"] = function(self, targets)
	local friends = sgs.QList2Table(targets)
	self:sort(friends, "hp")
	for _, friend in ipairs(friends) do
		if self:isFriend(friend) and friend ~= self.player then
		    return friend
		end
	end
	return friends[1]
end

-- butian
sgs.ai_skill_invoke["@butian"]=function(self,prompt,judge)
	judge = judge or self.player:getTag("Judge"):toJudge()

	if self:needRetrial(judge) then
		local cards = sgs.QList2Table(self.player:getHandcards())
		if self:getUnuseCard() then
			local card_id = self:getUnuseCard():getEffectiveId()
			return "@ButianCard=" .. card_id
		end
	end
	return "."
end
sgs.ai_skill_askforag["butian"] = function(self, card_ids)
	local judge = self.player:getTag("Judge"):toJudge()
	local cards = {}
	local card_id
	if self:needRetrial(judge) then
		for _, card_id in ipairs(card_ids) do
			local card = sgs.Sanguosha:getCard(card_id)
			table.insert(cards, card)
		end
		card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return card_id
		end
	end
	return card_ids[1]
end

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

-- jiachu
sgs.ai_skill_cardask["@jiachu"] = function(self)
	local target = self.room:getLord()
	if self:isFriend(target) then
		local allcards = self.player:getCards("he")
		for _, card in sgs.qlist(allcards) do
			if card:getSuit() == sgs.Card_Heart then
				return card:getEffectiveId()
			end
		end
	end
	return "."
end

