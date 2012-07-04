
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

-- shenhuo
sgs.ai_skill_invoke["shenhuo"] = true
local shenhuo_skill={}
shenhuo_skill.name = "shenhuo"
table.insert(sgs.ai_skills, shenhuo_skill)
shenhuo_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards,true)
	for _,acard in ipairs(cards)  do
		if acard:isRed() and acard:inherits("TrickCard") then
			card = acard
			break
		end
	end
	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("fire_attack:shenhuo[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end
function sgs.ai_cardneed.shenhuo(to, card, self)
	return (card:getSuit() == sgs.Card_Diamond or card:getSuit() == sgs.Card_Heart)
		and card:inherits("TrickCard")
end

-- binggong
sgs.ai_skill_use["@@binggong"] = function(self, prompt)
	local num = self.player:getMark("Bingo")
	if num < 3 and self.player:isWounded() then return "." end
	self:sort(self.friends_noself, "defense")
	local target = self.friends_noself[1]
	if not target then return "." end
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local card_ids = {}
	for i = 1, num do
		table.insert(card_ids, cards[i]:getEffectiveId())
	end
	return "@BinggongCard=" .. table.concat(card_ids, "+") .. "->" .. target:objectName()
end

-- shenpan
sgs.ai_skill_invoke["shenpan"] = function(self, data)
	local judge = data:toJudge()
	if not self:needRetrial(judge) then return false end
	local wizard_friend
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player == judge.who then break end
		if self:isFriend(player) then
			if (player:hasSkill("yixing") and self:getYixingCard(judge) > -1) or
				player:hasSkill("butian") then
				wizard_friend = player
				break
			end
		end
	end
	return not wizard_friend
end

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

-- linse
function sgs.ai_trick_prohibit.linse(card)
	return card:inherits("Dismantlement") or card:inherits("Snatch")
end
