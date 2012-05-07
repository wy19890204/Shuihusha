
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
function sgs.ai_cardneed.xiagu(to, card, self)
	return card:inherits("EquipCard")
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
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	if not self:slashIsEffective(slash, damage.from) then return "." end
	if self:isEnemy(damage.from) then
		local cards = self.player:getCards("he")
		cards = sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		local default
		for _, card in ipairs(cards) do
			if card:isBlack() then
				if not default then default = card end
				if self:getCardsNum("Jink", damage.from) == 0 and
					(card:inherits("Analeptic") or card:inherits("Weapon")) then
					return card:getEffectiveId()
				end
			end
		end
		if default then
			return default:getEffectiveId()
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

-- dalei
local dalei_skill={}
dalei_skill.name = "dalei"
table.insert(sgs.ai_skills, dalei_skill)
dalei_skill.getTurnUseCard = function(self)
    if self.player:hasUsed("DaleiCard") or self.player:isKongcheng() then return end
	if self.player:getHp() > 1 then
		self:sort(self.enemies, "handcard")
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isKongcheng() and enemy:getGeneral():isMale()
				and self.player:inMyAttackRange(enemy) then
				self.daleitarget = enemy
				break
			end
		end
		local max_card = self:getMaxCard()
		if self.daleitarget and max_card then
			return sgs.Card_Parse("@DaleiCard=" .. max_card:getEffectiveId())
		end
	else
		for _, friend in ipairs(self.friends_noself) do
			if friend:getHandcardNum() > 3 and not friend:isWounded()
				and friend:getGeneral():isMale() then
				self.daleitarget = friend
				break
			end
		end
		local min_num = 14, min_card
		for _, hcard in sgs.qlist(self.player:getHandcards()) do
			if hcard:getNumber() < min_num then
				min_num = hcard:getNumber()
				min_card = hcard
			end
		end
		if self.daleitarget and min_card then
			return sgs.Card_Parse("@DaleiCard=" .. min_card:getEffectiveId())
		end
	end
end
sgs.ai_skill_use_func["DaleiCard"]=function(card, use, self)
	if use.to then use.to:append(self.daleitarget) end
	use.card=card
	return
end
sgs.ai_skill_invoke["dalei"] = function(self, data)
	local damage = data:toDamage()
	self:sort(self.friends, "hp")
	local caninvoke = false
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() and friend ~= damage.to then
			caninvoke = true
			self.daleitarget = friend
			break
		end
	end
	return caninvoke
end
sgs.ai_skill_playerchosen["dalei"] = function(self, targets)
	return self.daleitarget
end

-- fuqin
sgs.ai_skill_choice["fuqin"] = function(self, choice)
	if choice == "qing+nil" then return "qing" end
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

-- fangzhen
function sgs.ai_trick_prohibit.fangzhen(card, self, to)
	return card:inherits("Duel") and self.player:getHp() > to:getHp()
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
		self:sortByUseValue(cards, true)
		return "@ButianCard=" .. cards[1]:getEffectiveId()
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

