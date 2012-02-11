-- juyi
juyi_skill={}
juyi_skill.name = "jui"
table.insert(sgs.ai_skills, juyi_skill)
juyi_skill.getTurnUseCard = function(self)
	local sj = self.room:getLord()
	if self.player:getKingdom() ~= "kou" or self.player:hasUsed("JuyiCard") or self:isEnemy(sj) then return end
	local mycardnum = self.player:getHandcardNum()
	local sjcardnum = sj:getHandcardNum()
	if mycardnum - sjcardnum > 1 then
		card = sgs.Card_Parse("@JuyiCard=.")
		return card
	end
end
sgs.ai_skill_use_func["JuyiCard"]=function(card,use,self)
	use.card = card
end
sgs.ai_skill_choice["jui"] = function(self, choice)
	local source = self.room:getCurrent()
	if self:isFriend(source) then
		return "agree"
	else
		return "deny"
	end
end

-- baoguo
sgs.ai_skill_cardask["@baoguo"] = function(self, data)
	if self.player:hasSkill("fushang") and self.player:getHp() > 3 then return "." end
	local damage = data:toDamage()
	if self:isFriend(damage.to) and not self.player:isKongcheng() then
		local pile = self:getCardsNum("Peach") + self:getCardsNum("Analeptic")
		local dmgnum = damage.damage
		if self.player:getHp() + pile - dmgnum > 0 then
			if self.player:getHp() + pile - dmgnum == 1 and pile > 0 then return "." end
			local card = self:getUnuseCard()
			if card then return card:getEffectiveId() end
		end
	end
	return "."
end

-- danshu
sgs.ai_skill_discard["danshu"] = function(self, discard_num, optional, include_equip)
	local to_discard = {}
	local cards = self.player:getHandcards()
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

-- haoshen
sgs.ai_skill_use["@@haoshen"] = function(self, prompt)
	if prompt == "@haoshen-draw" and not self.player:isKongcheng() then
		self:sort(self.friends_noself)
		local max_x = 2
		local target
		for _, friend in ipairs(self.friends_noself) do
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

-- cuju
sgs.ai_skill_invoke["cuju"] = true
sgs.ai_skill_use["@@cuju"] = function(self, prompt)
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

-- yixing
function SmartAI:getYixingCard(judge)
	local equips = {}
	for _, player in ipairs(self.enemies) do
		local pequips = player:getEquips()
		for _, equip in sgs.qlist(pequips) do
			table.insert(equips, equip)
		end
	end
	for _, player in ipairs(self.friends) do
		local pequips = player:getEquips()
		for _, equip in sgs.qlist(pequips) do
			table.insert(equips, equip)
		end
	end
	if #equips == 0 then return -1 end
	return self:getRetrialCardId(equips, judge)
end
sgs.ai_skill_use["@@yixing"] = function(self, prompt)
	local judge = self.player:getTag("Judge"):toJudge()
	if self:needRetrial(judge) then
		local players = sgs.QList2Table(self.room:getAllPlayers())
		local card_id = self:getYixingCard(judge)
		if card_id == -1 then return "." end
		for _, player in ipairs(players) do
			local pequips = player:getEquips()
			for _, equip in sgs.qlist(pequips) do
				if equip:getEffectiveId() == card_id then
					self.yixingcid = card_id
					return "@YixingCard=.->" .. player:objectName()
				end
			end
		end
	end
	return "."
end

-- qimen
sgs.ai_skill_invoke["qimen"] = function(self, data)
	local player = self.room:getCurrent()
	if player == self.player then return false end
--	local player = data:toPlayer()
--	self.qimentarget = player
	if self:isFriend(player) then return false end
	local rm = math.random(1, 3)
	return rm ~= 2
end
sgs.ai_skill_playerchosen["qimen"] = function(self, targets)
--	return targets[1]
	local target = self.room:getCurrent()
	return target
--	return self.qimentarget
end

-- ganlin
sgs.ai_skill_invoke["ganlin"] = function(self, data)
	return self.player:isKongcheng() and self.player:isWounded()
end
local ganlin_skill={}
ganlin_skill.name = "ganlin"
table.insert(sgs.ai_skills, ganlin_skill)
ganlin_skill.getTurnUseCard = function(self)
	if self.player:isKongcheng() then return end
	for _, player in ipairs(self.friends_noself) do
		if ((self:hasSkills("butian|qimen|longluo", player) or player:containsTrick("supply_shortage"))
			or (not player:containsTrick("indulgence") and (self:hasSkills("banzhuang|shouge", player)))
			)
			and player:faceUp() then
			return sgs.Card_Parse("@GanlinCard=.")
		end
	end
	if (self.player:usedTimes("GanlinCard") < 2 or self:getOverflow() > 0 or self:getCard("Shit")) then
		return sgs.Card_Parse("@GanlinCard=.")
	end
	if self.player:getLostHp() < 2 then
		return sgs.Card_Parse("@GanlinCard=.")
	end
end
sgs.ai_skill_use_func["GanlinCard"] = function(card, use, self)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards,true)
	local name = self.player:objectName()
	if #self.friends > 1 then
		local shijin, shiqian, wangding6, weidingguo, fanrui
		shijin = self.room:findPlayerBySkillName("xiagu")
		shiqian = self.room:findPlayerBySkillName("shentou")
		wangding6 = self.room:findPlayerBySkillName("kongying")
		weidingguo = self.room:findPlayerBySkillName("shenhuo")
		fanrui = self.room:findPlayerBySkillName("kongmen")
		for _, hcard in ipairs(cards) do
			if not hcard:inherits("Shit") then
				if hcard:inherits("Analeptic") or hcard:inherits("Peach") then
					self:sort(self.friends_noself, "hp")
					if #self.friends>1 and self.friends_noself[1]:getHp() == 1 then
						use.card = sgs.Card_Parse("@GanlinCard=" .. hcard:getId())
						if use.to then use.to:append(self.friends_noself[1]) end
						return
					end
				end
				self:sort(self.friends_noself, "hp")
				local friend = self.friends_noself[1]
				if fanrui and fanrui:isWounded() and fanrui:isKongcheng() and self:isFriend(fanrui) and  fanrui:objectName() ~= name then
					use.card = sgs.Card_Parse("@GanlinCard=" .. hcard:getId())
					if use.to then use.to:append(fanrui) end
					return
				end
				if wangding6 and self:isFriend(wangding6) and hcard:inherits("Jink") and wangding6:objectName() ~= name and
					self:getCardsNum("Jink")>1 then
					use.card = sgs.Card_Parse("@GanlinCard=" .. hcard:getId())
					if use.to then use.to:append(wangding6) end
					return
				end
				if shijin and self:isFriend(shijin) and hcard:inherits("EquipCard") and shijin:objectName() ~= name then
					use.card = sgs.Card_Parse("@GanlinCard=" .. hcard:getId())
					if use.to then use.to:append(shijin) end
					return
				end
				if shiqian and self:isFriend(shiqian) and hcard:getSuit() == sgs.Card_Club and shiqian:objectName() ~= name then
					use.card = sgs.Card_Parse("@GanlinCard=" .. hcard:getId())
					if use.to then use.to:append(shiqian) end
					return
				end
				if weidingguo and self:isFriend(weidingguo) and hcard:inherits("TrickCard") and hcard:isRed() and weidingguo:objectName() ~= name and
					not (weidingguo:containsTrick("indulgence") and not hcard:inherits("Nullification")) then
					use.card = sgs.Card_Parse("@GanlinCard=" .. hcard:getId())
					if use.to then use.to:append(weidingguo) end
					return
				end
				if self:getUseValue(hcard)<6 and #self.friends>1 then
					for _, friend in ipairs(self.friends_noself) do
						if sgs[friend:getGeneralName() .. "_suit_value"] and
							(sgs[friend:getGeneralName() .. "_suit_value"][hcard:getSuitString()] or 0)>=3.9 then
							use.card = sgs.Card_Parse("@GanlinCard=" .. hcard:getId())
							if use.to then use.to:append(friend) end
							return
						end
						if friend:getGeneral2Name()~="" then
							if sgs[friend:getGeneral2Name() .. "_suit_value"] and
								(sgs[friend:getGeneral2Name() .. "_suit_value"][hcard:getSuitString()] or 0)>=3.9 then
								use.card = sgs.Card_Parse("@GanlinCard=" .. hcard:getId())
								if use.to then use.to:append(friend) end
								return
							end
						end
						if hcard:getNumber()>10 and self:hasSkills("dalei|taolue|zhongzhen|suocai", friend) then
							use.card = sgs.Card_Parse("@GanlinCard=" .. hcard:getId())
							if use.to then use.to:append(friend) end
							return
						end
					end
				end
				if hcard:inherits("Armor") then
					self:sort(self.friends_noself, "defense")
					local v = 0
					local target
					for _, friend in ipairs(self.friends_noself) do
						if not friend:getArmor() and self:evaluateArmor(hcard, friend) > v and not friend:containsTrick("indulgence") then
							v = self:evaluateArmor(hcard, friend)
							target = friend
						end
					end
					if target then
						use.card = sgs.Card_Parse("@GanlinCard=" .. hcard:getId())
						if use.to then use.to:append(target) end
						return
					end
				end
				if hcard:inherits("EquipCard") then
					self:sort(self.friends_noself)
					for _, friend in ipairs(self.friends_noself) do
						if not self:hasSameEquip(hcard, friend) or friend:hasSkill("xiagu")
							or (self:hasSkills("feiqiang|cuihuo|yinlang|huxiao", friend) and not friend:containsTrick("indulgence"))  then
							use.card = sgs.Card_Parse("@GanlinCard=" .. hcard:getId())
							if use.to then use.to:append(friend) end
							return
						end
					end
				end
			end
		end
	end

	local shit
	shit = self:getCard("Shit")
	if shit then
		use.card = sgs.Card_Parse("@GanlinCard=" .. shit:getId())
		self:sort(self.enemies,"hp")
		if use.to then use.to:append(self.enemies[1]) end
		return
	end

	if #self.friends == 1 then return end

	if (self:getOverflow()>0 or (self.player:isWounded() and self.player:usedTimes("GanlinCard") < 2))
		then
		self:sort(self.friends_noself, "handcard")
		local friend
		if friend then
			local card_id = self:getCardRandomly(self.player, "h")
			use.card = sgs.Card_Parse("@GanlinCard=" .. card_id)
			if use.to then use.to:append(friend) end
			return
		end
	end

	if self.player:getHandcardNum()==1 then
		for _, enemy in ipairs(self.enemies) do
			if self:isEquip("GudingBlade", enemy) and enemy:canSlash(self.player, true) then return end
		end
	end

	if self.player:getLostHp() < 2 then
		local card
		self:sortByUseValue(cards, true)
		for _, hcard in ipairs(cards) do
			if hcard:inherits("Jink") then if self:getCardsNum("Jink")>#self.enemies/2 then card = hcard end
			elseif hcard:inherits("Slash") then if self:getCardsNum("Slash") > 1 then card = hcard end
			elseif not hcard:inherits("Nullification") then card = hcard end
			if card then break end
		end
		if card then
			self:sort(self.friends_noself, "handcard")
			for _, friend in ipairs(self.friends_noself) do
				local draw = 2
				if friend:containsTrick("supply_shortage") then draw = 0 end
				if friend:getHandcardNum() + draw < friend:getMaxCards() and not friend:containsTrick("indulgence") and	(not self:isWeak(friend) and self:hasSkills(sgs.need_kongcheng,friend)) then
					use.card = sgs.Card_Parse("@GanlinCard=" .. card:getId())
					if use.to then use.to:append(friend) end
					return
				end
			end
			self:sort(self.friends_noself, "handcard")
			for _, friend in ipairs(self.friends_noself) do
				if not friend:containsTrick("indulgence") then
					use.card = sgs.Card_Parse("@GanlinCard=" .. card:getId())
					if use.to then use.to:append(self.friends_noself[1]) end
					return
				end
			end
		end
	end
end
