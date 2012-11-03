-- AI for dragon package

-- zaochuan
local zaochuan_skill = {}
zaochuan_skill.name = "zaochuan"
table.insert(sgs.ai_skills, zaochuan_skill)
zaochuan_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards,true)
	for _,acard in ipairs(cards)  do
		if acard:getSuit() == sgs.Card_Club then
			card = acard
			break
		end
	end
	if not card then return nil end
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("iron_chain:zaochuan[club:%s]=%d"):format(number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end

-- yuanyin
local yuanyin_skill = {}
yuanyin_skill.name = "yuanyin"
table.insert(sgs.ai_skills, yuanyin_skill)
yuanyin_skill.getTurnUseCard = function(self)
	if not self:slashIsAvailable(self.player) then return end
	for _, enemy in ipairs(self.enemies) do
		local weapon = enemy:getWeapon()
		if weapon then
			return sgs.Card_Parse("@YuanyinCard=.")
		end
	end
end
sgs.ai_skill_use_func["YuanyinCard"] = function(card, use, self)
	self:sort(self.enemies, "threat")

	for _, friend in ipairs(self.friends_noself) do
		if friend:getWeapon() and self:hasSkills(sgs.lose_equip_skill, friend) then
			for _, enemy in ipairs(self.enemies) do
				if self.player:canSlash(enemy) then
					use.card = card
					if use.to then
						use.to:append(friend)
						use.to:append(enemy)
					end
					return
				end
			end
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if not self.room:isProhibited(self.player, enemy, card)
			and not self:hasSkill(sgs.lose_equip_skill, enemy)
			and enemy:getWeapon() then

			local enemies = self.enemies
			self:sort(enemies, "handcard")
			for _, enemy2 in ipairs(enemies) do
				if self.player:canSlash(enemy2) then
					use.card = card
					if use.to then
						use.to:append(enemy)
						if enemy ~= enemy2 then
							use.to:append(enemy2)
						end
					end
					return
				end
			end
		end
	end
	return "."
end

-- yuanyin-slash&jink
sgs.ai_skill_invoke["yuanyin"] = function(self, data)
	local asked = data:toString()
	for _, enemy in ipairs(self.enemies) do
		local weapon = enemy:getWeapon()
		local armor = enemy:getArmor() or enemy:getDefensiveHorse() or enemy:getOffensiveHorse()
		if (asked == "slash" and weapon) or (asked == "jink" and armor) then
			return true
		end
	end
	if self.player:getHp() < 2 then
		for _, target in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			local weapon = target:getWeapon()
			local armor = target:getArmor() or target:getDefensiveHorse() or target:getOffensiveHorse()
			if (asked == "slash" and weapon) or (asked == "jink" and armor) then
				return true
			end
		end
	end
	return false
end
sgs.ai_skill_playerchosen["yuanyin"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isEnemy(player) and not self:hasSkills(sgs.lose_equip_skill, player) then
			return player
		elseif self:isFriend(player) and (self:hasSkills(sgs.lose_equip_skill, player) or self.player:getHp() < 2) then
			return player
		end
	end
end
sgs.ai_skill_cardchosen["yuanyin"] = function(self, who)
	local ecards = who:getCards("e")
	ecards = sgs.QList2Table(ecards)
	for _, unweapon in ipairs(ecards) do
		if not unweapon:inherits("Weapon") then
			return unweapon
		end
	end
end

-- nusha
nusha_skill={}
nusha_skill.name = "nusha"
table.insert(sgs.ai_skills, nusha_skill)
nusha_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("NushaCard") then return end
	local enum = 0
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHandcardNum() > enum then
			enum = enemy:getHandcardNum()
		end
	end
	local fnum = 0
	for _, friend in ipairs(self.friends_noself) do
		if friend:getHandcardNum() > fnum then
			fnum = friend:getHandcardNum()
		end
	end
	if enum >= fnum then
		local slash = self:getCardId("Slash")
		if slash then
			return sgs.Card_Parse("@NushaCard=" .. slash)
		end
	end
	return
end
sgs.ai_skill_use_func["NushaCard"] = function(card, use, self)
	local enum = self.enemies[1]
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHandcardNum() > enum:getHandcardNum() then
			enum = enemy
		end
	end
	use.card = card
	if use.to then
		use.to:append(enum)
	end
end

-- kongying
sgs.ai_skill_invoke["kongying"] = true
sgs.ai_skill_playerchosen["kongying"] = function(self, targets)
	self:sort(self.enemies, "hp")
	return self.enemies[1]
end
function sgs.ai_cardneed.kongying(to, card, self)
	return card:inherits("Jink")
end
