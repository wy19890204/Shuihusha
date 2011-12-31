-- kaixian
sgs.ai_skill_invoke["kaixian"] = true

-- liba
sgs.ai_skill_invoke["liba"] = function(self, data)
	local damage = data:toDamage()
	return self:isEnemy(damage.to)
end

-- zhanchi
sgs.ai_skill_invoke["zhanchi"] = function(self, data)
	if self.player:hasWeapon("crossbow") then
		return true
	else
		return false
	end
end

-- butian
sgs.ai_skill_invoke["@butian"]=function(self,prompt,judge)
	judge = judge or self.player:getTag("Judge"):toJudge()

	if self:needRetrial(judge) then
		local cards = sgs.QList2Table(self.player:getHandcards())
		if self:getUnuseCard() then
			local card_id = self:getUnuseCard():getId()
			return "@ButianCard=" .. card_id
		end
	end
	return "."
end
sgs.ai_skill_askforag["butian"] = function(self, card_ids)
	local judge = self.player:getTag("Judge"):toJudge()
	local cards = {}
	local card_id
	if self:needRetrial(judge) then
		for _, card_id in ipairs(card_ids) do
			local card = sgs.Sanguosha:getCard(card_id)
			table.insert(cards, card)
		end
		card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return card_id
		end
	end
	return card_ids[1]
end

-- longluo
sgs.ai_skill_playerchosen["longluo"] = function(self, targets)
	self:sort(self.friends, "hp")
	return self.friends[1]
end

sgs.ai_get_cardType=function(card)
	if card:inherits("Weapon") then return 1 end
	if card:inherits("Armor") then return 2 end 
	if card:inherits("OffensiveHorse")then return 3 end 
	if card:inherits("DefensiveHorse") then return 4 end 
end

sgs.ai_skill_use["@@shensu2"]=function(self,prompt)
        self:updatePlayers(true)
	self:sort(self.enemies,"defense")
	
	local selfSub = self.player:getHp()-self.player:getHandcardNum()
	local selfDef = getDefense(self.player)
	
	local cards = self.player:getCards("he")
	
	cards=sgs.QList2Table(cards)
	
	local eCard
	local hasCard={0, 0, 0, 0}
	
	for _,card in ipairs(cards) do
		if card:inherits("EquipCard") then 
			hasCard[sgs.ai_get_cardType(card)] = hasCard[sgs.ai_get_cardType(card)]+1
		end		
	end
	
	for _,card in ipairs(cards) do
		if card:inherits("EquipCard") then 
			if hasCard[sgs.ai_get_cardType(card)]>1 or sgs.ai_get_cardType(card)>3 then 
				eCard = card 
				break
			end
			if not eCard and not card:inherits("Armor") then eCard = card end
		end
	end
	
	if not eCard then return "." end
	
	local effectslash, best_target, target
	local defense = 6
	for _,enemy in ipairs(self.enemies) do
		local def=getDefense(enemy)
		local amr=enemy:getArmor()
		local eff=(not amr) or self.player:hasWeapon("qinggang_sword") or not 
				((amr:inherits("Vine") and not self.player:hasWeapon("fan"))
				or (amr:objectName()=="eight_diagram") or enemy:hasSkill("bazhen"))
		
        if enemy:hasSkill("kongcheng") and enemy:isKongcheng() then
        elseif self:slashProhibit(nil, enemy) then
        elseif eff then 
			if enemy:getHp() == 1 and self:getCardsNum("Jink", enemy) == 0 then best_target = enemy break end
			if def < defense then
				best_target = enemy
				defense = def
			end
			target = enemy
		end
		if selfSub<0 then return "." end
	end
	
	if best_target then return "@ShensuCard="..eCard:getEffectiveId().."->"..best_target:objectName() end
	if target then return "@ShensuCard="..eCard:getEffectiveId().."->"..target:objectName() end
	
	return "."
end

sgs.ai_skill_invoke["@guidao"]=function(self,prompt)
    local judge = self.player:getTag("Judge"):toJudge()
	
	if self:needRetrial(judge) then
		self:log("guidao!!!!!!!!")
		local all_cards = self.player:getCards("he")
		local cards = {}
		for _, card in sgs.qlist(all_cards) do
			if card:isBlack() then
				table.insert(cards, card)
			end
		end
		local card_id = self:getRetrialCardId(cards, judge)
		if card_id ~= -1 then
			return "@GuidaoCard=" .. card_id
		end
	end
	
	return "."
end

local huangtianv_skill={}
huangtianv_skill.name="huangtianv"
table.insert(sgs.ai_skills,huangtianv_skill)

huangtianv_skill.getTurnUseCard=function(self)
    if self.player:hasUsed("HuangtianCard") then return nil end
    if self.player:isLord() then return nil end
    if self.player:getKingdom() ~= "kou" then return nil end
	if not self.room:getLord():hasSkill("huangtian") then return nil end

    local cards = self.player:getCards("h")	
    cards=sgs.QList2Table(cards)
	
	local card
	
	self:sortByUseValue(cards,true)
	
	for _,acard in ipairs(cards)  do
		if acard:inherits("Jink") then
			card = acard
			break
		end
	end
	
	if not card then 
		return nil
	end
	
	local card_id = card:getEffectiveId()
	local card_str = "@HuangtianCard="..card_id
	local skillcard = sgs.Card_Parse(card_str)
		
	assert(skillcard)	
	return skillcard		
end

sgs.ai_skill_use_func["HuangtianCard"]=function(card,use,self)
    local targets = {}
	for _, friend in ipairs(self.friends_noself) do
		if friend:hasLordSkill("Huangtian") then 
			table.insert(targets, friend)
		end
	end
	
	if #targets == 0 then return end
    
	use.card=card
	self:sort(targets, "defense")
	if use.to then
		use.to:append(targets[1]) 
    end	
end

sgs.ai_skill_askforag.buqu = function(self, card_ids)
-- find duplicated one or the first
	for i, card_id in ipairs(card_ids) do
		for j, card_id2 in sgs.list(card_ids) do
			if i ~= j and card_id == card_id2 then
				return card_id
			end
		end
	end

	return card_ids[1]
end