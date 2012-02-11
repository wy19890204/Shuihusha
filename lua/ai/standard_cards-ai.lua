local function hasExplicitRebel(room)
	for _, player in sgs.qlist(room:getAllPlayers()) do
		if sgs.isRolePredictable() and player:getRole() == "rebel" then return true end
		if sgs.ai_explicit[player:objectName()] and sgs.ai_explicit[player:objectName()]:match("rebel") then return true end
	end
	return false
end

function SmartAI:canPaoxiao(player)
	player = player or self.player
	if player:hasWeapon("crossbow") or player:hasSkill("paoxiao") or player:hasFlag("SlashbySlash") then
		return true
	end
	if player:hasSkill("shalu") and player:getMark("shalu") > 0 then
		return true
	end
	if player:hasSkill("qinlong") and not player:hasEquip() then
		return true
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

sgs.ai_skill_invoke.kylin_bow = function(self, data)
	local effect = data:toSlashEffect()

	if self:hasSkills(sgs.lose_equip_skill, effect.to) then
		return self:isFriend(effect.to)
	end

	return self:isEnemy(effect.to)
end

sgs.ai_skill_invoke.eight_diagram = function(self, data)
	if self.player:hasSkill("yueli") then return true end
	if self:getDamagedEffects(self) then return false end
	if self:getDamagedEffects(self) then return false end
	return true
end

function SmartAI:useCardAmazingGrace(card, use)
	if #self.friends >= #self.enemies or (self:hasSkills(sgs.need_kongcheng) and self.player:getHandcardNum() == 1)
		or self.player:hasSkill("longjiao") then
		use.card = card
	end
end

function SmartAI:useCardGodSalvation(card, use)
	local good, bad = 0, 0

	if self.player:hasSkill("wuyan") and self.player:isWounded() then
		use.card = card
		return
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

	if good > bad then
		use.card = card
	end
end

local function factorial(n)
	if n <= 0.1 then return 1 end
	return n*factorial(n-1)
end

function SmartAI:useCardDuel(duel, use)
	if self.player:hasSkill("wuyan") then return end
	self:sort(self.enemies,"handcard")
	local enemies = self:exclude(self.enemies, duel)
	for _, enemy in ipairs(enemies) do
		if self:objectiveLevel(enemy) > 3 then
			local n1 = self:getCardsNum("Slash")
			local n2 = enemy:getHandcardNum()
			if enemy:hasSkill("wushuang") then n2 = n2*2 end
			if self.player:hasSkill("wushuang") then n1 = n1*2 end
			local useduel
			if self:hasTrickEffective(duel, enemy) then
				if n1 >= n2 then
					useduel = true
				elseif n2 > n1*2 + 1 then
					useduel = false
				elseif n1 > 0 then
					local percard = 0.35
					if self:canPaoxiao(enemy) then percard = 0.2 end
					local poss = percard ^ n1 * (factorial(n1)/factorial(n2)/factorial(n1-n2))
					if math.random() > poss then useduel = true end
				end
				if useduel then
					use.card = duel
					if use.to then
						use.to:append(enemy)
						self:speak("duel", self.player:getGeneral():isFemale())
					end
					return
				end
			end
		end
	end
end

sgs.ai_card_intention.Duel=function(card,from,tos,source)
	sgs.updateIntentions(from, tos, 80)
end

function SmartAI:useCardExNihilo(card, use)
	use.card = card
	if not use.isDummy then
		self:speak("lucky")
	end
end

function SmartAI:useCardSnatch(snatch, use)
	if self.player:hasSkill("wuyan") then return end

	if (not self.has_wizard) and self:hasWizard(self.enemies)  then
		-- find lightning
		local players = self.room:getOtherPlayers(self.player)
		players = self:exclude(players, snatch)
		for _, player in ipairs(players) do
			if player:containsTrick("lightning") and not player:hasSkill("wuyan") then
				use.card = snatch
				if use.to then use.to:append(player) end

				return
			end
		end
	end

	self:sort(self.friends_noself,"defense")
	local friends = self:exclude(self.friends_noself, snatch)
	local hasLion, target
	for _, friend in ipairs(friends) do
		if self:hasTrickEffective(snatch, friend) then
			if (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) then
				use.card = snatch
				if use.to then use.to:append(friend) end
				return
			end
			if self:isEquip("SilverLion", friend) and friend:isWounded() and (friend:hasSkill("benghuai") or friend:getHp() < 4) then
				hasLion = true
				target = friend
			end
		end
	end

	if hasLion then
		use.card = snatch
		if use.to then use.to:append(target) end
		return
	end

	self:sort(self.enemies,"defense")
	if sgs.getDefense(self.enemies[1]) >= 8 then self:sort(self.enemies, "threat") end

	local enemies = self:exclude(self.enemies, snatch)
	for _, enemy in ipairs(enemies) do
		if not enemy:isNude() and self:hasTrickEffective(snatch, enemy) and
			not (self:hasSkills(sgs.lose_equip_skill, enemy) and enemy:getHandcardNum() == 0) and
			not (enemy:getCards("he"):length() == 1 and self:isEquip("GaleShell",enemy)) then
			if enemy:getHandcardNum() == 1 then
				if enemy:hasSkill("kongmen") then return end
			end
			use.card = snatch
			if use.to then
				use.to:append(enemy)
				self:speak("hostile", self.player:getGeneral():isFemale())
			end
			return
		end
	end
end

function SmartAI:useCardDismantlement(dismantlement, use)
	if self.player:hasSkill("wuyan") then return end
	if (not self.has_wizard) and self:hasWizard(self.enemies) then
		-- find lightning
		local players = self.room:getOtherPlayers(self.player)
		players = self:exclude(players, dismantlement)
		for _, player in ipairs(players) do
			if player:containsTrick("lightning") and not player:hasSkill("wuyan") then
				use.card = dismantlement
				if use.to then use.to:append(player) end
				return
			end
		end
	end

	self:sort(self.friends_noself,"defense")
	local friends = self:exclude(self.friends_noself, dismantlement)
	local hasLion, target
	for _, friend in ipairs(friends) do
		if self:hasTrickEffective(dismantlement, friend) then
			if (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage")) then
				use.card = dismantlement
				if use.to then use.to:append(friend) end
				return
			end
			if self:isEquip("SilverLion", friend) and friend:isWounded() and (friend:hasSkill("benghuai") or friend:getHp() < 4) then
				hasLion = true
				target = friend
			end
		end
	end

	if hasLion then
		use.card = dismantlement
		if use.to then use.to:append(target) end
		return
	end

	self:sort(self.enemies,"defense")
	if sgs.getDefense(self.enemies[1]) >= 8 then self:sort(self.enemies, "threat") end

	local enemies = self:exclude(self.enemies, dismantlement)
	for _, enemy in ipairs(enemies) do
		local equips = enemy:getEquips()
		if not enemy:isNude() and self:hasTrickEffective(dismantlement, enemy) and not enemy:hasSkill("tuntian") and
			not (self:hasSkills(sgs.lose_equip_skill, enemy) and enemy:getHandcardNum() == 0) and
			not (enemy:getCards("he"):length() == 1 and self:isEquip("GaleShell",enemy)) 
			and self:hasSkills("butian|yixing|qiaojiang|jishi|cuju|kongying", enemy) then
			if enemy:getHandcardNum() == 1 then
				if enemy:hasSkill("kongmen") then return end
			end
			use.card = dismantlement
			if use.to then
				use.to:append(enemy)
				self:speak("hostile", self.player:getGeneral():isFemale())
			end
			return
		end
	end	
	for _, enemy in ipairs(enemies) do
		local equips = enemy:getEquips()
		if not enemy:isNude() and self:hasTrickEffective(dismantlement, enemy) and not enemy:hasSkill("shouge") and
			not (self:hasSkills(sgs.lose_equip_skill, enemy) and enemy:getHandcardNum() == 0) and
			not (enemy:getCards("he"):length() == 1 and self:isEquip("GaleShell",enemy)) then
			if enemy:getHandcardNum() == 1 then
				if enemy:hasSkill("kongmen") then return end
			end
			use.card = dismantlement
			if use.to then
				use.to:append(enemy)
				self:speak("hostile", self.player:getGeneral():isFemale())
			end
			return
		end
	end
end

function SmartAI:useCardCollateral(card, use)
	if self.player:hasSkill("wuyan") then return end
	self:sort(self.enemies,"threat")

	for _, friend in ipairs(self.friends_noself) do
		if friend:getWeapon() and self:hasSkills(sgs.lose_equip_skill, friend) then

			for _, enemy in ipairs(self.enemies) do
				if friend:canSlash(enemy) then
					use.card = card
				end
				if use.to then use.to:append(friend) end
				if use.to then use.to:append(enemy) end
				return
			end
		end
	end

	local n = nil
	local final_enemy = nil
	for _, enemy in ipairs(self.enemies) do
		if not self.room:isProhibited(self.player, enemy, card)
			and self:hasTrickEffective(card, enemy)
			and not self:hasSkill(sgs.lose_equip_skill, enemy)
			and enemy:getWeapon() then

			for _, enemy2 in ipairs(self.enemies) do
				if enemy:canSlash(enemy2) then
					if enemy:getHandcardNum() == 0 then
						use.card = card
						if use.to then use.to:append(enemy) end
						if use.to then use.to:append(enemy2) end
						return
					else
						n = 1;
						final_enemy = enemy2
					end
				end
			end
			if n then use.card = card end
			if use.to then use.to:append(enemy) end
			if use.to then use.to:append(final_enemy) end
			return

		end
		n = nil
	end
end

sgs.ai_use_value.Collateral = 8.8
sgs.ai_use_priority.Collateral = 2.75

sgs.ai_card_intention.Collateral = function(card, from, tos, source)
	assert(#tos == 2)
	if tos[2]:objectName() == from:objectName() then
		sgs.updateIntention(from, tos[1], 80)
	elseif (sgs.ai_loyalty[tos[1]] or 0) * (sgs.ai_loyalty[tos[2]] or 0) > 0 then
		sgs.updateIntention(from, tos[2], 80)
	end
end

sgs.dynamic_value.control_card.Collateral = true

local function hp_subtract_handcard(a,b)
	local diff1 = a:getHp() - a:getHandcardNum()
	local diff2 = b:getHp() - b:getHandcardNum()

	return diff1 < diff2
end

function SmartAI:useCardIndulgence(card, use)
	table.sort(self.enemies, hp_subtract_handcard)
	
	local enemies = self:exclude(self.enemies, card)
	for _, enemy in ipairs(enemies) do
		if enemy:hasSkill("baoguo") and not enemy:containsTrick("indulgence") and not enemy:isKongcheng() and enemy:faceUp() then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end
	
	for _, enemy in ipairs(enemies) do
		if not enemy:containsTrick("indulgence") and not enemy:hasSkill("shemi") and enemy:faceUp() then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end
end

function SmartAI:useCardLightning(card, use)
	if self.player:containsTrick("lightning") then return end
	if self.player:hasSkill("weimu") and card:isBlack() then return end

	if not self:hasWizard(self.enemies) then--and self.room:isProhibited(self.player, self.player, card) then
		if self:hasWizard(self.friends) then
			use.card = card
			return
		end
		local players = self.room:getAllPlayers()
		players = sgs.QList2Table(players)

		local friends = 0
		local enemies = 0

		for _,player in ipairs(players) do
			if self:objectiveLevel(player) >= 4 then
				enemies = enemies + 1
			elseif self:isFriend(player) then
				friends = friends + 1
			end
		end

		local ratio

		if friends == 0 then ratio = 999
		else ratio = enemies/friends
		end

		if ratio > 1.5 then
			use.card = card
			return
		end
	end
end

