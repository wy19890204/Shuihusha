-- songjiang
sgs.ai_chaofeng.songjiang = -2
sgs.songjiang_suit_value =
{
	heart = 3.9,
}
sgs.songjiang_keep_value =
{
	Jink = 5.1,
}

-- ganlin
sgs.ai_card_intention.GanlinCard = -70
sgs.ai_use_value.GanlinCard = 8.5
sgs.ai_use_priority.GanlinCard = 5.8
sgs.dynamic_value.benefit.GanlinCard = true

sgs.ai_skill_invoke["ganlin"] = function(self, data)
	return self.player:getHandcardNum() < 2 and self.player:isWounded()
end
local ganlin_skill={}
ganlin_skill.name = "ganlin"
table.insert(sgs.ai_skills, ganlin_skill)
ganlin_skill.getTurnUseCard = function(self)
	if self.player:hasFlag("Ganlin") or self.player:isKongcheng() then return end
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
						if not self:getSameEquip(hcard, friend) or friend:hasSkill("xiagu")
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

-- lujunyi
sgs.ai_chaofeng.lujunyi = 5
-- baoguo
sgs.ai_skill_cardask["@baoguo"] = function(self, data)
	if self.player:hasSkill("fushang") and self.player:getHp() > 3 then return "." end
	local damage = data:toDamage()
	if self:isFriend(damage.to) and not self.player:isKongcheng() then
		local pile = self:getCardsNum("Peach") + self:getCardsNum("Analeptic")
		local dmgnum = damage.damage
		if self.player:getHp() + pile - dmgnum > 0 then
			if self.player:getHp() + pile - dmgnum == 1 and pile > 0 then return "." end
			local cards = sgs.QList2Table(self.player:getHandcards())
			self:sortByUseValue(cards, false)
			for _, fcard in ipairs(cards) do
				if fcard:inherits("BasicCard") and
					not fcard:inherits("Peach") and not fcard:inherits("Analeptic") then
					return fcard:getEffectiveId()
				end
			end
		end
	end
	return "."
end

-- wuyong
sgs.ai_chaofeng.wuyong = 6
-- huace
local huace_skill={}
huace_skill.name = "huace"
table.insert(sgs.ai_skills, huace_skill)
huace_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("HuaceCard") or self.player:isKongcheng() then return end
	local cards = sgs.QList2Table(self.player:getHandcards())
	local aoename = "savage_assault|archery_attack"
	local aoenames = aoename:split("|")
	local aoe
	local i
	local good, bad = 0, 0
	local huacetrick = "savage_assault|archery_attack|ex_nihilo|god_salvation"
	local huacetricks = huacetrick:split("|")
	for i=1, #huacetricks do
		local forbiden = huacetricks[i]
		forbid = sgs.Sanguosha:cloneCard(forbiden, sgs.Card_NoSuit, 0)
		if self.player:isLocked(forbid) then return end
	end
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() then
			good = good + 10/(friend:getHp())
			if friend:isLord() then good = good + 10/(friend:getHp()) end
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if enemy:isWounded() then
			bad = bad + 10/(enemy:getHp())
			if enemy:isLord() then
				bad = bad + 10/(enemy:getHp())
			end
		end
	end
	local card = -1
	self:sortByUseValue(cards, true)
	for _, acard in ipairs(cards)  do
		if acard:inherits("TrickCard") then
			card = acard:getEffectiveId()
			break
		end
	end
	if card < 0 then return end
	for i=1, #aoenames do
		local newhuace = aoenames[i]
		aoe = sgs.Sanguosha:cloneCard(newhuace, sgs.Card_NoSuit, 0)
		if self:getAoeValue(aoe) > -5 then
			local parsed_card=sgs.Card_Parse("@HuaceCard=" .. card .. ":" .. newhuace)
			return parsed_card
		end
	end
	if good > bad then
		local parsed_card = sgs.Card_Parse("@HuaceCard=" .. card .. ":" .. "god_salvation")
		return parsed_card
	end
	if self:getCardsNum("Jink") == 0 and self:getCardsNum("Peach") == 0 then
		local parsed_card = sgs.Card_Parse("@HuaceCard=" .. card .. ":" .. "ex_nihilo")
		return parsed_card
	end
end
sgs.ai_skill_use_func["HuaceCard"] = function(card, use, self)
	local userstring = card:toString()
	userstring = (userstring:split(":"))[3]
	local huacecard = sgs.Sanguosha:cloneCard(userstring, card:getSuit(), card:getNumber())
	self:useTrickCard(huacecard,use)
	if not use.card then return end
	use.card = card
end

-- yunchou
sgs.ai_skill_invoke["yunchou"] = true

-- gongsunsheng
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
	local target = self.room:getCurrent()
	return target
--	return self.qimentarget
end

-- guansheng
-- tongwu
sgs.ai_skill_invoke["tongwu"] = true
sgs.ai_skill_playerchosen["tongwu"] = function(self, targets)
	if not self:getCard("Jink") then return self.player end
	local targetlist=sgs.QList2Table(targets)
	self:sort(targetlist, "handcard")
	for _, target in ipairs(targetlist) do
		if self:isFriend(target) then
			return target
		end
	end
	return self.player
end

-- linchong
-- duijue
sgs.ai_skill_use["@@duijue"] = function(self, prompt)
	self:sort(self.enemies, "hp")
	local n1 = self:getCardsNum("Slash")
	local final
	local card = sgs.Sanguosha:cloneCard("duel", sgs.Card_NoSuit, 0)
	for _, enemy in ipairs(self.enemies) do
		if n1 + 1 > self:getCardsNum("Slash", enemy) and self:hasTrickEffective(card, enemy) then
			final = enemy
			break
		end
	end
	if final then
		return "@DuijueCard=.->" .. final:objectName()
	else
		return "."
	end
end

-- huarong
-- kaixian
sgs.ai_skill_invoke["kaixian"] = true

-- chaijin
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

-- zhutong
-- sijiu
sijiu_skill={}
sijiu_skill.name = "sijiu"
table.insert(sgs.ai_skills, sijiu_skill)
sijiu_skill.getTurnUseCard = function(self)
	if self:isWeak() then return end
	local peach = self:getCardId("Peach")
	if peach and type(peach) == "number" then
		return sgs.Card_Parse("@SijiuCard=" .. peach)
	end
end
sgs.ai_skill_use_func["SijiuCard"] = function(card, use, self)
	self:sort(self.friends, "hp")
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() then
			use.card = card
			if use.to then use.to:append(friend) end
			return
		end
	end
end

-- yixian
sgs.ai_skill_invoke["yixian"] = function(self, data)
	local damage = data:toDamage()
	if self:isFriend(damage.to) then
		return not damage.to:getJudgingArea():isEmpty() or not self:hasSkills(sgs.masochism_skill, damage.to)
	elseif self:isEnemy(damage.to) then
		return self:hasSkills(sgs.masochism_skill, damage.to)
	end
end
sgs.ai_skill_cardchosen["yixian"] = function(self, who)
	if self:isFriend(who) and not who:getJudgingArea():isEmpty() then
		return who:delayedTricks():first()
	end
	if self:isEnemy(who) then
		local cards = sgs.QList2Table(who:getCards("he"))
		if #cards > 0 then
			self:sortByUseValue(cards)
			return cards[1]
		end
	elseif self:isFriend(who) then
		local cards = sgs.QList2Table(who:getCards("he"))
		if #cards > 0 then
			self:sortByUseValue(cards, true)
			return cards[1]
		end
	end
	local cards = sgs.QList2Table(who:getCards("hej"))
	return cards[1]
end

-- luzhishen
-- liba
sgs.ai_skill_invoke["liba"] = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.to)
end

-- wusong
-- fuhu
sgs.ai_skill_cardask["@fuhu"] = function(self, data)
	local damage = data:toDamage()
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	if not self:slashIsEffective(slash, damage.from) then return "." end
	if self:isEnemy(damage.from) then
		local cards = self.player:getCards("he")
		cards = sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		local default
		for _, card in ipairs(cards) do
			if card:isBlack() then
				if not default then default = card end
				if self:getCardsNum("Jink", damage.from) == 0 and
					(card:inherits("Analeptic") or card:inherits("Weapon")) then
					return card:getEffectiveId()
				end
			end
		end
		if default then
			return default:getEffectiveId()
		end
	end
	return "."
end

-- yangzhi
-- maidao
maidao_skill={}
maidao_skill.name = "maidao"
table.insert(sgs.ai_skills, maidao_skill)
maidao_skill.getTurnUseCard = function(self)
	if self.player:getWeapon() then
		local range = sgs.weapon_range[self.player:getWeapon():className()]
		local cards
		if range < 4 then
			cards = self.player:getCards("he")
		else
			cards = self.player:getCards("h")
		end
		cards = sgs.QList2Table(cards)
		for _, acard in ipairs(cards)  do
			if acard:inherits("Weapon") then
				return sgs.Card_Parse("@MaidaoCard=" .. acard:getEffectiveId())
			end
		end
	end
	return
end
sgs.ai_skill_use_func["MaidaoCard"] = function(card, use, self)
	use.card = card
end

-- fengmang
sgs.ai_skill_use["@@fengmang"] = function(self, prompt)
	self:sort(self.enemies)
	local target = self.enemies[1]
	local cards = self.player:getHandcards()
	local card
	for _, c in sgs.qlist(cards) do
		if c:inherits("EventsCard") then
			card = c
			break
		end
	end
	if card then return "@FengmangCard=" .. card:getEffectiveId() .. "->" .. target:objectName()
	else return "@FengmangCard=.->" .. target:objectName()
	end
	return "."
end

-- mAIdao
mAIdao_skill = {}
mAIdao_skill.name = "mAIdao"
table.insert(sgs.ai_skills, mAIdao_skill)
mAIdao_skill.getTurnUseCard = function(self)
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
		self.yangzhi = yangzhi
		if #card_ids == 2 then
			return "@MAIdaoCard=" .. table.concat(card_ids, "+")
		end
	end
	return
end
sgs.ai_skill_use_func["MAIdaoCard"] = function(card, use, self)
	use.card = card
	if use.to then
		use.to:append(self.yangzhi)
	end
end

-- xuning
-- goulian
sgs.ai_skill_invoke["goulian"] = sgs.ai_skill_invoke["liba"]

-- jinjia
function sgs.ai_armor_value.jinjia(card)
	if not card then return 4 end
end

-- daizong
-- mitan
mitan_skill={}
mitan_skill.name = "mitan"
table.insert(sgs.ai_skills, mitan_skill)
mitan_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("Wiretap") then return end
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards, true)
	for _,acard in ipairs(cards) do
		if (acard:inherits("EventsCard") or acard:inherits("TrickCard")) and self:getUseValue(acard) < 4 then
			card = acard
			break
		end
	end
	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("wiretap:mitan[%s:%s]=%d"):format(suit, number, card_id)
	local wire = sgs.Card_Parse(card_str)
	assert(wire)
	return wire
end
sgs.ai_skill_askforag["mitan"] = function(self, card_ids)
	local effect = self.room:getTag("Wiretap"):toCardEffect()
	local cards = {}
	for _, card_id in ipairs(card_ids)  do
		table.insert(cards, sgs.Sanguosha:getCard(card_id))
	end
	if self:isEnemy(effect.to) then
		self:sortByUseValue(cards)
		return cards[1]:getEffectiveId()
	else
		return -1
	end
end

-- jibao
sgs.ai_skill_cardask["@jibao"] = function(self)
	if self.player:getHandcardNum() <= 2 then return "." end
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	return cards[1]:getEffectiveId()
end

-- likui
-- shalu
sgs.ai_skill_invoke["shalu"] = true

-- ruanxiao7
-- jueming
function sgs.ai_trick_prohibit.jueming(card, self, to)
	return to ~= self.room:getCurrent() and to:getHp() == 1 and (card:inherits("Duel") or card:inherits("Assassinate"))
end
function sgs.ai_slash_prohibit.jueming(self, to)
	if to ~= self.room:getCurrent() and to:getHp() == 1 then return true end
end

-- jiuhan
sgs.ai_skill_invoke["jiuhan"] = function(self, data)
	if self.player:getLostHp() > self.player:getMaxHP() then
		return true
	end
	return math.random(1, 3) == 2
end

-- yangxiong
-- xingxing
sgs.ai_skill_cardask["@xingxing"] = function(self, data)
	local dy = data:toDying()
	if self:isEnemy(dy.who) then
		local cards = self.player:getCards("h")
		for _, card in sgs.qlist(cards) do
			if card:getSuit() == sgs.Card_Spade then
				return card:getEffectiveId()
			end
		end
	end
	return "."
end

-- yanqing
-- dalei
local dalei_skill={}
dalei_skill.name = "dalei"
table.insert(sgs.ai_skills, dalei_skill)
dalei_skill.getTurnUseCard = function(self)
    if self.player:hasUsed("DaleiCard") or self.player:isKongcheng() then return end
	return sgs.Card_Parse("@DaleiCard=.")
end
sgs.ai_skill_use_func["DaleiCard"] = function(card, use, self)
	local target
	if self.player:getHp() > 1 then
		self:sort(self.enemies, "handcard")
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isKongcheng() and enemy:getGeneral():isMale()
				and self.player:inMyAttackRange(enemy) then
				target = enemy
				break
			end
		end
		local max_card = self:getMaxCard()
		if target and max_card then
			use.card = sgs.Card_Parse("@DaleiCard=" .. max_card:getEffectiveId())
			if use.to then use.to:append(target) end
		end
	else
		for _, friend in ipairs(self.friends_noself) do
			if friend:getHandcardNum() > 3 and not friend:isWounded()
				and friend:getGeneral():isMale() then
				target = friend
				break
			end
		end
		local min_num = 14, min_card
		for _, hcard in sgs.qlist(self.player:getHandcards()) do
			if hcard:getNumber() < min_num then
				min_num = hcard:getNumber()
				min_card = hcard
			end
		end
		if target and min_card then
			use.card = sgs.Card_Parse("@DaleiCard=" .. min_card:getEffectiveId())
			if use.to then use.to:append(target) end
		end
	end
end
sgs.ai_skill_invoke["dalei"] = function(self, data)
	local damage = data:toDamage()
	self:sort(self.friends, "hp")
	local caninvoke = false
	for _, friend in ipairs(self.friends) do
		if friend:isWounded() and friend ~= damage.to then
			caninvoke = true
			self.daleirecover = friend
			break
		end
	end
	return caninvoke
end
sgs.ai_skill_playerchosen["dalei"] = function(self, targets)
	return self.daleirecover
end

-- fuqin
sgs.ai_skill_choice["fuqin"] = function(self, choice)
	if choice == "qing+nil" then return "qing" end
	local source = self.player:getTag("FuqinSource"):toPlayer()
	if self:isFriend(source) then
		return "qing"
	else
		local rand = math.random(1, 2)
		if rand == 1 then
			return "yan"
		else
			return "qing"
		end
	end
end
sgs.ai_skill_playerchosen["fuqin"] = function(self, targets)
	self:sort(self.friends, "handcard")
	if self.friends[1]:getHandcardNum() > 2 then
		self:sort(self.friends, "hp")
		if self.friends[1]:getHp() > 2 then return self.player
		else return self.friends[1] end
	end
	return self.friends[1]
end

-- andaoquan
-- jishi
sgs.ai_skill_cardask["@jishi"] = function(self, data)
	local who = data:toPlayer()
	if self:isEnemy(who) or self.player:isKongcheng() then return "." end
	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		if card:inherits("TrickCard") or card:inherits("BasicCard") then
		    return card:getEffectiveId()
		end
	end
	return "."
end

-- fengyue
sgs.ai_skill_invoke["fengyue"] = true

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

-- hu3niang
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
	if self:slashIsAvailable() then return end
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

-- sun2niang
-- heidian
sgs.ai_skill_cardask["@heidian2"] = function(self)
	local ecards = sgs.QList2Table(self.player:getCards("e"))
	self:sortByUseValue(ecards, true)
	return ecards[1]:getEffectiveId() or "."
end

-- renrou
sgs.ai_skill_invoke["renrou"] = function(self, data)
	local shiti = data:toPlayer()
	local cards = shiti:getHandcards()
	local shit_num = 0
	for _, card in sgs.qlist(cards) do
		if card:inherits("Shit") then
			shit_num = shit_num + 1
			if card:getSuit() == sgs.Card_Spade then
				shit_num = shit_num + 1
			end
		end
	end
	return shit_num <= 1
end

-- gaoqiu
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

-- caijing
-- jiashu
jiashu_skill={}
jiashu_skill.name = "jiashu"
table.insert(sgs.ai_skills, jiashu_skill)
jiashu_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("JiashuCard") or self.player:isKongcheng() then return end
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	self.jiashusuit = cards[1]:getSuitString()
	return sgs.Card_Parse("@JiashuCard=" .. cards[1]:getEffectiveId())
end
sgs.ai_skill_use_func["JiashuCard"] = function(card, use, self)
	self:sort(self.enemies, "handcard")
	use.card = card
	if use.to then use.to:append(self.enemies[1]) end
end
sgs.ai_skill_suit["jiashu"] = function(self)
	local map = {}
	if self.jiashusuit == "spade" then
		map = {1,2,3}
	elseif self.jiashusuit == "club" then
		map = {0,2,3}
	elseif self.jiashusuit == "heart" then
		map = {0,1,3}
	elseif self.jiashusuit == "diamond" then
		map = {0,1,2}
	end
	return map[math.random(1,3)]
end

-- duoquan
sgs.ai_skill_invoke["duoquan"] = function(self, data)
	if self.player:getMark("@quan") == 0 then return false end
	local shiti = data:toPlayer()
	if shiti:getHandcardNum() <= 3 then
		return sgs.ai_chaofeng[shiti:getGeneralName()] > 4
	else
		return shiti:getHandcardNum() > 3
	end
end

-- fangla
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

-- zhiyuan
sgs.ai_skill_invoke["zhiyuan"] = true
sgs.ai_skill_cardask["@zhiyuan"] = function(self)
	local lord = self.room:getLord()
	if self:isFriend(lord) and not self.player:isKongcheng() then
		local cards = self.player:getCards("h")
		cards=sgs.QList2Table(cards)
		self:sortByUseValue(cards)
		return cards[1]:getEffectiveId()
	elseif self:isEnemy(lord) and lord:hasUsed("YongleCard") then
		local shit = self:getCardId("Shit")
		if shit and type(shit) == "number" then
			return shit
		end
	end
	return "."
end

-- wangqing
-- jiachu
sgs.ai_skill_cardask["@jiachu"] = function(self)
	local target = self.room:getLord()
	if self:isFriend(target) then
		local allcards = self.player:getCards("he")
		for _, card in sgs.qlist(allcards) do
			if card:getSuit() == sgs.Card_Heart then
				return card:getEffectiveId()
			end
		end
	end
	return "."
end

-- panjinlian
-- meihuo
sgs.ai_use_priority.MeihuoCard = 2.5
sgs.ai_card_intention.MeihuoCard = -80
sgs.dynamic_value.benefit.MeihuoCard = true

meihuo_skill={}
meihuo_skill.name = "meihuo"
table.insert(sgs.ai_skills, meihuo_skill)
meihuo_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("MeihuoCard") or not self.player:isWounded() then return end
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards)  do
		if card:getSuit() == sgs.Card_Heart then
		    return sgs.Card_Parse("@MeihuoCard=" .. card:getEffectiveId())
		end
	end
	return
end
sgs.ai_skill_use_func["MeihuoCard"] = function(card, use, self)
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
			if carduse.from:isLord() or carduse.from:getLostHp() > 1 then
				return fcard:getEffectiveId()
			end
		end
	end
	return "."
end

function SmartAI:isNoZhenshaMark()
	if not self.player:isWounded() then return true end
	for _, player in sgs.qlist(self.room:getAlivePlayers()) do
		if self:isEnemy(player) and not player:isKongcheng() and player:getMark("@vi") > 0 then return false end
	end
	return true
end

-- lishishi
-- qinxin
sgs.ai_skill_invoke["qinxin"] = true

-- yinjian
local yinjian_skill={}
yinjian_skill.name = "yinjian"
table.insert(sgs.ai_skills, yinjian_skill)
yinjian_skill.getTurnUseCard = function(self)
    if self.player:hasUsed("YinjianCard") or self.player:getHandcardNum() <= 2 then return end
	return sgs.Card_Parse("@YinjianCard=.")
end
sgs.ai_skill_use_func["YinjianCard"] = function(card, use, self)
	local from, to
	for _, friend in ipairs(self.friends_noself) do
		if friend:getGeneral():isMale() then
			from = friend
			break
		end
	end
	if from then
		for _, friend in ipairs(self.friends) do
			if friend:getGeneral():isMale() and friend ~= from
				and friend:getKingdom() ~= from:getKingdom() then
				to = friend
				break
			end
		end
		if to then
			local cards = sgs.QList2Table(self.player:getCards("h"))
			self:sortByUseValue(cards, true)
			local yinjiancards = {}
			table.insert(yinjiancards, cards[1]:getEffectiveId())
			table.insert(yinjiancards, cards[2]:getEffectiveId())
			use.card = sgs.Card_Parse("@YinjianCard=" .. table.concat(yinjiancards, "+"))
			if use.to then
				use.to:append(from)
				use.to:append(to)
			end
			return
		end
	end
end

-- yanxijiao
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

-- huakui
sgs.ai_skill_invoke["huakui"] = true
