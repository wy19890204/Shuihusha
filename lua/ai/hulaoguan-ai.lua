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

sgs.ai_skill_invoke.xiuluo = function(self, data)
	local hand_card = self.player:getHandcards()
	local judge_list = self.player:getCards("j")
	for _, judge in sgs.qlist(judge_list) do
		for _, card in sgs.qlist(hand_card) do
			if card:getSuit() == judge:getSuit() then return true end
		end
	end
	
	return false
end

sgs.ai_skill_cardask["@xiuluo"] = function(self)
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if card:getSuitString() == parsedPrompt[2] then return "$"..card:getEffectiveId() end
	end
	return "."
end

-- zombie
sgs.ai_filterskill_filter["ganran"] = function(card, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:getTypeId() == sgs.Card_Equip then return ("iron_chain:ganran[%s:%s]=%d"):format(suit, number, card_id) end
end
