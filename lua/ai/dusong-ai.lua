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

sgs.ai_cardsview["douzhan"] = function(class_name, player)
	if class_name ~= "Slash" then return end
	local cards = sgs.QList2Table(player:getCards("h"))
	local newcards = {}
	for _, card in ipairs(cards) do
		if not card:inherits("Peach") then table.insert(newcards, card) end
	end
	if #newcards<(player:getHp()+1) then return nil end
	if #newcards<2 then return nil end

	local dzcards = {}
	local suit
	for _, card1 in ipairs(newcards) do
		for _, card2 in ipairs(newcards) do
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
		return card_str
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

local ubunb_skill={}
ubunb_skill.name = "ubunb"
table.insert(sgs.ai_skills, ubunb_skill)
ubunb_skill.getTurnUseCard = function(self)
	if not self.player:hasFlag("Ubunb") then
		self.player:drawCards(10)
		self.player:setFlags("Ubunb")
	end
end

local ubunc_skill={}
ubunc_skill.name = "ubunc"
table.insert(sgs.ai_skills, ubunc_skill)
ubunc_skill.getTurnUseCard = function(self)
	if not self.player:hasFlag("Ubunc") then
		self.room:setPlayerProperty(self.player, "maxhp", sgs.QVariant(self.player:getMaxHp() + 1))
		self.player:setFlags("Ubunc")
	end
end
