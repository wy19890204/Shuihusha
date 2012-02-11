local function hasExplicitRebel(room)
	for _, player in sgs.qlist(room:getAllPlayers()) do
		if sgs.isRolePredictable() and player:getRole() == "rebel" then return true end
		if sgs.ai_explicit[player:objectName()] and sgs.ai_explicit[player:objectName()]:match("rebel") then return true end
	end
	return false
end

function SmartAI:slashProhibit(card,enemy)
	if card == nil then
		card = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	end

--	if self:isWeak() and self:hasSkills("enyuan|ganglie", enemy) then return true end
	if self:isFriend(enemy) then
		if card:inherits("FireSlash") or self.player:hasWeapon("fan") then
			if self:isEquip("Vine", enemy) then return true end
		end
		if enemy:isChained() and card:inherits("NatureSlash") and #(self:getChainedFriends())>1 and
			self:slashIsEffective(card,enemy) then return true end
		if self:getCardsNum("Jink",enemy) == 0 and enemy:getHp() < 2 and self:slashIsEffective(card,enemy) then return true end
		if enemy:isLord() and self:isWeak(enemy) and self:slashIsEffective(card,enemy) then return true end
		if self:hasSkills("duanchang|huilei|dushi", enemy) and self:isWeak(enemy) then return true end
		if self:isEquip("GudingBlade") and enemy:isKongcheng() then return true end
	else
		if card:inherits("NatureSlash") and self:isEquip("GoldArmor", enemy) then
			return true
		end

		if enemy:hasSkill("huanshu") then
			if self.player:hasSkill("jueqing") or	self:isEquip("MeteorSword") then return false end
			return true
		end

		if enemy:hasSkill("yueli") then
			if self:isEquip("EightDiagram", enemy) then return true end
		end

		if enemy:isChained() and #(self:getChainedFriends()) > #(self:getChainedEnemies()) and self:slashIsEffective(card,enemy) then
			return true
		end
		if enemy:hasSkill("foyuan") and self.player:getGeneral():isMale() and not self.player:hasEquip() then
			return true
		end

		if enemy:hasSkill("danshu") then
			local x = enemy:getLostHp()
			return self.player:getHandcardNum() <= x + 1
		end

		if enemy ~= self.room:getCurrent() and enemy:hasSkill("jueming") and enemy:getHp() == 1 then
			return true
		end
	end

	return not self:slashIsEffective(card, enemy)
end

function SmartAI:slashIsEffective(slash, to)
	if to:hasSkill("jueming") and to:getHp() == 1 then
		return false
	end
	if to:hasSkill("qianshui") and not self.player:getWeapon() then
		return false
	end

	local nature = {
		Slash = sgs.DamageStruct_Normal,
		FireSlash = sgs.DamageStruct_Fire,
		ThunderSlash = sgs.DamageStruct_Thunder,
	}

	if not self:damageIsEffective(to, nature[slash:className()]) then return false end

	if self.player:hasWeapon("qinggang_sword") or self.player:hasSkill("wuzu") or (self.player:hasSkill("yinyu") and self.player:getMark("qinggang") > 0) then
		return true
	end

	local armor = to:getArmor()
	if armor then
		if armor:objectName() == "renwang_shield" then
			return not slash:isBlack()
		elseif armor:objectName() == "vine" then
			return slash:inherits("NatureSlash") or self.player:hasWeapon("fan")
		elseif armor:objectName() == "gold_armor" then
			return not self.player:getWeapon() and not slash:inherits("NatureSlash")
		end
	end

	return true
end

function SmartAI:slashIsAvailable(player)
	player = player or self.player
--	if player:hasFlag("tianyi_failed") or player:hasFlag("xianzhen_failed") then return false end

	if self:canPaoxiao() then
		return true
	end
	if player:hasSkill("huafo") then
		local rand = math.random(1, 3)
		return rand == 2
	end
	return (player:usedTimes("Slash") + player:usedTimes("FireSlash") + player:usedTimes("ThunderSlash")) < 1
end

function SmartAI:useCardSlash(card, use)
	if not self:slashIsAvailable() then return end
	local no_distance = self.slash_distance_limit
	if card:getSkillName() == "paohong" and card:isBlack() then no_distance = true end
	if self.player:hasFlag("Longest") then no_distance = true end
	if self.player:hasSkill("qinlong") and not self.player:hasEquip() then
		self.slash_targets = 2
	end
	if self.player:hasWeapon("sun_bow") and card:isRed() and not card:inherits("NatureSlash") then
		self.slash_targets = 2
	end
	if (self.player:getHandcardNum() == 1
	and self.player:getHandcards():first():inherits("Slash")
	and self.player:getWeapon()
	and self.player:getWeapon():inherits("Halberd")) then
		self.slash_targets = 3
	end

	self.predictedRange = self.player:getAttackRange()
	local target_count = 0
	if self.player:hasSkill("jishi") and self:isWeak() and self:getOverflow() == 0 then return end
	for _, friend in ipairs(self.friends_noself) do
		local slash_prohibit = false
		slash_prohibit = self:slashProhibit(card,friend)
		if (self.player:hasSkill("qiangqu") and friend:getGeneral():isFemale() and friend:isWounded() and not friend:isNude())
		or (friend:hasSkill("kongying") 
		and (self:getCardsNum("Jink", friend) > 0 or (not self:isWeak(friend) and self:isEquip("EightDiagram",friend)))
		and (hasExplicitRebel(self.room) or not friend:isLord()))
		then
			if not slash_prohibit then
				if ((self.player:canSlash(friend, not no_distance)) or
					(use.isDummy and (self.player:distanceTo(friend) <= self.predictedRange))) and
					self:slashIsEffective(card, friend) then
					use.card = card
					if use.to then
						use.to:append(friend)
						self:speak("hostile", self.player:getGeneral():isFemale())
					end
					target_count = target_count+1
					if self.slash_targets <= target_count then return end
				end
			end
--				break
		end
	end

	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		local slash_prohibit = false
		slash_prohibit = self:slashProhibit(card,enemy)
		if not slash_prohibit then
			if (self.player:canSlash(enemy, not no_distance) or
			(use.isDummy and self.predictedRange and (self.player:distanceTo(enemy) <= self.predictedRange))) and
			self:objectiveLevel(enemy) > 3 and
			self:slashIsEffective(card, enemy) and
			not (not self:isWeak(enemy) and #self.enemies > 1 and #self.friends > 1 and self.player:hasSkill("shemi")
				and self:getOverflow() > 0 and not self:isEquip("Crossbow")) then
				-- fill the card use struct
				local mi = self:searchForEcstasy(use,enemy,card)
				if mi and self:getCardsNum("Jink", enemy) > 0 and self:getCardId("Slash") then
					use.card = mi
					if use.to then use.to:append(enemy) end
					return
				end
				if not use.to or use.to:isEmpty() then
					local anal = self:searchForAnaleptic(use,enemy,card)
					if anal and self:isNoZhenshaMark() and not self:isEquip("SilverLion", enemy) and not self:isWeak() then
						use.card = anal
						return
					end
					if self.player:getGender()~=enemy:getGender() and self:getCardsNum("DoubleSword",self.player,"h") > 0 then
						self:useEquipCard(self:getCard("DoubleSword"), use)
						if use.card then return end
					end
					if enemy:isKongcheng() and self:getCardsNum("GudingBlade", self.player, "h") > 0 then
						self:useEquipCard(self:getCard("GudingBlade"), use)
						if use.card then return end
					end
					if self:getOverflow()>0 and self:getCardsNum("Axe", self.player, "h") > 0 then
						self:useEquipCard(self:getCard("Axe"), use)
						if use.card then return end
					end
					if enemy:getArmor() and self:getCardsNum("Fan", self.player, "h") > 0 and
						(enemy:getArmor():inherits("Vine") or enemy:getArmor():inherits("GaleShell")) then
						self:useEquipCard(self:getCard("Fan"), use)
						if use.card then return end
					end
					if enemy:getDefensiveHorse() and self:getCardsNum("KylinBow", self.player, "h") > 0 then
						self:useEquipCard(self:getCard("KylinBow") ,use)
						if use.card then return end
					end
					end
				use.card = card
				if use.to then use.to:append(enemy) end
				target_count = target_count+1
				if self.slash_targets <= target_count then return end
			end
		end
	end

	for _, friend in ipairs(self.friends_noself) do
		if friend:hasSkill("baoguo") and friend:getHp() > 1 and
			not (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) then
			local slash_prohibit = false
			slash_prohibit = self:slashProhibit(card, friend)
			if not slash_prohibit then
				if ((self.player:canSlash(friend, not no_distance)) or
					(use.isDummy and (self.player:distanceTo(friend) <= self.predictedRange))) and
					self:slashIsEffective(card, friend) then
					use.card = card
					if use.to then
						use.to:append(friend)
						self:speak("yiji")
					end
					target_count = target_count+1
					if self.slash_targets <= target_count then return end
				end
			end
			break
		end
	end
end

sgs.ai_skill_use.slash = function(self, prompt)
	if prompt ~= "@askforslash" then return "." end
	local slash = self:getCard("Slash")
	if not slash then return "." end
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy, true) and not self:slashProhibit(slash, enemy) and self:slashIsEffective(slash, enemy) then
			return ("%s->%s"):format(slash:toString(), enemy:objectName())
		end
	end
	return "."
end

sgs.ai_skill_playerchosen.zero_card_as_slash = function(self, targets)
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	local targetlist=sgs.QList2Table(targets)
	self:sort(targetlist, "defense")
	for _, target in ipairs(targetlist) do
		if self:isEnemy(target) and not self:slashProhibit(slash ,target) and self:slashIsEffective(slash,target) then
			return target
		end
	end
	for i=#targetlist, 1, -1 do
		if not self:slashProhibit(slash, targetlist[i]) then
			return targetlist[i]
		end
	end
	return targets:first()
end

sgs.ai_card_intention.Slash = function(card,from,tos,source)
	if from:objectName() ~= source:objectName() then return end
	for _, to in ipairs(tos) do
		local value = 80
		if sgs.ai_collateral then sgs.ai_collateral=false value = 0 end

		if sgs.ai_leiji_effect then
			if from and from:hasSkill("liegong") then return end
			sgs.ai_leiji_effect = false
			if sgs.ai_pojun_effect then value = value/1.5 else value = -value/1.5 end
		end
		speakTrigger(card,from,to)
		if to:hasSkill("baoguo") then 
			value = value*(2-to:getHp())/1.1
		end
		if from:hasSkill("dujian") and to:getHp() > 3 then value = 0 end
		sgs.updateIntention(from, to, value)
	end
end


function SmartAI:useCardPeach(card, use)
	if not self.player:isWounded() then return end
	math.min(self.player:getMaxCards(), self.player:getHandcardNum()) + self.player:getCards("e"):length() > 3 then return end
	if not (self.player:hasSkill("rende") and self:getOverflow() > 1 and #self.friends_noself > 0) then
		local peaches = 0
		local cards = self.player:getHandcards()
		cards = sgs.QList2Table(cards)
		for _,card in ipairs(cards) do
			if card:inherits("Peach") then peaches = peaches+1 end
		end

		for _, friend in ipairs(self.friends_noself) do
			if (self.player:getHp()-friend:getHp() > peaches) and (friend:getHp() < 3) then return end
		end

		use.card = card
	end
end

sgs.ai_skill_invoke.ice_sword=function(self, data)
	if self.player:hasFlag("drank") then return false end
	local effect = data:toSlashEffect() 
	local target = effect.to
	if self:isFriend(target) then
		if self:isWeak(target) then return true
		elseif target:getLostHp()<1 then return false end
		return true
	else
		if self:isWeak(target) then return false end
		if target:getArmor() and self:evaluateArmor(target:getArmor(), target)>3 then return true end
		local num = target:getHandcardNum()
		if self.player:hasSkill("tieji") or (self.player:hasSkill("liegong")
			and (num >= self.player:getHp() or num <= self.player:getAttackRange())) then return false end
		if target:hasSkill("tuntian") then return false end
		if self:hasSkills(sgs.need_kongcheng, target) then return false end
		if target:getCards("he"):length()<4 and target:getCards("he"):length()>1 then return true end
		return false
	end
end

local spear_skill={}
spear_skill.name="spear"
table.insert(sgs.ai_skills,spear_skill)
spear_skill.getTurnUseCard=function(self,inclusive)
	local cards = self.player:getCards("h")	
	cards=sgs.QList2Table(cards)

	if #cards<(self.player:getHp()+1) then return nil end
	if #cards<2 then return nil end
	if self:getCard("Slash") then return nil end

	self:sortByUseValue(cards,true)

	local suit1 = cards[1]:getSuitString()
	local card_id1 = cards[1]:getEffectiveId()
	
	local suit2 = cards[2]:getSuitString()
	local card_id2 = cards[2]:getEffectiveId()

	local suit="no_suit"
	if cards[1]:isBlack() == cards[2]:isBlack() then suit = suit1 end

	local card_str = ("slash:spear[%s:%s]=%d+%d"):format(suit, 0, card_id1, card_id2)

	local slash = sgs.Card_Parse(card_str)

	return slash
	
end

local qixi_skill={}
qixi_skill.name="qixi"
table.insert(sgs.ai_skills,qixi_skill)
qixi_skill.getTurnUseCard=function(self,inclusive)
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)
	
	local black_card
	
	self:sortByUseValue(cards,true)
	
	local has_weapon=false
	
	for _,card in ipairs(cards)  do
		if card:inherits("Weapon") and card:isRed() then has_weapon=true end
	end
	
	for _,card in ipairs(cards)  do
		if card:isBlack()  and ((self:getUseValue(card)<sgs.ai_use_value["Dismantlement"]) or inclusive or self:getOverflow()>0) then
			local shouldUse=true

			if card:inherits("Armor") then
				if not self.player:getArmor() then shouldUse=false 
				elseif self:hasEquip(card) and not (card:inherits("SilverLion") and self.player:isWounded()) then shouldUse=false
				end
			end

			if card:inherits("Weapon") then
				if not self.player:getWeapon() then shouldUse=false
				elseif self:hasEquip(card) and not has_weapon and not card:inherits("YitianSword") then shouldUse=false
				end
			end
			
			if card:inherits("Slash") then
				local dummy_use = {isDummy = true}
				if self:getCardsNum("Slash") == 1 then
					self:useBasicCard(card, dummy_use)
					if dummy_use.card then shouldUse = false end
				end
			end

			if shouldUse then
				black_card = card
				break
			end
			
		end
	end

	if black_card then
		local suit = black_card:getSuitString()
		local number = black_card:getNumberString()
		local card_id = black_card:getEffectiveId()
		local card_str = ("dismantlement:qixi[%s:%s]=%d"):format(suit, number, card_id)
		local dismantlement = sgs.Card_Parse(card_str)
		
		assert(dismantlement)

		return dismantlement
	end
end

local wusheng_skill={}
wusheng_skill.name="wusheng"
table.insert(sgs.ai_skills,wusheng_skill)
wusheng_skill.getTurnUseCard=function(self,inclusive)
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)
	
	local red_card
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards) do
		if card:isRed() and not card:inherits("Slash") and not card:inherits("Peach") 				--not peach
			and ((self:getUseValue(card)<sgs.ai_use_value["Slash"]) or inclusive) then
			red_card = card
			break
		end
	end

	if red_card then		
		local suit = red_card:getSuitString()
		local number = red_card:getNumberString()
		local card_id = red_card:getEffectiveId()
		local card_str = ("slash:wusheng[%s:%s]=%d"):format(suit, number, card_id)
		local slash = sgs.Card_Parse(card_str)
		
		assert(slash)
		
		return slash
	end
end

local longdan_skill={}
longdan_skill.name="longdan"
table.insert(sgs.ai_skills,longdan_skill)
longdan_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("h")	
	cards=sgs.QList2Table(cards)
	
	local jink_card
	
	self:sortByUseValue(cards,true)
	
	for _,card in ipairs(cards)  do
		if card:inherits("Jink") then
			jink_card = card
			break
		end
	end
	
	if not jink_card then return nil end
	local suit = jink_card:getSuitString()
	local number = jink_card:getNumberString()
	local card_id = jink_card:getEffectiveId()
	local card_str = ("slash:longdan[%s:%s]=%d"):format(suit, number, card_id)
	local slash = sgs.Card_Parse(card_str)
	assert(slash)
	
	return slash
		
end

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

local kurou_skill={}
kurou_skill.name="kurou"
table.insert(sgs.ai_skills,kurou_skill)
kurou_skill.getTurnUseCard=function(self,inclusive)
	if  (self.player:getHp() > 3 and self.player:getHandcardNum() > self.player:getHp()) or		
		(self.player:getHp() - self.player:getHandcardNum() >= 2) then
		return sgs.Card_Parse("@KurouCard=.")
	end
		
	--if not inclusive then return nil end
		
	if self.player:getWeapon() and self.player:getWeapon():inherits("Crossbow") then
		for _, enemy in ipairs(self.enemies) do
			if self.player:canSlash(enemy,true) and self.player:getHp()>1 then
				return sgs.Card_Parse("@KurouCard=.")
			end
		end
	end
end

sgs.ai_skill_use_func["KurouCard"]=function(card,use,self)
	
	if not use.isDummy then self:speak("kurou") end
	
	use.card=card
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

local zhiheng_skill={}
zhiheng_skill.name="zhiheng"
table.insert(sgs.ai_skills, zhiheng_skill)
zhiheng_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("ZhihengCard") then 
		return sgs.Card_Parse("@ZhihengCard=.")
	end
end

sgs.ai_skill_use_func["ZhihengCard"] = function(card, use, self)
	local unpreferedCards={}
	local cards=sgs.QList2Table(self.player:getHandcards())
	
	if self.player:getHp() < 3 then
		local zcards = self.player:getCards("he")
		for _, zcard in sgs.qlist(zcards) do
			if not zcard:inherits("Peach") and not zcard:inherits("ExNihilo") then
				if self:getAllPeachNum()>0 or not zcard:inherits("Shit") then table.insert(unpreferedCards,zcard:getId()) end
			end	
		end
	end
	
	if #unpreferedCards == 0 then 
		if self:getCardsNum("Slash")>1 then 
			self:sortByKeepValue(cards)
			for _,card in ipairs(cards) do
				if card:inherits("Slash") then table.insert(unpreferedCards,card:getId()) end
			end
			table.remove(unpreferedCards,1)
		end
		
		local num=self:getCardsNum("Jink")-1							
		if self.player:getArmor() then num=num+1 end
		if num>0 then
			for _,card in ipairs(cards) do
				if card:inherits("Jink") and num>0 then 
					table.insert(unpreferedCards,card:getId())
					num=num-1
				end
			end
		end
		for _,card in ipairs(cards) do
			if card:inherits("EquipCard") then
				if (card:inherits("Weapon") and self.player:getHandcardNum() < 3) or
				card:inherits("OffensiveHorse") or
				(self:hasSameEquip(card, self.player) and not card:inherits("DefensiveHorse")) or
				card:inherits("AmazingGrace") or
				card:inherits("Lightning") then
					table.insert(unpreferedCards,card:getId())
				end
			end
		end
	
		if self.player:getWeapon() and self.player:getHandcardNum()<3 then
			table.insert(unpreferedCards, self.player:getWeapon():getId())
		end
				
		if (self:isEquip("SilverLion") and self.player:isWounded()) or self:isEquip("GaleShell") then
			table.insert(unpreferedCards, self.player:getArmor():getId())
		end	

		if self.player:getOffensiveHorse() and self.player:getWeapon() then
			table.insert(unpreferedCards, self.player:getOffensiveHorse():getId())
		end
	end	
	
	for index = #unpreferedCards, 1, -1 do
		if self.player:isJilei(sgs.Sanguosha:getCard(unpreferedCards[index])) then table.remove(unpreferedCards, index) end
	end
	
	if #unpreferedCards>0 then 
		use.card = sgs.Card_Parse("@ZhihengCard="..table.concat(unpreferedCards,"+")) 
		return 
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

