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
			if cards[1] then return cards[1]:getEffectiveId() end
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
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
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
	if self.player:hasUsed("HuaceCard") then return end
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
	local card
	self:sortByUseValue(cards, true)
	for _, acard in ipairs(cards)  do
		if acard:inherits("EventsCard") or acard:inherits("TrickCard") then
			card = acard:getEffectiveId()
			break
		end
	end
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
	for _, enemy in ipairs(self.enemies) do
		if n1 + 1 > self:getCardsNum("Slash", enemy) then
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
local jieyin_skill={}
jieyin_skill.name="jieyin"
table.insert(sgs.ai_skills,jieyin_skill)
jieyin_skill.getTurnUseCard=function(self)
	if self.player:getHandcardNum()<2 then return nil end
	if self.player:hasUsed("JieyinCard") then return nil end

	local cards = self.player:getHandcards()
	cards=sgs.QList2Table(cards)

	local first, second
	self:sortByUseValue(cards,true)
	for _, card in ipairs(cards) do
		if card:getTypeId() ~= sgs.Card_Equip and not (card:inherits("Shit") and self:isWeak() and self:getAllPeachNum()==0) then
			if not first then first  = cards[1]:getEffectiveId()
			else second = cards[2]:getEffectiveId()
			end
		end
		if second then break end
	end

	if not second then return end
	local card_str = ("@JieyinCard=%d+%d"):format(first, second)
	assert(card_str)
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func["JieyinCard"]=function(card,use,self)
	self:sort(self.friends, "hp")

	for _, friend in ipairs(self.friends) do
		if friend:getGeneral():isMale() and friend:isWounded() then
			use.card=card
			if use.to then use.to:append(friend) end
			return
		end
	end
end

local rende_skill={}
rende_skill.name="rende"
table.insert(sgs.ai_skills, rende_skill)
rende_skill.getTurnUseCard=function(self)
	if self.player:isKongcheng() then return end
	for _, player in ipairs(self.friends_noself) do
		if ((player:hasSkill("haoshi") and not player:containsTrick("supply_shortage"))
			or player:hasSkill("longluo")
			or (not player:containsTrick("indulgence") and (player:hasSkill("zhiheng") or player:hasSkill("yishe")))
			)
			and player:faceUp() then
			return sgs.Card_Parse("@RendeCard=.")
		end
	end
	if (self.player:usedTimes("RendeCard") < 2 or self:getOverflow() > 0 or self:getCard("Shit")) then
		return sgs.Card_Parse("@RendeCard=.")
	end
	if self.player:getLostHp() < 2 then
		return sgs.Card_Parse("@RendeCard=.")
	end
end

sgs.ai_skill_use_func["RendeCard"] = function(card, use, self)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards,true)
	local name = self.player:objectName()
	if #self.friends > 1 then
		local zhangfei, huatuo, zhangjiao, sunshangxiang, huangyueying
		zhangfei = self.room:findPlayerBySkillName("paoxiao")
		huatuo = self.room:findPlayerBySkillName("qingnang")
		zhangjiao = self.room:findPlayerBySkillName("leiji")
		huangyueying = self.room:findPlayerBySkillName("qicai")
		for _, hcard in ipairs(cards) do
			if not hcard:inherits("Shit") then
				if hcard:inherits("Analeptic") or hcard:inherits("Peach") then
					self:sort(self.friends_noself, "hp")
					if #self.friends>1 and self.friends_noself[1]:getHp() == 1 then
						use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
						if use.to then use.to:append(self.friends_noself[1]) end
						return
					end
				end
				self:sort(self.friends_noself, "hp")
				local friend = self.friends_noself[1]
				if friend and friend:getHp() == 1 and huatuo and self:isFriend(huatuo) and hcard:isRed() and huatuo:objectName()~=name then
					use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
					if use.to then use.to:append(huatuo) end
					return
				end
				if zhangfei and self:isFriend(zhangfei) and hcard:inherits("Slash") and zhangfei:objectName() ~= name and
					not zhangfei:containsTrick("indulgence") then
					use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
					if use.to then use.to:append(zhangfei) end
					return
				end
				if zhangjiao and self:isFriend(zhangjiao) and hcard:inherits("Jink") and zhangjiao:objectName() ~= name and
					self:getCardsNum("Jink")>1 then
					use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
					if use.to then use.to:append(zhangjiao) end
					return
				end
				if huangyueying and self:isFriend(huangyueying) and hcard:inherits("TrickCard") and huangyueying:objectName() ~= name and
					not (huangyueying:containsTrick("indulgence") and not hcard:inherits("Nullification")) then
					use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
					if use.to then use.to:append(huangyueying) end
					return
				end
				if self:getUseValue(hcard)<6 and #self.friends>1 then
					for _, friend in ipairs(self.friends_noself) do
						if sgs[friend:getGeneralName() .. "_suit_value"] and
							(sgs[friend:getGeneralName() .. "_suit_value"][hcard:getSuitString()] or 0)>=3.9 then
							use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
							if use.to then use.to:append(friend) end
							return
						end
						if friend:getGeneral2Name()~="" then
							if sgs[friend:getGeneral2Name() .. "_suit_value"] and
								(sgs[friend:getGeneral2Name() .. "_suit_value"][hcard:getSuitString()] or 0)>=3.9 then
								use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
								if use.to then use.to:append(friend) end
								return
							end
						end
						if hcard:getNumber()>10 and self:hasSkills("tianyi|xianzhen|quhu", friend) then
							use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
							if use.to then use.to:append(friend) end
							return
						elseif hcard:getNumber()<8 and friend:hasSkill("chengxiang") and friend:getHandcardNum() < 12 then
							use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
							if use.to then use.to:append(friend) end
							return
						end
					end
				end
				local dummy_use = {isDummy = true}
				self:useSkillCard(sgs.Card_Parse("@ZhibaCard=."), dummy_use)
				if dummy_use.card then
					local subcard = sgs.Sanguosha:getCard(dummy_use.card:getEffectiveId())
					if self:getUseValue(subcard) < 6 and #self.friends > 1 then
						for _, player in ipairs(self.friends_noself) do
							if player:getKingdom() == "wu" then
								use.card = sgs.Card_Parse("@RendeCard=" .. subcard:getId())
								if use.to then use.to:append(player) end
								return
							end
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
						use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
						if use.to then use.to:append(target) end
						return
					end
				end
				if hcard:inherits("EquipCard") then
					self:sort(self.friends_noself)
					for _, friend in ipairs(self.friends_noself) do
						if not self:hasSameEquip(hcard, friend) or friend:hasSkill("shensu")
							or (self:hasSkills("zhijian|mingce|xiaoji|xuanfeng", friend) and not friend:containsTrick("indulgence"))  then
							use.card = sgs.Card_Parse("@RendeCard=" .. hcard:getId())
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
		use.card = sgs.Card_Parse("@RendeCard=" .. shit:getId())
		self:sort(self.enemies,"hp")
		if use.to then use.to:append(self.enemies[1]) end
		return
	end

	local zhugeliang = self.room:findPlayerBySkillName("kongcheng")
	if zhugeliang and zhugeliang:objectName() ~= self.player:objectName() and self:isEnemy(zhugeliang) and zhugeliang:isKongcheng() then
		local shit = self:getCard("Shit") or self:getCard("Disaster") or self:getCard("GodSalvation") or self:getCard("AmazingGrace")
		if shit then
			use.card = sgs.Card_Parse("@RendeCard=" .. shit:getId())
			if use.to then use.to:append(zhugeliang) end
			return
		end
		for _, card in ipairs(self:getCards("EquipCard")) do
			if self:hasSameEquip(card, zhugeliang) or (card:inherits("OffensiveHorse") and not card:inherits("Monkey")) then
				use.card = sgs.Card_Parse("@RendeCard=" .. card:getId())
				if use.to then use.to:append(zhugeliang) end
				return
			end
		end
		if zhugeliang:getHp() < 2 then
			local slash = self:getCard("Slash")
			if slash then
				use.card = sgs.Card_Parse("@RendeCard=" .. slash:getId())
				if use.to then use.to:append(zhugeliang) end
				return
			end
		end
	end

	if #self.friends == 1 then return end

	if (self:getOverflow()>0 or (self.player:isWounded() and self.player:usedTimes("RendeCard") < 2)) then
		self:sort(self.friends_noself, "handcard")
		local friend
		for _, player in ipairs(self.friends_noself) do
			if (player:isKongcheng() and (player:hasSkill("kongcheng") or (player:hasSkill("zhiji") and not player:hasSkill("guanxing")))) or
				(not self:isWeak(player) and self:hasSkills(sgs.need_kongcheng,player)) then
			elseif not player:containsTrick("indulgence") then friend = player break end
		end
		if friend then
			local card_id = self:getCardRandomly(self.player, "h")
			use.card = sgs.Card_Parse("@RendeCard=" .. card_id)
			if use.to then use.to:append(friend) end
			return
		end
	end

	if self.player:getHandcardNum()==1 then
		for _, enemy in ipairs(self.enemies) do
			if self:isEquip("GudingBlade", enemy) and enemy:canSlash(self.player, true) then return end
		end
	end

	local special={}
	for _,player in ipairs(self.friends_noself) do
		if ((player:hasSkill("haoshi") and not player:containsTrick("supply_shortage"))
			or player:hasSkill("longluo")
			or (not player:containsTrick("indulgence") and (player:hasSkill("zhiheng") or player:hasSkill("yishe")))
			)
			and player:faceUp() then
			table.insert(special, player)
		end
	end

	if #special>0 then
		self:sort(special, "defense")
		for _, card in sgs.qlist(self.player:getHandcards()) do
			if not card:inherits("Jink") or self:getCardsNum("Jink")>1 then
				use.card = sgs.Card_Parse("@RendeCard=" .. card:getId())
				if use.to then use.to:append(special[1]) end
				return
			end
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
				if friend:getHandcardNum() + draw < friend:getMaxCards() and not friend:containsTrick("indulgence") and not
					((friend:isKongcheng() and (friend:hasSkill("kongcheng") or (friend:hasSkill("zhiji") and not friend:hasSkill("guanxing")))) or
					(not self:isWeak(friend) and self:hasSkills(sgs.need_kongcheng,friend))) then
					use.card = sgs.Card_Parse("@RendeCard=" .. card:getId())
					if use.to then use.to:append(friend) end
					return
				end
			end
			self:sort(self.friends_noself, "handcard")
			for _, friend in ipairs(self.friends_noself) do
				if not friend:containsTrick("indulgence") then
					use.card = sgs.Card_Parse("@RendeCard=" .. card:getId())
					if use.to then use.to:append(self.friends_noself[1]) end
					return
				end
			end
		end
	end
end

local lijian_skill={}
lijian_skill.name="lijian"
table.insert(sgs.ai_skills,lijian_skill)
lijian_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("LijianCard") then
		return
	end
	if not self.player:isNude() then
		local card
		local card_id
		if self:isEquip("SilverLion") and self.player:isWounded() then
			card = sgs.Card_Parse("@LijianCard=" .. self.player:getArmor():getId())
		elseif self.player:getHandcardNum() > self.player:getHp() then
			local cards = self.player:getHandcards()
			cards=sgs.QList2Table(cards)

			for _, acard in ipairs(cards) do
				if (acard:inherits("BasicCard") or acard:inherits("EquipCard") or acard:inherits("AmazingGrace"))
					and not acard:inherits("Peach") and not acard:inherits("Shit") then
					card_id = acard:getEffectiveId()
					break
				end
			end
		elseif not self.player:getEquips():isEmpty() then
			local player=self.player
			if player:getWeapon() then card_id=player:getWeapon():getId()
			elseif player:getOffensiveHorse() then card_id=player:getOffensiveHorse():getId()
			elseif player:getDefensiveHorse() then card_id=player:getDefensiveHorse():getId()
			elseif player:getArmor() and player:getHandcardNum()<=1 then card_id=player:getArmor():getId()
			end
		end
		if not card_id then
			cards=sgs.QList2Table(self.player:getHandcards())
			for _, acard in ipairs(cards) do
				if (acard:inherits("BasicCard") or acard:inherits("EquipCard") or acard:inherits("AmazingGrace"))
					and not acard:inherits("Peach") and not acard:inherits("Shit") then
					card_id = acard:getEffectiveId()
					break
				end
			end
		end
		if not card_id then
			return nil
		else
			card = sgs.Card_Parse("@LijianCard=" .. card_id)
			return card
		end
	end
	return nil
end

sgs.ai_skill_use_func["LijianCard"]=function(card,use,self)
	local findFriend_maxSlash=function(self,first)
		self:log("Looking for the friend!")
		local maxSlash = 0
		local friend_maxSlash
		for _, friend in ipairs(self.friends_noself) do
			if (self:getCardsNum("Slash", friend)> maxSlash) and friend:getGeneral():isMale() then
				maxSlash=self:getCardsNum("Slash", friend)
				friend_maxSlash = friend
			end
		end
		if friend_maxSlash then
			local safe = false
			if (first:hasSkill("ganglie") or first:hasSkill("fankui") or first:hasSkill("enyuan")) then
				if (first:getHp()<=1 and first:getHandcardNum()==0) then safe=true end
			elseif (self:getCardsNum("Slash", friend_maxSlash) >= first:getHandcardNum()) then safe=true end
			if safe then return friend_maxSlash end
		else self:log("unfound")
		end
		return nil
	end

	if not self.player:hasUsed("LijianCard") then
		self:sort(self.enemies, "hp")
		local males = {}
		local first, second
		local zhugeliang_kongcheng
		for _, enemy in ipairs(self.enemies) do
			if zhugeliang_kongcheng and #males==1 then table.insert(males, zhugeliang_kongcheng) end
			if enemy:getGeneral():isMale() and not enemy:hasSkill("wuyan") then
				if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then	zhugeliang_kongcheng=enemy
				else table.insert(males, enemy)	end
				if #males >= 2 then	break end
			end
		end
		if (#males==1) and #self.friends_noself>0 then
			self:log("Only 1")
			first = males[1]
			if zhugeliang_kongcheng then table.insert(males, zhugeliang_kongcheng)
			else
				local friend_maxSlash = findFriend_maxSlash(self,first)
				if friend_maxSlash then table.insert(males, friend_maxSlash) end
			end
		end
		if (#males >= 2) then
			first = males[1]
			second = males[2]
			local lord = self.room:getLord()
			if (first:getHp()<=1) then
				if self.player:isLord() or isRolePredictable() then
					local friend_maxSlash = findFriend_maxSlash(self,first)
					if friend_maxSlash then second=friend_maxSlash end
				elseif (lord:getGeneral():isMale()) and (not lord:hasSkill("wuyan")) then
					if (self.role=="rebel") and (not first:isLord()) then
						second = lord
					else
						if ((self.role=="loyalist" or (self.role=="renegade") and not (first:hasSkill("ganglie") and first:hasSkill("enyuan"))))
							and	( self:getCardsNum("Slash", first)<=self:getCardsNum("Slash", second)) then
							second = lord
						end
					end
				end
			end

			if first and second then
				if use.to then
					use.to:append(first)
					use.to:append(second)
				end
			end
			use.card=card
		end
	end
end
