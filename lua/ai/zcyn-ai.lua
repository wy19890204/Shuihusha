-- tongwu
sgs.ai_skill_invoke["tongwu"] = true
sgs.ai_skill_playerchosen["tongwu"] = function(self, targets)
	local targetlist=sgs.QList2Table(targets)
	self:sort(targetlist, "handcard")
	for _, target in ipairs(targetlist) do
		if self:isFriend(target) then
			return target
		end
	end
	return self.player
end

-- dujian
sgs.ai_skill_invoke["dujian"] = function(self, data)
	local rand = math.random(1, 2)
	return rand == 2
end
