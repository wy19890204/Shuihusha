-- AI for mustang package

-- manli
sgs.ai_skill_invoke["manli"] = sgs.ai_skill_invoke["liba"]

-- qiaogong

-- tianyan
sgs.ai_skill_invoke["tianyan"] = true
sgs.ai_skill_askforag["tianyan"] = function(self, card_ids)
	local player = self.player:getTag("TianyanTarget"):toPlayer()
	local cards = {}
	for _, card_id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(card_id)
		table.insert(cards, card)
	end
	self:sortByUseValue(cards, true)
	if self:isFriend(player) then
		for _, card in ipairs(cards) do
			if self:getUseValue(card) < 2.7 then
				return card:getEffectiveId()
			end
		end
	else
		self:sortByUseValue(cards)
		for _, card in ipairs(cards) do
			if self:getUseValue(card) > 5.4 then
				return card:getEffectiveId()
			end
		end
	end
	return -1
end

-- paohong
local paohong_skill={}
paohong_skill.name = "paohong"
table.insert(sgs.ai_skills, paohong_skill)
paohong_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local thunder_card
	self:sortByUseValue(cards, true)
	for _,card in ipairs(cards)  do
		if card:objectName() == "slash" and card:isBlack() then
			thunder_card = card
			break
		end
	end
	if thunder_card then
		local suit = thunder_card:getSuitString()
		local number = thunder_card:getNumberString()
		local card_id = thunder_card:getEffectiveId()
		local card_str = ("thunder_slash:paohong[%s:%s]=%d"):format(suit, number, card_id)
		return sgs.Card_Parse(card_str)
	end
end
sgs.ai_filterskill_filter["paohong"] = function(card, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:objectName() == "slash" and card:isBlack() then return ("thunder_slash:paohong[%s:%s]=%d"):format(suit, number, card_id) end
end

-- tuzai&longjiao
sgs.ai_skill_invoke["tuzai"] = true
sgs.ai_skill_invoke["longjiao"] = true

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

