
-- danshu
sgs.ai_skill_discard["danshu"] = function(self, discard_num, optional, include_equip)
	local to_discard = {}
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	self:sortByKeepValue(cards, true)
	for _, card in ipairs(cards) do
		if #to_discard >= discard_num then break end
		table.insert(to_discard, card:getId())
	end
	if #to_discard == discard_num then
		return to_discard
	else
		return {}
	end
end
function sgs.ai_slash_prohibit.danshu(self, to)
	local x = to:getLostHp()
	return self.player:getHandcardNum() <= x + 1
end

-- haoshen
sgs.ai_skill_use["@@haoshen"] = function(self, prompt)
	if prompt == "@haoshen-draw" and not self.player:isKongcheng() then
		self:sort(self.friends, "handcard")
		local max_x = 2
		local target
		for _, friend in ipairs(self.friends) do
			local x = friend:getMaxHP() - friend:getHandcardNum()
			if x > max_x then
				max_x = x
				target = friend
			end
		end
		if target then
			return "@HaoshenCard=.->" .. target:objectName()
		else
			return "."
		end
	elseif prompt == "@haoshen-play" and self.player:getHandcardNum() > 6 then
		self:sort(self.friends_noself, "handcard")
		local target = self.friends_noself[1]
		if not target then return "." end
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		local card_ids = {}
		for i = 1, math.floor((#cards + 1) / 2) do
			table.insert(card_ids, cards[i]:getEffectiveId())
		end
		return "@HaoshenCard=" .. table.concat(card_ids, "+") .. "->" .. target:objectName()
	else
		return "."
	end
end

-- huxiao
local huxiao_skill={}
huxiao_skill.name = "huxiao"
table.insert(sgs.ai_skills, huxiao_skill)
huxiao_skill.getTurnUseCard = function(self)
	if not self.player:isNude() then
		local cards = self.player:getCards("he")
		cards = sgs.QList2Table(cards)
		for _, fcard in ipairs(cards) do
			if fcard:inherits("EquipCard") then
				local suit, number, id = fcard:getSuitString(), fcard:getNumberString(), fcard:getId()
				local card_str = ("savage_assault:huxiao[%s:%s]=%d"):format(suit, number, id)
				local savage = sgs.Card_Parse(card_str)
				assert(savage)
				return savage
			end
		end
	end
end

-- baoguo&yinyu
sgs.ai_skill_invoke["baoguo"] = true
sgs.ai_skill_invoke["yinyu"] = true

-- yueli
function sgs.ai_slash_prohibit.yueli(self, to)
	if self:isEquip("EightDiagram", to) then return true end
end

-- taohui
sgs.ai_skill_playerchosen["taohui"] = function(self, targets)
	self:sort(self.friends, "handcard")
	return self.friends[1]
end

-- jishi
sgs.ai_skill_cardask["@jishi"] = function(self, data)
	local who = data:toPlayer()
	if self:isEnemy(who) or self.player:isKongcheng() then return "." end
	return self.player:getRandomHandCard():getEffectiveId() or "."
end

-- huanshu
sgs.ai_skill_use["@@huanshu"] = function(self, prompt)
	self:sort(self.enemies, "hp")
	local target = self.enemies[1]
	if target then return "@HuanshuCard=.".."->"..target:objectName() end
	return "."
end
function sgs.ai_slash_prohibit.huanshu(self, to)
	return not (self:hasSkills("butian|shenpan|yixing") or self:isEquip("MeteorSword"))
end

-- cuju
sgs.ai_skill_invoke["cuju"] = function(self, data)
	local damage = data:toDamage()
	return damage.damage > 0
end
sgs.ai_skill_use["@@cuju"] = function(self, prompt)
	if self.player:isKongcheng() then return "." end
	self:sort(self.enemies, "hp")
	local target = self.enemies[1]
	local card = self.player:getRandomHandCard()
	if target then return "@CujuCard="..card:getEffectiveId().."->"..target:objectName() end
	return "."
end

-- panquan
sgs.ai_skill_invoke["panquan"] = function(self, data)
	local gaoqiu = self.room:getLord()
	return self:isFriend(gaoqiu)
end

-- qiangqu
sgs.ai_skill_invoke["qiangqu"] = function(self, data)
	local damage = data:toDamage()
	return self:isFriend(damage.to)
end

-- huatian
sgs.ai_skill_invoke["huatian"] = function(self, data)
	if not self.friends_noself[1] then return false end
	self:sort(self.friends_noself, "hp")
	if self.player:getMark("HBTJ") == 1 then
		return self.friends_noself[1]:isWounded()
	end
	return true
end
sgs.ai_skill_playerchosen["huatian"] = function(self, targets)
	local mark = self.player:getMark("HBTJ")
	if mark == 1 then
		self:sort(self.friends_noself, "hp")
		for _, friend in ipairs(self.friends_noself) do
			if friend:isWounded() then
				return friend
			end
		end
	elseif mark == 2 then
		self:sort(self.enemies, "hp")
		return self.enemies[1]
	end
end

-- yanshou
yanshou_skill={}
yanshou_skill.name = "yanshou"
table.insert(sgs.ai_skills, yanshou_skill)
yanshou_skill.getTurnUseCard = function(self)
	if self.player:getMark("@life") < 1 then return end
	local cards = self.player:getHandcards()
	local hearts = {}
    cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		if card:getSuit() == sgs.Card_Heart then
		    table.insert(hearts, card:getId())
		end
		if #hearts == 2 then break end
	end
	if #hearts ~= 2 then return end
	return sgs.Card_Parse("@YanshouCard=" .. table.concat(hearts, "+"))
end
sgs.ai_skill_use_func["YanshouCard"]=function(card,use,self)
	self:sort(self.friends, "maxhp")
	for _, friend in ipairs(self.friends) do
		if friend:hasSkill("yuanyin") or (friend:hasSkill("yanshou") and not friend:isLord())
			or friend:hasSkill("wudao") then
			use.card = card
			if use.to then
				use.to:append(friend)
			end
			return
		end
	end
	use.card = card
	if use.to then use.to:append(self.friends[1]) end
end

-- hongjin
sgs.ai_skill_choice["hongjin"] = function(self, choices)
	local who = self.player:getTag("HongjinTarget"):toPlayer()
	if self:isFriend(who) then
		return "draw"
	else
		if who:getHandcardNum() == 1 or (who:isKongcheng() and not who:isNude()) then
			return "throw"
		else
			return "draw"
		end
	end
end

-- wuji
wuji_skill={}
wuji_skill.name = "wuji"
table.insert(sgs.ai_skills, wuji_skill)
wuji_skill.getTurnUseCard = function(self)
--	if self:slashIsAvailable() then return end
	local cards = self.player:getCards("h")
	local slashs = {}
    cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
	for _,card in ipairs(cards)  do
		if card:inherits("Slash") then
		    table.insert(slashs, card:getId())
		end
	end
	if #slashs == 0 then return end
	return sgs.Card_Parse("@WujiCard=" .. table.concat(slashs, "+"))
end
sgs.ai_skill_use_func["WujiCard"]=function(card,use,self)
	use.card = card
end
