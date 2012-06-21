module("extensions.zhimeng", package.seeall)
extension = sgs.Package("zhimeng")

diyliru = sgs.General(extension, "diyliru", "qun", 3)
diypanfeng = sgs.General(extension, "diypanfeng", "qun")
diychengyu = sgs.General(extension, "diychengyu", "wei", 3)
diyjiangwan = sgs.General(extension, "diyjiangwan", "shu", 3)
diyyuejin = sgs.General(extension, "diyyuejin", "wei")
diychendao = sgs.General(extension, "diychendao", "shu")
diysimazhao = sgs.General(extension, "diysimazhao", "wei", 3)
diysunluban = sgs.General(extension, "diysunluban", "wu", 3, false)
diyzhugejin = sgs.General(extension, "diyzhugejin", "wu")
diyjushou = sgs.General(extension, "diyjushou", "qun", 3)

dujicard=sgs.CreateSkillCard{
	name="dujicard",
	target_fixed=true,
	on_use = function(self, room, source, targets)
		source:addToPile("du", self:getSubcards():first())
	end
}

dujiViewAsSkill=sgs.CreateViewAsSkill{
	name="dujiViewAsSkill",
	n=1,

	view_filter = function(self, selected, to_select)
		return to_select:getSuit() == sgs.Card_Spade
	end,

	view_as = function(self, cards)
		if(#cards~=1) then return nil end
		local DJcard=dujicard:clone()
			DJcard:addSubcard(cards[1])
			DJcard:setSkillName(self:objectName())
		return DJcard
	end,

	enabled_at_play=function(self, player)
		return player:getPile("du"):isEmpty()
	end,
}

duji=sgs.CreateTriggerSkill{
	name="duji",
	frequency=sgs.Skill_NotFrequent,
	events={sgs.CardUsed},
	view_as_skill=dujiViewAsSkill,

	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
        if(player:getPhase() ~= sgs.Player_Play) then return false end

        local use = data:toCardUse()
        local liru = room:findPlayerBySkillName(self:objectName())
		if player:objectName() == liru:objectName() then return false end
        if(not liru or liru:getPile("du"):isEmpty()) then return false end

        if(use.card:inherits("Slash") and not player:hasFlag("drank") and room:askForSkillInvoke(liru,self:objectName())) then
            room:obtainCard(player, sgs.Sanguosha:getCard(liru:getPile("du"):first()))
            room:setPlayerFlag(player, "drank")
            if(room:askForChoice(player, self:objectName(), "djturn+djlp") == "djturn") then
                player:turnOver()
            else
                room:loseHp(player)
			end
		end
	end,

	can_trigger=function(self,target)
		return true
	end,
}

diyshipo=sgs.CreateProhibitSkill{
	name = "diyshipo",
	is_prohibited=function(self,from,to,card)
    	if (to:hasSkill(self:objectName())) then
       		return ((card:inherits("Slash") and card:getSuit() == sgs.Card_Spade) or (card:inherits("TrickCard")and card:getSuit() == sgs.Card_Spade))
  		end
	end,
}

liefu=sgs.CreateTriggerSkill{
	name="liefu",
	frequency = sgs.Skill_NotFrequent,
	events={sgs.SlashMissed},

	on_trigger=function(self,event,player,data)
		local effect = data:toSlashEffect()

        local room = player:getRoom()
        if(room:askForSkillInvoke(player,self:objectName(), data)) then
			local log= sgs.LogMessage()
            	log.type = "#Liefu"
            	log.from = player
            	log.to :append(effect.to)
            	log.arg = self:objectName()
            room:sendLog(log)

            room:slashResult(effect, nil)
            if(room:askForChoice(player, self:objectName(), "lfpan+lffeng") == "lfpan") then
				if player:getCardCount(true)<effect.to:getLostHp() then
					room:askForDiscard(player, self:objectName(), player:getCardCount(true), false, true)
				else
					room:askForDiscard(player, self:objectName(), effect.to:getLostHp(), false, true)
				end
            else
				local x
				if effect.to:getHp()>5 then
					x=5
				else
					x=effect.to:getHp()
				end
                effect.to:drawCards(x)
			end
		end
        return false
	end,
}

pengricard=sgs.CreateSkillCard
{
	name="pengricard",
	target_fixed=false,
	filter = function(self, targets, to_select, player)
		return #targets==0 and to_select ~= sgs.Self and not to_select:isNude() and to_select:canSlash(sgs.Self, false)
	end,

	on_use = function(self, room, source, targets)
		local card_id = room:askForCardChosen(souce, targets[1], "he", "pengri")
    	source:obtainCard(sgs.Sanguosha:getCard(card_id))

   		local slash = sgs.Sanguosha:cloneCard("slash",sgs.Card_NoSuit, 0)
		slash:setSkillName("pengri")
		local use=sgs.CardUseStruct()
			use.card = slash
			use.from = targets[1]
			use.to:append(source)
		room:useCard(use)
	end,
}

pengri=sgs.CreateViewAsSkill{
	name="pengri",
	n=0,

	view_as = function(self, cards)
		if(#cards~=0) then return nil end
		local PRcard=pengricard:clone()
			PRcard:setSkillName(self:objectName())
		return PRcard
	end,

	enabled_at_play=function(self, player)
		return not player:hasUsed("#pengricard")
	end,
}

gangli=sgs.CreateTriggerSkill{
	name="gangli",
	frequency = sgs.Skill_NotFrequent,
	events={sgs.Damaged},

	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local damage = data:toDamage()
        if(damage.damage ~= 1 or not damage.from or damage.from:objectName() == player:objectName()) then return end

        if(room:askForSkillInvoke(player, self:objectName(), data)) then
            local target = room:askForPlayerChosen(player, room:getOtherPlayers(damage.from), self:objectName())
            local slash = sgs.Sanguosha:cloneCard("slash",sgs.Card_NoSuit, 0)
            slash:setSkillName(self:objectName())
            local use=sgs.CardUseStruct()
            	use.card = slash
            	use.from = damage.from
            	use.to:append(target)
            room:useCard(use, false)
        end
	end,
}

yaliang=sgs.CreateTriggerSkill
{
	name="yaliang",
	frequency =sgs.Skill_NotFrequent,
	events={sgs.CardEffected},

	on_trigger=function(self,event,player,data)
		local effect = data:toCardEffect()
        if(effect.to:objectName() == effect.from:objectName()) then return false end

        if(effect.card:isNDTrick()) then
            local room = player:getRoom();

            if(room:askForSkillInvoke(player,self:objectName(), data))then
                player:drawCards(1)
                local log=sgs.LogMessage()
               		log.type = "#Yaliang"
                	log.from = effect.to
                	log.to:append(effect.from)
                	log.arg = effect.card:objectName()
                	log.arg2 = self:objectName()
                room:sendLog(log)

                room:playSkillEffect(self:objectName());
                local slash = sgs.Sanguosha:cloneCard("slash",sgs.Card_NoSuit, 0)
         	   	slash:setSkillName(self:objectName())
        	    local use=sgs.CardUseStruct()
        	    	use.card = slash
        	    	use.from = effect.from
        	    	use.to:append(player)
        	    room:useCard(use, false)
                return true
            end
        end
        return false
	end,
}

xunguicard=sgs.CreateSkillCard{
	name="xunguicard",
	target_fixed=true,

	on_use = function(self, room, source, targets)
		if(not source:getPile("gui"):isEmpty()) then
        	room:throwCard(sgs.Sanguosha:getCard(source:getPile("gui"):first()))
        	if(source:isWounded())then
				local recover = sgs.RecoverStruct()
				recover.recover = 1
				recover.card = nil
				recover.who = source
				room:recover(source, recover)
        	end
    	end
    	source:addToPile("gui", self:getSubcards():first())
	end,
}

xungui=sgs.CreateViewAsSkill{
	name="xungui",
	n=1,

	view_filter = function(self, selected, to_select)
		return to_select:isNDTrick()
	end,

	view_as = function(self, cards)
		if(#cards~=1) then return nil end
		local XGcard=xunguicard:clone()
			XGcard:addSubcard(cards[1])
			XGcard:setSkillName(self:objectName())
		return XGcard
	end,

	enabled_at_play=function(self, player)
		return not player:hasUsed("#xunguicard")
	end,
}

daojucard=sgs.CreateSkillCard{
	name="daojucard",
	target_fixed=false,
	filter = function(self, targets, to_select, player)
		return #targets<2
	end,
	on_use = function(self, room, source, targets)
		local cards={}
		cards[1]=sgs.Sanguosha:getCard(self:getSubcards():first())
		cards[2]=sgs.Sanguosha:getCard(self:getSubcards():at(1))
		local newcard=sgs.Sanguosha:cloneCard("iron_chain",cards[1]:getSuit(),0)
		newcard:addSubcard(cards[1])
		newcard:addSubcard(cards[2])
		newcard:setSkillName("daoju")
		local use=sgs.CardUseStruct()
        	use.card = newcard
        	use.from = source
			for x=1,#targets,1 do
        		use.to:append(targets[x])
			end
		room:useCard(use, true)
	end,
}

daojuvs=sgs.CreateViewAsSkill{
	name="daojuvs",
	n=2,
	view_filter = function(self, selected, to_select)
		if #selected ==0 then return not to_select:isEquipped() end
        if #selected == 1 then
            local cc = selected[1]:isRed()
            return (not to_select:isEquipped()) and to_select:isRed() == cc
        else return false end
	end,
	view_as=function(self, cards)
		if #cards~=2 then return nil end
		local cardname=sgs.Sanguosha:getCard(sgs.Self:getPile("gui"):first()):objectName()
		local cardsuit=cards[1]:getSuit()
		local DJcard
		if cardname=="iron_chain" then
			DJcard=daojucard:clone()
		else
			DJcard=sgs.Sanguosha:cloneCard(cardname,cardsuit,0)
		end
		DJcard:addSubcard(cards[1])
		DJcard:addSubcard(cards[2])
		DJcard:setSkillName("daoju")
		return DJcard
	end,
	enabled_at_play=function(self,player)
		return (not player:getPile("gui"):isEmpty())and(not player:hasFlag("daojuuse"))
	end,
}

daoju=sgs.CreateTriggerSkill{
	name="daoju",
	frequency = sgs.Skill_NotFrequent,
	events={sgs.CardUsed},
	view_as_skill=daojuvs,

	on_trigger=function(self,event,player,data)
		local use=data:toCardUse()
		if use.card:getSkillName() == "daoju" then
			player:getRoom():setPlayerFlag(player,"daojuuse")
		end
	end,
}

xiandeng=sgs.CreateTriggerSkill{
	name="xiandeng",
	frequency = sgs.Skill_NotFrequent,
	events = {sgs.DrawNCards,sgs.PhaseChange},

	on_trigger = function(self, event, player, data)
		local room = player:getRoom()
		local x = data:toInt()
		if event==sgs.PhaseChange then
			if player:getPhase() == sgs.Player_Finish then
				for _,p in sgs.qlist(room:getOtherPlayers(player)) do
					room:setPlayerFlag(p,"-xiandeng")
				end
			else
				return false
			end
		else
			if(room:askForSkillInvoke(player, self:objectName())) then
				room:playSkillEffect(self:objectName())
				local target=room:askForPlayerChosen(player,room:getOtherPlayers(player) ,self:objectName())
				room:setPlayerFlag(target,"xiandeng")
				data:setValue(x-1)
			end
		end
	end
}

xiandeng_buff=sgs.CreateDistanceSkill{
	name="#xiandeng_buff",
	correct_func = function(self,from,to)
		if from:hasSkill("xiandeng") and to:hasFlag("xiandeng") then
			return -99
		end
	end,
}

xiaoguojink={}
xiaoguo=sgs.CreateTriggerSkill{
	name="xiaoguo",
	frequency = sgs.Skill_NotFrequent,
	events={sgs.CardUsed,sgs.SlashMissed,sgs.CardFinished,sgs.CardResponsed},
	priority = 2,
	can_trigger=function(self,target)
		return target:getRoom():findPlayerBySkillName(self:objectName())
	end,
	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		local yuejin=room:findPlayerBySkillName(self:objectName())
		if event==sgs.CardUsed and data:toCardUse().card:inherits("Slash") and player:objectName()==yuejin:objectName() then
			for _,p in sgs.qlist(data:toCardUse().to) do
				room:setPlayerFlag(p,"xiaoguotarget")
			end
			return false
		elseif event==sgs.CardFinished and data:toCardUse().card:inherits("Slash") and player:objectName()==yuejin:objectName() then
			for _,p in sgs.qlist(data:toCardUse().to) do
				room:setPlayerFlag(p,"-xiaoguotarget")
				table.remove(xiaoguojink)
			end
			return false
		end
		if event==sgs.CardResponsed and data:toCard():inherits("Jink") and player:hasFlag("xiaoguotarget") then
			xiaoguojink[1]=data:toCard()
			return false
		end
		if event==sgs.SlashMissed and player:objectName()==yuejin:objectName() then
			local effect = data:toSlashEffect()
			local jink = xiaoguojink[1]
        	if(effect.slash:getSuit() ~= sgs.Card_Heart and not effect.slash:isVirtualCard()
and jink:getEffectiveId()~=-1 and effect.from:objectName()==yuejin:objectName())then
            	if(room:askForSkillInvoke(yuejin,self:objectName(), data))then
            	    room:playSkillEffect(self:objectName())
					effect.to:obtainCard(jink)

                	local name
					if effect.slash:isBlack() then
						name="supply_shortage"
					elseif effect.slash:isRed() then
						name="indulgence"
					end
					if not name then
						return false
					end
                	local new_card = sgs.Sanguosha:cloneCard(name, effect.slash:getSuit(), effect.slash:getNumber())
                	new_card:setSkillName("xiaoguo")
                	new_card:addSubcard(effect.slash)

                	if(not effect.from:isProhibited(effect.to, new_card) and not effect.to:containsTrick(name)) then
                    	local use=sgs.CardUseStruct()
                    	use.card = new_card
                    	use.from = effect.from
                    	use.to:append(effect.to)
                    	room:useCard(use)
                	end
            	end
        	end
        	return false
		end
	end,
}

jingruitmp={}
jingrui=sgs.CreateViewAsSkill{
	name="jingrui",
	n=1,

	view_filter = function(self, selected, to_select)
		return not to_select:isEquipped()
	end,

	view_as = function(self, cards)
		if #cards~=1 then return nil end
        local newcard = sgs.Sanguosha:cloneCard(jingruitmp[1],cards[1]:getSuit(),cards[1]:getNumber())
        newcard:addSubcard(cards[1])
        newcard:setSkillName(self:objectName())
        return newcard
	end,

	enabled_at_play=function(self, player)
		jingruitmp[1]="slash"
		return player:getHandcardNum() >= player:getHp() and ((player:canSlashWithoutCrossbow()) or (player:getWeapon() and player:getWeapon():className()=="Crossbow"))
	end,

	enabled_at_response=function(self,player,pattern)
		if ((pattern=="jink") or (pattern=="slash"))and player:getHandcardNum() >= player:getHp() then
			jingruitmp[1]=pattern
			return true
		end
	end,
}

zhaoxincard=sgs.CreateSkillCard{
	name="zhaoxincard",
	target_fixed=true,
	on_use = function(self, room, source, targets)
		room:showAllCards(source)
    	local samecount,unsamecount = 0,0
    	local card1=source:getRandomHandCard()
		for _,card2 in sgs.qlist(source:getHandcards()) do
			if(card1:getId() ~= card2:getId()) then
				if(card1:getSuit() == card2:getSuit()) then
					samecount=samecount+1
				else
					unsamecount=unsamecount+1
				end
			end
		end
    	if(unsamecount == 0)then
        	local targets=sgs.SPlayerList()
        	for _,i in sgs.qlist(room:getOtherPlayers(source)) do
            	if(not i:isNude()) then
                	targets:append(i)
				end
			end
        	if(not targets:isEmpty()) then
            	local t = room:askForPlayerChosen(source, targets, "zhaoxin")
            	local card_id = room:askForCardChosen(source, t, "he", "zhaoxin")
            	room:obtainCard(source, card_id)
        	end
		elseif(samecount == 0)then
       		local target = room:askForPlayerChosen(source, room:getAlivePlayers(), "zhaoxin")
        	room:loseHp(target)
    	end
	end,
}

zhaoxin=sgs.CreateViewAsSkill{
	name="zhaoxin",
	n=0,
	view_as = function(self, cards)
		if(#cards~=0) then return nil end
		local ZXcard=zhaoxincard:clone()
			ZXcard:setSkillName(self:objectName())
		return ZXcard
	end,
	enabled_at_play=function(self, player)
        return not player:hasUsed("#zhaoxincard") and player:getHandcardNum() >= player:getHp()
	end,
}

huaiyi=sgs.CreateTriggerSkill{
	name="huaiyi",
	frequency = sgs.Skill_Frequent,
	events={sgs.HpChanged},
	priority = -1,
	on_trigger=function(self,event,player,data)
		if(player:getRoom():askForSkillInvoke(player,self:objectName()))then
            player:drawCards(1)
		end
	end,
}

yinsi=sgs.CreateViewAsSkill{
	name="yinsi",
	n=1,
	view_filter = function(self, selected, to_select)
		return to_select:inherits("EquipCard")
	end,
	view_as = function(self, cards)
		if #cards~=1 then return nil end
		local YScard=sgs.Sanguosha:cloneCard("analeptic",cards[1]:getSuit(),cards[1]:getNumber())
			YScard:addSubcard(cards[1])
			YScard:setSkillName(self:objectName())
		return YScard
	end,

	enabled_at_play=function(self, player)
		return not player:hasUsed("Analeptic")
	end,

	enabled_at_response=function(self,player,pattern)
		return pattern=="peach+analeptic"
	end,
}

chanxiancard=sgs.CreateSkillCard{
	name="chanxiancard",
	will_throw=false,
	filter = function(self, targets, to_select, player)
	    return #targets==0 and to_select ~= player
	end,
	on_effect=function(self,effect)
		effect.to:obtainCard(self)
		local room = effect.to:getRoom()
    	local choice = room:askForChoice(effect.to, "chanxian", "cxslash+cxhsals")
    	if(choice == "cxslash")then
        	local players = room:getOtherPlayers(effect.to)
			local targets=sgs.SPlayerList()
        	for _,player in sgs.qlist(players) do
            	if(effect.to:canSlash(player) and player:objectName() ~= effect.from:objectName()) then
                	targets:append(player)
				end
			end
        	if(not targets:isEmpty())then
            	local target = room:askForPlayerChosen(effect.from, targets, "chanxian")
            	local slash = room:askForCard(effect.to, "slash", "@chanxian:"..target:objectName(), QVariant())
            	if(slash) then
                	room:cardEffect(slash, effect.to, target)
				end
			end
    	else
        	choice = room:askForChoice(effect.from, "chanxian", "cxget+cxhit")
        	if(choice == "cxget")then
            	local card_id = room:askForCardChosen(effect.from, effect.to, "he", "chanxian")
            	effect.from:obtainCard(sgs.Sanguosha:getCard(card_id))
        	else
            	local damage=sgs.DamageStruct()
            		damage.from = effect.from
            		damage.to = effect.to
            	room:damage(damage)
			end
    	end
	end,
}

chanxian=sgs.CreateViewAsSkill{
	name="chanxian",
	n=1,

	view_filter = function(self, selected, to_select)
		return to_select:getSuit()==sgs.Card_Diamond
	end,

	view_as = function(self, cards)
		if #cards~=1 then return nil end
		local CXcard=chanxiancard:clone()
			CXcard:addSubcard(cards[1])
			CXcard:setSkillName(self:objectName())
		return CXcard
	end,

	enabled_at_play=function(self, player)
		return not player:hasUsed("#chanxiancard")
	end,
}

yanhe=sgs.CreateTriggerSkill{
	name="yanhe",
	frequency = sgs.Skill_NotFrequent,
	events={sgs.PhaseChange},

	on_trigger=function(self,event,player,data)
        local room = player:getRoom()
		if(player:getPhase() ~= sgs.Player_Start or not player:isWounded()) then return end
        local targets=sgs.SPlayerList()
       	for _,p in sgs.qlist(room:getOtherPlayers(player))do
            if(p:hasEquip()) then
            	targets:append(p)
			end
        end
        if(not targets:isEmpty())then
			if not room:askForSkillInvoke(player,self:objectName())then return end
			room:playSkillEffect(self:objectName())
			local card_id
            local target = room:askForPlayerChosen(player, targets, "yanhe")
            for i = 1,player:getLostHp(),1 do
                card_id = room:askForCardChosen(player, target, "e", "yanhe")
                target:obtainCard(sgs.Sanguosha:getCard(card_id))
                if(not target:hasEquip()) then
                    break
				end
			end
        end
        return false
	end,
}

youqi=sgs.CreateTriggerSkill{
	name="youqi",
	frequency=sgs.Skill_Wake,
	events={sgs.PhaseChange},
	priority = -1,

	on_trigger=function(self,event,player,data)
		local room = player:getRoom()
		if player:getPhase()~=sgs.Player_Start then return end
		if player:getMark("youqi")~=0 then return end
		if player:getHp()~=1 then return end
		local log=sgs.LogMessage()
        	log.type = "#YouqiWake"
        	log.from = player
        	log.arg = self:objectName()
        room:sendLog(log)

        room:playSkillEffect(self:objectName())
        room:getThread():delay(2000)

        room:loseMaxHp(player)

        room:acquireSkill(player, "dimeng")
        room:acquireSkill(player, "kongcheng")

        player:addMark("youqi")

        return false
	end,
}

quanjiancard=sgs.CreateSkillCard{
	name="quanjiancard",
	will_throw=false,
	filter = function(self, targets, to_select, player)
		return #targets==0 and to_select ~= player
	end,

	on_effect=function(self,effect)
		effect.to:obtainCard(self)
		local room = effect.to:getRoom()
    	local card = effect.to:getRandomHandCard()
    	room:showCard(effect.to,card:getEffectiveId())
    	if(card:inherits("Jink"))then
        	effect.from:drawCards(1)
        	effect.to:drawCards(1)
		end
	end,
}

quanjian=sgs.CreateViewAsSkill{
	name="quanjian",
	n=1,
	view_filter = function(self, selected, to_select)
		return to_select:inherits("Jink")
	end,

	view_as = function(self, cards)
		if #cards~=1 then return nil end
		local QJcard=quanjiancard:clone()
			QJcard:addSubcard(cards[1])
			QJcard:setSkillName(self:objectName())
		return QJcard
	end,

	enabled_at_play=function(self, player)
		return not player:hasUsed("#quanjiancard")
	end,
}

sijie=sgs.CreateTriggerSkill{
	name="sijie",
	frequency =sgs.Skill_NotFrequent,
	events={sgs.Damaged},

	on_trigger=function(self,event,player,data)
		local room=player:getRoom()
		local damage=data:toDamage()
		local targets,target,x
		for i=1,damage.damage,1 do
			if not room:askForSkillInvoke(player,self:objectName(),data) then break end
			targets=sgs.SPlayerList()
			for _,p in sgs.qlist(room:getAlivePlayers())do
            	if(not p:isNude()) then
            	targets:append(p)
				end
			end
			if targets:isEmpty() then break end
			target = room:askForPlayerChosen(player, targets, "sijie")
			if not target:isWounded() then
				x=1
			else
				x=target:getLostHp()
			end
			if x>target:getCardCount(true) then
				x=target:getCardCount(true)
			end
			room:askForDiscard(target,"sijie",x,false,true)
        end
		return false
	end,
}

diyliru:addSkill(duji)
diyliru:addSkill(diyshipo)
diypanfeng:addSkill(liefu)
diychengyu:addSkill(pengri)
diychengyu:addSkill(gangli)
diyjiangwan:addSkill(yaliang)
diyjiangwan:addSkill(xungui)
diyjiangwan:addSkill(daoju)
diyyuejin:addSkill(xiandeng)
diyyuejin:addSkill(xiandeng_buff)
diyyuejin:addSkill(xiaoguo)
diychendao:addSkill(jingrui)
diysimazhao:addSkill(zhaoxin)
diysimazhao:addSkill(huaiyi)
diysunluban:addSkill(yinsi)
diysunluban:addSkill(chanxian)
diyzhugejin:addSkill(yanhe)
diyzhugejin:addSkill(youqi)
diyjushou:addSkill(quanjian)
diyjushou:addSkill(sijie)

sgs.LoadTranslationTable{
	["zhimeng"] = "3D·织梦",

	["#diyliru"] = "魔王的智囊", -- qun,3HP
	["diyliru"] = "李儒",
	["designer:diyliru"] = "catcat44",
	["illustrator:diyliru"] = "一骑当千",
	["duji"] = "毒计",
	[":duji"] = "出牌阶段，若你的武将牌上没有牌，你可以将一张黑桃牌置于你的武将牌上。当一名其他角色在其出牌阶段使用一张【杀】指定目标后，你可将此牌置于其手上，并令此【杀】当有【酒】效果的【杀】结算，然后该角色须执行下列一项：将武将牌翻面或失去1点体力。",
	["dujicard"] = "毒计",
	["djturn"] = "将自己翻面",
	["djlp"] = "失去1点体力",
	["du"] = "毒计",
	["diyshipo"] = "识破",
	[":diyshipo"] = "锁定技，你不能成为黑桃【杀】或黑桃锦囊的目标。",

	["#diypanfeng"] = "无双上将", -- qun,4HP
	["diypanfeng"] = "潘凤",
	["designer:diypanfeng"] = "Why.",
	["illustrator:diypanfeng"] = "凌秋宏",
	["liefu"] = "烈斧",
	[":liefu"] = "当你使用的【杀】被目标角色的【闪】抵消时，你可以令此【杀】依然造成伤害，若如此做，你选择一项：弃置等同于目标角色已损失的体力值数量的牌，不足则全弃；令目标角色摸等同于其当前体力值数量的牌，最多为5张。",
	["#Liefu"] = "%from 发动了【%arg】技能，对 %to 强制命中",
	["lfpan"] = "自己弃牌",
	["lffeng"] = "对方摸牌",

	["#diychengyu"] = "世之奇士", -- wei,3HP
	["diychengyu"] = "程昱",
	["designer:diychengyu"] = "流云潇雪",
	["illustrator:diychengyu"] = "樱花闪乱",
	["pengri"] = "捧日",
	[":pengri"] = "出牌阶段，你可以获得一名其他角ruguo 色的一张牌，视为该角色对你使用一张【杀】。每阶段限一次。",
	["pengricard"] = "捧日",
	["gangli"] = "刚戾",
	[":gangli"] = "每当你受到其他角色造成的1点伤害后，你可以：选择除伤害来源外的另一名角色，视为伤害来源对该角色使用一张【杀】（此【杀】无距离限制且不计入出牌阶段使用次数限制）。",

	["#diyjiangwan"] = "安阳亭侯", -- shu,3HP
	["diyjiangwan"] = "蒋琬",
	["designer:diyjiangwan"] = "CoffeeNO加糖",
	["illustrator:diyjiangwan"] = "小仓",
	["yaliang"] = "雅量",
	[":yaliang"] = "当你成为其他角色所使用的非延时类锦囊的目标时，你可以摸一张牌，若如此做，该锦囊对你无效，且视为锦囊使用者对你使用了一张【杀】(该【杀】不计入回合使用限制)。",
	["#Yaliang"] = "受到 %from 的【%arg2】影响， %to 的锦囊【%arg】对其无效",
	["xungui"] = "循规",
	[":xungui"] = "出牌阶段，你可以将一张非延时类锦囊置于你的武将牌上，称为“规”。若存在“规”，则弃掉代替之，且你回复1点体力。每阶段限用一次。",
	["xunguicard"] = "循规",
	["daoju"] = "蹈矩",
	[":daoju"] = "出牌阶段，你可以将两张相同颜色手牌当“规”所示锦囊使用。每阶段限用一次。",
	["daojucard"] = "蹈矩",
	["gui"] = "规",

	["#diyyuejin"] = "胆识英烈", -- wei,4HP
	["diyyuejin"] = "乐进",
	["designer:diyyuejin"] = "R_Shanks",
	["illustrator:diyyuejin"] = "火神网",
	["xiandeng"] = "先登",
	[":xiandeng"] = "摸牌阶段，你可少摸一张牌，然后你无视一名其他角色的距离直到回合结束。",
	["xiaoguo"] = "骁果",
	[":xiaoguo"] = "出牌阶段，每当你使用非红桃【杀】被目标角色的【闪】抵消时，你可令该【闪】返回该角色手牌中，然后将此【杀】当一张延时类锦囊对该角色使用（黑色当【兵粮寸断】，方块当【乐不思蜀】）。",

	["#diychendao"] = "猛将之烈", -- shu,4HP
	["diychendao"] = "陈到",
	["designer:diychendao"] = "游神ViVA",
	["illustrator:diychendao"] = "楚汉风云",
	["jingrui"] = "精锐",
	[":jingrui"] = "若你的手牌数不小于你的体力值，你可以将一张手牌当做【杀】或【闪】使用或打出。",

	["#diysimazhao"] = "权倾谋朝", -- wei,3HP
	["diysimazhao"] = "司马昭",
	["designer:diysimazhao"] = "殇の腥",
	["illustrator:diysimazhao"] = "火神网",
	["zhaoxin"] = "昭心",
	[":zhaoxin"] = "出牌阶段，若你的手牌数不小于你的体力值，你可以展示你全部手牌。若其均为不同花色，你令一名角色失去1点体力。若其均为同一种花色，你获得一名其他角色一张牌。每阶段限一次。",
	["zhaoxincard"] = "昭心",
	["huaiyi"] = "怀异",
	[":huaiyi"] = "每当你体力值发生一次变化后，你可以摸一张牌。",

	["#diysunluban"] = "矫矜的毒刺", -- wu,3HP
	["diysunluban"] = "孙鲁班",
	["designer:diysunluban"] = "小掉线仙",
	["illustrator:diysunluban"] = "阎魔爱",
	["yinsi"] = "淫肆",
	[":yinsi"] = "你可以将一张装备牌当【酒】使用。",
	["chanxian"] = "谗陷",
	[":chanxian"] = "出牌阶段，你可以将一张方片牌交给一名其他角色，该角色进行二选一：1、对其攻击范围内的另一名由你指定的角色使用一张【杀】。2.令你选择获得其一张牌或对其造成一点伤害。每阶段限一次。 ",
	["chanxiancard"] = "谗陷",
	["cxslash"] = "出杀",
	["cxhsals"] = "摊手",
	["cxget"] = "获得其一张牌",
	["cxhit"] = "对其造成一点伤害",
	["@chanxian"] = "受到【谗陷】影响，你可以对 %src 使用一张【杀】",

	["#diyzhugejin"] = "温厚的盟使", -- wu,4HP
	["diyzhugejin"] = "诸葛瑾",
	["designer:diyzhugejin"] = "catcat44",
	["illustrator:diyzhugejin"] = "战国风云",
	["yanhe"] = "言和",
	[":yanhe"] = "回合开始阶段开始时，若你已受伤，你可令一名其他角色装备区里的至多X张牌回到手牌（X为你已损失的体力值）。",
	["youqi"] = "忧戚",
	[":youqi"] = "觉醒技，回合开始阶段结束时，若你的体力为1，你须减1点体力上限，并永久获得技能“缔盟”和“空城”。 ",
	["#YouqiWake"] = "%from 的觉醒技【%arg】被触发",

	["#diyjushou"] = "忠贞义烈", -- qun,3HP
	["diyjushou"] = "沮授",
	["designer:diyjushou"] = "神·冥狐",
	["illustrator:diyjushou"] = "一骑当千",
	["quanjian"] = "劝谏",
	[":quanjian"] = "出牌阶段，你可以交给一名其他角色一张【闪】，展示其一张手牌：若为【闪】，则你与该角色各摸一张牌。每阶段限一次。",
	["quanjiancard"] = "劝谏",
	["sijie"] = "死节",
	[":sijie"] = "每当你受到1点伤害后，可使一名角色弃置X张牌（X为该角色已损失的体力值，且至少为1）。 ",
}