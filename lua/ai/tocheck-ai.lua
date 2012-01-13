-- this scripts contains the AI classes for generals of tocheck package

-- tie suo lian huan
function SmartAI:useCardIronChain(card, use)
	local mengkang = self.room:findPlayerBySkillName("mengchong")
	local mk = mengkang and self:isFriend(mengkang)
	local targets = {}
	self:sort(self.friends,"defense")
	for _, friend in ipairs(self.friends) do
		if friend:isChained() and not mk then
			table.insert(targets, friend)
		elseif not friend:isChained() and mk then
			table.insert(targets, friend)
		end
	end

	self:sort(self.enemies,"defense")
	for _, enemy in ipairs(self.enemies) do
		if not self.room:isProhibited(self.player, enemy, card) and not enemy:hasSkill("longjiao")
			and self:hasTrickEffective(card, enemy) and not (self:objectiveLevel(enemy) <= 3) then
			if not enemy:isChained() and not mk then
				table.insert(targets, enemy)
			elseif enemy:isChained() and mk then
				table.insert(targets, enemy)
			end
		end
	end

	use.card = card

	if targets[2] and not self.player:hasSkill("wuyan") then
		if use.to then use.to:append(targets[1]) end
		if use.to then use.to:append(targets[2]) end
	end
end

-- huo gong
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

	if self.player:hasSkill("hongyan") then
		lack["spade"] = true
	end

	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if (self:objectiveLevel(enemy) > 3) and not enemy:isKongcheng() and self:hasTrickEffective(fire_attack, enemy) then

			local cards = enemy:getHandcards()
			local success = true
			for _, card in sgs.qlist(cards) do
				if lack[card:getSuitString()] then
					success = false
					break
				end
			end

			if success then
				if enemy:hasSkill("fushang") and enemy:getHp() > 3 and not enemy:hasSkill("fenhui") then
					table.insert(targets_succ, 1, enemy)
					break
				elseif self:isEquip("Vine", enemy) then
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
	elseif #targets_fail > 0 and self:getOverflow(self.player) > 0 then
		use.card = fire_attack
		local r = math.random(1, #targets_fail)
		if use.to then use.to:append(targets_fail[r]) end
	end
end

-- bing liang cun duan
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
			not enemy:containsTrick("supply_shortage") and enemy:faceUp() then
			use.card = card
			if use.to then use.to:append(enemy) end

			return
		end
	end
	for _, enemy in ipairs(enemies) do
		if ((#enemies == 1) or not enemy:hasSkill("tiandu")) and not enemy:containsTrick("supply_shortage") and enemy:faceUp() then
			use.card = card
			if use.to then use.to:append(enemy) end

			return
		end
	end
end
