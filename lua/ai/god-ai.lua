-- this script file contains the AI classes for gods

-- xianji
sgs.ai_skill_cardask["@xianji"] = sgs.ai_skill_cardask["@fuji"]

-- feihuang
sgs.ai_skill_use["@@feihuang"] = function(self, prompt)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local num = self.player:getHandcardNum() - self.player:getHp()
	local shis = {}
	local i = 0
	for _, card in ipairs(cards) do
		table.insert(shis, card:getId())
		i = i + 1
		if i == num then break end
	end
	return "@FeihuangCard=" .. table.concat(shis, "+") .. "->."
end

-- meiyu
meiyu_skill={}
meiyu_skill.name = "meiyu"
table.insert(sgs.ai_skills, meiyu_skill)
meiyu_skill.getTurnUseCard = function(self)
	if self.player:getPile("stone"):isEmpty() then return end
	return sgs.Card_Parse("@MeiyuCard=.")
end
sgs.ai_skill_use_func["MeiyuCard"] = function(card, use, self)
	self:sort(self.enemies, "handcard")
	for _, enemy in ipairs(self.enemies) do
		if self:getCardsNum("Jink", enemy) == 0 then
			if use.to then use.to:append(enemy) end
			use.card = card
			break
		end
	end
end

-- huafo
sgs.ai_view_as["huafo"] = function(card, player, card_place, class_name)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()

	if card:inherits("BasicCard") then
		if class_name == "Analeptic" then
			return ("analeptic:huafo[%s:%s]=%d"):format(suit, number, card_id)
		else
			return ("slash:huafo[%s:%s]=%d"):format(suit, number, card_id)
		end
	end
end

-- jiebei
sgs.ai_skill_cardask["@jiebei"] = function(self, data)
	local unuse = self:getUnuseCard()
	return unuse:getEffectiveId() or "."
end
sgs.ai_skill_playerchosen["jiebei"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isFriend(player) then
			return player
		end
	end
	for _, player in sgs.qlist(targets) do
		return player
	end
end
sgs.ai_skill_invoke["jiebei"] = true
