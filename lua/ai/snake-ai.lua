-- AI for snake package

-- muhong
-- wuzu

-- xuanzan
-- konghe
sgs.ai_skill_invoke["konghe"] = function(self, data)
	local damage = data:toDamage()
	if damage then
		local cur = self.room:getCurrent()
		return cur:getNextAlive():objectName() ~= self.player:objectName()
	else
		local source = data:toPlayer()
		if source then
			if self:isFriend(source) and
				(source:containsTrick("indulgence", false) or source:containsTrick("supply_shortage", false)) then
				self.konghe = "k3"
				return true
			elseif self:isEnemy(source) then
				if source:containsTrick("indulgence", false) or source:containsTrick("supply_shortage", false) then
					return false
				elseif source:getHandcardNum() >= source:getHp() then
					self.konghe = "k2"
					return true
				else
					self.konghe = "k1"
					return true
				end
			end
		end
		return false
	end
end

sgs.ai_skill_choice["konghe"] = function(self, choice, data)
	return self.konghe
end

-- oupeng
-- zhanchi
sgs.ai_skill_invoke["zhanchi"] = function(self, data)
	if self.player:hasWeapon("crossbow") and math.random(1, 3) == 1 then
		return true
	else
		return false
	end
end

-- lingzhen
-- paohong
local paohong_skill={}
paohong_skill.name = "paohong"
table.insert(sgs.ai_skills, paohong_skill)
paohong_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local thunder_card
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		if card:objectName() == "slash" and card:isBlack() then
			thunder_card = card
			break
		end
	end
	if thunder_card then
		local suit = thunder_card:getSuitString()
		local number = thunder_card:getNumberString()
		local card_id = thunder_card:getEffectiveId()
		local card_str = ("thunder_slash:paohong[%s:%s]=%d"):format(suit, number, card_id)
		return sgs.Card_Parse(card_str)
	end
end
sgs.ai_filterskill_filter["paohong"] = function(card, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:objectName() == "slash" and card:isBlack() then return ("thunder_slash:paohong[%s:%s]=%d"):format(suit, number, card_id) end
end

-- baoxu
-- sinue
sgs.ai_skill_use["@@sinue"] = function(self, prompt)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	for _, enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy) == 1 then
			return "@SinueCard=" .. cards[1]:getEffectiveId() .. "->."
		end
	end
	return "."
end

-- fanrui
-- jindajian
-- fangzao
local fangzao_skill={}
fangzao_skill.name = "fangzao"
table.insert(sgs.ai_skills, fangzao_skill)
fangzao_skill.getTurnUseCard = function(self)
	if not self.player:hasUsed("FangzaoCard") then
		self:sort(self.enemies, "handcard2")
		if #self.enemies == 0 or self.enemies[1]:isKongcheng() then return end
		return sgs.Card_Parse("@FangzaoCard=.")
	end
end
sgs.ai_skill_use_func["FangzaoCard"] = function(card,use,self)
	self:sort(self.enemies, "handcard2")
	if use.to then
		use.to:append(self.enemies[1])
	end
	use.card=card
end

function fangzao_card(self, card, name)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local id = card:getEffectiveId()

	local card_str = ("%s:fangzao[%s:%s]=%d"):format(name, suit, number, id)
	local angzao = sgs.Card_Parse(card_str)
	assert(angzao)
	return angzao
end

fangzao2_skill={}
fangzao2_skill.name = "fangzao"
table.insert(sgs.ai_skills, fangzao2_skill)
fangzao2_skill.getTurnUseCard = function(self)
	if not self.player:hasFlag("fangzao") or self.player:isKongcheng() then return end
	local card_id = self.player:getMark("fangzao")
	local copycard = sgs.Sanguosha:getCard(card_id)
	if not copycard then return end

	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local card = cards[1]
--	if card:objectName() ~= "peach" then return nil end
	return fangzao_card(self, card, card:objectName())
end

-- jiangxin
sgs.ai_skill_invoke["jiangxin"] = true

-- houjian
-- feizhen
local feizhen_skill = {}
feizhen_skill.name = "feizhen"
table.insert(sgs.ai_skills, feizhen_skill)
feizhen_skill.getTurnUseCard = function(self)
	if not self:slashIsAvailable(self.player) then return end
	for _, enemy in ipairs(self.enemies) do
		local weapon = enemy:getWeapon()
		if weapon then
			return sgs.Card_Parse("@FeizhenCard=.")
		end
	end
end
sgs.ai_skill_use_func["FeizhenCard"] = function(card, use, self)
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

-- feizhen-slash&jink
sgs.ai_skill_invoke["feizhen"] = function(self, data)
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
sgs.ai_skill_playerchosen["feizhen"] = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isEnemy(player) and not self:hasSkills(sgs.lose_equip_skill, player) then
			return player
		elseif self:isFriend(player) and (self:hasSkills(sgs.lose_equip_skill, player) or self.player:getHp() < 2) then
			return player
		end
	end
end
sgs.ai_skill_cardchosen["feizhen"] = function(self, who)
	local ecards = who:getCards("e")
	ecards = sgs.QList2Table(ecards)
	for _, unweapon in ipairs(ecards) do
		if not unweapon:inherits("Weapon") then
			return unweapon
		end
	end
end

-- dengfei
-- jiejiu
sgs.ai_skill_use["@@jiejiu"] = function(self, prompt)
	local damage = self.player:getTag("Jiejiu"):toDamage()
	local source = damage.from
	if self:isEnemy(damage.to) then return "." end

	if self.player:getHandcardNum() > 2 or damage.to == self.player then
		local max_card = self:getMaxCard()
		if max_card then
			local max_care = self:getMaxCard(source)
			if (max_care and max_card:getNumber() > max_care:getNumber()) or  max_card:getNumber() > 9 then
				return "@JiejiuCard=" .. max_card:getEffectiveId() .. "->."
			end
		end
	end
	return "."
end

-- huangfuduan
-- xiangma
sgs.ai_skill_use["@@xiangma"] = function(self, prompt)
	local who
	for _, player in sgs.qlist(self.room:getAllPlayers()) do
		if player:hasFlag("xmtarget") then
			who = player
			break
		end
	end
	if not who or self:isEnemy(who) then return "." end
	local target
	for _, player in sgs.qlist(self.room:getOtherPlayers(who)) do
		if player:getOffensiveHorse() or player:getDefensiveHorse() then
			target = player
			break
		end
	end
	if target then
		return "@XiangmaCard=." .. "->" .. target:objectName()
	else
		return "."
	end
end

-- yima
sgs.ai_skill_invoke["yima"] = true
sgs.ai_skill_choice["yima"] = function(self, choice, data)
	local player = data:toPlayer()
	if self:isFriend(player) then
		return "yes"
	else
		return "no"
	end
end

-- liangshijie
-- sougua
sgs.ai_skill_use["@@sougua"] = function(self, prompt)
	local enemies = {}
	self:sort(self.enemies, "handcard")
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isKongcheng() then
			table.insert(enemies, enemy:objectName())
			if #enemies >= 4 then break end
		end
	end
	if #enemies < 2 then return "." end
	if #enemies == 2 and math.random(0,1) == 1 then return "." end
	return "@SouguaCard=.->" .. table.concat(enemies, "+")
end

-- liushou

-- suyuanjing
-- zhaoan
-- fuxu
sgs.ai_skill_invoke["fuxu"] = function(self, data)
	local daamge = data:toDamage()
	if self:isEnemy(daamge.to) then
		return math.random(0, 1) == 1
	else
		return true
	end
end
sgs.ai_skill_cardask["@fuxu"] = function(self, data)
	local damage = data:toDamage()
	if self:isEnemy(damage.to) or self.player:isKongcheng() then return "." end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards, false)
	for _, acard in ipairs(cards) do
		if acard:isRed() then
			return acard:getEffectiveId()
		end
	end
end

