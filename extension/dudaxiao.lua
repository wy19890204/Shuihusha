luadaxiaocard = sgs.CreateSkillCard
{
	name = "luadaxiaocard",
	target_fixed = true,
	will_throw = false,
	filter = function(self, selected, to_select)
		return true
	end,
	on_use = function(self, room, source, targets)
		local room = source:getRoom()
		local wxb = room:findPlayerBySkillName("luadaxiaovs")
		if wxb == nil then return false end
		local log = sgs.LogMessage()
		log.type = "#luadaxiaovs"
		log.from = wxb
		room:sendLog(log)
		
		local card = self:getSubcards()
		local sum = 0
		for _,cardid in sgs.qlist(card) do
			local num = sgs.Sanguosha:getCard(cardid):getNumber()
			wxb:addToPile("ldx_pile", cardid, false)
			sum = sum + num
		end
		local result = ""
		if sum < 20 then
			result = "xiao" 
		elseif sum > 20 then 
			result = "da"
		elseif sum == 20 then 
			result = "tongchi"
		end
		local third = sgs.Sanguosha:getCard(wxb:getPile("ldx_pile"):at(2)):getNumber()
		local second = sgs.Sanguosha:getCard(wxb:getPile("ldx_pile"):at(1)):getNumber()
		local first = sgs.Sanguosha:getCard(wxb:getPile("ldx_pile"):at(0)):getNumber()
		if first == second and second == third and third == first then
			result = "baozi"
		end
		local sp = room:getOtherPlayers(wxb)
		for _,p in sgs.qlist(sp) do
			local style = ""
			if p:isKongcheng() then
				style = room:askForChoice(p, "style", "1hp+1hpmax")
				if style == "1hp" then
					room:loseHp(p, 1)
					room:setPlayerMark(p, "1hp", 1)
				elseif style == "1hpmax" then 
					room:loseMaxHp(p, 1)
					room:setPlayerMark(p, "1hpmax", 1)
				end
			else
				style = room:askForChoice(p, "style", "1hp+1hpmax+cd2_1")
				if style == "1hp" then
					room:loseHp(p, 1)
					room:setPlayerMark(p, "1hp", 1)
				elseif style == "1hpmax" then 
				
					local log = sgs.LogMessage()
					log.type = "#luadaxiaoxz1hpmax"
					log.from = p
					room:sendLog(log)
				
					room:loseMaxHp(p, 1)
					room:setPlayerMark(p, "1hpmax", 1)
				else
					room:attachSkillToPlayer(p, "luadaxiaoxz")
					local cost = room:askForUseCard(p, "@@xiazhu", "@luaxiazhu")
					if not cost then
						p:addToPile("cost_pile", room:askForCardChosen(wxb, p, "he", "luaxiazhu"), false)
					end
					room:setPlayerMark(p, "cd2_1", 1)
					room:detachSkillFromPlayer(p, "luadaxiaoxz")
				end	
			end
			
			local log = sgs.LogMessage()
			log.type = "#luadaxiaostyle"
			log.from = p
			log.arg = style
			room:sendLog(log)
			
			local choice = room:askForChoice(p, "luadaxiaoy", "da+xiao+baozi")
			if choice == "da" then 
				room:setEmotion(p, "da")
				room:setPlayerMark(p, "da", 1)
			elseif choice == "xiao" then 
				room:setEmotion(p, "xiao")
				room:setPlayerMark(p, "xiao", 1)
			elseif choice == "baozi" then 
				room:setEmotion(p, "baozi")
				room:setPlayerMark(p, "baozi", 1)
			end
			local log = sgs.LogMessage()
			log.type = "#luadaxiaoselect"
			log.from = p
			log.arg = choice
			room:sendLog(log)
		end
		local cards = sgs.IntList()
		for _,cdid in sgs.qlist(card) do
			cards:append(cdid)
		end
	
		local log = sgs.LogMessage()
		log.type = "#luadaxiaoresult"
		log.from = wxb
		log.arg = sum
		log.arg2 = result
		room:sendLog(log)
		
		for _,p in sgs.qlist(sp) do
			room:fillAG(cards, p)
			local chosen = ""
			if p:getMark("da") == 1 then chosen = "da" 
			elseif p:getMark("xiao") == 1 then chosen = "xiao" 
			elseif p:getMark("baozi") == 1 then chosen = "baozi" 
			end
			if chosen == result then
				local log = sgs.LogMessage()
				log.type = "#luadaxiaoresultgb"
				log.from = p
				log.arg = "right"
				room:sendLog(log)
				
				local styled = ""
				if p:getMark("1hp") == 1 then styled = "1hpd" 
				elseif p:getMark("1hpmax") == 1 then styled = "1hpmaxd" 
				elseif p:getMark("cd2_1") == 1 then styled = "cd2_1d" 
				end
				
				local log = sgs.LogMessage()
				log.type = "#luadaxiaoresultgain"
				log.from = p
				log.arg = styled
				room:sendLog(log)
				
				if styled == "1hpd" then
					if chosen == "baozi" then
						room:loseHp(wxb, 2)
						local recover = sgs.RecoverStruct()
						recover.reason = "luadaxiaowin"
						recover.card = nil
						recover.who = p
						recover.recover = 3
						room:recover(p, recover)
					else
						room:loseHp(wxb, 1)
						local recover = sgs.RecoverStruct()
						recover.reason = "luadaxiaowin"
						recover.card = nil
						recover.who = p
						recover.recover = 2
						room:recover(p, recover)
					end
				elseif styled == "1hpmaxd" then
					if chosen == "baozi" then
						room:loseMaxHp(wxb, 2)
						local x = p:getHp()
						room:setPlayerProperty(p, "maxhp", sgs.QVariant(p:getMaxHP() + 3))
						room:setPlayerProperty(p, "hp", sgs.QVariant(x + 2))
						
						local log = sgs.LogMessage()
						log.type = "#luadaxiaorgain1hpmaxbz"
						log.from = p
						log.to:append(wxb)
						room:sendLog(log)
						
					else
						room:loseMaxHp(wxb, 1)
						local x = p:getHp()
						room:setPlayerProperty(p, "maxhp", sgs.QVariant(p:getMaxHP() + 2))
						room:setPlayerProperty(p, "hp", sgs.QVariant(x + 1))
						
						local log = sgs.LogMessage()
						log.type = "#luadaxiaorgain1hpmax"
						log.from = p
						log.to:append(wxb)
						room:sendLog(log)
						
					end
				elseif styled == "cd2_1d" then
					if p:getPile("cost_pile"):length() == 1 then
						local t = 1
						if chosen == "baozi" then 
							t = 2 
						end
						for v = 1, t, 1 do
							if wxb:isNude() then
								room:loseHp(wxb, 1)
								wxb:drawCards(1)
							else
								local acard = room:askForCardChosen(p, wxb, "hej", "luadaxiao")
								room:moveCardTo(acard, p, sgs.Player_Hand, false)
							end
						end
					elseif p:getPile("cost_pile"):length() == 2 then
						local pchoice = room:askForChoice(p, "luadaxiaowin", "gain2cdfromwxb+recover1hp")
						if pchoice == "gain2cdfromwxb" then 
							local t = 2
							if chosen == "baozi" then 
								t = 3 
							end
							for v = 1, t, 1 do
								if wxb:isNude() then
									room:loseHp(wxb, 1)
									wxb:drawCards(1)
								else
									local acard = room:askForCardChosen(p, wxb, "hej", "luadaxiao")
									room:moveCardTo(acard, p, sgs.Player_Hand, false)
								end
							end
						elseif pchoice == "recover1hp" then
							local t = 1
							if chosen == "baozi" then 
								t = 2 
							end
							local recover = sgs.RecoverStruct()
							recover.reason = "luadaxiaowin"
							recover.card = nil
							recover.who = p
							recover.recover = t
							room:recover(p, recover)
						end
					end
					for var = 0, p:getPile("cost_pile"):length(), 1 do 
						p:obtainCard(sgs.Sanguosha:getCard(p:getPile("cost_pile"):at(var)))
					end
				end
				
				
				
			else 
				local log = sgs.LogMessage()
				log.type = "#luadaxiaoresultgb"
				log.from = p
				log.arg = "wrong"
				room:sendLog(log)
				
				local styled = ""
				if p:getMark("1hp") == 1 then styled = "1hpd" 
				elseif p:getMark("1hpmax") == 1 then styled = "1hpmaxd" 
				elseif p:getMark("cd2_1") == 1 then styled = "cd2_1d" 
				end
				
				local log = sgs.LogMessage()
				log.type = "#luadaxiaoresultlose"
				log.from = p
				log.arg = styled
				room:sendLog(log)
				
				if styled == "1hpd" then
					if wxb:isWounded() then
						local recover = sgs.RecoverStruct()
						recover.reason = "luadaxiaowin"
						recover.card = nil
						recover.who = wxb
						recover.recover = 1
						room:recover(wxb, recover)
					else 
						wxb:drawCards(2)
					end
				elseif styled == "1hpmaxd" then
				
					local log = sgs.LogMessage()
					log.type = "#luadaxiaorgain1hpmaxwxb"
					log.from = wxb
					room:sendLog(log)
				
					local x = wxb:getHp()
					room:setPlayerProperty(wxb, "maxhp", sgs.QVariant(wxb:getMaxHP() + 1))
					room:setPlayerProperty(wxb, "hp", sgs.QVariant(x))
				elseif styled == "cd2_1d" then
					for var = 0, p:getPile("cost_pile"):length(), 1 do 
						wxb:obtainCard(sgs.Sanguosha:getCard(p:getPile("cost_pile"):at(var)))
					end
					room:fillAG(cards, wxb)
					local dxc = room:askForAG(wxb, cards, true, "luadaxiao")
					local cardu = sgs.Sanguosha:getCard(dxc)
					wxb:invoke("clearAG")
					p:clearPrivatePiles()
					if cardu:inherits("Slash") or cardu:inherits("Peach") then 
						local use = sgs.CardUseStruct()
						use.card = cardu
						use.to:append(p)
						use.from = wxb
						room:useCard(use, false)
					end
				end	
			end
			room:setPlayerMark(p, "1hp", 0)
			room:setPlayerMark(p, "1hpmax", 0)
			room:setPlayerMark(p, "cd2_1", 0)
			room:setPlayerMark(p, "da", 0)
			room:setPlayerMark(p, "xiao", 0)
			room:setPlayerMark(p, "baozi", 0)
			p:invoke("clearAG")
			p:clearPrivatePiles()
		end
		room:setPlayerFlag(wxb, "luadx_used")
		wxb:clearPrivatePiles()
		room:throwCard(self)
	end,
}				
luadaxiaovs = sgs.CreateViewAsSkill
{
	name = "luadaxiaovs",
	n = 3,
	view_filter = function(self, selected, to_select)
		return not to_select:isEquipped()
	end,
	view_as = function (self, cards)
		if #cards < 3 then return nil end
		if #cards == 3 then 
			local acard = luadaxiaocard:clone()
			for v = 1, 3, 1 do
				acard:addSubcard(cards[v])  
			end
			acard:setSkillName("luadaxiaocard")
			return acard
		end
	end,
	enabled_at_play = function(self, player, pattern)
		return not sgs.Self:hasFlag("luadx_used")
	end,
	enabled_at_response = function (self, player, pattern)
		return false
	end,
}
luaxiazhucard = sgs.CreateSkillCard
{
	name = "luaxiazhucard",
	target_fixed = true,
	will_throw = false,
	on_use = function(self, room, source, targets)
		local room = source:getRoom()
		local card = self:getSubcards()
		for _,cardid in sgs.qlist(card) do
			source:addToPile("cost_pile", cardid, false)
		end
	end,
}
luadaxiaoxz = sgs.CreateViewAsSkill
{
	name = "luadaxiaoxz",
	n = 2,
	view_filter = function(self, selected, to_select)
		return not to_select:isEquipped()
	end,
	view_as = function(self, cards)        
		if #cards == 0 then return end
		local acard = luaxiazhucard:clone()
		for var = 1, #cards, 1 do  
			acard:addSubcard(cards[var])                
		end
		acard:setSkillName(self:objectName())
		return acard        
	end,
	enabled_at_play = function(self, player, pattern)
		return false
	end,
	enabled_at_response = function (self, player, pattern)
		return pattern == "@@xiazhu"
	end,
}
luaxiazhuskill = sgs.CreateTriggerSkill
{
	name = "luaxiazhu",
	frequency = sgs.Skill_Compulsory,
	events = sgs.GameStart,
	on_trigger = function (self, event, player, data)
		local room = player:getRoom()
		room:attachSkillToPlayer(player, "luadaxiaoxz")
	end,
}

  --["luadaxiaovs"] = "赌大小",
	[":luadaxiaovs"] = "出牌阶段，你可以将三张牌扣置于桌面上，其他角色依次将一张手牌作为赌注置于赌注区，并押“大”、“小”、“豹子”。所有人下注完毕后，翻开三张牌，若三张牌点数总和大于20，则为“大”，若三张牌点数总和小于20，则为“小”，若三张牌点数相同，则为“豹子”，若三张牌点数总和等于20，则为“通吃”",
	["luadaxiaointro"] = "赌博简介",
	[":luadaxiaointro"] = "先选择下注类型（1点体力，1点体力上限，手牌）再选择押什么",
	["da"] = "大",
	["xiao"] = "小",
	["baozi"] = "豹子",
	["tongchi"] = "通吃",
	["#luadaxiaovs"] = "%from 开始坐庄，扣置了三张牌，请下注",
	["#luadaxiaoselect"] = "%from 押了 <font color='yellow'><b>%arg</b></font>",
	["#luadaxiaoresult"] = "%from 三张牌开出的点数总和是 <font color='yellow'><b>%arg</b></font> 结果是 <font color='yellow'><b>%arg2</b></font>",
	["#luadaxiaoresultgb"] = "%from 押 <font color='yellow'><b>%arg</b></font> 了",
	["right"] = "对",
	["wrong"] = "错",
	["hprecover"] = "回复 1 点体力",
	["gaincards"] = "获得赌注并摸 2 张牌",
	["self"] = "回复 1 体力并摸 1 张牌（满血时直接摸3张牌）",
	["target"] = "对方弃 2 张牌或流失 1 点体力",
	["cost_pile"] = "赌注",
	["ldx_pile"] = "骰子",
	["style"] = "请问你想怎么赌",
	["1hp"] = "赌1点体力值",
	["1hpmax"] = "赌1点体力上限",
	["cd2_1"] = "赌手牌（至多2张）",
	["luadaxiaoy"] = "请选择押什么",
	["luadaxiaowin"] = "请选择一项",
	["luaxiazhu"] = "下注",
	[":luaxiazhu"] = "请点击【下注】技能按钮来选择一张牌或两张牌作为赌注（赌注牌其他人均不可见）",
	["gain2cdfromwxb"] = "获得庄家的两张牌",
	["recover1hp"] = "回复1点体力",
	["#luadaxiaostyle"] = "%from 选择了 <font color='yellow'><b>%arg</b></font>",
	["#luadaxiaoxz1hpmax"] = "%from 失去了 <b><font color='#98fb98'>1</font></b> 点体力上限",
	["#luadaxiaoresultgain"] = "%from 赢得了 <font color='yellow'><b>%arg</b></font>", 
	["#luadaxiaoresultlose"] = "%from 输掉了 <font color='yellow'><b>%arg</b></font>", 
	["1hpd"] = "1点体力值",
	["1hpmaxd"] = "1点体力上限",
	["cd2_1d"] = "手牌",
	["luadaxiaoxz"] = "下注",
	["#luadaxiaorgain1hpmaxbz"] = "%from 增加了 <b><font color='#98fb98'>2</font></b> 点体力上限，%to 失去了 <b><font color='#98fb98'>2</font></b> 点体力上限",
	["#luadaxiaorgain1hpmax"] = "%from 增加了 <b><font color='#98fb98'>1</font></b> 点体力上限，%to 失去了 <b><font color='#98fb98'>1</font></b> 点体力上限",
	["#luadaxiaorgain1hpmaxwxb"] = "%from 增加了 <b><font color='#98fb98'>1</font></b> 点体力上限",