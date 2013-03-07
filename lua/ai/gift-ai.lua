-- ai for gift-package

function SmartAI:useCardZongzi(card, use)
	if self.player:hasMark("HaveEaten") then return end
	use.card = card
	if not use.isDummy then
		self:speak("zongzi")
	end
end

sgs.ai_use_value.Zongzi = 4.4
sgs.ai_keep_value.Zongzi = 0
sgs.dynamic_value.benefit.Zongzi = true

function SmartAI:useCardMoonpie(card, use)
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if self.player:inMyAttackRange(enemy) and enemy:getMark("HaveEaten2") == 0 then
			if self.player ~= enemy then
				use.card = card
				if use.to then
					use.to:append(enemy)
				end
				return
			end
		end
	end
end

sgs.ai_use_value.Moonpie = 3.3
sgs.ai_keep_value.Moonpie = 0
sgs.dynamic_value.control_card.Moonpie = true
sgs.ai_card_intention.Moonpie = function(card, from, tos, self)
	self:speak("moonpie", tos[1])
end

function SmartAI:useCardRiceBall(card, use)
	if self.player:hasMark("HaveEaten3") then return end
	self:sort(self.friends_noself)
	for _, friend in ipairs(self.friends_noself) do
		if self.player:distanceTo(friend) > 2 then
			use.card = card
			if use.to then
				use.to:append(friend)
			end
			return
		end
	end
end

sgs.ai_use_value.RiceBall = 0.3
sgs.ai_keep_value.RiceBall = 0
sgs.ai_card_intention.RiceBall = function(card, from, tos, self)
	self:speak("riceball", tos[1])
end
