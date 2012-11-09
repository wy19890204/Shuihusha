
-- feiqiang
feiqiang_skill={}
feiqiang_skill.name = "feiqiang"
table.insert(sgs.ai_skills, feiqiang_skill)
feiqiang_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("FeiqiangCard") then
		local cards = self.player:getCards("he")
		cards = sgs.QList2Table(cards)
		for _, acard in ipairs(cards)  do
			if acard:inherits("Weapon") then
				return sgs.Card_Parse("@FeiqiangCard=" .. acard:getEffectiveId())
			end
		end
	end
	return
end
sgs.ai_skill_use_func["FeiqiangCard"] = function(card, use, self)
	self:sort(self.enemies, "defense")
	use.card = card
	if use.to then
		use.to:append(self.enemies[1])
	end
end
sgs.ai_skill_choice["feiqiang"] = function(self, choices)
	return "gong"
end

-- citan
sgs.ai_skill_invoke["citan"] = sgs.ai_skill_invoke["lihun"]
sgs.ai_skill_askforag["citan"] = function(self, card_ids)
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
	local first_found = false
	local second_found = false
	local first_card, second_card
	if self.player:getHandcardNum() >= 2 then
		local cards = self.player:getHandcards()
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

-- lingdi
local lingdi_skill={}
lingdi_skill.name = "lingdi"
table.insert(sgs.ai_skills, lingdi_skill)
lingdi_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("LingdiCard") or self.player:isNude() then return end
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	return sgs.Card_Parse("@LingdiCard=" .. cards[1]:getEffectiveId())
end
sgs.ai_skill_use_func["LingdiCard"]=function(card,use,self)
	local up, down
	for _, enemy in ipairs(self.enemies) do
		if enemy:faceUp() then
			up = enemy
			break
		end
	end
	for _, friend in ipairs(self.friends) do
		if not friend:faceUp() then
			down = friend
			break
		end
	end
	if up and down then
		if use.to then
			use.to:append(up)
			use.to:append(down)
		end
		use.card=card
	end
end

-- qiaodou
sgs.ai_skill_invoke["qiaodou"] = function(self, data)
	local sb = data:toPlayer()
	return sb:faceUp() and self:isEnemy(sb)
end

-- cihu
sgs.ai_skill_invoke["@cihu"] = function(self, prompt)
	local num = self.player:getMark("CihuNum")
	local ogami = self.player:getTag("CihuOgami"):toPlayer()
	if self:isFriend(ogami) then return "." end
	local caninvoke = false
	local women = {}
	local players = self.room:getMenorWomen("female")
	players = sgs.QList2Table(players)
	for _, woman in ipairs(players) do
		if woman:isWounded() and self:isFriend(woman) then
			caninvoke = true
			table.insert(women, woman)
		end
	end
	if self.player:getHp() > 2 and caninvoke then
		self:sort(women, "hp")
		local cards = self.player:getCards("he")
		cards = sgs.QList2Table(cards)
		local card_ids = {}
		for i = 1, num do
			table.insert(card_ids, cards[i]:getEffectiveId())
		end
		return "@CihuCard=" .. table.concat(card_ids, "+") .. "->" .. women[1]:objectName()
	end
	return "."
end

