-- duijue
sgs.ai_skill_use["@@duijue"] = function(self, prompt)
	self:sort(self.enemies, "hp")
	local n1 = self:getCardsNum("Slash")
	local final
	for _, enemy in ipairs(self.enemies) do
		if n1 + 1 > self:getCardsNum("Slash", enemy) then
			final = enemy
			break
		end
	end
	if final then
		return "@DuijueCard=.->"..final:objectName()
	else
		return "."
	end
end

-- maidao
maidao_skill={}
maidao_skill.name = "maidao"
table.insert(sgs.ai_skills, maidao_skill)
maidao_skill.getTurnUseCard = function(self)
	if self.player:getWeapon() then
		local cards = self.player:getCards("h")
		cards = sgs.QList2Table(cards)
		for _, acard in ipairs(cards)  do
			if acard:inherits("Weapon") then
				return sgs.Card_Parse("@MaidaoCard=" .. acard:getId())
			end
		end
	end
	return
end
sgs.ai_skill_use_func["MaidaoCard"] = function(card, use, self)
	use.card = card
end

-- fengmang
sgs.ai_skill_invoke["fengmang"] = true
sgs.ai_skill_playerchosen["fengmang"] = function(self, targets)
	self:sort(self.enemies, "hp")
	for _, target in ipairs(self.enemies) do
		if not self:slashProhibit(nil, target) then
			return target
		end
	end
	return self.enemies[1]
end

-- shalu&shunshui&lihun
sgs.ai_skill_invoke["shalu"] = true
sgs.ai_skill_invoke["shunshui"] = true
sgs.ai_skill_invoke["lihun"] = function(self, data)
	local from = data:toPlayer()
	return self:isEnemy(from)
end
sgs.ai_skill_playerchosen["lihun"] = sgs.ai_skill_playerchosen["taolue"]

-- shenhuo
sgs.ai_skill_invoke["shenhuo"] = true
local shenhuo_skill={}
shenhuo_skill.name = "shenhuo"
table.insert(sgs.ai_skills, shenhuo_skill)
shenhuo_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards,true)
	for _,acard in ipairs(cards)  do
		if acard:isRed() and acard:inherits("TrickCard") then
			card = acard
			break
		end
	end
	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("fire_attack:shenhuo[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end

-- tongxia
sgs.ai_skill_invoke["tongxia"] = true
sgs.ai_skill_askforag["tongxia"] = function(self, card_ids)
	return card_ids[1]
end
sgs.ai_skill_playerchosen["tongxia"] = function(self, targets)
	local card = self.player:getTag("TongxiaCard"):toCard()
	self:sort(self.enemies, "hp")
	for _, enemy in ipairs(self.enemies) do
		if card:inherits("GaleShell") or card:inherits("Shit") then
			return enemy
		end
	end
	self:sort(self.enemies, "defense")
	for _, friend in ipairs(self.friends) do
		if card:inherits("EquipCard") then
			if (card:inherits("Weapon") and not friend:getWeapon()) or
				(card:inherits("Armor") and (not friend:getArmor() or self:isEquip("GaleShell", friend))) or
				(card:inherits("DefensiveHorse") and not friend:getDefensiveHorse()) or
				(card:inherits("OffensiveHorse") and not friend:getOffensiveHorse()) then
				return friend
			end
		else
			return self.player
		end
	end
	return self.friends_noself[1]
end

-- binggong
sgs.ai_skill_use["@@binggong"] = function(self, prompt)
	local num = self.player:getMark("Bingo")
	if num < 3 and self.player:isWounded() then return "." end
	self:sort(self.friends_noself, "defense")
	local target = self.friends_noself[1]
	if not target then return "." end
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local card_ids = {}
	for i = 1, num do
		table.insert(card_ids, cards[i]:getEffectiveId())
	end
	return "@BinggongCard=" .. table.concat(card_ids, "+") .. "->" .. target:objectName()
end

-- shenpan
sgs.ai_skill_invoke["shenpan"] = function(self, data)
	local judge = data:toJudge()
	if not self:needRetrial(judge) then return false end
	local wizard_friend
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player == judge.who then break end
		if self:isFriend(player) then
			if (player:hasSkill("yixing") and self:getYixingCard(judge) > -1) or
				player:hasSkill("butian") then
				wizard_friend = player
				break
			end
		end
	end
	return not wizard_friend
end

-- yixian
sgs.ai_skill_invoke["yixian"] = function(self, data)
	local damage = data:toDamage()
	if self:isFriend(damage.to) then
		return not self:hasSkills(sgs.masochism_skill, damage.to)
	elseif self:isEnemy(damage.to) then
		return self:hasSkills(sgs.masochism_skill, damage.to)
	end
end

-- feiqiang
feiqiang_skill={}
feiqiang_skill.name = "feiqiang"
table.insert(sgs.ai_skills, feiqiang_skill)
feiqiang_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("FeiqiangCard") then
		local cards = self.player:getCards("he")
		cards = sgs.QList2Table(cards)
		for _, acard in ipairs(cards)  do
			if acard:inherits("Weapon") then
				return sgs.Card_Parse("@FeiqiangCard=" .. acard:getEffectiveId())
			end
		end
	end
	return
end
sgs.ai_skill_use_func["FeiqiangCard"] = function(card, use, self)
	self:sort(self.enemies, "defense")
	use.card = card
	if use.to then
		use.to:append(self.enemies[1])
	end
end
sgs.ai_skill_choice["feiqiang"] = function(self, choices)
	return "gong"
end

-- shentou
shentou_skill={}
shentou_skill.name = "shentou"
table.insert(sgs.ai_skills, shentou_skill)
shentou_skill.getTurnUseCard = function(self)
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
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("snatch:shentou[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end

--[[
-- maida0
maida0_skill={}
maida0_skill.name = "maida0"
table.insert(sgs.ai_skills, maida0_skill)
maida0_skill.getTurnUseCard = function(self)
	local yangzhi = self.room:findPlayerBySkillName("maidao")
	if yangzhi and not yangzhi:getPile("knife"):isEmpty() and self:isEnemy(yangzhi) then
		local cards = self.player:getCards("h")
		cards = sgs.QList2Table(cards)
		if #cards < 4 then return end
		self:sortByUseValue(cards, true)
		local card_ids = {}
		for i = 1, 2 do
			if self:getUseValue(cards[i]) > 4 then return end
			table.insert(card_ids, cards[i]:getEffectiveId())
		end
		return "@Maida0Card=" .. table.concat(card_ids, "+")
	end
	return
end
sgs.ai_skill_use_func["Maida0Card"] = function(card, use, self)
	local yangzhi = self.room:findPlayerBySkillName("maidao")
	use.card = card
	if yangzhi and not yangzhi:getPile("knife"):isEmpty() and use.to then
		use.to:append(yangzhi)
	end
end
--]]
