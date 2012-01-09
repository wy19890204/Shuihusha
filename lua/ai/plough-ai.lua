-- this scripts contains the AI classes for generals of plough package

-- bi shang liang shan
function SmartAI:useCardDrivolt(drivolt, use)
--	if self.player:hasSkill("wuyan") then return end
	use.card = drivolt
	self:sort(self.enemies, "hp")
	if self.enemies[1]:getHp() == 1 and self.enemies[1]:getKingdom() ~= self.player:getKingdom() then
		if use.to then use.to:append(self.enemies[1]) end
		return
	end
	for _, friend in ipairs(self.friends_noself) do
		if not friend:isWounded() and friend:getKingdom() ~= self.player:getKingdom() then
			if use.to then use.to:append(friend) end
			return
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() == 2 and enemy:getKingdom() ~= self.player:getKingdom() then
			if use.to then use.to:append(enemy) end
			return
		end
	end
	local players = {}
	for _, player in ipairs(sgs.QList2Table(self.room:getOtherPlayers(self.player))) do
		if player:getKingdom() ~= self.player:getKingdom() then table.insert(players, player) end
	end
	local r = math.random(1, #players)
	if use.to then use.to:append(players[r]) end
end

-- tan ting
function SmartAI:useCardWiretap(wiretap, use)
--	if self.player:hasSkill("wuyan") then return end
	local targets = {}
	if #self.friends_noself > 0 then
		self:sort(self.friends_noself, "handcard")
		table.insert(targets, self.friends_noself[#self.friends_noself])
	end
	if #self.enemies > 0 then
		self:sort(self.enemies, "handcard")
		table.insert(targets, self.enemies[#self.enemies])
	end
	use.card = wiretap
	if use.to then
		local r = math.random(1, 2)
		use.to:append(targets[r])
	end
end

-- xing ci
function SmartAI:useCardAssassinate(ass, use)
--	if self.player:hasSkill("wuyan") then return end
	if not self.enemies[1] then return end
	for _, enemy in ipairs(self.enemies) do
		if (enemy:hasSkill("fushang") and enemy:getHp() > 3) or enemy:hasSkill("huoshui") then
			use.card = ass
			if use.to then
				use.to:append(enemy)
			end
			return
		end
	end
	self:sort(self.enemies, "hp")
	local target
	for _, enemy in ipairs(self.enemies) do
		if enemy:hasFlag("ecst") or
			(not self:isEquip("EightDiagram", enemy) and enemy:getHandcardNum() < 6) then
			target = enemy
		end
	end
	use.card = ass
	if use.to then
		if target then
			use.to:append(target)
		else
			use.to:append(self.enemies[1])
		end
	end
end

-- sheng chen gang
function SmartAI:useCardTreasury(card, use)
	if not self.player:containsTrick("treasury") then
		use.card = card
	end
end

-- hai xiao
function SmartAI:useCardTsunami(card, use)
	if self.player:containsTrick("tsunami") then return end
--	if self.player:hasSkill("weimu") and card:isBlack() then return end

	if not self:hasWizard(self.enemies) then--and self.room:isProhibited(self.player, self.player, card) then
		if self:hasWizard(self.friends) then
			use.card = card
			return
		end
		local players = self.room:getAllPlayers()
		players = sgs.QList2Table(players)

		local friends = 0
		local enemies = 0

		for _,player in ipairs(players) do
			if self:objectiveLevel(player) >= 4 then
				enemies = enemies + 1
			elseif self:isFriend(player) then
				friends = friends + 1
			end
		end

		local ratio

		if friends == 0 then ratio = 999
		else ratio = enemies/friends
		end

		if ratio > 1.5 then
			use.card = card
			return
		end
	end
end

-- ji cao tun liang
function SmartAI:useCardProvistore(provistore, use)
--	if self.player:hasSkill("wuyan") then return end
	use.card = provistore
	for _, friend in ipairs(self.friends) do
		if not friend:containsTrick("provistore") and friend:getHandcardNum() > 3 then
			if use.to then
				use.to:append(friend)
			end
			return
		end
	end
	self:sort(self.friends, "hp")
	if use.to then
		use.to:append(self.friends[1])
	end
end
