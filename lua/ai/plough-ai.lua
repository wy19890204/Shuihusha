-- this scripts contains the AI classes for generals of plough package

-- bi shang liang shan
function SmartAI:useCardDrivolt(drivolt, use)
--	if self.player:hasSkill("wuyan") then return end
	self:sort(self.friends_noself, "handcard")
	for _, friend in ipairs(self.friends_noself) do
		if not friend:isWounded() and friend:getKingdom() ~= self.player:getKingdom() then
			use.card = drivolt
			if use.to then
				use.to:append(friend)
			end
			break
		end
	end
end

-- tan ting
function SmartAI:useCardWiretap(wiretap, use)
--	if self.player:hasSkill("wuyan") then return end
	use.card = wiretap
	if use.to then
		use.to:append(self.player:getNextAlive())
	end
end

-- xing ci
function SmartAI:useCardAssassinate(ass, use)
--	if self.player:hasSkill("wuyan") then return end
	self:sort(self.enemies, "threat")
	use.card = ass
	for _, enemy in ipairs(self.enemies) do
		if enemy ~= self.player and use.to then
			use.to:append(enemy)
			return
		end
	end
	return
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
		use.card = assassinate
		if use.to and not friend:containsTrick("provistore") then
			use.to:append(friend)
		end
	end
end

-- mengjin
sgs.ai_skill_invoke.mengjin = function(self, data)
	local effect = data:toSlashEffect()
	return not self:isFriend(effect.to)
end

local qiangxi_skill={}
qiangxi_skill.name="qiangxi"
table.insert(sgs.ai_skills,qiangxi_skill)
qiangxi_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("QiangxiCard") then
		return sgs.Card_Parse("@QiangxiCard=.")
	end
end

sgs.ai_skill_use_func["QiangxiCard"] = function(card, use, self)
	local weapon = self.player:getWeapon()
	if weapon then
		local hand_weapon, cards
		cards = self.player:getHandcards()
		for _, card in sgs.qlist(cards) do
			if card:inherits("Weapon") then
				hand_weapon = card
				break
			end
		end
		self:sort(self.enemies)
		for _, enemy in ipairs(self.enemies) do
			if hand_weapon and self.player:inMyAttackRange(enemy) then
				use.card = sgs.Card_Parse("@QiangxiCard=" .. hand_weapon:getId())
				if use.to then
					use.to:append(enemy)
				end
				break
			end
			if self.player:distanceTo(enemy) <= 1 then
				use.card = sgs.Card_Parse("@QiangxiCard=" .. weapon:getId())
				if use.to then
					use.to:append(enemy)
				end
				return
			end
		end
	else
		self:sort(self.enemies, "hp")
		for _, enemy in ipairs(self.enemies) do
			if self.player:inMyAttackRange(enemy) and self.player:getHp() > enemy:getHp() and self.player:getHp() > 2 then
				use.card = sgs.Card_Parse("@QiangxiCard=.")
				if use.to then
					use.to:append(enemy)
				end
				return
			end
		end
	end
end

--shuangxiong

sgs.ai_skill_invoke["shuangxiong"]=function(self,data)
	if self.player:isSkipped(sgs.Player_Play) or self.player:getHp() < 2 then
		return false
	end

	local cards=self.player:getCards("h")
	cards=sgs.QList2Table(cards)

	local handnum=0

	for _,card in ipairs(cards) do
		if self:getUseValue(card)<8 then
			handnum=handnum+1
		end
	end

	handnum=handnum/2
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if (self:getCardsNum("Slash", enemy)+enemy:getHp()<=handnum) and (self:getCardsNum("Slash")>=self:getCardsNum("Slash", enemy)) then return true end
	end

	return self.player:getHandcardNum()>=self.player:getHp()
end
