-- yushui
yushui_skill={}
yushui_skill.name = "yushui"
table.insert(sgs.ai_skills, yushui_skill)
yushui_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("YushuiCard") or not self.player:isWounded() then return end
	local cards = self.player:getCards("he")
    cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards)  do
		if card:getSuit() == sgs.Card_Heart then
		    return sgs.Card_Parse("@YushuiCard=" .. card:getEffectiveId())
		end
	end
	return
end
sgs.ai_skill_use_func["YushuiCard"] = function(card, use, self)
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() and friend:getGeneral():isMale() then
			use.card = card
			if use.to then use.to:append(friend) end
			return
		end
	end
end

-- zhensha
sgs.ai_skill_cardask["@zhensha"] = function(self, data)
	local carduse = data:toCardUse()
	if self:isFriend(carduse.from) then return "." end
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	for _, fcard in ipairs(cards) do
		if fcard:getSuit() == sgs.Card_Spade then
			if carduse.from:isLord() or carduse.from:getHp() > 1 then
				return fcard:getEffectiveId()
			end
		end
	end
	return "."
end

function SmartAI:isNoZhenshaMark()
	for _, player in sgs.qlist(self.room:getAlivePlayers()) do
		if self:isEnemy(player) and not player:isKongcheng() and player:getMark("@vi") > 0 then return false end
	end
	return true
end

-- shengui
function sgs.ai_trick_prohibit.shengui(card, self, to)
	return not to:faceUp() and self.player:getGeneral():isMale()
end

-- meicha
meicha_skill={}
meicha_skill.name = "meicha"
table.insert(sgs.ai_skills, meicha_skill)
meicha_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards,true)
	for _,acard in ipairs(cards)  do
		if (acard:getSuit() == sgs.Card_Club) then
			card = acard
			break
		end
	end
	if not card then return nil end
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("analeptic:meicha[club:%s]=%d"):format(number, card_id)
	local analeptic = sgs.Card_Parse(card_str)
	assert(analeptic)
	return analeptic
end
sgs.ai_view_as["meicha"] = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()

	if card_place ~= sgs.Player_Equip and card:getSuit() == sgs.Card_Club then
		return ("analeptic:meicha[%s:%s]=%d"):format(suit, number, card_id)
	end
end

-- fanwu
sgs.ai_skill_use["@@fanwu"] = function(self, prompt)
	if self.player:isKongcheng() then return "." end
	local card = self.player:getRandomHandCard()
	local damage = self.player:getTag("FanwuStruct"):toDamage()
	local dmg = damage.damage
	if damage.to:getArmor() and damage.to:getArmor():objectName() == "vine" and damage.nature == sgs.DamageStruct_Fire then dmg = dmg + 1 end
	if damage.to:getArmor() and damage.to:getArmor():objectName() == "silver_lion" then dmg = 1 end
	if damage.to:getRole() == "loyal" and self:isEnemy(damage.to) and damage.to:getHp() - dmg < 1
		and self.room:getLord() and self.room:getLord():getGeneral():isMale() then
		return "@FanwuCard=" .. card:getEffectiveId() .. "->" .. self.room:getLord():objectName()
	end
	if damage.to:getRole() == "rebel" and self:isEnemy(damage.from) and damage.to:getHp() - dmg < 1 then
		self:sort(self.friends, "handcard")
		for _, t in ipairs(self.friends) do
			if t:getGeneral():isMale() then
				return "@FanwuCard=" .. card:getEffectiveId() .. "->" .. t:objectName()
			end
		end
	end
	return "."
end

-- panxin
sgs.ai_skill_invoke["panxin"] = sgs.ai_skill_invoke["dujian"]

-- foyuan
function sgs.ai_slash_prohibit.foyuan(self)
	if self.player:getGeneral():isMale() and not self.player:hasEquip() then return true end
end

-- banzhuang
local banzhuang_skill={}
banzhuang_skill.name = "banzhuang"
table.insert(sgs.ai_skills, banzhuang_skill)
banzhuang_skill.getTurnUseCard = function(self,inclusive)
    local cards = self.player:getHandcards()
    cards = sgs.QList2Table(cards)
	for _,card in ipairs(cards)  do
		if card:getSuit() == sgs.Card_Heart or inclusive then
			local number = card:getNumberString()
			local card_id = card:getEffectiveId()
			local card_str = ("ex_nihilo:banzhuang[heart:%s]=%d"):format(number, card_id)
			local exnihilo = sgs.Card_Parse(card_str)
			assert(exnihilo)
			return exnihilo
		end
	end
end

-- zhuying
local zhuying_skill={}
zhuying_skill.name = "zhuying"
table.insert(sgs.ai_skills, zhuying_skill)
zhuying_skill.getTurnUseCard = function(self)
	if not self.player:isWounded() then return end
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local anal_card
	self:sortByUseValue(cards, true)
	for _,card in ipairs(cards)  do
		if card:inherits("Analeptic") then
			anal_card = card
			break
		end
	end
	if anal_card then
		local suit = anal_card:getSuitString()
		local number = anal_card:getNumberString()
		local card_id = anal_card:getEffectiveId()
		local card_str = ("peach:zhuying[%s:%s]=%d"):format(suit, number, card_id)
		local peach = sgs.Card_Parse(card_str)
		return peach
	end
end
sgs.ai_filterskill_filter["zhuying"] = function(card, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:inherits("Analeptic") then return ("peach:zhuying[%s:%s]=%d"):format(suit, number, card_id) end
end

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

-- zishi
sgs.ai_skill_use["@@zishi"] = function(self, prompt)
	if self.player:isKongcheng() then return "." end
	local target = self.player:getTag("ZishiSource"):toPlayer()
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	for _,card in ipairs(cards) do
		if card:isBlack() and self:getUseValue(card) < 6 then
		    if (self:isFriend(target) and target:getHandcardNum() < 3) or self:isEnemy(target) then
				return "@ZishiCard=" .. card:getEffectiveId() .. "->."
			end
		end
	end
	return "."
end
sgs.ai_skill_choice["zishi"] = function(self, choice)
	local source = self.player:getTag("ZishiSource"):toPlayer()
	if self:isFriend(source) then
		return "duo"
	else
		return "shao"
	end
end

-- zhangshi
local zhangshi_skill={}
zhangshi_skill.name="zhangshi"
table.insert(sgs.ai_skills,zhangshi_skill)
zhangshi_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("ZhangshiCard") or not self:slashIsAvailable() then return end
	local card_str = "@ZhangshiCard=."
	local slash = sgs.Card_Parse(card_str)
	assert(slash)
	return slash
end
sgs.ai_skill_use_func["ZhangshiCard"]=function(card,use,self)
	self:sort(self.enemies, "defense")
	local target_count=0
	for _, enemy in ipairs(self.enemies) do
		if ((self.player:canSlash(enemy, not no_distance)) or
			(use.isDummy and (self.player:distanceTo(enemy)<=self.predictedRange))) and
			self:objectiveLevel(enemy)>3 and
			self:slashIsEffective(card, enemy) then
			use.card=card
			if use.to then
				use.to:append(enemy)
			end
			target_count=target_count+1
			if self.slash_targets<=target_count then return end
		end
	end
end
sgs.ai_skill_invoke["zhangshi"] = function(self, data)
	local cards = self.player:getHandcards()
	local slash = self:getCard("Slash")
	if slash then return false end
	return true
end
sgs.ai_skill_cardask["@zhangshi"] = function(self, data)
	local who = data:toPlayer()
	if not self:isFriend(who) then return "." end
	return self:getCardId("Slash") or "."
end

-- suocai
local suocai_skill={}
suocai_skill.name = "suocai"
table.insert(sgs.ai_skills, suocai_skill)
suocai_skill.getTurnUseCard = function(self)
    if not self.player:hasUsed("SuocaiCard") and not self.player:isKongcheng() then
		local max_card = self:getMaxCard()
		if max_card and self.player:getHandcardNum() > 2 then
			return sgs.Card_Parse("@SuocaiCard=" .. max_card:getEffectiveId())
		end
	end
end
sgs.ai_skill_use_func["SuocaiCard"]=function(card,use,self)
	self:sort(self.enemies, "handcard")
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isKongcheng() and enemy:getGeneral():isMale() then
            use.card = card
		    if use.to then use.to:append(enemy) end
            return
		end
	end
end

-- eyan
local eyan_skill={}
eyan_skill.name = "eyan"
table.insert(sgs.ai_skills, eyan_skill)
eyan_skill.getTurnUseCard = function(self)
	local jink = self:getCard("Jink")
    if self.player:hasUsed("EyanCard") or not jink then return end
	return sgs.Card_Parse("@EyanCard=.")
end
sgs.ai_skill_use_func["EyanCard"]=function(card,use,self)
	self:sort(self.enemies, "handcard")
	for _, enemy in ipairs(self.enemies) do
		if enemy:inMyAttackRange(self.player) and enemy:getGeneral():isMale() then
            use.card = card
		    if use.to then use.to:append(enemy) end
            return
		end
	end
end

-- chiyuan
sgs.ai_skill_choice["chiyuan"] = function(self, choice)
	if self.player:isWounded() then
		return "qiao"
	else
		return "nu"
	end
end
sgs.ai_skill_cardask["@chiyuan"] = function(self, data)
	local rv = data:toRecover()
	if rv.card:inherits("SilverLion") then return "." end -- will crash
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	if self:isEnemy(rv.who) then
		return cards[1]:getEffectiveId()
	end
	return "."
end

-- huoshui
local huoshui_skill={}
huoshui_skill.name = "huoshui"
table.insert(sgs.ai_skills, huoshui_skill)
huoshui_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local driver_card
	self:sortByUseValue(cards, true)
	for _,card in ipairs(cards)  do
		if card:inherits("Weapon") or card:inherits("Slash") then
			driver_card = card
			break
		end
	end
	if driver_card then
		local suit = driver_card:getSuitString()
		local number = driver_card:getNumberString()
		local card_id = driver_card:getEffectiveId()
		local card_str = ("drivolt:huoshui[%s:%s]=%d"):format(suit, number, card_id)
		local driver = sgs.Card_Parse(card_str)
		return driver
	end
end
sgs.ai_filterskill_filter["huoshui"] = function(card, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:inherits("Weapon") or card:inherits("Slash") then return ("drivolt:huoshui[%s:%s]=%d"):format(suit, number, card_id) end
end

-- huakui
sgs.ai_skill_invoke["huakui"] = true

-- zhiyu
sgs.ai_skill_invoke["zhiyu"] = function(self, data)
	local player = data:toPlayer()
	self.zhiyusource = player
	return not self.player:isKongcheng()
end
sgs.ai_skill_askforag["zhiyu"] = function(self, card_ids)
	local cards = {}
	for _, card_id in ipairs(card_ids)  do
		table.insert(cards, sgs.Sanguosha:getCard(card_id))
	end
	if self:isFriend(self.zhiyusource) then
		self:sortByUseValue(cards, true)
	else
		self:sortByUseValue(cards)
	end
	return cards[1]:getEffectiveId()
end
sgs.ai_cardshow["zhiyu"] = function(self, requestor)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	if self:isFriend(requestor) then
		self:sortByUseValue(cards)
	else
		self:sortByUseValue(cards, true)
	end
	return cards[1]
end

-- chumai
sgs.ai_skill_cardask["@chumai"] = function(self, data)
	local target = data:toPlayer()
	if self:isEnemy(target) then
		local cards = self.player:getHandcards()
		for _, card in sgs.qlist(cards) do
			if card:isBlack() then return card:getEffectiveId() end
		end
	end
	return "."
end

-- yinlang
local yinlang_skill={}
yinlang_skill.name = "yinlang"
table.insert(sgs.ai_skills, yinlang_skill)
yinlang_skill.getTurnUseCard = function(self)
	if self.player:isKongcheng() then return end
	for _, player in ipairs(self.friends_noself) do
		if player:faceUp() then
			return sgs.Card_Parse("@YinlangCard=.")
		end
	end
	if (self.player:usedTimes("YinlangCard") < 2 or self:getOverflow() > 0) then
		return sgs.Card_Parse("@YinlangCard=.")
	end
	if self.player:getLostHp() < 2 then
		return sgs.Card_Parse("@YinlangCard=.")
	end
end
sgs.ai_skill_use_func["YinlangCard"] = function(card, use, self)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards, true)
	local name = self.player:objectName()
	if #self.friends > 1 then
		for _, hcard in ipairs(cards) do
			if hcard:inherits("EquipCard") then
				self:sort(self.friends_noself, "handcard")
				for _, player in ipairs(self.friends_noself) do
					if player:getGeneral():isMale() then
						use.card = sgs.Card_Parse("@YinlangCard=" .. hcard:getEffectiveId())
						if use.to then use.to:append(player) end
						return
					end
				end
			end
		end
	end
end

-- qianxian
local qianxian_skill={}
qianxian_skill.name = "qianxian"
table.insert(sgs.ai_skills, qianxian_skill)
qianxian_skill.getTurnUseCard = function(self)
    if self.player:hasUsed("QianxianCard") then return end
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	for _, acard in ipairs(cards) do
		if acard:isNDTrick() and acard:isBlack() then
			return sgs.Card_Parse("@QianxianCard=" .. acard:getEffectiveId())
		end
	end
end
sgs.ai_skill_use_func["QianxianCard"] = function(card,use,self)
	self:sort(self.enemies, "handcard")
	local first, second
	for _, tmp in ipairs(self.enemies) do
		if not tmp:isChained() or tmp:faceUp() then
			if not first then
				first = tmp
			elseif tmp:getMaxHP() ~= first:getMaxHP() then
				second = tmp
			end
			if first and second then break end
		end
	end
	if not first then
		for _, tmp in ipairs(self.friends_noself) do
			if tmp:getHandcardNum() > 2 and (not tmp:faceUp() or tmp:isChained()) then
				first = tmp
			elseif tmp:getMaxHP() ~= first:getMaxHP() then
				second = tmp
			end
			if first and second then break end
		end
	elseif not second then
		for _, tmp in ipairs(self.friends_noself) do
			if tmp:getHandcardNum() > 2 and (not tmp:faceUp() or tmp:isChained()) then
				second = tmp
			end
			if first and second then break end
		end
	end
	if first and second and use.to then
		use.card = card
		use.to:append(first)
		use.to:append(second)
	end
end

-- baoen
sgs.ai_skill_cardask["@baoen"] = function(self, data)
	local rev = data:toRecover()
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards, true)
	if self:isEnemy(rev.who) or not cards[1] then return "." end
	return cards[1]:getEffectiveId() or "."
end
