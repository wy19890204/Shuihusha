
-- zhanchi
sgs.ai_skill_invoke["zhanchi"] = function(self, data)
	if self.player:hasWeapon("crossbow") then
		return true
	else
		return false
	end
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
sgs.ai_skill_playerchosen["taolue"] = sgs.ai_skill_playerchosen["lihun"]

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

-- yueli
function sgs.ai_slash_prohibit.yueli(self, to)
	if self:isEquip("EightDiagram", to) then return true end
end

-- taohui
sgs.ai_skill_playerchosen["taohui"] = function(self, targets)
	self:sort(self.friends, "handcard")
	return self.friends[1]
end

-- qiangqu
sgs.ai_skill_invoke["qiangqu"] = function(self, data)
	local damage = data:toDamage()
	return self:isFriend(damage.to)
end

-- huatian
sgs.ai_skill_invoke["huatian"] = function(self, data)
	if not self.friends_noself[1] then return false end
	self:sort(self.friends_noself, "hp")
	if self.player:getMark("HBTJ") == 1 then
		return self.friends_noself[1]:isWounded()
	end
	return true
end
sgs.ai_skill_playerchosen["huatian"] = function(self, targets)
	local mark = self.player:getMark("HBTJ")
	if mark == 1 then
		self:sort(self.friends_noself, "hp")
		for _, friend in ipairs(self.friends_noself) do
			if friend:isWounded() then
				return friend
			end
		end
	elseif mark == 2 then
		self:sort(self.enemies, "hp")
		return self.enemies[1]
	end
end
