module("extensions.draggon",package.seeall)
extension=sgs.Package("draggon")
dofile "extensions/ai/funs.lua"

yzdisnum={3};

--ADD Generals--

splujunyi=sgs.General(extension,"splujunyi$","min",4)
splinchong=sgs.General(extension,"splinchong$","kou",4)
spluzhishen=sgs.General(extension,"spluzhishen$","jiang",4)
spwusong=sgs.General(extension,"spwusong","min",4)
spzhulei=sgs.General(extension,"spzhulei","kou",4)
spzhuwu=sgs.General(extension,"spzhuwu","min",3)
sphuyanzhuo=sgs.General(extension,"sphuyanzhuo","guan",4)
spbird=sgs.General(extension,"spbird","god",4)
spyuefei=sgs.General(extension,"spyuefei","god",4)
sphuangxin=sgs.General(extension,"sphuangxin","guan",4)
---------------
luajueding=sgs.CreateTriggerSkill{
	name="luajueding",
	events=sgs.PhaseChange,
	frequency=sgs.Skill_Compulsory,
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()
		if player:getPhase()==sgs.Player_Discard then
			local x=player:getMaxHp()*2-player:getHp()
			local y=player:getHandcardNum()
			if y>player:getHp() then SkillLog(player,self:objectName(),0,0) end
			if y>x then room:askForDiscard(player,"luajueding",y-x,y-x,false,false) end
			return true
		end
	end,
}

luajinshen=sgs.CreateTriggerSkill{
	name="luajinshen",
	events=sgs.CardEffected,
	frequency=sgs.Skill_Compulsory,
	on_trigger=function(self,event,player,data)
		local card=data:toCardEffect().card--记录使用的卡
		if player:getArmor() then return false end
		if not (card:inherits("Slash") and card:isRed()) then return false end--卡片种类不符则不发动
		SkillLog(player,self:objectName(),0,0)
		return true--卡片无效
	end
}

luayijiucard=sgs.CreateSkillCard{--技能卡
	name="luayijiucard",
	target_fixed=true,--应该是不用选对象
	on_use=function(self,room,source,targets)
		for _,p in sgs.qlist(room:getOtherPlayers(source)) do--循环，逐一检视其它角色
			if p:isAlive() and not p:isNude() and p:getKingdom()=="jiang" then--未死，有牌
				local card=room:askForCard(p,".|spade","askforluayijiu")--请求弃牌
				if card then
					local card=sgs.Sanguosha:cloneCard("analeptic",sgs.Card_NoSuit,0)
					card:setSkillName(self:objectName())
					local use=sgs.CardUseStruct()
					use.card=card
					use.from=source
					room:useCard(use)
					return
				end
			end
		end
	end,
}
luayijiu=sgs.CreateViewAsSkill{
	name="luayijiu$",
	n=0,
	view_as=function(self,cards)
		local acard=luayijiucard:clone()
		return acard
	end,
	enabled_at_play=function()
		return not sgs.Self:hasUsed("Analeptic")
	end,
	enabled_at_response=function(self,player,pattern)
		return string.find(pattern,"analeptic")
	end
}


spluzhishen:addSkill(luajueding)
spluzhishen:addSkill(luajinshen)
spluzhishen:addSkill(luayijiu)

luaweiyan=sgs.CreateTriggerSkill{
	name="luaweiyan",
	events={sgs.SlashProceed,sgs.Predamage},
	frequency=sgs.Skill_Compulsory,
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()--获取房间
		if player:getMark("@luayusui")==1 then return false end
		if event==sgs.SlashProceed then
			local effect=data:toSlashEffect()
			local to=effect.to
			local from=effect.from
			if to:getHp()<=from:getHp() and to:getHandcardNum()<=from:getHandcardNum() then
				SkillLog(player,self:objectName(),0,0)
				room:slashResult(effect,nil)--结算杀中了，然后无效原来的
				return true
			end
		end
		if event==sgs.Predamage then
			local damage=data:toDamage()
			local to=damage.to
			local from=damage.from
			local reason=damage.card
			if not reason then return false end
			if reason:inherits("Slash") then
				if to:getHp()>=from:getHp() and to:getHandcardNum()>=from:getHandcardNum() then
					SkillLog(player,self:objectName(),0,0)
					damage.damage=damage.damage+1
					data:setValue(damage)
					return false
				end
			end
			return false
		end
	end,
}

luayusui=sgs.CreateTriggerSkill{
	name="luayusui",
	events=sgs.PhaseChange,--阶段改变时发动
	frequency=sgs.Skill_Limited,
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()--获取房间
		if player:getMark("@luayusui")==1 then return false end
		if player:getPhase()==sgs.Player_Finish then--出牌阶段
			if room:askForSkillInvoke(player,"luayusui") then--确认发动
				player:gainMark("@luayusui")
				player:turnOver()
				if player:isWounded() then
					local recover=sgs.RecoverStruct()
					recover.recover=1
					recover.who=player
					room:recover(player,recover)--回复
				end
				room:acquireSkill(player,"baoguo")
				room:detachSkillFromPlayer(player,"luaweiyan")
			end
			return false--不跳过阶段
		end
	end,
}

luamingwang=sgs.CreateTriggerSkill{
	name="#luamingwang$",
	events=sgs.PhaseChange,
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()
		local lord=room:getLord()
		if lord:objectName()==player:objectName() then return false end
		if not lord:hasSkill(self:objectName()) then return false end
		if player:getPhase()==sgs.Player_Start then--开始阶段
			if player:getKingdom()=="min" and player:getHp()==1 then
				if room:askForSkillInvoke(player,"luamingwangvs") then
					room:loseMaxHp(player)
					room:setPlayerProperty(lord,"maxhp",sgs.QVariant(lord:getMaxHp()+1))
				end
			end
		end
	end,
	can_trigger=function(self,player)
		return player and player:isAlive()
	end,
}
luamingwangcard=sgs.CreateSkillCard{--技能卡
	name="luamingwangcard",
	target_fixed=true,--应该是不用选对象
	on_use=function(self,room,source,targets)
		local lord=source
		for _,theplayer in sgs.qlist(room:getOtherPlayers(lord)) do--循环，逐一检视所有角色
			if theplayer:getKingdom()=="min" and theplayer:getHp()==1 then
				if room:askForSkillInvoke(theplayer,"luamingwangvs") then
					room:loseMaxHp(theplayer)
					room:setPlayerProperty(lord,"maxhp",sgs.QVariant(lord:getMaxHp()+1))
				end
			end
		end
	end,
}
luamingwangvs=sgs.CreateViewAsSkill{
	name="luamingwangvs$",
	n=0,
	view_as=function(self,cards)
		local acard=luamingwangcard:clone()
		return acard
	end,
}

luabaoguo=sgs.CreateTriggerSkill{--触发技
	name="luabaoguo",
	events={sgs.Damaged,sgs.Predamaged},--受到伤害时
	on_trigger=function(self,event,player,data)--要执行的动作
		local room=player:getRoom()--获取房间
		if event==sgs.Damaged then
			if player:hasSkill(self:objectName()) then
				SkillLog(player,self:objectName(),0,0)
				player:drawCards(player:getLostHp())
			end
			return false
		end
		if event==sgs.Predamaged then
			local pattern=".a,BasicCard"--基本牌，锦囊牌，对应花色，加上.a可以让系统不播放牌的音效
	--		player:speak("111")
			for _,theplayer in sgs.qlist(room:getOtherPlayers(player)) do--循环，逐一检视所有角色
				if theplayer:hasSkill("luabaoguo") then--有这个技能，不是使用中（否则两张杀换来换去死循环）
	--		theplayer:speak("111")
					local card=room:askForCard(theplayer,pattern,"askforluabaoguo",data)--请求弃牌
					if card then
						SkillLog(theplayer,self:objectName(),2,0)
						local damage=data:toDamage()
						damage.to=theplayer
						room:damage(damage)
						return true
					end
				end
			end
		end
	end,
	can_trigger=function(self,player)
		return player and player:isAlive()--重载cantrigger，不是自己也能用（相当于略去hasskill）
	end,
}


splujunyi:addSkill(luaweiyan)
splujunyi:addSkill(luayusui)
splujunyi:addSkill(luamingwang)
splujunyi:addSkill(luamingwangvs)
--splujunyi:addSkill(luabaoguo)
local skill=sgs.Sanguosha:getSkill("luabaoguo")
if not skill then
	local skillList=sgs.SkillList()
	skillList:append(luabaoguo)
	sgs.Sanguosha:addSkills(skillList)
end

luashenqiangcard=sgs.CreateSkillCard{
	name="luashenqiangcard",
	filter=function(self,targets,to_select,player)
		if #targets==0 then--未选定借刀杀人的第一个对象
			local pl=sgs.PlayerList()
			local card=sgs.Sanguosha:cloneCard("slash",sgs.Card_NoSuit,0)
			return card:targetFilter(pl,to_select,player)
		else
			return false
		end
	end,
	on_use=function(self,room,source,targets)
		room:throwCard(self)
		room:setPlayerFlag(source,"luashenqiangUsed")
		if #targets<1 then return false end--未选够两个人
		local victim=targets[1]--被杀的人
		local card=sgs.Sanguosha:cloneCard("slash",sgs.Card_NoSuit,0)
		card:setSkillName(self:objectName())
		local use=sgs.CardUseStruct()
		use.card=card
		use.from=source
		use.to:append(victim)
		room:useCard(use,false)
	end,
}
luashenqiang=sgs.CreateViewAsSkill{
	name="luashenqiang",
	n=2,
	view_filter=function(self,selected,to_select)
		return to_select:inherits("BasicCard")--要求是杀
	end,
	view_as=function(self,cards)
		if #cards==2 then--选了一张牌
			local acard=luashenqiangcard:clone()--复制技能卡
			acard:addSubcard(cards[1]:getId())
			acard:addSubcard(cards[2]:getId())
			acard:setSkillName(self:objectName())
			return acard
		end
	end,
	enabled_at_play=function()
		return not sgs.Self:hasFlag("luashenqiangUsed")
	end,
}
luashenqiangTR=sgs.CreateTriggerSkill{
	name="#luashenqiangTR",
	events=sgs.Predamage,
	on_trigger=function(self,event,player,data)
		local damage=data:toDamage()
		local room=player:getRoom()
		local reason=damage.card
		if not reason then return false end
		if reason:inherits("Slash") and reason:getSkillName()=="luashenqiangcard" then
			SkillLog(player,"luashenqiang",0,0)
			damage.damage=damage.damage+1
			data:setValue(damage)
			return false
		end
	end
}

luahuopin=sgs.CreateTriggerSkill{
	name="luahuopin$",
	events=sgs.PhaseChange,--阶段改变时发动
	frequency=sgs.Skill_Compulsory,
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()--获取房间
		if player:getPhase()==sgs.Player_Start then--出牌阶段
			if player:getMark("@luahuopin")==0 then return false end
			SkillLog(player,self:objectName(),0,0)
			local bool=room:askForDiscard(player,"luahuopin",3,true,true)
			if bool then
				if player:isWounded() then
					local recover=sgs.RecoverStruct()
					recover.recover=1
					recover.who=player
					room:recover(player,recover)--回复
				end
			else
				room:loseHp(player)
				player:drawCards(3)
			end
			return false--不跳过阶段
		end
	end,
}
luahuopinTR=sgs.CreateTriggerSkill{
	name="#luahuopinTR",
	events=sgs.Death,--阶段改变时发动
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()--获取房间
		local lord=room:getLord()
		if not lord:hasSkill(self:objectName()) or lord:getMark("@luahuopin")==1 then return false end
		lord:gainMark("@luahuopin")
	end,
	can_trigger=function(self,player)
		return player and player:getKingdom()=="kou"
	end,
}


splinchong:addSkill(luashenqiang)
splinchong:addSkill(luashenqiangTR)
splinchong:addSkill(luahuopin)
splinchong:addSkill(luahuopinTR)

luashenchou=sgs.CreateTriggerSkill{--奸雄 by hypercross
	name="luashenchou",
	events=sgs.Damaged,
	frequency=sgs.Skill_Compulsory,--锁定技，
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()
		local card=data:toDamage().card
		SkillLog(player,self:objectName(),0,0)
		if card then room:obtainCard(player,card) end
		local card2=room:askForCard(player,".|.|.|.","~luashenchou")
		room:playSkillEffect("luashenchou",math.random(1,2))
		if card2 then player:addToPile("luashenchou",card2) end
	end
}
luaduanbi=sgs.CreateTriggerSkill{
	name="luaduanbi",
	events=sgs.PhaseChange,
	frequency=sgs.Skill_Limited,
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()
		if player:getPhase()~=sgs.Player_Start or player:getHp()~=1 or player:getMark("luaduanbi")==1 then return false end
		if not room:askForSkillInvoke(player,"luaduanbi",data) then return false end
		room:setPlayerMark(player,"luaduanbi",1)
		room:loseMaxHp(player,player:getMaxHp()-1)
		local pls=room:getOtherPlayers(player)
		local target=room:askForPlayerChosen(player,pls,"luaduanbi")
		if room:askForChoice(player,"luaduanbi","recover+damage")=="recover" then
			room:playSkillEffect("luaduanbi")
			local recover=sgs.RecoverStruct()
			recover.recover=2
			recover.who=player
			room:recover(target,recover)--回复
		else
			room:playSkillEffect("luaduanbi")
			local damage = sgs.DamageStruct()
			damage.from = player
			damage.to = target
			damage.damage=2
			room:damage(damage)
		end
	end,
}
luawujie=sgs.CreateTriggerSkill{
	name="luawujie",
	events=sgs.PhaseChange,
	frequency=sgs.Skill_Wake,
	priority=3,
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()
		if player:getPhase()~=sgs.Player_Start or player:getPile("luashenchou"):length()<3 or player:getMark("luazhusha")==1 then return false end
		room:setPlayerMark(player,"luazhusha",1)
		SkillLog(player,self:objectName(),0,0)
		room:playSkillEffect("luawujie")
		room:loseMaxHp(player)
		room:acquireSkill(player,"luazhusha")
	end,
}
luazhusha_card=sgs.CreateSkillCard{--技能卡
	name="luazhusha_card",
	filter=function(self,targets,to_select,player)
		if #targets>1 then return false end
		local slash=CreateCard("slash")
		local pl=sgs.PlayerList()
		return player:canSlash(to_select,false)--slash:targetFilter(pl,to_select,player)
	end,
	on_use=function(self,room,source,targets)
		local parts=source:getPile("luashenchou")--获取圣谕牌
 		room:fillAG(parts,source)--填充ag界面
		local cdid=room:askForAG(source,parts,false,"luazhusha")
		local cd=sgs.Sanguosha:getCard(cdid)
		if cd:inherits("Weapon") or cd:inherits("EventsCard") then room:setPlayerFlag(source,"luazhusha") end
		room:throwCard(cdid,source)--弃掉
		source:invoke("clearAG")
		room:playSkillEffect("luazhusha",math.random(1,2))
		local use=sgs.CardUseStruct()--卡牌使用结构体
		use.card=CreateCard("slash",nil,"luazhusha")
		use.from=source
		local targetnum=#targets
		for var=1,targetnum,1 do
			use.to:append(targets[var])
		end
		room:useCard(use,false)
		room:setPlayerFlag(source,"-luazhusha")
	end,
}
luazhusha_vs=sgs.CreateViewAsSkill{
	name="luazhusha_vs",
	view_as=function(self,cards)
		return CreateCard(luazhusha_card)
	end,
	enabled_at_play=function(self,player)
	--	local slash=CreateCard("slash")
		return --[[slash:isAvailable(sgs.Self) and]] sgs.Self:getPile("luashenchou"):length()>0--牌堆有牌
	end,
}
luazhusha=sgs.CreateTriggerSkill{
	name="luazhusha",
	events=sgs.Predamage,
	view_as_skill=luazhusha_vs,
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()--获取房间
		local damage=data:toDamage()
		local from=damage.from
		if from:hasFlag("luazhusha") then
			SkillLog(player,self:objectName(),0,0)
			damage.damage=damage.damage+1
			data:setValue(damage)
			return false
		end
	end,
}

spwusong:addSkill(luawujie)
spwusong:addSkill(luaduanbi)
TempSkill(luazhusha)
spwusong:addSkill(luashenchou)

sgs.LoadTranslationTable{
	["draggon"]="群龙逆天",

	["spluzhishen"]="鲁智深",
	["#spluzhishen"]="聚义逆天",
	["luajueding"]="绝顶",
	[":luajueding"]="锁定技,你的手牌上限等于你的体力上限加上你的已损失体力值.",
	["luajinshen"]="金身",
	[":luajinshen"]="锁定技,若你没有装备防具,红色【杀】对你无效",
	["luayijiu"]="义酒",
	["luayijiucard"]="义酒",
	[":luayijiu"]="主公技,当你需要一张【酒】时,其他将势力角色弃置一张黑桃牌，若如此做,视为你使用一张【酒】",
	["askforluayijiu"]="你可以弃置一张黑桃牌，若如此做,视为主公使用一张【酒】",

	["splujunyi"]="卢俊义",
	["#splujunyi"]="无双无对",
	["luaweiyan"]="威严",
	[":luaweiyan"]="锁定技,你使用【杀】指定的目标的当前体力值和手牌数均不大于你的,则该【杀】不可被【闪】响应;若目标角色的当前体力值和手牌数均不少于你的.则该【杀】造成的伤害+1.",
	["luayusui"]="玉碎",
	["@luayusui"]="玉碎",
	[":luayusui"]="限定技,回合结束阶段,你可以将你的武将牌翻面并且回复一点体力,然后你失去技能【威严】,获得技能【报国】",
	["luamingwangvs"]="名望",
	["luamingwangcard"]="名望",
	[":luamingwangvs"]="主公技,其他体力为1的民势力角色在其回合开始阶段可减1点体力上限，然后令你加1点体力上限.",
	["luabaoguo"]="报国",
	["askforluabaoguo"]="你可以弃置一张基本牌，将该伤害转移给你",
	[":luabaoguo"]="当其他角色受到伤害时，你可以弃置一张基本牌，将该伤害转移给你；你每受到一次伤害，可以摸X张牌（X为你已损失的体力值）。",

	["splinchong"]="林冲",
	["#splinchong"]="火拼夺位",
	["luashenqiang"]="神枪",
	["luashenqiangcard"]="神枪",
	[":luashenqiang"]="限制技,出牌阶段,你可以弃掉两张基本牌,若如此做,则视为你对你攻击范围内的任意一名角色使用了一张【杀】.该【杀】造成的伤害+1.",
	["luahuopin"]="火拼",
	["@luahuopin"]="火拼",
	[":luahuopin"]="主公技.锁定技,回合开始阶段,若场上存在已死亡的寇势力角色,你须执行下列两项中的一项：1.摸取3张牌，然后失去1点体力；2.弃置3张牌，然后回复1点体力。",
	["spwusong"]="武松",
	["#spwusong"]="杀人者打虎",
	["luashenchou"]="深仇",
	[":luashenchou"]="你始终获得对你造成伤害的牌;你每受到一次伤害你可以将一张牌置于你的武将牌上称为“仇”。",
	["~luashenchou"]="你始终获得对你造成伤害的牌;你每受到一次伤害你可以将一张牌置于你的武将牌上称为“仇”。",
	["luawujie"]="无戒",
	[":luawujie"]="觉醒技,回合开始阶段,你的“仇”为3张或更多时,你须减少一点体力上限,然后获得技能【诛杀】(出牌阶段,你可弃置一张“仇”视为对至多两名角色使用一张【杀】,额外的,“仇”为武器牌或事件牌,此【杀】造成伤害+1)",
	["luazhusha"]="诛杀",
	recover="回复2",
	damage="伤害2",
	["luazhusha_card"]="诛杀",
	[":luazhusha"]="出牌阶段,你可弃置一张“仇”视为对至多两名角色使用一张【杀】,额外的,“仇”为武器牌或事件牌,此【杀】造成伤害+1",
	["luaduanbi"]="断臂",
	[":luaduanbi"]="限定技,回合开始阶段,若你的体力为1,你可以将体力上限减至1,然后让任一角色回复或失去2点体力.",

}

luaqiankun=sgs.CreateProhibitSkill{
	name="luaqiankun",
	is_prohibited=function(self,from,to,card)
		return to:hasSkill(self:objectName()) and from:getHp()>to:getHp() and (card:inherits("Duel") or card:inherits("Slash"))--有喝酒标记的人的杀无效
	end,
}

function getLeftAlive(player)
	local i=player
	while i:getNextAlive():objectName()~=player:objectName() do
		i=i:getNextAlive()
	end
	return i
end
luanizhuan=sgs.CreateTriggerSkill{
	name="luanizhuan",
	events={sgs.AskForRetrial,sgs.PhaseChange},
	frequency=sgs.Skill_Limited,
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()
		local source=player
		if player:getPhase()~=sgs.Player_Finish then return false end
		if player:getMark("@luanizhuan")==1 then return false end
		if room:askForCard(player,".a,DelayedTrick","askforluanizhuan") then
			source:gainMark("@luanizhuan",1)
			local right=source:getNextAlive()
			local left=getLeftAlive(source)
			while right:objectName()~=left:objectName() do
				left:speak("left")
				right:speak("right")
				room:swapSeat(right,left)
				local t=left
				left=right
				right=t
				if right:getNextAlive():objectName()==left:objectName() then break end
				right=right:getNextAlive()
				left=getLeftAlive(left)
			end
		end
		return false
	end,
}
--[[luanizhuan=sgs.CreateViewAsSkill{
	name="luanizhuan",
	n=1,
	view_filter=function(self,selected,to_select)
		return to_select:inherits("DelayedTrick")--要求是杀
	end,
	view_as=function(self,cards)
		if #cards==0 then return nil end
		acard=luanizhuanCard:clone()--复制一张卡的效果
		acard:addSubcard(cards[1])
		return acard--返回一张新卡
	end,
	enabled_at_play=function()
		return sgs.Self:getMark("@luanizhuan")==0
	end,
}]]

luaqianlv=sgs.CreateTriggerSkill{
	name="luaqianlv",
	events={sgs.AskForRetrial,sgs.PhaseChange},
	on_trigger=function(self,event,player,data)
		local room=player:getRoom()
		if event==sgs.AskForRetrial then
			local judge=data:toJudge()
			if judge.who:objectName()~=player:objectName() then return false end
			if not room:askForSkillInvoke(player,self:objectName(),data) then return false end
			local id=room:getNCards(1):at(0)
			local card=sgs.Sanguosha:getCard(id)
			room:throwCard(judge.card)
			judge.card=card
			room:throwCard(card)
			local log=sgs.LogMessage()
			log.type="$ChangedJudge"
			log.from=player
			log.to:append(judge.who)
			log.card_str=card:getEffectIdString()
			room:sendLog(log)
			room:sendJudgeResult(judge)
			return false
		end
		if player:getPhase()~=sgs.Player_Draw then return false end
		if not room:askForSkillInvoke(player,self:objectName()) then return false end
		local idlist=room:getNCards(2)
		room:askForGuanxing(player,idlist,false)
		local newlist=sgs.IntList()
		local pile=room:getDrawPile()
		local n=pile:length()
		newlist:append(pile:at(0))
		newlist:append(pile:at(1))
	--	local newlist=room:getNCards(2)
		local uplist=sgs.IntList()
		for _,id in sgs.qlist(idlist) do
			local up=false
			for _,id2 in sgs.qlist(newlist) do
				if id==id2 then up=true end
			end
			if not up then
				room:throwCard(id)
			else
				uplist:append(id)
			end
		end
--		if uplist:length()>0 then room:askForGuanxing(player,uplist,true) end
	end,
}

spzhuwu:addSkill(luaqianlv)
spzhuwu:addSkill(luaqiankun)
spzhuwu:addSkill(luanizhuan)
sgs.LoadTranslationTable{
	["spzhuwu"]="朱武",
	["#spzhuwu"]="千虑之师",
	["luaqianlv"]="千虑",
	[":luaqianlv"]="在你的判定牌生效前，你可以亮出牌堆顶的一张牌代替之；摸牌阶段开始时，你可以观看牌堆顶的两张牌并可以将其置入弃牌堆。",
	["luaqiankun"]="乾坤",
	[":luaqiankun"]="锁定技，你不能成为体力比你多的角色使用【杀】和【决斗】的目标。",
	["luanizhuan"]="逆转",
	["@luanizhuan"]="逆转",
	["askforluanizhuan"]="回合结束阶段开始时，你可以弃置一张延时类锦囊牌，若如此做，所有角色行动和响应群体性锦囊的方向均反转。",
	[":luanizhuan"]="限定技，回合结束阶段开始时，你可以弃置一张延时类锦囊牌，若如此做，所有角色行动和响应群体性锦囊的方向均反转。",
}

luachongfeng=sgs.CreateViewAsSkill{--基本就是“武圣”，没啥必要注释了吧。。。
	name="luachongfeng",
	n=1,
	view_filter=function(self,selected,to_select)
		return to_select:inherits("DefensiveHorse") or to_select:inherits("OffensiveHorse")
	end,
	view_as=function(self,cards)
		if #cards==1 then return CreateCard("iron_chain",cards,"luachongfeng") end
	end,
}
luamahuan=sgs.CreateDistanceSkill{
	name="luamahuan",
	correct_func=function(self,from,to)
		local t=0
		if from:isChained() then t=t+1 end
		for _,ap in sgs.qlist(from:getSiblings()) do
			if ap:isChained() then t=t+1 end
		end
		if from:hasSkill(self:objectName()) then return 0-t end
		if to:hasSkill(self:objectName()) then return 0+t end
		return 0
	end,
}


sphuyanzhuo:addSkill(luachongfeng)
sphuyanzhuo:addSkill(luamahuan)
sgs.LoadTranslationTable{
	["sphuyanzhuo"]="呼延灼",
	["#sphuyanzhuo"]="来去如风",
	["luachongfeng"]="马环",
	[":luachongfeng"]="出牌阶段,你可以将你的坐骑牌当成【铁索连环】使用或重铸",
	["luamahuan"]="冲锋",
	[":luamahuan"]="锁定技,你与其他角色计算距离-X,其他角色计算与你计算距离+X,X为场上横置角色数.",
}

--Skills--
luasaodangcard=sgs.CreateSkillCard{
name="luasaodangcard",
filter=function(self,targets,to_select,player)
if (#targets>=3) then return false end
if to_select:hasSkill("luasaodang") then return false end
return true
end,
on_effect=function(self,effect)
	local room=effect.from:getRoom()
	local judge=sgs.JudgeStruct()
    judge.pattern=sgs.QRegExp("(.*):(heart|diamond):(.*)")
    judge.good=true
    judge.reason=self:objectName()
    judge.who=effect.to
    room:judge(judge)
	if judge:isGood() then
		if effect.to:getHandcardNum()<2 or not room:askForDiscard(effect.to, self:objectName(),2,2,true,true) then
			local damage=sgs.DamageStruct()
			damage.damage=1
			damage.from=effect.from
			damage.to=effect.to
			room:damage(damage)
		end
	end
end
}
luasaodangvs=sgs.CreateViewAsSkill{
name="luasaodangvs",
n=0,
view_filter=function(self, selected, to_select)
	return false
end,
view_as=function(self, cards)
	if #cards==0 then
	local acard=luasaodangcard:clone()
	acard:setSkillName("luasaodangcard")
	return acard end
end,
enabled_at_play=function()
	return false
end,
enabled_at_response=function(self,player,pattern)
	return pattern == "@@luasaodangcard"
end,
}

luasaodang=sgs.CreateTriggerSkill{
name="luasaodang",
frequency=sgs.Skill_NotFrequent,
view_as_skill=luasaodangvs,
events={sgs.Damage},
on_trigger=function(self,event,player,data)
		if not player:getPhase()==sgs.Player_play then return end
        local room=player:getRoom()
		local damage=data:toDamage()
        local card = damage.card
		if not card then return end
		--if not player:getPhase()==sgs.Player_play then return end
		if not card:inherits("Slash") then return end
        if not room:askForSkillInvoke(player,self:objectName(),data) then return end
		room:loseHp(player)
        room:askForUseCard(player, "@@luasaodangcard", "@luasaodang")
end
}
--ADD Skill--
sphuangxin:addSkill(luasaodang)
--LANG--
sgs.LoadTranslationTable{
	["sphuangxin"]="黄信",
	["#sphuangxin"]="三山之岳",
	["luasaodangvs"]="扫荡",
	["luasaodangcard"]="扫荡",
	["luasaodang"]="扫荡",
	[":luasaodang"]="出牌阶段,你的【杀】造成伤害后，你可以自减1点体力，然后指定最多3名角色进行一次判定，\
	若判定结果为红色,该角色须弃两张牌或受到1点伤害。",
	["@luasaodang"]="请指定1-3名其它角色",
	["~luasaodang"]="除你以外",
	["@luasaodangdiscard"]="请弃2张牌 否则将受到1点伤害",
}

luajinshucard=sgs.CreateSkillCard{
name="luajinshucard",
target_fixed=true,
will_throw=true,
on_use=function(self,room,source,targets)
	local x=self:getSubcards():length()
	for i=1,x,1 do
		local card_id = room:drawCard()
		local card=sgs.Sanguosha:getCard(card_id)
		if source:addToPile("spxi",card) then
			local log=sgs.LogMessage()
			log.from =source
			log.type ="#luajinshu"
			log.arg  =card:objectName()
			log.arg2 =card:getSuitString()
			room:sendLog(log)
		end
	end
	room:throwCard(self)
end
}

luajinshuvs=sgs.CreateViewAsSkill{
name="luajinshuvs",
n=999,
view_filter=function(self, selected, to_select)
	return not to_select:isEquipped()
end,
view_as=function(self, cards)
	if #cards==0 then return end
	local acard=luajinshucard:clone()
	for var=1,#cards,1 do
        acard:addSubcard(cards[var])
    end
	acard:setSkillName("luajinshucard")
	--assert(acard)
	return acard
end,
enabled_at_play=function()
	return false
end,
enabled_at_response=function(self,player,pattern)
	return pattern == "@@luajinshucard"
end,
}

luajinshu=sgs.CreateTriggerSkill{
name="luajinshu",
events={sgs.Predamage,sgs.Predamaged,sgs.Damage,sgs.Damaged,sgs.PhaseChange},
priority=3,
view_as_skill=luajinshuvs,
can_trigger=function(self,player)
	return true
end,
frequency=sgs.Skill_NotFrequent,
on_trigger=function(self,event,player,data)
	local room=player:getRoom()
	local owner=room:findPlayerBySkillName(self:objectName())
	if not owner:isAlive() then return end
	local baoguo=sgs.Sanguosha:getTriggerSkill("baoguo")
	local duijue=sgs.Sanguosha:getTriggerSkill("duijue")
	local dujian=sgs.Sanguosha:getTriggerSkill("dujian")
if event==sgs.PhaseChange and player:getPhase()==sgs.Player_Discard then
	if player:objectName()~=owner:objectName() then return end
	--if (room:askForSkillInvoke(owner,self:objectName(),data)~=true) then return end
	room:askForUseCard(player, "@@luajinshucard", "@luajinshu")
elseif event==sgs.Predamaged then
	if owner:getPile("spxi"):length()==0 then return end
	local damage=data:toDamage()
	if damage.to:objectName()==owner:objectName() then return end
		--if owner:getPile("spxi"):length()==0 then return end
		if (room:askForSkillInvoke(owner,"luajinshubaoguo2",data)~=true) then return end
		--room:throwCard(owner:getPile("spxi"):first())
		room:playSkillEffect("baoguo",math.random(1,2))
		local cd = room:askForCard(owner, "BasicCard", "@luajinshubaoguo2",data)
		if cd then
			local tmp=sgs.DamageStruct()
			tmp.damage=damage.damage
			tmp.from=damage.from
			tmp.to=owner
			room:damage(tmp)
			return true
		end
elseif event==sgs.Predamage then
	if player:objectName()==owner:objectName() then
		local damage=data:toDamage()
		if damage.to:canSlash(damage.from) then return end
		if owner:getPile("spxi"):length()==0 then return end
		if (room:askForSkillInvoke(owner,"luajinshudujian",data)~=true) then return end
		room:throwCard(owner:getPile("spxi"):first())
		room:playSkillEffect("dujian",math.random(1,2))
		damage.to:turnOver()
		return true
	end
elseif event==sgs.Damaged or sgs.Damage then
	if owner:getPile("spxi"):length()==0 then return end
	local damage=data:toDamage()
	if event==sgs.Damaged and player:objectName()==owner:objectName() then
		if owner:getPile("spxi"):length()==0 then return end
		if (room:askForSkillInvoke(owner,"luajinshubaoguo",data)~=true) then return end
		room:throwCard(owner:getPile("spxi"):first())
		room:playSkillEffect("baoguo",math.random(1,2))
		player:drawCards(player:getLostHp())
	end
	if not damage.card then return end
	if event==sgs.Damaged and damage.card:inherits("Slash") then
		if not damage.to:hasSkill(self:objectName()) then return end
		if owner:getPile("spxi"):length()==0 then  return end
		if (room:askForSkillInvoke(owner,"luajinshuduijue",data)~=true) then return end
		room:throwCard(owner:getPile("spxi"):first())
		room:playSkillEffect("duijue",2)
		if not owner:hasSkill("duijue") then room:acquireSkill(owner,"duijue") end
		duijue:trigger(event,room,owner,data)
		room:detachSkillFromPlayer(owner,"duijue")
	elseif event==sgs.Damage and damage.card:inherits("Slash") then
		if not damage.from:hasSkill(self:objectName()) then return end
		if owner:getPile("spxi"):length()==0 then  return end
		if (room:askForSkillInvoke(owner,"luajinshuduijue",data)~=true) then return end
		room:throwCard(owner:getPile("spxi"):first())
		room:playSkillEffect("duijue",2)
		if not owner:hasSkill("duijue") then room:acquireSkill(owner,"duijue") end
		duijue:trigger(event,room,owner,data)
		room:detachSkillFromPlayer(owner,"duijue")
	end
end
end,
}

luazhuoxie=sgs.CreateTriggerSkill{
name="luazhuoxie",
frequency=sgs.Skill_NotFrequent,
events={sgs.Damage},
can_trigger=function(self,target)
return not target:hasSkill(self:objectName())
end,
on_trigger=function(self,event,player,data)
        local room=player:getRoom()
		local damage=data:toDamage()
		if not damage.from then return end
		if not damage.from:isAlive() then return end
		local owner = room:findPlayerBySkillName(self:objectName())
		if owner:getMark("luazhuanshi")>0 then return end
        if not room:askForSkillInvoke(owner,self:objectName(),data) then return end
		room:loseHp(owner,1)
		local card_id = room:askForCardChosen(owner,damage.from,"he",self:objectName())
		room:throwCard(card_id)
		owner:drawCards(1)
end
}

luazhuanshi=sgs.CreateTriggerSkill{
name="luazhuanshi",
events={sgs.AskForPeaches,sgs.Predamage,sgs.Predamaged,sgs.Damage,sgs.Damaged,sgs.PhaseChange},
priority=3,
view_as_skill=luajinshuvs,
can_trigger=function(self,player)
	return true
end,
frequency=Skill_NotFrequent,
on_trigger=function(self,event,player,data)
	local room=player:getRoom()
	local owner=room:findPlayerBySkillName(self:objectName())
	if not owner:isAlive() then return end
	local baoguo=sgs.Sanguosha:getTriggerSkill("baoguo")
	local duijue=sgs.Sanguosha:getTriggerSkill("duijue")
	local dujian=sgs.Sanguosha:getTriggerSkill("dujian")
if event==sgs.AskForPeaches and player:objectName()==owner:objectName() then
		if player:getMark("luazhuanshi")==1 then return end
		local room=player:getRoom()
		room:setPlayerMark(player,"luazhuanshi",1)
		local recover=sgs.RecoverStruct()
		recover.recover=1
		recover.who=player
		room:recover(player,recover)
		player:drawCards(2)
		room:detachSkillFromPlayer(player,"luazhuoxie")
		local log=sgs.LogMessage()
		log.type="#luazhuanshi"
		log.from=player
		room:sendLog(log)
elseif event==sgs.PhaseChange and player:getPhase()==sgs.Player_Discard then
	if player:objectName()~=owner:objectName() then return end
	if player:getMark("luazhuanshi")~=1 then return end
	room:askForUseCard(player, "@@luajinshucard", "@luajinshu")
elseif event==sgs.Predamaged then
	if player:getMark("luazhuanshi")~=1 then return end
	if owner:getPile("spxi"):length()==0 then return end
	local damage=data:toDamage()
	if damage.to:objectName()==owner:objectName() then return end
		if (room:askForSkillInvoke(owner,"luajinshubaoguo2",data)~=true) then return end
		room:playSkillEffect("baoguo",math.random(1,2))
		local cd = room:askForCard(owner, "BasicCard", "@luajinshubaoguo2",data)
		if cd then
			local tmp=sgs.DamageStruct()
			tmp.damage=damage.damage
			tmp.from=damage.from
			tmp.to=owner
			room:damage(tmp)
			return true
		end
elseif event==sgs.Predamage then
	if player:getMark("luazhuanshi")~=1 then return end
	if player:objectName()==owner:objectName() then
		local damage=data:toDamage()
		if damage.to:canSlash(damage.from) then return end
		if owner:getPile("spxi"):length()==0 then return end
		if (room:askForSkillInvoke(owner,"luajinshudujian",data)~=true) then return end
		room:throwCard(owner:getPile("spxi"):first())
		room:playSkillEffect("dujian",math.random(1,2))
		damage.to:turnOver()
		return true
	end
elseif event==sgs.Damaged or sgs.Damage then
	if owner:getMark("luazhuanshi")~=1 then return end
	if owner:getPile("spxi"):length()==0 then return end
	local damage=data:toDamage()
	if event==sgs.Damaged and player:objectName()==owner:objectName() then
		if owner:getPile("spxi"):length()==0 then return end
		if (room:askForSkillInvoke(owner,"luajinshubaoguo",data)~=true) then return end
		room:throwCard(owner:getPile("spxi"):first())
		room:playSkillEffect("baoguo",math.random(1,2))
		player:drawCards(player:getLostHp())
	end
	if not damage.card then return end
	if event==sgs.Damaged and damage.card:inherits("Slash") then
		if not damage.to:hasSkill(self:objectName()) then return end
		if owner:getPile("spxi"):length()==0 then  return end
		if (room:askForSkillInvoke(owner,"luajinshuduijue",data)~=true) then return end
		room:throwCard(owner:getPile("spxi"):first())
		room:playSkillEffect("duijue",2)
		if not owner:hasSkill("duijue") then room:acquireSkill(owner,"duijue") end
		duijue:trigger(event,room,owner,data)
		room:detachSkillFromPlayer(owner,"duijue")
	elseif event==sgs.Damage and damage.card:inherits("Slash") then
		if not damage.from:hasSkill(self:objectName()) then return end
		if owner:getPile("spxi"):length()==0 then  return end
		if (room:askForSkillInvoke(owner,"luajinshuduijue",data)~=true) then return end
		room:throwCard(owner:getPile("spxi"):first())
		room:playSkillEffect("duijue",2)
		if not owner:hasSkill("duijue") then room:acquireSkill(owner,"duijue") end
		duijue:trigger(event,room,owner,data)
		room:detachSkillFromPlayer(owner,"duijue")
	end
end
end,
}


luayizongcard=sgs.CreateSkillCard{
name="luayizongcard",
target_fixed=true,
will_throw=true,
on_use=function(self,room,source,targets)
	local x=self:getSubcards():length()
	for i=1,x,1 do
		local card_id = room:drawCard()
		local card=sgs.Sanguosha:getCard(card_id)
		source:addToPile("spyz",card)
	end
	room:throwCard(self)
end
}

luayizongvs=sgs.CreateViewAsSkill{
name="luayizongvs",
n=999,
view_filter=function(self, selected, to_select)
	return not to_select:isEquipped()
end,
view_as=function(self, cards)
	if #cards==0 then return end
	local acard=luayizongcard:clone()
	for var=1,#cards,1 do
        acard:addSubcard(cards[var])
    end
	acard:setSkillName("luayizong")
	return acard
end,
enabled_at_play=function()
	return false
end,
enabled_at_response=function(self,player,pattern)
	return pattern == "@@luayizongcard"
end,
}
luayizong=sgs.CreateTriggerSkill{
name="luayizong",
events={sgs.CardDiscarded,sgs.PhaseChange},
priority=3,
view_as_skill=luayizongvs,
default_choice = "dis",
can_trigger=function(self,target)
	return (not target:hasSkill(self:objectName())) and target:isWounded()
end,
frequency=sgs.Skill_NotFrequent,
on_trigger=function(self,event,player,data)
	local room=player:getRoom()
	local owner=room:findPlayerBySkillName(self:objectName())
	local yzcardids=sgs.IntList()
if event == sgs.CardDiscarded and player:getPhase()==sgs.Player_Discard then
    if player:objectName()~=owner:objectName() then
	--if (room:askForSkillInvoke(owner,"test",data)~=true) then return end
		room:setPlayerFlag(player,"yznow")
		local card = data:toCard()
		if not card:isVirtualCard() then
			yzcardids:append(card:getId())
		else
			for _, cid in sgs.qlist(card:getSubcards()) do
				yzcardids:append(cid)
			end
		end
	end
	if (room:askForSkillInvoke(owner,self:objectName(),data)~=true) then return end
	room:askForUseCard(owner, "@@luayizongcard", "@luayizong")
	local x=owner:getPile("spyz"):length()
	for i=1,owner:getPile("spyz"):length(),1 do
		room:throwCard(owner:getPile("spyz"):first())
	end
	if x>yzcardids:length() then x=yzcardids:length() end
	local get=0
	for i=1,x,1 do
		room:fillAG(yzcardids, owner)
		local card_id = room:askForAG(owner, yzcardids, true, "luayizong")
		if card_id then
			yzcardids:removeOne(card_id)
			player:obtainCard(sgs.Sanguosha:getCard(card_id))
			get=get+1
			owner:invoke("clearAG")
		end
		if(card_id == -1) then break end
	end
	--owner:invoke("clearAG")
	if owner and room:askForChoice(owner, self:objectName(), "dis+draw") == "dis" then
		room:playSkillEffect("guzong",math.random(1,3))
		if player:isAllNude() or get==0 then return end
		for i=1,get,1 do
			local card_id = room:askForCardChosen(owner,player,"hej",self:objectName())
			room:throwCard(card_id)
		end
	else
		room:playSkillEffect("yixian",math.random(1,2))
		owner:drawCards(1)
	end


end
end,
}

spyuefei:addSkill(luajinshu)
spbird:addSkill(luazhuanshi)
spbird:addSkill(luazhuoxie)
spzhulei:addSkill(luayizong)

sgs.LoadTranslationTable{
	["#spbird"]="护法明王",
	["spbird"]="大鹏金翅鸟",
	["$luazhuanshi"]="转世重生",
	["luazhuanshi"]="[醒]转世",
	[":luazhuanshi"]="<b>觉醒技</b>,当你处于濒死状态时,你须回复至1点体力并\
摸两张牌,然后你须失去技能【啄邪】并获得技能【尽术】的效果:\
弃牌阶段开始时,你可以弃置任意张牌,然后亮出牌堆顶等量的牌放置在你的武将牌上，称为“习”\
你可以在合理时机弃置一张“习”发动【对决】【毒箭】【报国】",
	["#luazhuanshi"]="%from 【转世】觉醒了，获得了技能【尽术】的效果",
	["luazhuoxie"]="啄邪",
	[":luazhuoxie"]="其他角色造成伤害后,你可以自减一点体力,然后弃置其一张牌,若如此做,你摸一张牌",
	["#spyuefei"]="金翅雏鹰",
	["spyuefei"]="少年岳飞",
	["@luajinshu"]="请选择任意张手牌用以发动【尽术】",
	["luajinshu"]="尽术",
	["luajinshuvs"]="尽术",
	["luajinshucard"]="尽术",
	[":luajinshu"]="弃牌阶段开始时,你可以弃置任意张牌,\
	然后亮出牌堆顶等量的牌放置在你的武将牌上，	称为“习”\
	你可以在合理时机弃置一张“习”\
	发动【对决】【毒箭】【报国】.",
	["#luajinshu"]="%from把%arg2%arg被锻炼成了【习】",
	["spxi"]="【习】",
	["#test"]="There is %from using card",
	["luajinshubaoguo"]="学武·报国(你每受到一次伤害，可以摸X张牌（X为你已损失的体力值）)",
	["luajinshubaoguo2"]="学武·报国(当其他角色受到伤害时，你可以弃置一张基本牌，将该伤害转移给你)",
	["@luajinshubaoguo2"]="弃置一张基本牌，将该伤害转移给你",
	["luajinshuduijue"]="学武·对决(你每使用【杀】造成一次伤害或受到一次其他角色使用【杀】造成的伤害，可以令除你外的任一角色进行一次判定\
	若结果不为黑桃，则视为你对其使用一张【决斗】)",
	["luajinshudujian"]="学武·毒箭(当你对其他角色造成伤害时，若你不在其攻击范围内，则你可以防止该伤害，令该角色将其武将牌翻面)",
	["#spzhulei"]="义不容辞",
	["spzhulei"]="朱仝雷横",
	["luayizong"] = "义纵",
	["luayizongvs"] = "义纵",
	["luayizongcard"] = "义纵",
	["@luayizong"]="请选择任意张牌用以发动【义纵】",
	["spyz"]="【义】",
	["#yztest"] = "%arg now",
	[":luayizong"] = "其他角色弃牌阶段结束时,若其已受伤,你可以弃置X张手牌,\
	将其弃牌中X张弃牌返回其手牌,然后你可以选择弃置其等量的牌或摸1张牌。",
	["luayizong:dis"] = "弃他牌",
    ["luayizong:draw"] = "摸1张",
}


--[[GeneralNameTable]]--
mygname={
"splujunyi",
"splinchong",
"spluzhishen",
"spwusong",
"spzhulei",
"spzhuwu",
"sphuyanzhuo",
"spbird",
"spyuefei",
"sphuangxin",
}
gnum={
"001",
"002",
"003",
"004",
"005",
"006",
"007",
"008",
"009",
"0012",
}
--[[AutoFillDesigner]]--
for j=1,#mygname,1 do
	sgs.LoadTranslationTable
	{
		["designer:"..mygname[j]]="群龙令&烨子",
		["illustrator:"..mygname[j]] = "江某",
		["$"..mygname[j]] =gnum[j],
	}
	if j>=7 or j==5 then
		sgs.LoadTranslationTable{["coder:"..mygname[j]]="roxiel",}
	end
end