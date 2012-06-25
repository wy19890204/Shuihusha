module("extensions.fuckyou", package.seeall)
extension = sgs.Package("fuckyou")

miheng = sgs.General(extension, "miheng", "god", 3)

jieao=sgs.CreateTriggerSkill{
	name="jieao",
	frequency = sgs.Skill_Compulsory,
	events={sgs.PhaseChange},

	on_trigger = function(self,event,player,data)
        local room = player:getRoom()
		if player:getPhase() == sgs.Player_Start and player:getHp() > player:getHandcardNum() then
            local log = sgs.LogMessage()
            log.type = "#TriggerSkill";
            log.from = player;
            log.arg = self:objectName();
            room:sendLog(log);
            player:drawCards(2);
        end
        return false
	end
}

yulucard=sgs.CreateSkillCard{
	name = "yulu",
	target_fixed = true,
	will_throw = false,
	on_use = function(self, room, source, targets)
		for _, word_id in sgs.qlist(self:getSubcards()) do
			source:addToPile("word", word_id)
		end
	end
}

yulu=sgs.CreateViewAsSkill{
	name = "yulu",
	n = 998,
	
	view_filter = function(self, selected, to_select)
		return not to_select:isEquipped()
	end,

	view_as = function(self, cards)
		if #cards > 0 then
			local Yulucard = yulucard:clone()
			local i = 0
			while(i < #cards) do
				i = i + 1
				Yulucard:addSubcard(cards[i]:getId())
			end
			Yulucard:setSkillName(self:objectName())
			return Yulucard
		else return nil
		end
	end,
}

ViewMyWordscard=sgs.CreateSkillCard{
	name = "viewmywords",
	target_fixed = true,
	on_use = function(self, room, source, targets)
		local words = source:getPile("word")
		if words:isEmpty() then return end
		room:fillAG(words, source)
		local card_id = room:askForAG(source, words, true, "viewmywords")
		if card_id > -1 then
			words:removeOne(card_id);
			room:moveCardTo(sgs.Sanguosha:getCard(card_id), source, sgs.Player_Hand, false)
		end
		source:invoke("clearAG");
	--	words:clear()
	end
}

ViewMyWords=sgs.CreateViewAsSkill{
	name = "numa",
	n = 0,

	view_as = function(self, cards)
		if #cards ~= 0 then return nil end
		local VMWcard = ViewMyWordscard:clone()
		return VMWcard
	end,

	enabled_at_play=function(self, player)
        return not player:getPile("word"):isEmpty()
	end,
}

numa=sgs.CreateTriggerSkill{
	name = "numa",
	frequency = sgs.Skill_NotFrequent,
	events = {sgs.PhaseChange},
	view_as_skill = ViewMyWords,
	
	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		if player:getPhase() ~= sgs.Player_Finish then return false end
		if player:getPile("word"):isEmpty() then return false end
        if not room:askForSkillInvoke(player, self:objectName()) then return false end
        
		local c, word = "", ""
		for _, i in sgs.qlist(player:getPile("word")) do
			c = string.sub(sgs.Sanguosha:getCard(i):getSuitString(), 1, 1)
		
			local log = sgs.LogMessage()
			log.type = "#Numasingle"
			log.from = player
			log.arg = self:objectName() .. c
			room:sendLog(log)
			word = word .. c
		end

        local gitlog = sgs.LogMessage()
		gitlog.type = "#Numa_" .. word
		gitlog.from = player
		gitlog.arg = "numa_notice"
		if word == "hc" then
			room:sendLog(gitlog)
			--womei:recover self
			local womei = sgs.RecoverStruct()
			womei.who = player
			room:recover(player, womei)
		elseif word == "dc" then
			room:sendLog(gitlog)
			--nimei:throw single player 2 cards
			local players = sgs.SPlayerList()
        	for _, tmp in sgs.qlist(room:getAlivePlayers()) do
				if tmp:getHandcardNum() < 2 then
					if tmp:hasSkill("lianying") then
                        players:append(tmp)
					elseif tmp:hasSkill("shangshi") and tmp:isWounded() then
                        players:append(tmp)
					end
				else
                    players:append(tmp)
				end
			end
			if not players:isEmpty() then
				room:askForDiscard(room:askForPlayerChosen(player, players, self:objectName()), self:objectName(), 2);
            end
		elseif word == "cc" then
			room:sendLog(gitlog)
			--meimei:clear single player's all judge_area
			local players = sgs.SPlayerList()
            for _, tmp in sgs.qlist(room:getAlivePlayers()) do
				if not tmp:getJudgingArea():isEmpty() then players:append(tmp) end
			end
			if not players.isEmpty() then
				local target = room:askForPlayerChosen(player, players, self:objectName())
				for _, c in sgs.qlist(target:getJudgingArea()) do
					room:throwCard(c:getId())
				end
            end
		elseif word == "sd" then
			room:sendLog(gitlog)
			--rini:let single player tribute a card and recover 1 hp
			local players = sgs.SPlayerList()
			for _, tmp in sgs.qlist(room:getOtherPlayers(player)) do
				if tmp:isWounded() and not tmp:isKongcheng() then players:append(tmp) end
			end
			if not players.isEmpty() then
                local target = room:askForPlayerChosen(player, players, self:objectName())
				local card = room:askForCardShow(target, player, self:objectName())
				player:obtainCard(card)
				local rini = sgs.RecoverStruct()
				rini.card = card
				rini.who = player
				room:recover(target, rini)
			end
		elseif word == "hs" then
			room:sendLog(gitlog)
			--wori:get skill fanchun
			local judge = sgs.JudgeStruct()
			judge.pattern = sgs.QRegExp("(Peach|GodSalvation):(.*):(.*)")
			judge.good = true
			judge.reason = self:objectName()
			judge.who = player
			room:judge(judge)
			if judge.isGood() then room:acquireSkill(player, "fanchun") end
		elseif word == "hsc" or word == "hsd" then
			room:sendLog(gitlog)
			--worimei&worini:recover hp with a girl or a boy
			local players = sgs.SPlayerList()
			for _, tmp in sgs.qlist(room:getOtherPlayers(player)) do
				if ((word == "hsc" and tmp:getGeneral():isFemale()) or
				   (word == "hsd" and tmp:getGeneral():isMale())) and tmp:isWounded() then
					players:append(tmp)
				end
			end
			if not players.isEmpty() then
				local target = room:askForPlayerChosen(player, players, self:objectName())
				local worimei = sgs.RecoverStruct()
				worimei.card = nil
				worimei.who = player
				room:recover(target, worimei)
				room:recover(player, worimei)
			end
		elseif word == "dsh" then
			room:sendLog(gitlog)
			--niriwo:call slash me! or taking away all his cards
			local players = sgs.SPlayerList()
			for _, tmp in sgs.qlist(room:getAlivePlayers()) do
				if tmp:canSlash(player) then players:append(tmp) end
			end
			local target = room:askForPlayerChosen(player, players, self:objectName())
			local slash = room:askForCard(target, "slash", self:objectName())
			if slash then
				local niriwo = sgs.CardUseStruct()
				niriwo.card = slash
				niriwo.to:append(player)
				niriwo.from = target
				room:useCard(niriwo)
			elseif not target:isNude() then
				local cards = target:getCards("hej")
				for _, tmp in sgs.qlist(cards) do
					room:moveCardTo(tmp, player, sgs.Player_Hand, false)
				end
			end
		elseif word == "shc" then
			room:sendLog(gitlog)
			--riwomei:let single player damage myself and recover himself
			local riwmei = sgs.DamageStruct()
			local target = room:askForPlayerChosen(player, room:getAlivePlayers(), self:objectName())
			riwmei.from = target
			riwmei.to = player
			room:damage(riwmei)

			local riwomei = sgs.RecoverStruct()
			riwomei.card = nil
			riwomei.who = player
			room:recover(target, riwomei)
		elseif word == "hhh" then
			room:sendLog(gitlog)
			--wowowo:the same to Jushou
			player:turnOver()
			player:drawCards(3)
		elseif word == "sss" then
			room:sendLog(gitlog)
			--ririri:the same to Fangzhu
			local target = room:askForPlayerChosen(player, room:getAlivePlayers(), self:objectName())
			target:turnOver()
			target:drawCards(player:getMaxHP() - player:getHp())
		elseif word == "ddd" then
			room:sendLog(gitlog);
			--ninini:let a player obtain word-card
			local target = room:askForPlayerChosen(player, room:getAlivePlayers(), self:objectName())
			for _, i in sgs.qlist(player:getPile("word")) do
				room:moveCardTo(sgs.Sanguosha:getCard(i), target, sgs.Player_Hand)
			end
		elseif word == "ccc" then
			room:sendLog(gitlog)
			--meimeimei:clear single player's all equip_area
			local target = room:askForPlayerChosen(player, room:getAlivePlayers(), self:objectName())
			target:throwAllEquips()
		elseif word == "dcdc" then
			room:sendLog(gitlog)
			--nimeinimei:make a extra turn
			local target = room:askForPlayerChosen(player, room:getAlivePlayers(), self:objectName())
			for _, i in sgs.qlist(player:getPile("word")) do
				room:throwCard(i)
			end
			target:gainAnExtraTurn(player)
		elseif word == "sdc" or word == "hsdc" then
			room:sendLog(gitlog)
			--rinimei:slash
			--worinimei:drank and slash
			if word == "hsdc" then room:setPlayerFlag(player, "drank") end

			local players = sgs.SPlayerList()
			for _, tmp in sgs.qlist(room:getAlivePlayers()) do
				if player:canSlash(tmp, false) then players:append(tmp) end
			end
			local target = room:askForPlayerChosen(player, players, self:objectName())

			local slashtype = sgs.Sanguosha:getCard(player:getPile("word"):first()):getNumber()

			if not players:isEmpty() then
				local worinimei = sgs.CardUseStruct()
				local card;
				if word == "sdc" and slashtype < 5 then
					card = sgs.Sanguosha:cloneCard("thunder_slash", sgs.Card_NoSuit, 0)
				elseif word == "sdc" and slashtype > 9 then
					card = sgs.Sanguosha:cloneCard("fire_slash", sgs.Card_NoSuit, 0)
				else
					card = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
				end
				card:setSkillName(self:objectName())
				worinimei.card = card
				worinimei.from = player
				worinimei.to:append(target)
				room:useCard(worinimei)
			end
		elseif word == "ccsh" then
			room:sendLog(gitlog)
			--nimeiriwo:hp full
			room:setPlayerProperty(player, "hp", sgs.QVariant(player:getMaxHP()))
		elseif word == "dsdc" then
			room:sendLog(gitlog);
			--nimeiriwo:show one player's handcard to other one
			local source = room:askForPlayerChosen(player, room:getAlivePlayers(), self:objectName())
			local target = room:askForPlayerChosen(player, room:getAlivePlayers(), self:objectName())

			local log=sgs.LogMessage()
			log.type = "#Info_dsdc"
			log.from = source
			log.to:append(target)
			room:sendLog(log)

			room:showAllCards(target, source)
		elseif word == "dshc" then
			room:sendLog(gitlog)
			--niriwomei:kill-self
			--[[if Config.FreeChoose and room:askForChoice(player, "numat", "kno+kyes") == "kno" then
				gitlog.type = "#Numa_tequan"
				gitlog.from = player
				room:sendLog(gitlog)
			else]]
			local damage = sgs.DamageStruct()
			damage.from = player
			room:killPlayer(player, damage)
		elseif word == "hhhhh" then
			room:sendLog(gitlog);
			--niriwomei:goto dengai
			if sgs.Sanguosha:getGeneral("dengai") then
				room:transfigure(player, "dengai", true)
				for _, i in sgs.qlist(player:getPile("word")) do
					room:throwCard(i)
					player:addToPile("field", i)
				end
			end
		elseif word == "dshcc" then
			room:sendLog(gitlog);
			--niriwomeimei:throw other 4 card and make 2 damage to self
			local target = room:askForPlayerChosen(player, room:getAlivePlayers(), self:objectName())
			for i=4, i > 0, -1 do
				room:throwCard(room:askForCardChosen(player, target, "he", self:objectName()))
			end
			local niriwomm = sgs.DamageStruct()
			niriwomm.from = player
			niriwomm.to = player
			niriwomm.damage = 2
			room:damage(niriwomm)
		elseif word == "hsdcc" and player:getMark("hsdcc") == 0 then
			room:sendLog(gitlog)
			--worinimeimei:Limited-Skill, like GreatYeyan
			local target = room:askForPlayerChosen(player, room:getAlivePlayers(), self:objectName())
			local worinimm = sgs.DamageStruct()
			worinimm.from = player
			worinimm.to = target
			worinimm.nature = sgs.DamageStruct_Thunder
			room:damage(worinimm)
			worinimm.nature = sgs.DamageStruct_Fire
			room:damage(worinimm)
			worinimm.nature = sgs.DamageStruct_Normal;
			room:damage(worinimm)
			room:loseHp(player, 2)
			player:addMark("hsdcc")
		elseif word == "dcshc" and player:getMark("dcshc") == 0 then
			room:sendLog(gitlog)
			--worinimeimei:Limited-Skill, like Guixin
			room:loseHp(player)
			for _, tmp in sgs.qlist(room:getAllPlayers()) do
				if not tmp:isKongcheng() then
					local card_id = tmp:getRandomHandCardId()
					room:obtainCard(player, card_id)
				end
			end
			player:turnOver()
			player:addMark("dcshc")
		elseif word == "ssdcc" and player:getMark("ssdcc") == 0 then
			room:sendLog(gitlog)
			--ririnimeimei:lightning
			local players = sgs.SPlayerList()
			for _, tmp in sgs.qlist(room:getAlivePlayers()) do
				for _, lightning in sgs.qlist(tmp:getJudgingArea()) do
					if lightning:objectName() == "lightning" then
						players:append(tmp)
						break
					end
				end
			end
			if not players.isEmpty() then
				local target = room:askForPlayerChosen(player, players, self:objectName())
				for _, lightning in sgs.qlist(target:getJudgingArea()) do
					if lightning:objectName() == "lightning" then
						room:throwCard(lightning:getId())
						break
					end
				end
				local damage = sgs.DamageStruct()
				damage.to = target
				damage.nature = sgs.DamageStruct_Thunder
				damage.damage = 3
				room:damage(damage)

				player:addMark("ssdcc")
			end
		elseif word == "ssscc" and player:getMark("ssscc") == 0 then
			room:sendLog(gitlog)
			--riririmeimei:let single player acquire benghuai or wumou
			local players = sgs.SPlayerList()
			for _, tmp in sgs.qlist(room:getOtherPlayers(player)) do
				if tmp:getMaxHP() > player:getMaxHP() then players:append(tmp) end
			end
			if not players.isEmpty() then
				local target = room:askForPlayerChosen(player, players, self:objectName())
				local choice = room:askForChoice(target, self:objectName(), "benghuai+wumou")
				room:setPlayerProperty(target, "maxhp", sgs.QVariant(target:getMaxHP() + 1))
				room:acquireSkill(target, choice)
				player:addMark("ssscc");
			end
		elseif string.len(word) == 4 then
			gitlog.type = "#Numa_4wd"
			gitlog.from = player
			room:sendLog(gitlog)
			--worinimeimei:Wake-Skill, lost all skills
			--[[if Config.FreeChoose and room:askForChoice(player, "numat", "suno+suyes") == "suno" then
				gitlog.type = "#Numa_tequan"
				gitlog.from = player
				room:sendLog(gitlog)
			else]]
				for _, skill in sgs.qlist(player:getVisibleSkillList()) do
					room:detachSkillFromPlayer(player, skill:objectName())
				end
				room:setPlayerProperty(player, "general", sgs.QVariant("sujiang"))
				room:setPlayerProperty(player, "general2", sgs.QVariant("sujiangf"))
				room:setPlayerProperty(player, "maxhp", sgs.QVariant(player:getMaxHP() + 2))
		elseif string.len(word) == 5 and player:getMark("fivewd") == 0 then
			gitlog.type = "#Numa_5wd"
			gitlog.from = player
			room:sendLog(gitlog)
			--worinimeimei:Wake-Skill, learn longhun
			--[[if(Config.FreeChoose && room:askForChoice(player, "numat", "lhno+lhyes") == "lhno" then
				gitlog.type = "#Numa_tequan";
				gitlog.from = player;
				room:sendLog(gitlog);
			}
			else{]]
				room:loseMaxHp(player);
				if player:isAlive() and sgs.Sanguosha:getSkill("longhun") then
					room:acquireSkill(player, "longhun")
					player:addMark("fivewd")
				end
		elseif string.len(word) > 5 and player:getMark("othwd") == 0 then
			gitlog.type = "#Numa_wds"
			gitlog.from = player
			room:sendLog(gitlog)
			--worinimeimei:Wake-Skill, learn wuyan and buqu
			room:loseMaxHp(player, 2)
			if player:isAlive() then
				room:acquireSkill(player, "wuyan")
				room:acquireSkill(player, "buqu")
				player:addMark("othwd")
			end
		else
			gitlog.type = "#Numa_git"
			gitlog.from = player
			room:sendLog(gitlog)
		end
		for _, i in sgs.qlist(player:getPile("word")) do
			room:throwCard(i)
		end
        return false;
	end,
}

fanchun=sgs.CreateTriggerSkill
{
	name = "fanchun",
	frequency = sgs.Skill_NotFrequent,
	events={sgs.Damaged},

	on_trigger=function(self,event,player,data)
        local room = player:getRoom()
        local card = data:toDamage().card
		local data = sgs.QVariant(0)
		data:setValue(card)
        if room:askForSkillInvoke(player, self:objectName(), data) then
            if not card:getSubcards():isEmpty() then
				for _, cd in sgs.qlist(card:getSubcards()) do
                    player:addToPile("word", cd)
				end
            else
                player:addToPile("word", card:getEffectiveId())
			end
        end
	end
}

noqing=sgs.CreateTriggerSkill{
	name="noqing",
	frequency = sgs.Skill_Compulsory,
	events={sgs.Damaged},
	priority = -1,

	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		for _, tmp in sgs.qlist(room:getOtherPlayers(player)) do
			if tmp:getHp() < player:getHp() then
				return false
			end
		end
		for _, tmp in sgs.qlist(room:getAllPlayers()) do
			local choice = room:askForChoice(tmp, self:objectName(), "hp+max_hp")
			local log = sgs.LogMessage()
			log.from = player
			log.arg = self:objectName()
			log.to:append(tmp)
			if(choice == "hp") then
				log.type = "#NoqingLoseHp"
				room:sendLog(log)
				room:loseHp(tmp)
			else
				log.type = "#NoqingLoseMaxHp"
				room:sendLog(log)
				room:loseMaxHp(tmp)
			end
		end
		return false
	end
}

miheng:addSkill(yulu)
miheng:addSkill(numa)
miheng:addSkill(jieao)

sgs.LoadTranslationTable{
	["fuckyou"] = "实干家",

	["#miheng"] = "孤胆愤青",
	["miheng"] = "祢衡",
	["designer:miheng"] = "Log67、宇文天启",
	["illustrator:miheng"] = "dannyke画客",
	["cv:miheng"] = "",
	["yulu"] = "语录",
	[":yulu"] = "出牌阶段，你可以将2~5张手牌按一定顺序移出游戏作为语录的词汇：红桃视为〖我〗，黑桃视为〖日〗，方片视为〖你〗，梅花视为〖妹〗",
	["numa"] = "怒骂",
	[":numa"] = "回合结束阶段，你可以大声朗读（使用）当前语录（见详解）",
	["viewmywords"] = "查看语录",
	["jieao"] = "桀骜",
	[":jieao"] = "锁定技，回合开始阶段，若你的手牌数小于当前体力值，立即摸2张牌",
	["word"] = "语录表",
	["yuluword"] = "请选择一张手牌加入语录表",
	["numah"] = "我",
	["numas"] = "日",
	["numad"] = "你",
	["numac"] = "妹",
	["#Numasingle"] = "%from 说：%arg",
	["#Numa_hc"] = "语录词汇匹配成功！效果：回复1点体力",
	["#Numa_dc"] = "语录词汇匹配成功！效果：令一名角色弃掉两张手牌",
	["#Numa_cc"] = "语录词汇匹配成功！效果：清空一名角色的判定区",
	["#Numa_sd"] = "语录词汇匹配成功！效果：令一名其他角色给你一张手牌，然后其回复1点体力",
	["#Numa_hs"] = "语录词汇匹配成功！效果：做一次判定，若为肉或梁山聚义，你永久获得技能【反唇】（你可以把对你造成伤害的牌加入语录）",
	["fanchun"] = "反唇",
	[":fanchun"] = "祢衡专用隐藏技，你可以把对你造成伤害的牌加入语录表",
	["#Numa_hsc"] = "语录词汇匹配成功！效果：指定一名其他女性角色，你和她各回复1点体力",
	["#Numa_hsd"] = "语录词汇匹配成功！效果：指定一名其他男性角色，你和他各回复1点体力",
	["#Numa_dsh"] = "语录词汇匹配成功！效果：令一名角色对你使用【杀】，否则将所有牌给你",
	["#Numa_sdc"] = "语录词汇匹配成功！效果：视为对任意一名角色使用一张【杀】；若〖日〗牌点数小于5，此杀为雷杀，若大于9，此杀为火杀",
	["#Numa_shc"] = "语录词汇匹配成功！效果：令一名角色对自己造成1点伤害，然后其回复一点体力",
	["#Numa_hhh"] = "语录词汇匹配成功！效果：将自己翻面并摸三张牌",
	["#Numa_sss"] = "语录词汇匹配成功！效果：令一名角色翻面并摸X张牌，X为你损失的体力",
	["#Numa_ddd"] = "语录词汇匹配成功！效果：将自己当前语录表的词汇牌给一名角色",
	["#Numa_ccc"] = "语录词汇匹配成功！效果：清空一名角色的装备区",
	["#Numa_dcdc"] = "语录词汇匹配成功！效果：指定一名角色开始一个额外的回合",
	["#Numa_hsdc"] = "语录词汇匹配成功！效果：视为喝酒后对任意一名角色使用一张【杀】",
	["#Numa_ccsh"] = "语录词汇匹配成功！效果：回复体力至体力上限",
	["#Numa_dsdc"] = "语录词汇匹配成功！效果：令一名角色观看另一名角色的手牌",
	["#Info_dsdc"] = "%from 观看了 %to 的手牌",
	["#Numa_dshc"] = "语录词汇匹配成功！效果：自杀",
	["numat"] = "测试员特权",
	["numat:kno"] = "不，还不能死！",
	["numat:kyes"] = "不想活了……",
	["#Numa_dshcc"] = "语录词汇匹配成功！效果：弃掉一名角色四张牌，之后对自己造成2点伤害",
	["#Numa_hsdcc"] = "语录词汇匹配成功！效果：限定技，对一名角色连续造成1点雷电伤害、1点火焰伤害、1点无属性伤害，之后自己流失2点体力",
	["#Numa_dcshc"] = "语录词汇匹配成功！效果：限定技，自减1点体力并从每名角色那里获得一张手牌，然后将你的武将牌翻面",
	["#Numa_ssdcc"] = "语录词汇匹配成功！效果：限定技，立即激活场上存在的一个【闪电】",
	["#Numa_ssscc"] = "语录词汇匹配成功！效果：限定技，指定一名体力上限比你多的角色并令其增加1点体力上限并永久获得你指定的一个负面技",
	["#Numa_4wd"] = "语录词汇貌似匹配完成……效果：觉醒技，增加2点体力上限并失去当前的所有技能",
	["#Numa_tequan"] = "%from 使用了测试员特权，取消了当前语法的效果",
	["numat:suno"] = "我不要变素将！",
	["numat:suyes"] = "变素将？我喜欢~",
	["#Numa_5wd"] = "语录词汇匹配紊乱……效果：觉醒技，减去1点体力上限并永久获得技能【龙魂】",
	["numat:lhno"] = "龙魂神马的最讨厌了！",
	["numat:lhyes"] = "龙魂？神技啊，我喜欢~",
	["#Numa_wds"] = "语录系统故障，原因为负载过多……效果：觉醒技，减去2点体力上限并永久获得技能【无言】和【不屈】",
	["#Numa_git"] = "语录词汇匹配失败！你没有获得任何效果",
	["~miheng"] = "独在异乡为异客，穿越之后倍思亲……",
}
