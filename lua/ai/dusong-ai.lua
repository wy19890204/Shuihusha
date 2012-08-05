sgs.ai_skill_invoke["weapon_recast"] = function(self, data)
	if self.player:isLord() then
		local card_use = data:toCardUse()
		if card_use.card:objectName() ~= "crossbow" then return true else return false end
	else
		if self.player:getWeapon() then return true else return false end
	end
end

sgs.ai_skill_invoke["draw_1v3"] = function(self, data)
	return not (self.player:hasSkill("kongcheng") and self.player:isKongcheng())
end

douzhan_skill={}
douzhan_skill.name = "douzhan"
table.insert(sgs.ai_skills, douzhan_skill)
douzhan_skill.getTurnUseCard = function(self)
	if not self:slashIsAvailable() or self.player:getHandcardNum() < 4 then return end
	local cards = sgs.QList2Table(self.player:getCards("h"))
	local newcards = {}
	for _, card in ipairs(cards) do
		if not card:inherits("Peach") then table.insert(newcards, card) end
	end
	if #newcards<(self.player:getHp()+1) then return nil end
	if #newcards<2 then return nil end

	local dzcards = {}
	for _, card1 in ipairs(newcards) do
		for _, card2 in ipairs(newcards) do
			if card1 ~= card2 and card1:getSuit() == card2:getSuit() then
				table.insert(dzcards, card1:getEffectiveId())
				table.insert(dzcards, card2:getEffectiveId())
				break
			end
		end
		if #dzcards >= 2 then
			break
		end
	end
	if #dzcards == 2 then
		return sgs.Card_Parse("@DouzhanCard=" .. dzcards[1] + dzcards[1])
	end
end
sgs.ai_skill_use_func["DouzhanCard"] = function(card, use, self)
	local n = 2
	if self.player:hasSkill("shuangzhan") then
        local x = self.player:getPlayersInMyAttackRange():length()
        if x > 2 then n = n + 1 end
    end
    if self.player:hasSkill("qinlong") and not self.player:hasEquip() then
		n = n + 1
	end
    if self.player:hasWeapon("sun_bow") and card:isRed() and card:objectName() == "slash" then
        n = n + 1
    end

	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy) then
			if use.to then
				use.to:append(enemy)
			end
		end
		if use.to and #use.to == n then break end
	end
	if use.to and #use.to <= n then
		use.card = card
	end
end

local douzhan_skill = {}
douzhan_skill.name = "douzhan"
table.insert(sgs.ai_skills, douzhan_skill)
douzhan_skill.getTurnUseCard = function(self, inclusive)
	local cards = sgs.QList2Table(self.player:getHandcards())
	if #cards < (self.player:getHp()+1) then return nil end
	if #cards < 2 then return nil end

	self:sortByUseValue(cards, true)
	local dzcards = {}
	local suit
	for _, card1 in ipairs(cards) do
		for _, card2 in ipairs(cards) do
			if card1 ~= card2 and card1:getSuit() == card2:getSuit() then
				table.insert(dzcards, card1:getEffectiveId())
				table.insert(dzcards, card2:getEffectiveId())
				suit = card1:getSuitString()
				break
			end
		end
		if #dzcards >= 2 then
			break
		end
	end

	if #dzcards == 2 then
		local card_str = ("slash:douzhan[%s:%s]=%d+%d"):format(suit, 0, dzcards[1], dzcards[2])
		local slash = sgs.Card_Parse(card_str)
		return slash
	end
end

sgs.ai_skill_invoke["zhengzhuang"] = true

sgs.ai_skill_invoke["feizhi"] = true
sgs.ai_skill_playerchosen["feizhi"] = sgs.ai_skill_playerchosen["shunshui"]

