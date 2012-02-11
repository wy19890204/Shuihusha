-- shemi
sgs.ai_skill_invoke["shemi"] = function(self, data)
	return self.player:getHandcardNum() >= self.player:getHp()
end

-- nongquan
sgs.ai_skill_invoke["nongquan"] = function(self, data)
	local lord = self.room:getLord()
	if lord:hasLordSkill("nongquan") and not lord:faceUp() and self.player:getHandcardNum() > 2 then
		return self:isFriend(lord)
	end
end

-- yuzhong
sgs.ai_skill_choice["yuzhong"] = function(self, choice)
	if choice == "hp+card+cancel" then
		local lord = self.room:getLord()
		if self:isFriend(lord) then
			if lord:getLostHp() > 1 then
				return "hp"
			else
				return "card"
			end
		else
			return "cancel"
		end
	else
		local king = self.room:getKingdoms()
		if #self.friends >= king then
			return "all"
		else
			return "me"
		end
	end
end
sgs.ai_skill_use["@@yuzhong"] = function(self, prompt)
	local king = self.room:getKingdoms()
	local friends = {}
	self:sort(self.friends, "handcard")
	for _, friend in ipairs(self.friends) do
		table.insert(friends, friend)
		if #friends >= king then break end
	end
	return "@YuzhongCard=.->" .. table.concat(friends, "+")
end

-- jiebao
sgs.ai_skill_use["@@jiebao"] = function(self, prompt)
	local enemies = {}
	local i = 0
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isNude() then
			table.insert(enemies, enemy:objectName())
			i = i + 1
		end
		if i == 2 then break end
	end
	if #enemies <= 2 then
		return "@JiebaoCard=.->" .. table.concat(enemies, "+")
	else
		return "."
	end
end

-- dushi
sgs.ai_skill_invoke["dushi"] = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.from)
end
sgs.ai_skill_playerchosen["dushi"] = function(self, targets)
	local mx = 1
	local mxp = self.player:getNextAlive()
	for _, player in sgs.qlist(targets) do
		local kuro = 0
		for _, hcard in sgs.qlist(target:getHandcards()) do
			if hcard:isBlack() then kuro = kuro + 1 end
		end
		if kuro > mx then
			mx = kuro
			mxp = player
		end
	end
	return mxp
end

-- xiaduo
sgs.ai_skill_cardask["@xiaduo"] = function(self, data)
	local caninvoke = false
	local damage = data:toDamage()
	for _, enemy in sgs.qlist(self.room:getOtherPlayers(damage.to)) do
		if self:isEnemy(enemy) and self.player:distanceTo(enemy) == 1 then
			self.xiaduotarget = enemy
			caninvoke = true
			break
		end
	end
	if not caninvoke then return end
	local cards = self.player:getCards("he")
	for _, card in sgs.qlist(cards) do
		if card:inherits("EquipCard") then
			return card:getEffectiveId()
		end
	end
	return "."
end
sgs.ai_skill_playerchosen["xiaduo"] = function(self, targets)
	local target = self.xiaduotarget
	return target
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

