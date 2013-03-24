function SmartAI:crazyAI(event, player, data)
	player = player or self.player
	if player:getState() ~= "robot" then return end
	if event == sgs.SlashEffect then -- 杀人时随机令一名队友补一张牌
		if math.random(0, 3) == 1 and #self.friends_noself > 0 then
			self.friends_noself[1]:drawCards(1)
		end
	elseif event == sgs.SlashHit then -- 杀中目标时随机获得对方一张牌，然后还一张牌
		if math.random(0, 3) == 1 then
			local effect = data:toSlashEffect()
			if not effect.to:isNude() then
				local cards = sgs.QList2Table(effect.to:getCards("he"))
				self:sortByUseValue(cards)
				effect.from:obtainCard(cards[1])
				cards = sgs.QList2Table(effect.from:getCards("he"))
				self:sortByUseValue(cards, true)
				effect.to:obtainCard(cards[1])
			end
		end
	elseif event == sgs.PreDeath then -- 杀死一人前补满手牌和体力回满
		local damage = data:toDamage()
		if damage and damage.from then
			local x = damage.from:getMaxCards() - damage.from:getHandcardNum()
			if x > 0 then damage.from:drawCards(x) end
			local recover = sgs.RecoverStruct()
			recover.recover = damage.from:getLostHp()
			recover.card = damage.card
			recover.who = damage.to
			self.room:recover(damage.from, recover)
		end
	elseif event == sgs.DamageProceed then -- 对女性出杀随机+1或-1
		local damage = data:toDamage()
		if damage.to:getGeneral():isFemale() then
			local dt = math.random(0, 2) - 1
			damage.damage = damage.damage + dt
			data:setValue(damage)
		end
	elseif event == sgs.DamageComplete then -- 受到伤害可以肛裂
		if math.random(0, 2) == 1 then
			local damage = data:toDamage()
			damage.from = data:toDamage().to
			damage.to = data:toDamage().from
			self.room:damage(damage)
		end
	elseif event == sgs.FinishJudge then -- 判定结束后对一名其他角色发动一次幻术
		if math.random(0, 2) == 1 then
			sgs.ai_skill_use["@@huanshu"](self, "@huanshu")
		end
	elseif event == sgs.Pindian then -- 拼点摸一张牌
		local pindian = data:toPindian()
		pindian.from:drawCards(1)
	elseif event == sgs.MaxHpChanged then -- 上限改变时复活他人
		if math.random(0, 2) == 1 then
			local players = sgs.QList2Table(self.room:getPlayers())
			self:sort(players)
			for _, dead in ipairs(players) do
				if dead:isDead() then
					dead:setRole(player:getRole())
					self.room:revivePlayer(dead)
					self.room:broadcastProperty(dead, "role")
					break
				end
			end
		end
	elseif event == sgs.PhaseChange then
		if player:getPhase() == sgs.Player_Finish then -- 回合结束阶段随机将一名敌人翻面
			if math.random(0, 2) == 1 and #self.enemies > 0 then
				self.enemies[1]:turnOver()
			end
		elseif player:getPhase() == sgs.Player_Start then -- 回合开始阶段随机将一名敌人铁索
			if math.random(0, 3) == 1 and #self.enemies > 0 then
				self.enemies[1]:setChained(true)
				self.room:broadcastProperty(self.enemies[1], "chained")
				self.room:setEmotion(self.enemies[1], "chain")
			end
		elseif player:getPhase() == sgs.Player_Play then -- 出牌阶段随机产生跳出弃牌阶段效果
			if math.random(0, 2) == 1 then
				player:skip(sgs.Player_Discard)
			end
		end
	end
end
