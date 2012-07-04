function speak(to,type)
	if not sgs.GetConfig("AIChat", true) then return end
	if not to or to:getState() ~= "robot" then return end
	
	local i =math.random(1,#sgs.ai_chat[type])
	to:speak(sgs.ai_chat[type][i])
end

function speakTrigger(card,from,to,event)
	if (event=="death") and from:hasSkill("zuohua") then
		speak(from,"zuohua_death")
	end

	if not card then return end

	if card:inherits("Indulgence") and (to:getHandcardNum()>to:getHp()) then
		speak(to,"indulgence")
	elseif card:inherits("CujuCard") then
		speak(from,"cuju")
	elseif card:inherits("DaleiCard") then
		speak(from,"dalei")
		speak(to,"dalei_target")
	elseif card:inherits("Ecstasy") then
		speak(from,"ecstasy_source")
		speak(to,"ecstasy")
	elseif card:inherits("QimenCard") then
		speak(from,"qimen_source")
		speak(to,"qimen")
	elseif card:inherits("Slash") and to:hasSkill("baoguo") and (to:getHp()<=1) then
		speak(to,"lujunyi_weak")
	elseif card:inherits("SavageAssault") and (to:hasSkill("fuhu") or to:hasSkill("shalu")) then
		speak(to,"tiger")
	elseif card:inherits("FireAttack") and to:hasSkill("shenhuo") then
		speak(to,"shenhuo")
	elseif card:inherits("Zongzi") then
		speak(from,"gift")
	end
end

function SmartAI:speak(type, isFemale, to)
	if not sgs.GetConfig("AIChat", true) then return end
	to = to or self.player
	if to:getState() ~= "robot" then return end
	
	local i =math.random(1,#sgs.ai_chat[type])
	if isFemale then
		type = type .. "_female"
	end
	to:speak(sgs.ai_chat[type][i])
end

sgs.ai_chat={}

sgs.ai_chat.hostile_female=
{
"啧啧啧，来帮你解决点手牌吧",
"叫你欺负人!" ,
"手牌什么的最讨厌了"
}

sgs.ai_chat.hostile={
"yoooo少年，不来一发么",
"果然还是看你不爽",
"我看你霸气侧漏，不得不防啊"
}

sgs.ai_chat.qiongtu={
"擦，还我菊花！",
"内牛满面了",
"哎哟我去"
}

sgs.ai_chat.heidian=
{
"二娘饶命……",
"没留意竟然进了黑店！"
}

sgs.ai_chat.renrou=
{
"这是您的~",
"这个给您~"
}

sgs.ai_chat.friendly=
{ "。。。" }

sgs.ai_chat.respond_friendly=
{ "谢了。。。" }

sgs.ai_chat.duel_female=
{
"哼哼哼，怕了吧~"
}

sgs.ai_chat.duel=
{
"来吧！像男人一样决斗吧！"
}

sgs.ai_chat.lucky=
{
"哎哟运气好",
"哈哈哈哈哈"
}

sgs.ai_chat.collateral_female=
{
"别以为这样就算赢了！"
}

sgs.ai_chat.collateral=
{
"你妹啊，我的刀！"
}

sgs.ai_chat.assassinate_female=
{
" やめて ~  いや……"
}

sgs.ai_chat.assassinate=
{
"啊！有刺客！"
}

sgs.ai_chat.ecstasy_source=
{
"嘿嘿，看我耍点阴招……",
"躺下吧！"
}
sgs.ai_chat.ecstasy=
{
"我中了迷……药……",
"额……身体……不听使唤了……"
}

--indulgence
sgs.ai_chat.indulgence=
{
"我画个圈圈诅咒你！",
"擦，动我",
"放我出去！"
}

--bishangliangshan
sgs.ai_chat.drivolt=
{
"逼，逼你妹啊逼！",
"擦，我的肉！",
"我不要上梁山！"
}

--pangwanchun
sgs.ai_chat.lianzhu=
{
"放箭！",
"看我射不死你们！",
}

--huanshu
sgs.ai_chat.huanshu=
{
"竟敢伤我，你活够了？",
"挣扎吧，在火与雷的地狱中！",
"老子最牛逼，不服单挑"
}

--dalei
sgs.ai_chat.dalei=
{
"出大的！",
"来来来拼点了",
"哟，拼点吧"
}
sgs.ai_chat.dalei_target=
{
"老子怕你不成！",
"暂且看看，鹿死谁手吧！",
}

--cuju
sgs.ai_chat.cuju=
{
"好球！",
"踢个球玩玩~",
"想和我一起组队踢世界杯吗？",
"看什么看，踢的就是你！"
}

--yinyu
sgs.ai_chat.yinyu=
{
"靠石头吃饭不容易，兄弟们担待点~",
"看我一石激起千层浪！"
}

--qimen
sgs.ai_chat.qimen_source=
{
"封印术·八卦封印！",
"封印术·五行封印！",
"封印术·四象封印！",
"封印术·三才封印！",
"封印术·两仪封印！",
}
sgs.ai_chat.qimen=
{
"咦？我的技能哪去了？",
"公孙老道，你丫给我等着！",
"被、被封印了么- -"
}

--jishi
sgs.ai_chat.jishi=
{
"这盒金疮药接住了~",
"医生真苦逼，各种疗伤……",
"掌仙术！",
}

--salvageassault
sgs.ai_chat.tiger=
{
"好一只吊睛白额的大虫！",
"难道……这就是传说中的景阳冈花瓣太岁么- -",
"看什么，俺可是打虎英雄！",
"好大只，压力好大……"
}

--luzhishen
sgs.ai_chat.zuohua_death=
{
"本来无一物，何处惹尘埃……"
}

sgs.ai_chat.lujunyi_weak=
{
"擦，再卖血会卖死的",
"不敢再卖了诶诶诶诶"
}

sgs.ai_chat.zhangshi_female=
{
"别指望下次我会帮你哦"
}

sgs.ai_chat.zhangshi=
{
"美人儿莫怕，我来啦~"
}

sgs.ai_chat.gift=
{
"哇，有礼物耶！"
}
