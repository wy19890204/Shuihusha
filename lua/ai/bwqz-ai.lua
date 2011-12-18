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
	return false
end
sgs.ai_skill_playerchosen["yuanyin"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isEnemy(player) and not self:hasSkills(sgs.lose_equip_skill, player) then
			return player
		elseif self:isFriend(player) and self:hasSkills(sgs.lose_equip_skill, player) then
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
