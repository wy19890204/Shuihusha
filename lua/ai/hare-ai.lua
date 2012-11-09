-- AI for hare package

-- haosiwen
-- sixiang
sgs.ai_card_intention.SixiangCard = function(card, from, tos)
	for _, to in ipairs(tos) do
		if to:getHandcardNum() > from:getRoom():getKingdoms() then
			sgs.updateIntention(from, to, 50, card)
		else
			sgs.updateIntention(from, to, -50, card)
		end
	end
end

sgs.ai_skill_use["@@sixiang"] = function(self, prompt)
	local king = self.room:getKingdoms()
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local targets = {}
	local delta = 0
	if king >= 3 then
		self:sort(self.friends, "handcard")
		for _, friend in ipairs(self.friends) do
			if friend:getHandcardNum() < king then
				table.insert(targets, friend:objectName())
				delta = delta + (king - friend:getHandcardNum())
			end
		end
		if delta > king then
			return "@SixiangCard=" .. cards[1]:getEffectiveId() .. "->" .. table.concat(targets, "+")
		end
	else
		self:sort(self.enemies, "handcard2")
		for _, enemy in ipairs(self.enemies) do
			if enemy:getHandcardNum() > king then
				table.insert(targets, enemy:objectName())
				delta = delta + (enemy:getHandcardNum() - king)
			end
		end
		if delta >= king then
			return "@SixiangCard=" .. cards[1]:getEffectiveId() .. "->" .. table.concat(targets, "+")
		end
	end
end

-- weidingguo
-- shenhuo
sgs.ai_skill_invoke["shenhuo"] = true
local shenhuo_skill={}
shenhuo_skill.name = "shenhuo"
table.insert(sgs.ai_skills, shenhuo_skill)
shenhuo_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards,true)
	for _,acard in ipairs(cards)  do
		if acard:isRed() and acard:inherits("TrickCard") then
			card = acard
			break
		end
	end
	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("fire_attack:shenhuo[%s:%s]=%d"):format(suit, number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end
function sgs.ai_cardneed.shenhuo(to, card, self)
	return (card:getSuit() == sgs.Card_Diamond or card:getSuit() == sgs.Card_Heart)
		and card:inherits("TrickCard")
end

-- xiaorang
-- linmo
sgs.ai_skill_invoke["linmo"] = true

local linmo_skill={}
linmo_skill.name = "linmo"
table.insert(sgs.ai_skills, linmo_skill)
linmo_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("LinmoCard") or self.player:isKongcheng() then return end
	local zis = self.player:getPile("zi")
	zis = sgs.QList2Table(zis)
	local words = {} -- all zi not used
	local aoenames = {}
	local hasexn = false
	local hasgodsa = false
	for _, word in ipairs(zis) do
		local card = sgs.Sanguosha:getCard(word)
		table.insert(words, card:objectName())
		if card:getSubtype() == "aoe" then
			table.insert(aoenames, card:objectName())
		end
		if card:objectName() == "ex_nihilo" then
			hasexn = true
		elseif card:objectName() == "god_salvation" then
			hasgodsa = true
		elseif card:objectName() == "peach" and self.player:isWounded() then
			local parsed_card = sgs.Card_Parse("@LinmoCard=" .. card:getEffectiveId() .. ":" .. "peach")
			return parsed_card
		end
	end
	local cards = sgs.QList2Table(self.player:getHandcards())
	local good, bad = 0, 0
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
		local newlinmo = aoenames[i]
		local aoe = sgs.Sanguosha:cloneCard(newlinmo, sgs.Card_NoSuit, 0)
		if self:getAoeValue(aoe) > -5 then
			local parsed_card=sgs.Card_Parse("@LinmoCard=" .. card .. ":" .. newlinmo)
			return parsed_card
		end
	end
	if good > bad and hasgodsa then
		local parsed_card = sgs.Card_Parse("@LinmoCard=" .. card .. ":" .. "god_salvation")
		return parsed_card
	end
	if hasexn and self:getCardsNum("Jink") == 0 and self:getCardsNum("Peach") == 0 then
		local parsed_card = sgs.Card_Parse("@LinmoCard=" .. card .. ":" .. "ex_nihilo")
		return parsed_card
	end
end
sgs.ai_skill_use_func["LinmoCard"] = function(card, use, self)
	local userstring = card:toString()
	userstring = (userstring:split(":"))[3]
	local linmocard = sgs.Sanguosha:cloneCard(userstring, card:getSuit(), card:getNumber())
	self:useTrickCard(linmocard,use)
	if not use.card then return end
	use.card = card
end

-- zhaixing
sgs.ai_skill_invoke["@zhaixing"] = function(self,prompt)
	local judge = self.player:getTag("Judge"):toJudge()
	local all_cards = self.player:getCards("he")
	local cards = sgs.QList2Table(all_cards)

	if #cards == 0 then return "." end
	local card_id = self:getRetrialCardId(cards, judge)
	if card_id == -1 then
		if self:needRetrial(judge) then
			self:sortByUseValue(cards, true)
			if self:getUseValue(judge.card) > self:getUseValue(cards[1]) then
				return "@ZhaixingCard=" .. cards[1]:getId()
			end
		end
	elseif self:needRetrial(judge) or self:getUseValue(judge.card) > self:getUseValue(sgs.Sanguosha:getCard(card_id)) then
		return "@ZhaixingCard=" .. card_id
	end
-- zhaixing can draw card
	for _, card in ipairs(cards) do
		if card:getSuit() == sgs.Card_Diamond then
			return "@ZhaixingCard=" .. card:getId()
		end
	end
	return "."
end

-- peixuan
-- shenpan
sgs.ai_skill_invoke["shenpan"] = function(self, data)
	local judge = data:toJudge()
	if not self:needRetrial(judge) then return false end
	local wizard_friend
	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player == judge.who then break end
		if self:isFriend(player) then
			if (player:hasSkill("yixing") and self:getYixingCard(judge) > -1) or
				player:hasSkill("butian") then
				wizard_friend = player
				break
			end
		end
	end
	return not wizard_friend
end

-- binggong
sgs.ai_card_intention.BinggongCard = -50

sgs.ai_skill_use["@@binggong"] = function(self, prompt)
	local num = self.player:getMark("Bingo")
	if num < 3 and self.player:isWounded() then return "." end
	self:sort(self.friends_noself, "defense")
	local target = self.friends_noself[1]
	if not target then return "." end
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local card_ids = {}
	for i = 1, num do
		table.insert(card_ids, cards[i]:getEffectiveId())
	end
	return "@BinggongCard=" .. table.concat(card_ids, "+") .. "->" .. target:objectName()
end

-- ligun
-- hengchong
sgs.ai_skill_playerchosen["hengchong"] = sgs.ai_skill_playerchosen["shunshui"]
sgs.ai_skill_cardask["@hengchong"] = function(self, data)
	if self.room:getAlivePlayers():length() == 2 then return "." end
	local effect = data:toSlashEffect()
	if self:isFriend(effect.to) and not self:hasSkills(sgs.masochism_skill, effect.to) then
		return "."
	end
	local suit = effect.slash:getSuitString()
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		if card:getSuitString() == suit then
			return card:getEffectiveId()
		end
	end
	return "."
end

-- tongwei
-- dalang
sgs.ai_skill_invoke["dalang"] = function(self, data)
	if self.player:getHandcardNum() < 2 then return false end
	for _, friend in ipairs(self.friends_noself) do
		if (friend:containsTrick("indulgence") or friend:containsTrick("supply_shortage"))
			and friend:getHandcardNum() > friend:getHp() then
			self.dalangtarget = friend
			return true
		end
	end
	return false
end
sgs.ai_skill_playerchosen["dalangfrom"] = function(self, targets)
	local target = self.dalangtarget
	return target
end
sgs.ai_skill_askforag["dalang"] = function(self, card_ids)
	return card_ids[1]
end
sgs.ai_skill_playerchosen["dalangtu"] = sgs.ai_skill_playerchosen["shunshui"]

-- songqing
-- jiayao
sgs.ai_skill_invoke["jiayao"] = true

-- sheyan
local sheyan_skill = {}
sheyan_skill.name = "sheyan"
table.insert(sgs.ai_skills, sheyan_skill)
sheyan_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("SheyanCard") then return end
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards, true)
	for _, acard in ipairs(cards) do
		if acard:getSuit() == sgs.Card_Heart then
			card = acard
			break
		end
	end
	if card then
		return sgs.Card_Parse("@SheyanCard=" .. card:getEffectiveId())
	end
end
sgs.ai_skill_use_func["SheyanCard"] = function(card,use,self)
	use.card=card
end

-- dingdesun
sgs.dingdesun_keep_value =
{
	Jink = 6,
}

-- songwan
-- yijie
sgs.ai_card_intention.YijieCard = -50

local yijie_skill = {}
yijie_skill.name = "yijie"
table.insert(sgs.ai_skills, yijie_skill)
yijie_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("YijieCard") then return end
	if self.player:getHp() > 2 then
		return sgs.Card_Parse("@YijieCard=.")
	end
end
sgs.ai_skill_use_func["YijieCard"] = function(card,use,self)
	use.card = card
	if use.to then
		self:sort(self.friends_noself, "handcard")
		use.to:append(self.friends_noself[1])
	end
end

sgs.ai_skill_invoke["yijie"] = function(self, data)
	self:sort(self.friends_noself, "hp")
	for _, friend in ipairs(self.friends_noself) do
		if friend:isWounded() then
			self.yijietarget = friend
			return true
		end
	end
	return false
end
sgs.ai_skill_playerchosen["yijie"] = function(self, targets)
	return self.yijietarget
end

-- zhoutong
-- qiangqu
sgs.ai_skill_invoke["qiangqu"] = function(self, data)
	local damage = data:toDamage()
	return self:isFriend(damage.to)
end

-- huatian
sgs.ai_card_intention.HuatianCard = function(card, from, tos)
	if from:getMark("Huatian") == 1 then
		sgs.updateIntentions(from, tos, -60)
	elseif from:getMark("Huatian") == 2 then
		sgs.updateIntentions(from, tos, 60)
	else
		sgs.updateIntentions(from, tos, 0)
	end
end

sgs.ai_skill_use["@@huatian"] = function(self, prompt)
	local flag = self.player:getMark("Huatian")
	if flag == 1 then -- ai
		if not self.friends_noself[1] then return "." end
		self:sort(self.friends_noself, "hp")
		if self.friends_noself[1]:isWounded() then
			return "@HuatianCard=.->" .. self.friends_noself[1]:objectName()
		end
	elseif flag == 2 then -- cuo
		self:sort(self.enemies)
		if #self.enemies > 0 then
			return "@HuatianCard=.->" .. self.enemies[1]:objectName()
		end
	end
end

-- zhugui
-- shihao
sgs.ai_skill_invoke["shihao"] = function(self, data)
	return self.player:getHandcardNum() >= 3
end
sgs.ai_skill_playerchosen["shihao"] = function(self, targets)
	local wiretap = sgs.Sanguosha:cloneCard("wiretap", sgs.Card_NoSuit, 0)
	local use = sgs.CardUseStruct()
	self:useCardWiretap(wiretap, use)
	return use.to:first()
end

-- laolian
sgs.ai_skill_invoke["laolian"] = true
sgs.ai_skill_playerchosen["laolian"] = function(self, targets)
	self:sort(self.enemies, "defense")
	return self.enemies[1]
end

-- zhaoji
-- shemi
sgs.ai_skill_use["@@shemi"] = function(self, prompt)
	if self.player:getHandcardNum() < self.player:getHp() then return "." end
	local cards = self.player:getCards("he")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	return "@ShemiCard=" .. cards[1]:getEffectiveId()
end

-- nongquan
sgs.ai_skill_invoke["nongquan"] = function(self, data)
	local lord = self.room:getLord()
	if lord:hasLordSkill("nongquan") then
		if self:isFriend(lord) and not lord:faceUp() then
			return self.player:getHandcardNum() > 1
		elseif self:isEnemy(lord) and lord:faceUp() and lord:getHandcardNum() < 3 then
			return self.player:getHandcardNum() > 2
		end
	end
	return false
end
