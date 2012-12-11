function hasSilverLion(who)
	if not who then return false end
	if who:hasSkill("jintang") and who:getHp() <= 2 then
		return true
	end
	return who:getArmor() and who:getArmor():objectName() == "silver_lion"
end

function SmartAI:useCardThunderSlash(...)
	self:useCardSlash(...)
end

sgs.ai_card_intention.ThunderSlash = sgs.ai_card_intention.Slash

sgs.ai_use_value.ThunderSlash = 4.5
sgs.ai_keep_value.ThunderSlash = 2.5
sgs.ai_use_priority.ThunderSlash = 2.5

function SmartAI:useCardFireSlash(...)
	self:useCardSlash(...)
end

sgs.ai_card_intention.FireSlash = sgs.ai_card_intention.Slash

sgs.ai_use_value.FireSlash = 4.4
sgs.ai_keep_value.FireSlash = 2.6
sgs.ai_use_priority.FireSlash = 2.6

sgs.weapon_range.Fan = 4
sgs.ai_use_priority.Fan = 2.655
sgs.ai_use_priority.Vine = 0.6

sgs.ai_skill_invoke.fan = function(self, data)
	local target = data:toSlashEffect().to
		if self:isFriend(target) then
			return target:isChained() and self:isGoodChainTarget(target)
		else
			return not (target:isChained() and not self:isGoodChainTarget(target))
		end
end

function sgs.ai_weapon_value.fan(self, enemy)
	if enemy and (self:isEquip("Vine", enemy) or self:isEquip("GaleShell", enemy)) then return 3 end
end

function sgs.ai_armor_value.vine(player, self)
	for _, enemy in ipairs(self:getEnemies(player)) do
		if (enemy:canSlash(player) and self:isEquip("Fan",enemy)) or enemy:hasSkill("shenhuo") then return -1 end
		if enemy:objectName() == self.player:objectName() and (self:getCardId("FireSlash", enemy) or self:getCardId("FireAttack",enemy)) then return -1 end
	end
	if #(self:getEnemies(player))<3 then return 4 end
	return 3
end

function SmartAI:searchForAnaleptic(use,enemy,slash)
	if not self.toUse then return nil end

	for _,card in ipairs(self.toUse) do
		if card:getId()~= slash:getId() then return nil end
	end

	if not use.to then return nil end
	if self.player:hasUsed("Analeptic") then return nil end

	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:fillSkillCards(cards)
	local allcards = self.player:getCards("he")
	allcards = sgs.QList2Table(allcards)

	if hasSilverLion(enemy) then
		return
	end

	if ((enemy:getArmor() and enemy:getArmor():objectName() == "eight_diagram") or enemy:getHandcardNum() > 2)
		and not ((self:isEquip("Axe") and #allcards > 4) or self.player:getHandcardNum() > 1+self.player:getHp()) then
		return
	end

	local card_str = self:getCardId("Analeptic")
	if card_str then return sgs.Card_Parse(card_str) end

	for _, anal in ipairs(cards) do
		if (anal:className() == "Analeptic") and not (anal:getEffectiveId() == slash:getEffectiveId()) and
			not isCompulsoryView(anal, "Slash", self.player, sgs.Player_Hand) then
			return anal
		end
	end
end

sgs.dynamic_value.benefit.Analeptic = true

sgs.ai_use_value.Analeptic = 5.98
sgs.ai_keep_value.Analeptic = 4.5
sgs.ai_use_priority.Analeptic = 2.7

local function handcard_subtract_hp(a, b)
	local diff1 = a:getHandcardNum() - a:getHp()
	local diff2 = b:getHandcardNum() - b:getHp()

	return diff1 < diff2
end

function SmartAI:useCardSupplyShortage(card, use)
	table.sort(self.enemies, handcard_subtract_hp)
	local enemies = self:exclude(self.enemies, card)
	for _, enemy in ipairs(enemies) do
		if (self:hasSkills("yongsi|haoshi|tuxi", enemy) or (enemy:hasSkill("zaiqi") and enemy:getLostHp() > 1)) and
			not enemy:containsTrick("supply_shortage") and enemy:faceUp() and self:objectiveLevel(enemy) > 3 then
			use.card = card
			if use.to then use.to:append(enemy) end

			return
		end
	end
	for _, enemy in ipairs(enemies) do
		if ((#enemies == 1) or not self:hasSkills("yueli|qimen",enemy)) and not enemy:containsTrick("supply_shortage") and enemy:faceUp() and self:objectiveLevel(enemy) > 3 then
			use.card = card
			if use.to then use.to:append(enemy) end

			return
		end
	end
end

sgs.ai_use_value.SupplyShortage = 7

sgs.ai_card_intention.SupplyShortage = sgs.ai_card_intention.Indulgence

sgs.dynamic_value.control_usecard.SupplyShortage = true

function SmartAI:getChainedFriends()
	local chainedFriends = {}
	for _, friend in ipairs(self.friends) do
		if friend:isChained() then
			table.insert(chainedFriends,friend)
		end
	end
	return chainedFriends
end

function SmartAI:getChainedEnemies()
	local chainedEnemies = {}
	for _, enemy in ipairs(self.enemies) do
		if enemy:isChained() then
			table.insert(chainedEnemies,enemy)
		end
	end
	return chainedEnemies
end

function SmartAI:isGoodChainPartner(player)
	player = player or self.player
	if player:getRole() == "lord" then
		return false
	end
	if player:hasSkill("buqu") or (self:hasSkills(sgs.masochism_skill,player) and player:getHp() > 1) or
		(self.player:hasSkill("niepan") and self.player:getMark("@@nirvana") > 0) then
		return true
	end
	return false
end

function SmartAI:isGoodChainTarget(who)
	local haslord
	local good = #(self:getChainedEnemies(self.player))
	local bad = #(self:getChainedFriends(self.player))
	for _, friend in ipairs(self:getChainedFriends(self.player)) do
		if friend:getRole() == "lord" then
			return false
		end
		if friend:objectName() == self.player:objectName() and not self:isGoodChainPartner(self.player) then
			return false
		end
		if self:cantbeHurt(friend) then
			return false
		end
		if self:isGoodChainPartner(friend) then
			good = good+1
		end
		if self:isWeak(friend) then
			good = good-1
		end
	end
	for _, enemy in ipairs(self:getChainedEnemies(self.player)) do
		if enemy:getHp() < 3 and enemy:getRole() == "lord" and self.player:getRole() == "renegade" then
			return false
		end
		if self:cantbeHurt(enemy) then
			return false
		end
		if self:isGoodChainPartner(enemy) then
			bad = bad+1
		end
		if self:isWeak(enemy) then
			bad = bad-1
		end
	end
	return good > bad
end


function SmartAI:useCardIronChain(card, use)
	use.card = card
	if #self.enemies == 1 and #(self:getChainedFriends()) <= 1 then return end
	local friendtargets = {}
	local enemytargets = {}
	local zourun = self.room:findPlayerBySkillName("longjiao")
	self:sort(self.friends,"defense")
	for _, friend in ipairs(self.friends) do
		if friend:isChained() and not self:isGoodChainPartner(friend) and self:hasTrickEffective(card, friend) and not friend:hasSkill("longjiao") then
			table.insert(friendtargets, friend)
		end
	end
	self:sort(self.enemies,"defense")
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isChained() and not self.room:isProhibited(self.player, enemy, card) and not enemy:hasSkill("danlao")
			and self:hasTrickEffective(card, enemy) and not (self:objectiveLevel(enemy) <= 3) then
			table.insert(enemytargets, enemy)
		end
	end
	if not self.player:hasSkill("wuyan") then
		if #friendtargets > 1 then
			if use.to then use.to:append(friendtargets[1]) end
			if use.to then use.to:append(friendtargets[2]) end
		elseif #friendtargets == 1 then
			if #enemytargets > 0 then
				if use.to then use.to:append(friendtargets[1]) end
				if use.to then use.to:append(enemytargets[1]) end
			elseif zourun and self:isFriend(zourun) then
				if use.to then use.to:append(friendtargets[1]) end
				if use.to then use.to:append(zourun) end
			end
		elseif #enemytargets > 1 then
			if use.to then use.to:append(enemytargets[1]) end
			if use.to then use.to:append(enemytargets[2]) end
		elseif #friendtargets == 1 then
			if zourun and self:isFriend(zourun) then
				if use.to then use.to:append(enemytargets[1]) end
				if use.to then use.to:append(zourun) end
			end
		end
	end
	if use.to then assert(use.to:length() < 3) end
end

sgs.ai_card_intention.IronChain=function(card,from,tos)
	for _, to in ipairs(tos) do
		if to:isChained() then
			sgs.updateIntention(from, to, 80)
		else
			sgs.updateIntention(from, to, -80)
		end
	end
end

sgs.ai_use_value.IronChain = 5.4
sgs.ai_use_priority.IronChain = 2.8

sgs.dynamic_value.benefit.IronChain = true

function SmartAI:useCardFireAttack(fire_attack, use)
	if self.player:hasSkill("wuyan") then return end
	local lack = {
		spade = true,
		club = true,
		heart = true,
		diamond = true,
	}

	local targets_succ = {}
	local targets_fail = {}
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		if card:getEffectiveId() ~= fire_attack:getEffectiveId() then
			lack[card:getSuitString()] = false
		end
	end

	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if (self:objectiveLevel(enemy) > 3) and not enemy:isKongcheng() and not self.room:isProhibited(self.player, enemy, fire_attack)
			and self:damageIsEffective(enemy, sgs.DamageStruct_Fire, self.player) and self:hasTrickEffective(fire_attack, enemy)
			and not self:cantbeHurt(enemy)
			and not (enemy:isChained() and not self:isGoodChainTarget(enemy)) then

			local cards = enemy:getHandcards()
			local success = true
			for _, card in sgs.qlist(cards) do
				if lack[card:getSuitString()] then
					success = false
					break
				end
			end

			if success then
				if self:isEquip("Vine", enemy) or
					(enemy:isChained() and self:isGoodChainTarget(enemy)) or
					(enemy:hasSkill("jintang") and enemy:getHp() == 1) or
					(enemy:hasSkill("fushang") and enemy:getHp() > 3 and not enemy:hasSkill("fenhui")) then
					table.insert(targets_succ, 1, enemy)
					break
				else
					table.insert(targets_succ, enemy)
				end
			else
				table.insert(targets_fail, enemy)
			end
		end
	end

	if #targets_succ > 0 then
		use.card = fire_attack
		if use.to then use.to:append(targets_succ[1]) end
	elseif self.player:isChained() and self:isGoodChainTarget(self.player) and (self:isGoodChainPartner(self.player)
	or (self:isEquip("SilverLion") and self:hasSkill("fankui"))) and self.player:getHandcardNum() > 1 then
		use.card = fire_attack
		if use.to then use.to:append(self.player) end
	elseif #targets_fail > 0 and self:getOverflow(self.player) > 0 then
		use.card = fire_attack
		local r = math.random(1, #targets_fail)
		if use.to then use.to:append(targets_fail[r]) end
	end
end

sgs.ai_cardshow.fire_attack = function(self, requestor)
	local priority  =
	{
	heart = 4,
	spade = 3,
	club = 2,
	diamond = 1
	}
	local index = 0
	local result
	local cards = self.player:getHandcards()
	for _, card in sgs.qlist(cards) do
		local pity = priority[card:getSuitString()]
		if pity and pity > index then
			result = card
			index = pity
		end
	end

	return result
end

sgs.ai_use_value.FireAttack = 4.8
sgs.ai_use_priority.FireAttack = 2

sgs.ai_card_intention.FireAttack = function(card, from, tos)
	speakTrigger(card,from,tos[1])
	sgs.updateIntentions(from, tos, 80)
end
