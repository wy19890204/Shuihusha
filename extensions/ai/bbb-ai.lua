
dofile "funs.lua"

sgs.ai_skill_cardask.qll_shenchou=function(self, data, pattern, target, target2)
	local cards=sgs.QList2Table(player:getCards("he"))
	for _,cd in ipairs(cards) do
		if cd:inherits("Weapon") or cd:inherits("EventsCard") then return cd end
	end
end
qll_duanbi_target=nil
qll_duanbi_choice=nil
sgs.ai_skill_invoke.qll_duanbi=function(self,data)
	if self.player:getMark("qll_zhusha")==0 then return false end
	self:sort(self.enemies,"hp")
	self:sort(self.friends_noself,"hp")
	local e1=self.enemies[1]
	if e1:getHp()<=2 then
		qll_duanbi_target=e1
		qll_duanbi_choice="damage"
		return true
	end
	for _,f1 in ipairs(self.friends_noself) do
		if f1:getLostHp()>=2 then
			qll_duanbi_target=f1
			qll_duanbi_choice="recover"
			return true
		end
	end
	return false
end
sgs.ai_skill_playerchosen.qll_duanbi=function(self,targets)
	return qll_duanbi_target
end
sgs.ai_skill_choice.qll_duanbi=function(self,choices)
	return qll_duanbi_choice
end

local qll_zhusha_skill={}
qll_zhusha_skill.name="qll_zhusha"
table.insert(sgs.ai_skills,qll_zhusha_skill)
qll_zhusha_skill.getTurnUseCard=function(self,inclusive)
	if self.player:getPile("qll_shenchou"):length()>0 then return sgs.Card_Parse("#qll_zhusha_card:.:") end
end
sgs.ai_skill_use_func["#qll_zhusha_card"]=function(card,use,self)
	self:sort(self.enemies,"defense")
	local slash=CreateCard("slash")
	local fe=nil
	local se=nil
	local tc=0
	for _,enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy) and not self:slashProhibit(slash,enemy) and tc<=2 then 
			if tc==0 then
				fe=enemy
			else
				se=enemy
			end
			tc=tc+1
		end
	end
	if tc==2 then
		use.card=card
		if use.to then
			use.to:append(fe)
			use.to:append(se)
		end
		return
	end
	if tc==1 and fe:getHp()==1 or self.player:getPile("qll_shenchou"):length()>1 then
		use.card=card
		if use.to then
			use.to:append(fe)
		end
		return
	end
end

function sgs.ai_armor_value.LuaJinShen(card)
	if not card then return 4 end
end

function sgs.ai_slash_prohibit.LuaJinShen(self,enemy,card)
	return card:isRed() and not enemy:getArmor()
end

sgs.ai_skill_cardask["askforLuaYiJiu"]=function(self,data)
	local lord=self.room:getLord()
	if lord and self:isFriend(lord) then return nil end
	return "."
end

sgs.ai_skill_invoke.LuaYuSui=function(self,data)
	return self.player:isWounded() or not self.player:faceUp()
end

sgs.ai_skill_invoke.LuaMingWangVS=function(self,data)
	self.player:speak(self.player:getRole())
	return self.player:isWounded() and self.player:getRole()=="loyalist"
end

sgs.ai_skill_cardask["askforLuaBaoGuo"]=function(self,data)
	if self.player:hasSkill("fushang") and self.player:getHp() > 3 then return "." end
	local damage = data:toDamage()
	if self:isFriend(damage.to) and not self.player:isKongcheng() then
		if self.player:getRole()=="lord" and self.player:hasSkill("LuaMingWangVS") and damage.to:getKingdom()=="min" and damage.to:getHp()>1 then return "." end
		local pile = self:getCardsNum("Peach") + self:getCardsNum("Analeptic")
		local dmgnum = damage.damage
		if self.player:getHp() + pile - dmgnum > 0 then
			if self.player:getHp() + pile - dmgnum == 1 and pile > 0 then return "." end
			local cards = sgs.QList2Table(self.player:getHandcards())
			self:sortByUseValue(cards, false)
			for _, fcard in ipairs(cards) do
				if fcard:inherits("BasicCard") and
					not fcard:inherits("Peach") and not fcard:inherits("Analeptic") then
					return fcard:getEffectiveId()
				end
			end
		end
	end
	return "."
end
--[[sgs.ai_skill_cardask["askforLuaBaoGuo"]=function(self,data)
	if not self:isFriend(data:toDamage().to) then return "." end
	return nil
end]]

local LuaShenQiang_skill={}
LuaShenQiang_skill.name="LuaShenQiang"
table.insert(sgs.ai_skills,LuaShenQiang_skill)
LuaShenQiang_skill.getTurnUseCard=function(self,inclusive)
	if not self.player:hasFlag("LuaShenQiangUsed") then return sgs.Card_Parse("#LuaShenQiangCard:.:") end
end
sgs.ai_skill_use_func["#LuaShenQiangCard"]=function(card,use,self)
	local cards=sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards,true)
	local card1=nil
	for _,hcard in ipairs(cards) do
		if hcard:inherits("BasicCard") then
			if not card1 then
				card1=hcard
			else
				for _,enemy in ipairs(self.enemies) do
					if not self:slashProhibit(nil,enemy) and self.player:canSlash(enemy) then
						use.card=sgs.Card_Parse("#LuaShenQiangCard:"..card1:getId().."+"..hcard:getId()..":")
						if use.to then use.to:append(enemy) end
						return
					end
				end
			end
		end
	end
end

sgs.ai_skill_discard.LuaHuoPin=function(self,discard_num,optional,include_equip)
	if self.player:getCardCount(true)<3 then return {} end--牌不够，掉血
	local to_discard={}
	local str="he"
	if self.player:getHandcardNum()>self.player:getHp() then str="h" end
	local cards=sgs.QList2Table(self.player:getCards(str))
	self:sortByUseValue(cards,true)--扔最废的两张，如果这两张有桃或者酒宁愿掉血
	for _,card in ipairs(cards) do
		if #to_discard>=3 then break end
		if (card:inherits("Peach") or card:inherits("Analeptic")) and self.player:getHandcardNum()<=self.player:getHp()-1 then return {} end
		table.insert(to_discard,card:getEffectiveId())
	end
	if #to_discard>=3 then return to_discard else return {} end
end

local LuaKuaiYi_skill={}
LuaKuaiYi_skill.name="LuaKuaiYi"
table.insert(sgs.ai_skills,LuaKuaiYi_skill)
LuaKuaiYi_skill.getTurnUseCard=function(self,inclusive)
	return sgs.Card_Parse("#LuaKuaiYiCard:.:")
end
sgs.ai_skill_use_func["#LuaKuaiYiCard"]=function(card,use,self)
	local cards=sgs.QList2Table(self.player:getCards("h"))
	self:sortByUseValue(cards,true)
	self:sort(self.enemies,"defense")
	for _,enemy in ipairs(self.enemies) do
		if not enemy:isKongcheng() and not enemy:hasFlag("LuaKuaiYiTarget") and enemy:getCardCount(false)<4 then 
			for _,card in ipairs(cards) do
				if card:isRed() then
					use.card=sgs.Card_Parse("#LuaKuaiYiCard:"..card:getId()..":")
					if use.to then use.to:append(enemy) return end
				end
			end
		end
	end
end

sgs.ai_skill_cardask["askforLuaFuHu"]=function(self,data)
	local damage = data:toDamage()
	local slash = sgs.Sanguosha:cloneCard("slash", sgs.Card_NoSuit, 0)
	if not self:slashIsEffective(slash, damage.from) then return "." end
	if self:isEnemy(damage.from) then
		local cards = self.player:getCards("he")
		cards = sgs.QList2Table(cards)
		self:sortByUseValue(cards, true)
		local default
		for _, card in ipairs(cards) do
			if card:isBlack() then
				if not default then default = card end
				if self:getCardsNum("Jink", damage.from) == 0 and
					(card:inherits("Analeptic") or card:inherits("Weapon")) then
					return card:getEffectiveId()
				end
			end
		end
		if default then
			return default:getEffectiveId()
		end
	end
	return "."
end
--[[sgs.ai_skill_cardask["askforLuaFuHu"]=function(self,data)
	if self:isFriend(data:toDamage().from) then return "." end
	return nil
end]]


sgs.ai_skill_invoke.LuaQianLv=function(self,data)
	if not data then return true end
	local judge=data:toJudge()
	if not judge then return true end
	if not judge:isGood() then 
	return true end
	return false
end
dofile "lua/ai/guanxing-ai.lua"

function getLeftAlive(player)
	local i=player
	while i:getNextAlive():objectName()~=player:objectName() do
		i=i:getNextAlive()
	end
	return i
end
sgs.ai_skill_cardask["askforLuaNiZhuan"]=function(self,data)
	local left=getLeftAlive(self.player)
	local right=self.player:getNextAlive()
	left:speak("left--")
	right:speak("right--")
	if not (self:isFriend(left) and self:isEnemy(right)) then return "." end
	return nil 
end

local qll_chongfeng_skill={}
qll_chongfeng_skill.name="qll_chongfeng"
table.insert(sgs.ai_skills,qll_chongfeng_skill)
qll_chongfeng_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)

	local card

	self:sortByUseValue(cards,true)

	for _,acard in ipairs(cards)  do
		if acard:inherits("DefensiveHorse") or acard:inherits("OffensiveHorse") then
			card = acard
			break
		end
	end

	if not card then return nil end
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("iron_chain:qll_chongfeng[%s:%s]=%d"):format(card:getSuit(),number, card_id)
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end

sgs.ai_chaofeng.sphuangxin = 4
sgs.ai_skill_invoke.luasaodang  =  true
sgs.ai_skill_use["@@luasaodangcard"] = function(self, prompt)
	self.room:writeToConsole("Now fuin @@ saodang!!!!!")
	self:sort(self.enemies, "hp")
	local targets={}
	local first,second,third
	self:sort(self.enemies,"hp")
	for _,enemy in ipairs(self.enemies) do
		table.insert(targets, enemy:objectName())
		self.room:writeToConsole(enemy:objectName())
		if #targets >= 3 then break end	
	end	
	return "#luasaodangcard:.:->".. table.concat(targets,"+") 	
end
