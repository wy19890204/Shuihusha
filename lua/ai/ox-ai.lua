-- AI for ox package

-- gaolian
-- guibing
local guibing_skill = {}
guibing_skill.name = "guibing"
table.insert(sgs.ai_skills, guibing_skill)
guibing_skill.getTurnUseCard = function(self)
	if self.player:hasFlag("Guibing") or not self:slashIsAvailable() then return end
	local card_str = "@GuibingCard=."
	local slash = sgs.Card_Parse(card_str)
	assert(slash)
	return slash
end
sgs.ai_skill_use_func["GuibingCard"] = sgs.ai_skill_use_func["ZhangshiCard"]
function sgs.ai_cardneed.guibing(to, card, self)
	return card:isBlack()
end

-- tongguan
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
	if self.player:getHp() >= #enemies and #enemies > 0 then
		return "@ZhengfaCard=.->" .. table.concat(enemies, "+")
	else
		return "."
	end
end

-- huyanzhuo
-- dongchaoxueba
-- pangwanchun
-- lianzhu
local lianzhu_skill = {}
lianzhu_skill.name = "lianzhu"
table.insert(sgs.ai_skills, lianzhu_skill)
lianzhu_skill.getTurnUseCard = function(self)
	if not self.player:faceUp() or self.player:hasUsed("LianzhuCard") or self:isWeak() then return end
	if #self.enemies > #self.friends_noself then
		return sgs.Card_Parse("@LianzhuCard=.")
	end
end
sgs.ai_skill_use_func["LianzhuCard"] = function(card,use,self)
	if not use.isDummy then self:speak("lianzhu") end
    use.card=card
end

-- huangxin
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

-- luozhenren
-- butian
sgs.ai_skill_invoke["@butian"]=function(self,prompt,judge)
	judge = judge or self.player:getTag("Judge"):toJudge()

	if self:needRetrial(judge) then
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByUseValue(cards, true)
		return "@ButianCard=" .. cards[1]:getEffectiveId()
	end
	return "."
end
sgs.ai_skill_askforag["butian"] = function(self, card_ids)
	local judge = self.player:getTag("Judge"):toJudge()
	local cards = {}
	local card_id
	if self:needRetrial(judge) then
		for _, card_id in ipairs(card_ids) do
			local card = sgs.Sanguosha:getCard(card_id)
			table.insert(cards, card)
		end
		card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return card_id
		end
	end
	return card_ids[1]
end

-- huaxian
sgs.ai_skill_invoke["huaxian"] = true

-- lili
-- moucai
sgs.ai_skill_invoke["moucai"] = sgs.ai_skill_invoke["qiongtu"]

-- duoming
sgs.ai_skill_invoke["duoming"] = sgs.ai_skill_invoke["liba"]

-- shijin
-- wubang
sgs.ai_skill_invoke["wubang"] = true

-- xiagu
sgs.ai_skill_cardask["@xiagu"] = function(self, data)
	local damage = data:toDamage()
	if self:isFriend(damage.to) then
		if damage.to:hasSkill("fushang") and
			damage.to:getMaxHP() > 3 and damage.damage < 2 then return "." end
		local allcards = self.player:getCards("he")
		for _, card in sgs.qlist(allcards) do
			if card:inherits("EquipCard") then
				return card:getEffectiveId()
			end
		end
	end
	return "."
end
function sgs.ai_cardneed.xiagu(to, card, self)
	return card:inherits("EquipCard")
end

-- lijun
-- xiezhen
-- linniangzi
-- shouwang
shouwang_skill={}
shouwang_skill.name = "shouwang"
table.insert(sgs.ai_skills, shouwang_skill)
shouwang_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("ShouwangCard") then return end
	local slash = self:getCard("Slash")
	if not slash then return end
	return sgs.Card_Parse("@ShouwangCard=" .. slash:getEffectiveId())
end
sgs.ai_skill_use_func["ShouwangCard"] = function(card, use, self)
	self:sort(self.friends,"threat")
	for _, friend in ipairs(self.friends) do
		if friend:getGeneral():isMale() then
			use.card = card
			if use.to then use.to:append(friend) end
			return
		end
	end
end

-- zhongzhen
sgs.ai_skill_invoke["zhongzhen"] = function(self, data)
	local damage = data:toDamage()
	local max_card = self:getMaxCard()
	if max_card and max_card:getNumber() > 11 and self:isEnemy(damage.from) then
		return true
	else
		return false
	end
end
