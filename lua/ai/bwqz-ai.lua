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

-- qiongtu
sgs.ai_skill_invoke["qiongtu"] = function(self, data)
	local target = data:toPlayer()
	return self:isEnemy(target)
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

-- manli
sgs.ai_skill_invoke["manli"] = sgs.ai_skill_invoke["liba"]

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

-- shouge
sgs.ai_skill_invoke["shouge"] = true
shouge_skill={}
shouge_skill.name = "shouge"
table.insert(sgs.ai_skills, shouge_skill)
shouge_skill.getTurnUseCard = function(self)
	if not self.player:isWounded() then
		local cards = self.player:getCards("h")
		cards = sgs.QList2Table(cards)
		for _, acard in ipairs(cards) do
			if acard:inherits("Peach") or acard:inherits("Analeptic") then
				return sgs.Card_Parse("@ShougeCard=" .. acard:getId())
			end
		end
	end
	return
end
sgs.ai_skill_use_func["ShougeCard"] = function(card, use, self)
	use.card = card
end

-- kongying
sgs.ai_skill_invoke["kongying"] = true
sgs.ai_skill_playerchosen["kongying"] = function(self, targets)
	self:sort(self.enemies, "hp")
	return self.enemies[1]
end

-- zhengfa
local zhengfa_skill={}
zhengfa_skill.name = "zhengfa"
table.insert(sgs.ai_skills, zhengfa_skill)
zhengfa_skill.getTurnUseCard = function(self)
    if self.player:hasUsed("ZhengfaCard") or self.player:isKongcheng() then return end
	local enemies = {}
	for _, enemy in ipairs(self.enemies) do
		if self.player:inMyAttackRange(enemy) then
			table.insert(enemies, enemy)
		end
	end
	if #enemies < 2 then return end
	if self.player:getHp() >= #enemies and self.player:getHp() >= 2 then
		local max_card = self:getMaxCard()
		return sgs.Card_Parse("@ZhengfaCard=" .. max_card:getEffectiveId())
	end
end
sgs.ai_skill_use_func["ZhengfaCard"]=function(card,use,self)
	self:sort(self.friends_noself, "handcard")
	for i = #self.friends_noself, 1, -1 do
		if not self.friends_noself[i]:isKongcheng() and self.player:getGender() ~= self.friends_noself[i]:getGender() then
		    if use.to then use.to:append(self.friends_noself[i]) end
            use.card=card
            break
		end
	end
end
sgs.ai_skill_use["@@zhengfa"] = function(self, prompt)
	local enemies = {}
	local i = 0
	for _, enemy in ipairs(self.enemies) do
		if self.player:inMyAttackRange(enemy) then
			table.insert(enemies, enemy:objectName())
			i = i + 1
		end
		if i >= self.player:getHp() then break end
	end
	if self.player:getHp() >= #enemies then
		return "@ZhengfaCard=.->" .. table.concat(enemies, "+")
	else
		return "."
	end
end

-- yongle
local yongle_skill={}
yongle_skill.name = "yongle"
table.insert(sgs.ai_skills, yongle_skill)
yongle_skill.getTurnUseCard = function(self)
    if self.player:hasUsed("YongleCard") then return end
	return sgs.Card_Parse("@YongleCard=.")
end
sgs.ai_skill_use_func["YongleCard"]=function(card,use,self)
	local king = self.room:getKingdoms()
	local enemies = {}
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isKongcheng() then
			table.insert(enemies, enemy)
			if #enemies >= king then break end
		end
	end
	use.card = card
	if use.to then
		for _, enemy in ipairs(enemies) do
			use.to:append(enemy)
		end
	end
end
sgs.ai_cardshow["yongle"] = function(self, requestor)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	return cards[1]
end
