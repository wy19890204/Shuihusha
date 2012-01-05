-- kaixian
sgs.ai_skill_invoke["kaixian"] = true

-- liba
sgs.ai_skill_invoke["liba"] = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.to)
end

-- zhanchi
sgs.ai_skill_invoke["zhanchi"] = function(self, data)
	if self.player:hasWeapon("crossbow") then
		return true
	else
		return false
	end
end

-- butian
sgs.ai_skill_invoke["@butian"]=function(self,prompt,judge)
	judge = judge or self.player:getTag("Judge"):toJudge()

	if self:needRetrial(judge) then
		local cards = sgs.QList2Table(self.player:getHandcards())
		if self:getUnuseCard() then
			local card_id = self:getUnuseCard():getId()
			return "@ButianCard=" .. card_id
		end
	end
	return "."
end
sgs.ai_skill_askforag["butian"] = function(self, card_ids)
	local judge = self.player:getTag("Judge"):toJudge()
	local cards = {}
	local card_id
	if self:needRetrial(judge) then
		for _, card_id in ipairs(card_ids) do
			local card = sgs.Sanguosha:getCard(card_id)
			table.insert(cards, card)
		end
		card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return card_id
		end
	end
	return card_ids[1]
end

-- longluo
sgs.ai_skill_playerchosen["longluo"] = function(self, targets)
	self:sort(self.friends, "hp")
	return self.friends[1]
end
