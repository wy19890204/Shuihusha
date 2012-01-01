-- tongwu
sgs.ai_skill_invoke["tongwu"] = true
sgs.ai_skill_playerchosen["tongwu"] = function(self, targets)
	self:sort(targets, "handcard")
	for _, target in ipairs(targets) do
		if self:isFriend(target) then
			return target
		end
	end
	return self.player
end

-- shemi
sgs.ai_skill_invoke["shemi"] = function(self, data)
	return self.player:getHandcardNum() >= self.player:getHp()
end

-- nongquan
sgs.ai_skill_invoke["nongquan"] = function(self, data)
	local lord = self.room:getLord()
	if lord:hasLordSkill("nongquan") and not lord:faceUp() and self.player:getHandcardNum() > 2 then
		return self:isFriend(lord)
	end
end
