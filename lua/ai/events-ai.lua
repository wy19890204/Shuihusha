function SmartAI:useEventsCard(card, use)
	if card:inherits("Tifanshi") then
		return
	elseif card:inherits("Daojia") then
		return
	elseif card:inherits("FuckGaolian") then
		return
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
sgs.ai_skill_use["daojia"] = function(self, prompt)
	local evc = self:getCard("Daojia")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getArmor() then
			return ("%s->%s"):format(evc:toString(), enemy:objectName())
		end
	end
	return
end

-- fuckgaolian
sgs.ai_skill_use["fuckgaolian"] = function(self, prompt)
	local evc = self:getCard("FuckGaolian")
	self:sort(enemies, "hp")
	local enemy = enemies[1]
	return ("%s->%s"):format(evc:toString(), enemy:objectName())
end
