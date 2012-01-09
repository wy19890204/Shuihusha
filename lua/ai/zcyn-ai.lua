-- tongwu
sgs.ai_skill_invoke["tongwu"] = true
sgs.ai_skill_playerchosen["tongwu"] = function(self, targets)
	local targetlist=sgs.QList2Table(targets)
	self:sort(targetlist, "handcard")
	for _, target in ipairs(targetlist) do
		if self:isFriend(target) then
			return target
		end
	end
	return self.player
end

-- dujian
sgs.ai_skill_invoke["dujian"] = function(self, data)
	local rand = math.random(1, 2)
	return rand == 2
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

-- hengchong
sgs.ai_skill_playerchosen["hengchong"] = function(self, targets)
	local targetlist = sgs.QList2Table(targets)
	for _, target in ipairs(targetlist) do
		if self:isEnemy(target) then
			return target
		end
	end
	return targetlist[1]
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
