-- meicha
meicha_skill={}
meicha_skill.name = "meicha"
table.insert(sgs.ai_skills, meicha_skill)
meicha_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards,true)
	for _,acard in ipairs(cards)  do
		if (acard:getSuit() == sgs.Card_Club) then
			card = acard
			break
		end
	end
	if not card then return nil end
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("analeptic:meicha[club:%s]=%d"):format(number, card_id)
	local analeptic = sgs.Card_Parse(card_str)
	assert(analeptic)
	return analeptic
end

-- banzhuang
local banzhuang_skill={}
banzhuang_skill.name = "banzhuang"
table.insert(sgs.ai_skills, banzhuang_skill)
banzhuang_skill.getTurnUseCard = function(self,inclusive)
    local cards = self.player:getHandcards()
    cards = sgs.QList2Table(cards)
	for _,card in ipairs(cards)  do
		if card:getSuit() == sgs.Card_Heart or inclusive then
			local number = card:getNumberString()
			local card_id = card:getEffectiveId()
			local card_str = ("ex_nihilo:banzhuang[heart:%s]=%d"):format(number, card_id)
			local exnihilo = sgs.Card_Parse(card_str)
			assert(exnihilo)
			return exnihilo
		end
	end
end

-- zhuying
local zhuying_skill={}
zhuying_skill.name = "zhuying"
table.insert(sgs.ai_skills, zhuying_skill)
zhuying_skill.getTurnUseCard = function(self)
	if not self.player:isWounded() then return end
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)
	local anal_card
	self:sortByUseValue(cards, true)
	for _,card in ipairs(cards)  do
		if card:inherits("Analeptic") then
			anal_card = card
			break
		end
	end
	if anal_card then
		local suit = anal_card:getSuitString()
		local number = anal_card:getNumberString()
		local card_id = anal_card:getEffectiveId()
		local card_str = ("peach:zhuying[%s:%s]=%d"):format(suit, number, card_id)
		local peach = sgs.Card_Parse(card_str)
		return peach
	end
end

