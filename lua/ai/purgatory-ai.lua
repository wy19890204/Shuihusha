-- ai for purgatory-package

sgs.ai_use_value.Mastermind = 3
sgs.ai_keep_value.Mastermind = 0

function SmartAI:useCardMastermind(card, use)
	local en, fr
	self:sort(self.enemies, "defense")
	if #self.enemies > 0 then
		en = self.enemies[1]
	end
	self:sort(self.friends, "hp")
	if #self.friends > 0 then
		fr = self.friends[1]
	end
	if en and fr then
		use.card = card
		if use.to then
			use.to:append(fr)
			use.to:append(en)
		end
		return
	end
end

sgs.ai_use_value.SpinDestiny = 3
sgs.ai_keep_value.SpinDestiny = 0

function SmartAI:useCardSpinDestiny(card, use)
	if #self.enemies > #self.friends_noself then
		use.card = card
	end
end

sgs.ai_use_value.EdoTensei = 3
sgs.ai_keep_value.EdoTensei = 0
