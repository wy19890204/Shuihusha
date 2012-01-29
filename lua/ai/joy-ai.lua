-- when enemy using the peach
sgs.ai_skill_invoke["grab_peach"] = function(self, data)
	local struct = data:toCardUse()
	return self:isEnemy(struct.from) and (struct.to:isEmpty() or self:isEnemy(struct.to:first()))
end

function SmartAI:useGaleShell(card, use)
	for _, enemy in ipairs(self.enemies) do
		if self.player:distanceTo(enemy) <=1 and not self:hasSkills("jijiu|wusheng|longhun",enemy) then
			use.card = card
			if use.to then
				use.to:append(enemy)
			end
			return
		end
	end
end

