jiuchi_skill={}
jiuchi_skill.name="jiuchi"
table.insert(sgs.ai_skills,jiuchi_skill)
jiuchi_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("h")
	cards=sgs.QList2Table(cards)

	local card

	self:sortByUseValue(cards,true)

	for _,acard in ipairs(cards)  do
		if (acard:getSuit() == sgs.Card_Spade) then --and (self:getUseValue(acard)<sgs.ai_use_value["Analeptic"]) then
			card = acard
			break
		end
	end

		if not card then return nil end
		local number = card:getNumberString()
		local card_id = card:getEffectiveId()
		local card_str = ("analeptic:jiuchi[spade:%s]=%d"):format(number, card_id)
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
