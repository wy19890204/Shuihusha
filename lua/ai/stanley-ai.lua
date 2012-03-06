-- youxia
sgs.ai_skill_invoke["youxia"] = function(self, data)
	local move = data:toCardMove()
	if self:isEnemy(move.from) and self.player:isWounded() then
		return true
	elseif self:isFriend(move.from) and self.player:getHandcardNum() > 2 then
		return true
	end
	return false
end

-- lianzang
sgs.ai_skill_invoke["lianzang"] = function(self, data)
	local damage = data:toDamage()
	local target = damage.to
	if self.player:getMaxHP() < 4 then
		return false
	end
	return true
end
