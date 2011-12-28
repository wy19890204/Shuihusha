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

-- menghan
local menghan_skill={}
menghan_skill.name = "menghan"
table.insert(sgs.ai_skills, menghan_skill)
menghan_skill.getTurnUseCard = function(self, inclusive)
    local cards = self.player:getCards("he")
    cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
	for _,card in ipairs(cards)  do
		if card:getSuit() == sgs.Card_Spade then
		    local suit = card:getSuitString()
			local number = card:getNumberString()
			local card_id = card:getEffectiveId()
			local card_str = ("ecstasy:menghan[%s:%s]=%d"):format(suit, number, card_id)
			local ecstasy = sgs.Card_Parse(card_str)
			assert(ecstasy)
			return ecstasy
		end
	end
end

-- qiaogong
qiaogong_skill={}
qiaogong_skill.name = "qiaogong"
table.insert(sgs.ai_skills, qiaogong_skill)
qiaogong_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("QiaogongCard") then
		return sgs.Card_Parse("@QiaogongCard=.")
	end
end

sgs.ai_skill_use_func["QiaogongCard"] = function(card, use, self)
	local lost_hp = self.player:getLostHp()
	local enemy_equip = 0
	local target

--[[local has_xiaoji = false
	local xiaoji_equip = 0
	local sunshangxiang
	for _, friend in ipairs(self.friends) do
		if friend:hasSkill("xiaoji") then
			has_xiaoji = true
			xiaoji_equip = self:getCardsNum(".", friend, "e")
			sunshangxiang = friend
			break
		end
	end
	if has_xiaoji then
		local max_equip, max_friend = 0
		local min_equip, min_friend = 5
		for _, friend in ipairs(self.friends) do
			if not friend:hasSkill("xiaoji") then
				if (self:getCardsNum(".", friend, "e") > max_equip) and (self:getCardsNum(".", friend, "e")-xiaoji_equip<=lost_hp) then
					max_equip = self:getCardsNum(".", friend, "e")
					max_friend = friend
				elseif (self:getCardsNum(".", friend, "e") < min_equip) and (xiaoji_equip-self:getCardsNum(".", friend, "e")<=lost_hp) then
					min_equip = self:getCardsNum(".", friend, "e")
					min_friend = friend
				end
			end
		end

		local equips  = {}
		if sunshangxiang and (max_equip~=0 or min_equip~=5) then
			if (max_equip ~= 0) and ((max_equip-self:getCardsNum(".", sunshangxiang, "e"))>=0) then
				use.card = sgs.Card_Parse("@QiaogongCard=.")
				if use.to then use.to:append(sunshangxiang) use.to:append(max_friend) end
				return
			elseif(min_equip ~= 5) and ((self:getCardsNum(".", sunshangxiang, "e")-min_equip)>=0) then
				use.card = sgs.Card_Parse("@QiaogongCard=.")
				if use.to then use.to:append(sunshangxiang) use.to:append(min_friend) end
				return
			end
		end
	end]]

	for _, friend in ipairs(self.friends) do
		for _, enemy in ipairs(self.enemies) do
			if not self:hasSkills(sgs.lose_equip_skill, enemy) then
				local ee = self:getCardsNum(".",enemy,"e")
				if self:isEquip("GaleShell", enemy) then ee = ee - 1 end
				local fe = self:getCardsNum(".",friend, "e")
				if self:isEquip("GaleShell", friend) then fe = fe - 1 end
				if self:hasSkills(sgs.lose_equip_skill, friend) then ee = ee + fe end
				local value = self:evaluateArmor(enemy:getArmor(),friend) - self:evaluateArmor(friend:getArmor(),enemy)
					- self:evaluateArmor(friend:getArmor(),friend) + self:evaluateArmor(enemy:getArmor(),enemy)
				if math.abs(self:getCardsNum(".", enemy, "e")-self:getCardsNum(".", friend, "e")) <= lost_hp and
					self:getCardsNum(".", enemy, "e")>0 and
					(ee > fe or (ee == fe and value>0)) then
					use.card = sgs.Card_Parse("@QiaogongCard=.")
					if use.to then
						use.to:append(friend)
						use.to:append(enemy)
					end
					return
				end
			end
		end
	end

	target = nil
	for _,friend in ipairs(self.friends) do
		if self:isEquip("YitianSword", friend) or (self:isEquip("SilverLion",friend) and friend:isWounded()) 
			or self:hasSkills(sgs.lose_equip_skill, friend) then target = friend break end
	end
	if not target then return end
	for _,friend in ipairs(self.friends) do
		if friend~=target and math.abs(self:getCardsNum(".", friend, "e")-self:getCardsNum(".", target, "e")) <= lost_hp then
			use.card = sgs.Card_Parse("@QiaogongCard=.")
			if use.to then
				use.to:append(friend)
				use.to:append(target)
			end
			return
		end
	end
end
