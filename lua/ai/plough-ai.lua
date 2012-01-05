-- this scripts contains the AI classes for generals of plough package

-- bi shang liang shan
function SmartAI:useCardDrivolt(drivolt, use)
--	if self.player:hasSkill("wuyan") then return end
	self:sort(self.friends_noself, "handcard")
	for _, friend in ipairs(self.friends_noself) do
		if not friend:isWounded() and friend:getKingdom() ~= self.player:getKingdom() then
			if use.to then
				use.card = drivolt
				use.to:append(friend)
				return
			end
			break
		end
	end
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
--		if not self:hasSkills(sgs.masochism_skill, enemy) then
		if enemy:getKingdom() ~= self.player:getKingdom() then
			if use.to then
				use.card = drivolt
				use.to:append(friend)
				return
			end
			break
		end
	end
	return "."
end

-- tan ting
function SmartAI:useCardWiretap(wiretap, use)
--	if self.player:hasSkill("wuyan") then return end
	if use.to then
		use.card = wiretap
		use.to:append(self.player:getNextAlive())
	end
end

-- xing ci
function SmartAI:useCardAssassinate(ass, use)
--	if self.player:hasSkill("wuyan") then return end
	self:sort(self.enemies, "threat")
	for _, enemy in ipairs(self.enemies) do
		if (enemy:hasSkill("fushang") and enemy:getHp() > 3) or enemy:hasSkill("huoshui") then
			if use.to then
				use.card = ass
				use.to:append(enemy)
				return
			end
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if use.to then
			use.card = ass
			use.to:append(enemy)
			return
		end
	end
	return "."
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
	self:sort(self.friends, "hp")
	for _, friend in ipairs(self.friends) do
		if use.to and not friend:containsTrick("provistore") then
			use.card = assassinate
			use.to:append(friend)
			return
		end
	end
end
