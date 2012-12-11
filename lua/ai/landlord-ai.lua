-- fangdai
sgs.ai_use_value.FangdaiCard = 9
sgs.dynamic_value.benefit.FangdaiCard = true

local fangdai_skill = {}
fangdai_skill.name = "fangdai"
table.insert(sgs.ai_skills, fangdai_skill)
fangdai_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("FangdaiCard") then
		return sgs.Card_Parse("@FangdaiCard=.")
	end
end
sgs.ai_skill_use_func.FangdaiCard = function(card, use, self)
	local unpreferedCards={}
	local cards=sgs.QList2Table(self.player:getHandcards())

	if self.player:getHp() < 3 then
		local zcards = self.player:getCards("he")
		for _, zcard in sgs.qlist(zcards) do
			if not zcard:inherits("Peach") and not zcard:inherits("ExNihilo") then
				if self:getAllPeachNum()>0 then table.insert(unpreferedCards,zcard:getId()) end
			end
		end
	end

	if #unpreferedCards == 0 then
		if self:getCardsNum("Slash")>1 then
			self:sortByKeepValue(cards)
			for _,card in ipairs(cards) do
				if card:inherits("Slash") then table.insert(unpreferedCards,card:getId()) end
			end
			table.remove(unpreferedCards,1)
		end

		local num=self:getCardsNum("Jink")-1
		if self.player:getArmor() then num=num+1 end
		if num>0 then
			for _,card in ipairs(cards) do
				if card:inherits("Jink") and num>0 then
					table.insert(unpreferedCards,card:getId())
					num=num-1
				end
			end
		end
		for _,card in ipairs(cards) do
			if (card:inherits("Weapon") and self.player:getHandcardNum() < 3) or card:inherits("OffensiveHorse") or
				self:getSameEquip(card, self.player) or	card:inherits("AmazingGrace") or card:inherits("Lightning") then
				table.insert(unpreferedCards,card:getId())
			end
		end
	end

	if #unpreferedCards>0 then
		use.card = sgs.Card_Parse("@FangdaiCard=" .. table.concat(unpreferedCards, "+"))
		return
	end
end

-- youxia
sgs.ai_skill_invoke["youxia"] = function(self, data)
	local move = data:toCardMove()
	if self:isEnemy(move.from) and self.player:isWounded() then
		return true
	elseif self:isFriend(move.from) and self.player:getHandcardNum() > 2 then
		return true
	end
	return false
end
