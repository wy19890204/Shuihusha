-- AI for rat package

-- liying
-- kongliang
sgs.ai_skill_invoke["kong1iang"] = function(self, data)
	local showcardnum = self.player:getMaxHP() + self.player:getLostHp() + self.player:getHandcardNum()
	return showcardnum > 8
end
sgs.ai_skill_askforag["kong1iang"] = function(self, card_ids)
	local final = sgs.Sanguosha:getCard(card_ids[1])
	local suitnum = 100
	for _, card_id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(card_id)
		local suit = card:getSuitString()
		if final and final:getSuitString() ~= suit then
			local num = self:getSuitNum(suit, false)
			if num < suitnum then
				suitnum = num
				final = card
			end
		end
	end
	return final:getEffectiveId()
end

-- dongping
-- shuangzhan
sgs.ai_skill_cardask["@shuangzhan-jink-1"] = sgs.ai_skill_cardask["@assas1"]

-- zhangqing
-- yinyu
sgs.ai_skill_invoke["yinyu"] = function(self, data)
	if self.player:getMark("mengshi") == 0 then
		self:speak("yinyu")
	end
	return true
end

-- ruanxiaoer
-- fuji
sgs.ai_skill_cardask["@fuji"] = function(self, data)
	local who = data:toPlayer()
	if self:isFriend(who) or self.player:isKongcheng() then return "." end
	return self.player:getRandomHandCard():getEffectiveId() or "."
end

-- zhangshun
-- shunshui
sgs.ai_skill_cardask["@shunshui"] = function(self, data)
	local move = data:toCardMove()
	local suit = sgs.Sanguosha:getCard(move.card_id):getSuitString()
	local cards = self.player:getCards("he")
    cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		if card:getSuitString() == suit then
		    return card:getEffectiveId()
		end
	end
	return "."
end
sgs.ai_skill_playerchosen["shunshui"] = function(self, targets)
	local targetlist = sgs.QList2Table(targets)
	self:sort(targetlist)
	for _, target in ipairs(targetlist) do
		if self:isEnemy(target) then
			return target
		end
	end
	return targetlist[1]
end

-- lihun
sgs.ai_skill_invoke["lihun"] = function(self, data)
	local from = data:toPlayer()
	return self:isEnemy(from)
end
sgs.ai_skill_playerchosen["lihun"] = function(self, targets)
	local friends = sgs.QList2Table(targets)
	self:sort(friends, "hp")
	for _, friend in ipairs(friends) do
		if self:isFriend(friend) and friend ~= self.player then
		    return friend
		end
	end
	return friends[1]
end

-- zhuwu
-- fangzhen
sgs.ai_skill_cardask["@fangzhen"] = function(self, data)
	local damage = data:toDamage()
	if self:hasSkills(sgs.masochism_skill) then return end
	if self:isFriend(damage.from) then
		if self:hasSkills("yixian|qiangqu") then return	end
	end
	local suit = damage.card:getSuitString()
	local cards = self.player:getCards("he")
    cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		if card:getSuitString() == suit then
		    return card:getEffectiveId()
		end
	end
	return "."
end

-- caiyuanzizhangqing
-- shouge
sgs.ai_skill_invoke["shouge"] = function(self, data)
	return not self.player:containsTrick("indulgence")
end
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

-- qiongtu
sgs.ai_skill_invoke["qiongtu"] = function(self, data)
	local target = data:toPlayer()
	if self.player:hasSkill("qiongtu") then
		speak(target, "qiongtu")
	end
	return self:isEnemy(target)
end

-- baisheng
sgs.baisheng_suit_value =
{
	spade = 4
}

-- xiayao
local xiayao_skill={}
xiayao_skill.name = "xiayao"
table.insert(sgs.ai_skills, xiayao_skill)
xiayao_skill.getTurnUseCard = function(self, inclusive)
    local cards = self.player:getCards("h")
    cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		if card:getSuit() == sgs.Card_Spade then
		    local suit = card:getSuitString()
			local number = card:getNumberString()
			local card_id = card:getEffectiveId()
			local card_str = ("ecstasy:xiayao[%s:%s]=%d"):format(suit, number, card_id)
			local ecstasy = sgs.Card_Parse(card_str)
			assert(ecstasy)
			return ecstasy
		end
	end
end
sgs.ai_view_as["xiayao"] = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:getSuit() == sgs.Card_Spade then
		return ("ecstasy:xiayao[%s:%s]=%d"):format(suit, number, card_id)
	end
end

-- shiqian
sgs.shiqian_suit_value =
{
	club = 4
}

-- feiyan
function sgs.ai_trick_prohibit.feiyan(card)
	return card:inherits("SupplyShortage") or card:inherits("Snatch")
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
	for _, acard in ipairs(cards)  do
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
function sgs.ai_cardneed.shentou(to, card, self)
	return card:getSuit() == sgs.Card_Club
end

-- shiwengong
-- dujian
sgs.ai_skill_invoke["dujian"] = function(self, data)
	local damage = data:toDamage()
	if damage.damage ~= 1 then return false end
	if self:isFriend(damage.to) then
		if not damage.to:faceUp() then return true end
		if damage.to:getHandcardNum() > 5 and damage.to:getHp() < 2 then
			return true
		end
	else
		if #self.enemies < 2 then return false end
		return damage.to:faceUp()
	end
end

-- qiaodaoqing
-- huanshu
sgs.ai_card_intention.HuanshuCard = 100

sgs.ai_skill_use["@@huanshu"] = function(self, prompt)
	self:sort(self.enemies)
	local target = self.enemies[1]
	if self.player:isChained() and self:isWeak() then
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isChained() then
				target = enemy
				break
			end
		end
	end
	if target then
		self:speak("huanshu")
		return "@HuanshuCard=." .. "->" .. target:objectName()
	end
	return "."
end
function sgs.ai_slash_prohibit.huanshu(self, to)
	return not (self:hasSkills("butian|shenpan|yixing") or self:isEquip("MeteorSword"))
end

-- qiongying
sgs.qiongying_suit_value =
{
	heart = 3.5,
	diamond = 2.5
}

-- yuanpei
sgs.ai_card_intention.YuanpeiCard = -40

local yuanpei_skill={}
yuanpei_skill.name = "yuanpei"
table.insert(sgs.ai_skills, yuanpei_skill)
yuanpei_skill.getTurnUseCard = function(self)
    if self.player:hasUsed("YuanpeiCard") then
		if not self.player:hasFlag("yuanpei") or self.player:isKongcheng() then return end
		local cards = sgs.QList2Table(self.player:getCards("h"))
		self:sortByUseValue(cards, true)
		for _, card in ipairs(cards) do
			if card:isRed() then
				local suit = card:getSuitString()
				local number = card:getNumberString()
				local card_id = card:getEffectiveId()
				local card_str = ("slash:yuanpei[%s:%s]=%d"):format(suit, number, card_id)
				local slash = sgs.Card_Parse(card_str)
				assert(slash)
				return slash
			end
		end
	else
		return sgs.Card_Parse("@YuanpeiCard=.")
	end
end
sgs.ai_skill_use_func["YuanpeiCard"] = function(card,use,self)
	self:sort(self.friends, "defense")
	for _, enemy in ipairs(self.friends) do
		if enemy:getGeneral():isMale() then
            use.card = card
		    if use.to then use.to:append(enemy) end
            return
		end
	end
end
