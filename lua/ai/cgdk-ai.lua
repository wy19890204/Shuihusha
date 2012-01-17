-- jiuhan&linmo
sgs.ai_skill_invoke["jiuhan"] = true
sgs.ai_skill_invoke["linmo"] = true

-- zhaixing
sgs.ai_skill_invoke["@zhaixing"]=function(self,prompt)
	local judge = self.player:getTag("Judge"):toJudge()
	local all_cards = self.player:getCards("he")
	local cards = sgs.QList2Table(all_cards)

	if #cards == 0 then return "." end
	local card_id = self:getRetrialCardId(cards, judge)
	if card_id == -1 then
		if self:needRetrial(judge) then
			self:sortByUseValue(cards, true)
			if self:getUseValue(judge.card) > self:getUseValue(cards[1]) then
				return "@ZhaixingCard=" .. cards[1]:getId()
			end
		end
	elseif self:needRetrial(judge) or self:getUseValue(judge.card) > self:getUseValue(sgs.Sanguosha:getCard(card_id)) then
		return "@ZhaixingCard=" .. card_id
	end
-- zhaixing can draw card
	for _, card in ipairs(cards) do
		if card:getSuit() == sgs.Card_Diamond then
			return "@ZhaixingCard=" .. card:getId()
		end
	end
	return "."
end

-- liehuo&citan
sgs.ai_skill_invoke["liehuo"] = sgs.ai_skill_invoke["lihun"]
sgs.ai_skill_invoke["citan"] = sgs.ai_skill_invoke["lihun"]
sgs.ai_skill_askforag["citan"] = function(self, card_ids)
	if card_ids:isEmpty() then return -1 end
	local cards = {}
	for _, card_id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(card_id)
		table.insert(cards, card)
	end
	self:sortByUseValue(cards)
	return cards[1]:getEffectiveId()
end
sgs.ai_skill_playerchosen["citan"] = function(self, targets)
	local friends = sgs.QList2Table(targets)
	self:sort(friends, "handcard")
	for _, friend in ipairs(friends) do
		if self:isFriend(friend) then
		    return friend
		end
	end
	return targets[1]
end

-- bingji
local bingji_skill={}
bingji_skill.name = "bingji"
table.insert(sgs.ai_skills, bingji_skill)
bingji_skill.getTurnUseCard = function(self)
	if not self:slashIsAvailable() or not self.player:isWounded() then return end
	local first_found, second_found = false, false
	local first_card, second_card
	if self.player:getHandcardNum() >= 2 then
		local cards = self.player:getHandcards()
		local same_suit=false
		cards = sgs.QList2Table(cards)
		for _, fcard in ipairs(cards) do
			if not (fcard:inherits("Peach") or fcard:inherits("ExNihilo") or fcard:inherits("AOE")) then
				first_card = fcard
				first_found = true
				for _, scard in ipairs(cards) do
					if first_card ~= scard and scard:getType() == first_card:getType() and 
						not (scard:inherits("Peach") or scard:inherits("ExNihilo") or scard:inherits("AOE")) then
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
		return sgs.Card_Parse("@BingjiCard=" .. first_card:getId() + second_card:getId())
	end
end
sgs.ai_skill_use_func["BingjiCard"] = function(card, use, self)
	local targetnum = self.player:getLostHp()
	self:sort(self.enemies, "defense")
	local a = 0
	for _, enemy in ipairs(self.enemies) do
		if use.to then
			use.to:append(target)
			a = a + 1
		end
		if a == 2 then
			use.card = card
			return
		end
	end
end

--tuntian
sgs.ai_skill_invoke.tuntian = true

--fangquan
sgs.ai_skill_invoke.fangquan = function(self, data)
	if #self.friends == 1 then
		return false
	end

	local limit = self.player:getMaxCards()
	return self.player:getHandcardNum() <= limit
end

sgs.ai_skill_playerchosen.fangquan = function(self, targets)
	for _, target in sgs.qlist(targets) do
		if self:isFriend(target) then
			return target
		end
	end
end

--jixi
local jixi_skill={}
jixi_skill.name="jixi"
table.insert(sgs.ai_skills, jixi_skill)
jixi_skill.getTurnUseCard = function(self)
	if self.player:getPile("field"):isEmpty()
		or (self.player:getHandcardNum()>=self.player:getHp() and
		self.player:getPile("field"):length()<= self.room:getAlivePlayers():length()/2) then
		return
	end
	local snatch=sgs.Sanguosha:getCard(self.player:getPile("field"):first())
	snatch=sgs.Sanguosha:cloneCard("snatch", snatch:getSuit(), snatch:getNumber())
	local use={isDummy=true}
	self:useCardSnatch(snatch,use)
	if use.card then return sgs.Card_Parse("@JixiCard=.") end
end

sgs.ai_skill_use_func["JixiCard"] = function(card, use, self)
	use.card = sgs.Card_Parse("@JixiCard=.")
end

sgs.ai_skill_playerchosen.jixi = function(self, targets)
	local snatch = sgs.Sanguosha:getCard(self.jixi)
	snatch = sgs.Sanguosha:cloneCard("snatch", snatch:getSuit(), snatch:getNumber())
	local choices = {}
	for _, target in sgs.qlist(targets) do
		if self:isEnemy(target) and not target:getCards("he"):isEmpty()
			and self:hasTrickEffective(snatch, target) then
			table.insert(choices, target)
		elseif self:isFriend(target) and not target:getCards("j"):isEmpty()
			and self:hasTrickEffective(snatch, target) then
			table.insert(choices, target)
		end
	end

	if #choices == 0 then return targets:at(0) end

	self:sort(choices, "hp")
	return choices[1]
end

sgs.ai_skill_askforag.jixi = function(self, card_ids)
	self.jixi=card_ids[math.random(1,#card_ids)]
	return self.jixi
end

--tiaoxin
local tiaoxin_skill={}
tiaoxin_skill.name="tiaoxin"
table.insert(sgs.ai_skills, tiaoxin_skill)
tiaoxin_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("TiaoxinCard") then return end
	return sgs.Card_Parse("@TiaoxinCard=.")
end

sgs.slash_property = {}
sgs.ai_skill_use_func["TiaoxinCard"] = function(card,use,self)
	local targets = {}
	for _, enemy in ipairs(self.enemies) do
		sgs.slash_property =
		{
			is_black = false,
			is_red = false,
			is_normal = false,
			is_fire = false,
			is_thunder = false
		}

		local cards = enemy:getHandcards()
		cards = sgs.QList2Table(cards)

		for _, card in ipairs(cards) do
			if card:inherits("Slash") then
				if card:isBlack() then sgs.slash_property["is_black"] = true end
				if card:isRed() then sgs.slash_property["is_red"] = true end
				if card:inherits("FireSlash") then sgs.slash_property["is_fire"] = true
				elseif card:inherits("ThunderSlash") then sgs.slash_property["is_thunder"] = true
				else sgs.slash_property["is_normal"] = true
				end
			end
		end

		local slash_useless = false
		local has_armor = self.player:getArmor()
		if has_armor then
			if self.player:getArmor():objectName() == "vine" then
				if not (sgs.slash_property["is_fire"] or sgs.slash_property["is_thunder"]) then
					slash_useless = true
				end
			elseif self.player:getArmor():objectName() == "renwang_shield" then
				if not sgs.slash_property["is_red"] then
					slash_useless = true
				end
			end
		end

		if enemy:inMyAttackRange(self.player) and
			(self:getCardsNum("Slash", enemy) == 0 or slash_useless or self:getCardsNum("Jink") > 0) and
			not enemy:isNude() then
			table.insert(targets, enemy)
		end
	end

	if #targets == 0 then return end

	if use.to then
		self:sort(targets, "hp")
		use.to:append(targets[1])
	end
	use.card = sgs.Card_Parse("@TiaoxinCard=.")
end

--zhiji
sgs.ai_skill_choice["zhiji"] = function(self, choice)
	if self.player:getHp() < self.player:getMaxHP()-1 then return "recover" end

	return "draw"
end

--zhiba
local zhiba_skill={}
zhiba_skill.name="zhiba_pindian"
table.insert(sgs.ai_skills, zhiba_skill)
zhiba_skill.getTurnUseCard = function(self)
	local lord = self.room:getLord()
	if lord:getHandcardNum() == 0
		or self.player:getHandcardNum() == 0
		or self.player:getHandcardNum() < self.player:getHp()
		or self.player == lord
		or self.player:getKingdom() ~= "wu"
		or self.player:hasUsed("ZhibaCard")
		or not lord:hasSkill("sunce_zhiba") then
		return
	end

	local zhiba_str
	local cards = self.player:getHandcards()

	local max_num = 0, max_card
	local min_num = 14, min_card
	for _, hcard in sgs.qlist(cards) do
		if hcard:getNumber() > max_num then
			max_num = hcard:getNumber()
			max_card = hcard
		end

		if hcard:getNumber() <= min_num and not (self:isFriend(lord) and hcard:inherits("Shit")) then
			if hcard:getNumber() == min_num then
				if min_card and self:getKeepValue(hcard) > self:getKeepValue(min_card) then
					min_num = hcard:getNumber()
					min_card = hcard
				end
			else
				min_num = hcard:getNumber()
				min_card = hcard
			end
		end
	end

	local lord_max_num = 0, lord_max_card
	local lord_min_num = 14, lord_min_card
	local lord_cards = lord:getHandcards()
	for _, lcard in sgs.qlist(lord_cards) do
		if lcard:getNumber() > lord_max_num then
			lord_max_card = lcard
			lord_max_num = lcard:getNumber()
		end
		if lcard:getNumber() < lord_min_num then
			lord_min_num = lcard:getNumber()
			lord_min_card = lcard
		end
	end

	if self:isEnemy(lord) and max_num > lord_max_num then
		zhiba_str = "@ZhibaCard=" .. max_card:getEffectiveId()
	end
	if self:isFriend(lord) and min_num < lord_min_num then
		zhiba_str = "@ZhibaCard=" .. min_card:getEffectiveId()
	end

 	if not zhiba_str then return end

	return sgs.Card_Parse(zhiba_str)
end

sgs.ai_skill_use_func["ZhibaCard"] = function(card, use, self)
	use.card = card
	if use.to then
		use.to:append(self.room:getLord())
	end
end

sgs.ai_skill_choice["zhiba_pindian"] = function(self, choices)
	local who = self.room:getCurrent()
	if self:isEnemy(who) then return "reject"
	else return "accept"
	end
end

sgs.ai_skill_choice["huashen"] = function(self, choices)
	local parseprompt = choices:split("+")
	local index = math.random(1, #parseprompt)
	return choices[index]
end