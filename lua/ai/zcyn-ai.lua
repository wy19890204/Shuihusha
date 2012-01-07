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

-- tuzai
sgs.ai_skill_invoke["tuzai"] = true
