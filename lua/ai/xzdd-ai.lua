-- maidao
maidao_skill={}
maidao_skill.name = "maidao"
table.insert(sgs.ai_skills, maidao_skill)
maidao_skill.getTurnUseCard = function(self)
	if self.player:getWeapon() then
		local cards = self.player:getCards("h")
		cards = sgs.QList2Table(cards)
		for _, acard in ipairs(cards)  do
			if acard:inherits("Weapon") then
				return sgs.Card_Parse("@MaidaoCard=" .. acard:getId())
			end
		end
	end
	return
end
sgs.ai_skill_use_func["MaidaoCard"] = function(card, use, self)
	use.card = card
end

-- fengmang
sgs.ai_skill_invoke["fengmang"] = true
sgs.ai_skill_playerchosen["fengmang"] = function(self, targets)
	self:sort(self.enemies, "hp")
	return self.enemies[1]
end
