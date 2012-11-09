----技能触发信息
function SkillLog(player,str,typ,num)
	local room=player:getRoom()
	local log1=sgs.LogMessage()
	if num==0 then
		room:playSkillEffect(str)
	else
		room:playSkillEffect(str,num)
		str=str..num
	end
	if typ==1 then
		log1.type="$"..str
	else
		if typ==0 then
			log1.type="#TriggerSkill"
		else
			log1.type="#InvokeSkill"
		end
		log1.from=player
		log1.arg=str
	end
	room:sendLog(log1)
	return nil
end
--CreateCard的三个参数：
--1.你要复制的卡的objectname（string），或者技能卡的变量名
--2.子卡，可以是number，card，table，qlist
--3.技能名，string
function CreateCard(name,cards,skill)
	local ct={}
	if cards then
		if type(cards)=="userdata" and cards.at then 
			local ncards=sgs.QList2Table(cards)
			cards=ncards
		end
		if type(cards)=="table" then
			n=1
			for _,acard in ipairs(cards) do
				if type(acard)=="userdata" then ct[n]=acard end
				if type(acard)=="number" then ct[n]=sgs.Sanguosha:getCard(acard) end
				n=n+1
			end
			n=n-1
		end
		if type(cards)=="userdata" then ct[1]=cards end
		if type(cards)=="number" then ct[1]=sgs.Sanguosha:getCard(cards) end
	end
	local suit=nil
	local num=0
	for _,acard in ipairs(ct) do
		if suit and suit~=acard:getSuit() then suit=sgs.Card_NoSuit end
		if not suit then suit=acard:getSuit() end
		num=num+acard:getNumber()
	end
	if not suit then suit=sgs.Card_NoSuit end
	if num>13 then num=0 end
	local card
	if type(name)=="string" then
		card=sgs.Sanguosha:cloneCard(name,suit,num)
	else
		card=name:clone()
	end
	for _,acard in ipairs(ct) do
		card:addSubcard(acard)
	end
	if skill then card:setSkillName(skill) end
	return card
end
--添加触发技能
function TempSkill(skill)
	if not sgs.Sanguosha:getSkill(skill:objectName()) then
		local skillList=sgs.SkillList()
		skillList:append(skill)
		sgs.Sanguosha:addSkills(skillList)
	end
end