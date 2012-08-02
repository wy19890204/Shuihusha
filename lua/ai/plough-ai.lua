-- this scripts contains the AI classes for generals of plough package

sgs.ai_skill_invoke.double_whip = function(self, data)
	local carduse = data:toCardUse()
	local tos = sgs.QList2Table(carduse.to)
	local mengkang = self.room:findPlayerBySkillName("mengchong")
	local mk = mengkang and self:isFriend(mengkang)
	for _, target in ipairs(tos) do
		if target:isChained() then
			return self:isFriend(target) and not mk
		else
			return self:isEnemy(target) and not mk
		end
		break
	end
end

sgs.weapon_range.DoubleWhip = 2
sgs.weapon_range.MeteorSword = 3
sgs.weapon_range.SunBow = 5

function sgs.ai_armor_value.gold_armor(player, self)
	return 4
end

function SmartAI:searchForEcstasy(use,enemy,slash)
	if not self.toUse then return nil end

	for _,card in ipairs(self.toUse) do
		if card:getId()~= slash:getId() then return nil end
	end

	if not use.to then return nil end
	if self.player:hasUsed("Ecstasy") then return nil end

	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:fillSkillCards(cards)

	if sgs.getDefense(self.player) < sgs.getDefense(enemy) and
		self.player:getHandcardNum() < self.player:getHp() + 1 then
			return
	end

	if not self.player:canSlash(enemy) or enemy:hasFlag("ecst") then
		return
	end

	local card_str = self:getCardId("Ecstasy")
	if card_str then return sgs.Card_Parse(card_str) end

	for _, mi in ipairs(cards) do
		if (mi:className() == "Ecstasy") and not (mi:getEffectiveId() == slash:getEffectiveId()) and
			not isCompulsoryView(mi, "Slash", self.player, sgs.Player_Hand) then
			return mi
		end
	end
end

sgs.ai_card_intention.Ecstasy = function(card, from, tos)
	for _, to in ipairs(tos) do
		speakTrigger(card,from,to)
	end
	sgs.updateIntentions(from, tos, 80)
end
sgs.dynamic_value.control_card.Ecstasy = true
sgs.dynamic_value.benefit.Counterplot = true

-- bi shang liang shan
function SmartAI:useCardDrivolt(drivolt, use)
--	if self.player:hasSkill("wuyan") then return end
	local target
	self:sort(self.enemies, "hp")
	if #self.enemies > 0 and self.enemies[1]:getHp() == 1 and self.enemies[1]:getKingdom() ~= self.player:getKingdom() then
		target = self.enemies[1]
	end
	if not target then
		for _, friend in ipairs(self.friends_noself) do
			if not friend:isWounded() and not self:isWeak(friend) and
				friend:getKingdom() ~= self.player:getKingdom() then
				target = friend
				break
			end
		end
	end
	if not target then
		for _, enemy in ipairs(self.enemies) do
			if enemy:getHp() == 2 and enemy:getKingdom() ~= self.player:getKingdom() then
				target = enemy
				break
			end
		end
	end
	local players = {}
	for _, player in ipairs(sgs.QList2Table(self.room:getOtherPlayers(self.player))) do
		if player:getKingdom() ~= self.player:getKingdom() then table.insert(players, player) end
	end
	if #players < 1 then
		use.card = nil
		return "."
	else
		use.card = drivolt
	end

	if not target then
		local r = math.random(1, #players)
		target = players[r]
	end
	if use.to then
		speak(target, "drivolt")
		use.to:append(target)
	end
end

sgs.ai_card_intention.Drivolt = function(card, from, tos)
	for _, to in ipairs(tos) do
		local value = 80
		if not to:isWounded() then value = -50 end
		sgs.updateIntention(from, to, value)
	end
end
sgs.dynamic_value.damage_card.Drivolt = true
sgs.dynamic_value.control_card.Drivolt = true

-- tan ting
function SmartAI:useCardWiretap(wiretap, use)
	local targets = {}
	if #self.friends_noself > 0 then
		self:sort(self.friends_noself, "handcard2")
		if self.friends_noself[1]:getHandcardNum() > 0 then
			table.insert(targets, self.friends_noself[1])
		end
	end
	if #self.enemies > 0 then
		self:sort(self.enemies, "handcard2")
		if self.enemies[1]:getHandcardNum() > 0 then
			table.insert(targets, self.enemies[1])
		end
	end
	if #targets == 0 then
		for _, t in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if not t:isKongcheng() then
				table.insert(targets, t)
			end
		end
	end
	if #targets == 0 then return "." end
	use.card = wiretap
	if use.to then
		local r = math.random(1, #targets)
		use.to:append(targets[r])
	end
end

sgs.dynamic_value.benefit.Wiretap = true

-- xing ci
function SmartAI:useCardAssassinate(ass, use)
--	if self.player:hasSkill("wuyan") then return end
	if not self.enemies[1] then return end
	for _, enemy in ipairs(self.enemies) do
		if (enemy:hasSkill("fushang") and enemy:getHp() > 3) or enemy:hasSkill("huoshui") then
			use.card = ass
			if use.to then
				use.to:append(enemy)
			end
			return
		end
	end
	self:sort(self.enemies, "hp")
	local target
	for _, enemy in ipairs(self.enemies) do
		if enemy:hasFlag("ecst") or
			(not self:isEquip("EightDiagram", enemy) and enemy:getHandcardNum() < 6) then
			target = enemy
		end
	end
	use.card = ass
	if use.to then
		if target then
			use.to:append(target)
		else
			use.to:append(self.enemies[1])
		end
	end
end

sgs.ai_skill_cardask["@assas1"] = function(self, data, pattern, target)
	self:speak("assassinate", self.player:getGeneral():isFemale(), to)
	if sgs.ai_skill_cardask.nullfilter(self, data, pattern, target) then return "." end
	if self:getCardsNum("Jink") < 2 and not (self.player:getHandcardNum() == 1 and self:hasSkills(sgs.need_kongcheng)) then return "." end
end

sgs.ai_card_intention.Assassinate = 90
sgs.dynamic_value.damage_card.Assassinate = true

-- sheng chen gang
function SmartAI:useCardTreasury(card, use)
	if not self.player:containsTrick("treasury") then
		use.card = card
	end
end

sgs.dynamic_value.lucky_chance.Treasury = true

-- hai xiao
function SmartAI:useCardTsunami(card, use)
	if self.player:containsTrick("tsunami") then return end
--	if self.player:hasSkill("weimu") and card:isBlack() then return end

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

sgs.dynamic_value.lucky_chance.Tsunami = true

-- ji cao tun liang
function SmartAI:useCardProvistore(provistore, use)
--	if self.player:hasSkill("wuyan") then return end
	use.card = provistore
	for _, friend in ipairs(self.friends) do
		if not friend:containsTrick("provistore") and friend:getHandcardNum() > 3 then
			if use.to then
				use.to:append(friend)
			end
			return
		end
	end
	self:sort(self.friends, "hp")
	if use.to then
		use.to:append(self.friends[1])
	end
end

sgs.ai_card_intention.Provistore = -80
sgs.dynamic_value.control_usecard.Provistore = true
sgs.dynamic_value.benefit.Provistore = true

-- feng xiong hua ji
function SmartAI:useCardInspiration(inspiration, use)
	self:sort(self.friends, "hp")
	local f = 0
	for _, friend in ipairs(self.friends) do
		f = f + friend:getLostHp()
	end
	self:sort(self.enemies, "hp")
	local e = 0
	for _, enemy in ipairs(self.enemies) do
		e = e + enemy:getLostHp()
	end
	if e > f then return "." end
	use.card = inspiration
end

sgs.dynamic_value.benefit.Inspiration = true
