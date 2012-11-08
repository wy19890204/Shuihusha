-- AI for dragon package

-- zaochuan
local zaochuan_skill = {}
zaochuan_skill.name = "zaochuan"
table.insert(sgs.ai_skills, zaochuan_skill)
zaochuan_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards,true)
	for _,acard in ipairs(cards)  do
		if acard:getSuit() == sgs.Card_Club then
			card = acard
			break
		end
	end
	if not card then return nil end
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("iron_chain:zaochuan[club:%s]=%d"):format(number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end

-- nusha
nusha_skill={}
nusha_skill.name = "nusha"
table.insert(sgs.ai_skills, nusha_skill)
nusha_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("NushaCard") then return end
	local enum = 0
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHandcardNum() > enum then
			enum = enemy:getHandcardNum()
		end
	end
	local fnum = 0
	for _, friend in ipairs(self.friends_noself) do
		if friend:getHandcardNum() > fnum then
			fnum = friend:getHandcardNum()
		end
	end
	if enum >= fnum then
		local slash = self:getCardId("Slash")
		if slash then
			return sgs.Card_Parse("@NushaCard=" .. slash)
		end
	end
	return
end
sgs.ai_skill_use_func["NushaCard"] = function(card, use, self)
	local enum = self.enemies[1]
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHandcardNum() > enum:getHandcardNum() then
			enum = enemy
		end
	end
	use.card = card
	if use.to then
		use.to:append(enum)
	end
end

-- kongying
sgs.ai_skill_invoke["kongying"] = true
sgs.ai_skill_playerchosen["kongying"] = function(self, targets)
	self:sort(self.enemies, "hp")
	return self.enemies[1]
end
function sgs.ai_cardneed.kongying(to, card, self)
	return card:inherits("Jink")
end
