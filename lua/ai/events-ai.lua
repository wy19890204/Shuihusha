function SmartAI:useEventsCard(card, use)
	if card:inherits("Tifanshi") then
		return
	elseif card:inherits("FuckGaolian") or card:inherits("Jiangjieshi") or card:inherits("NanaStars") then
		return
	elseif card:inherits("Daojia") then
		return math.random(1, 3) == 2
	elseif card:inherits("NinedayGirl") then
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isKongcheng() then
				use.card = card
				if use.to then use.to:append(enemy) end
				return
			end
		end
	elseif card:inherits("Jiefachang") then
		for _, friend in ipairs(self.friends) do
			if friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage") then
				use.card = card
				if use.to then use.to:append(friend) end
				return
			end
		end
	end
	return
end

-- daojia
sgs.ai_skill_use["Daojia"] = function(self, prompt)
	local evc = self:getCard("Daojia")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getArmor() then
			return ("%s->%s"):format(evc:toString(), enemy:objectName())
		end
	end
	return
end

-- fuckgaolian
sgs.ai_skill_use["FuckGaolian"] = function(self, prompt)
	local evc = self:getCard("FuckGaolian")
	self:sort(self.enemies, "hp")
	local enemy = self.enemies[1]
	return ("%s->%s"):format(evc:toString(), enemy:objectName())
end
sgs.ai_skill_cardask["@fuckl"] = function(self, data)
	local judge = data:toJudge()
	local fuck = self:getCard("FuckGaolian")
	if self:needRetrial(judge) then
		local cards = {}
		table.insert(cards, fuck)
		local card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return fuck:getEffectiveId()
		end
	end
	return "."
end

-- nanastars
sgs.ai_skill_use["NanaStars"] = function(self, prompt)
	local evc = self:getCard("NanaStars")
	for _, target in sgs.qlist(self.room:getAllPlayers()) do
		if target:containsTrick("treasury") then
		return ("%s->%s"):format(evc:toString(), target:objectName())
	end
	return
end
sgs.ai_skill_cardask["@7stars"] = function(self, data)
	local damage = data:toDamage()
	if self:isFriend(damage.from) then return end
	local star = self:getCard("NanaStars")
	return star:getEffectiveId()
end
