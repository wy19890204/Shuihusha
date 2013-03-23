-- the ai for events-package
sgs.dynamic_value.control_card.Events = true

function SmartAI:useEventsCard(card, use)
	if card:isKindOf("Tifanshi") then
		for _, enemy in ipairs(self.enemies) do
			if enemy:getHandcardNum() == 1 then
				use.card = card
				if use.to then use.to:append(enemy) end
				return
			end
		end
	elseif card:isKindOf("FuckGaolian") or card:isKindOf("Jiangjieshi") or card:isKindOf("NanaStars") then
		return
	elseif card:isKindOf("Daojia") then
		return math.random(1, 3) == 2
	elseif card:isKindOf("NinedayGirl") then
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isKongcheng() then
				use.card = card
				if use.to then use.to:append(enemy) end
				return
			end
		end
	elseif card:isKindOf("Jiefachang") then
		for _, target in sgs.qlist(self.room:getAllPlayers()) do
			local juds = target:getJudgingArea():length()
			if juds > 0 and self:isFriend(target) then
				if target:containsTrick("indulgence", false) or target:containsTrick("supply_shortage", false) then
					use.card = card
					if use.to then use.to:append(target) end
					return
				end
			end
		end
	end
	return
end

-- jiutianxuannv
sgs.ai_card_intention.NinedayGirl = 55
sgs.dynamic_value.benefit.NinedayGirl = true

-- daojia
sgs.ai_card_intention.Daojia = 50

sgs.ai_skill_use["Daojia"] = function(self, prompt)
	local evc = self:getCard("Daojia")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getArmor() then
			return ("%s->%s"):format(evc:toString(), enemy:objectName())
		end
	end
	return
end

-- jiefachang
sgs.ai_card_intention.Jiefachang = -70
sgs.dynamic_value.benefit.Jiefachang = true

sgs.ai_skill_use["Jiefachang"] = function(self, prompt)
	local evc = self:getCard("Jiefachang")
	for _, friend in ipairs(self.friends) do
		if not friend:faceUp() then
			return ("%s->%s"):format(evc:toString(), friend:objectName())
		end
	end
	return
end

-- tifanshi
sgs.ai_card_intention.Tifanshi = 40

sgs.ai_skill_use["Tifanshi"] = function(self, prompt)
	local evc = self:getCard("Tifanshi")
	local num = 0
	for _, player in sgs.qlist(self.room:getAlivePlayers()) do
		if player:getRole() == "rebel" then
			num = num + 1
		end
	end
	if num > 1 then
		return ("%s->."):format(evc:toString())
	end
	return
end

-- pogaolian
sgs.ai_card_intention.FuckGaolian = 60
sgs.dynamic_value.damage_card.FuckGaolian = true

sgs.ai_skill_use["FuckGaolian"] = function(self, prompt)
	local evc = self:getCard("FuckGaolian")
	if #self.enemies > 0 then
		self:sort(self.enemies, "hp")
		local enemy = self.enemies[1]
		return ("%s->%s"):format(evc:toString(), enemy:objectName())
	end
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

-- zhiqushengchengang
sgs.ai_card_intention.NanaStars = 80
sgs.dynamic_value.benefit.NanaStars = true

sgs.ai_skill_use["NanaStars"] = function(self, prompt)
	local evc = self:getCard("NanaStars")
	for _, target in sgs.qlist(self.room:getAllPlayers()) do
		if target:containsTrick("treasury", false) then
			return ("%s->%s"):format(evc:toString(), target:objectName())
		end
	end
	return
end
sgs.ai_skill_cardask["@7stars"] = function(self, data)
	local damage = data:toDamage()
	if damage.from and self:isEnemy(damage.from) then
		local star = self:getCard("NanaStars")
		return star:getEffectiveId()
	end
	return "."
end

-- zuidajiangmenshen
sgs.dynamic_value.damage_card.Jiangjieshi = true

-- xiaobawang
sgs.ai_card_intention.Xiaobawang = 70

sgs.ai_skill_use["Xiaobawang"] = function(self, prompt)
	local evc = self:getCard("Xiaobawang")
	local player = self.player:getTag("Xiaob"):toPlayer()
	if self:isEnemy(player) and player:getHandcardNum() > 1 then
		return ("%s->."):format(evc:toString())
	end
	return
end
sgs.ai_skill_cardask["@xiaobawang2"] = function(self, data)
	local damage = data:toDamage()
	if self:isEnemy(damage.to) and not damage.to:isNude() then
		local xbw = self:getCard("Xiaobawang")
		return xbw:getEffectiveId()
	end
	return "."
end
