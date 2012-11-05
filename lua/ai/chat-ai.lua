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

	if card:inherits("Indulgence") then
		speak(from, "indulgence_source")
		if to:getHandcardNum() > to:getHp() then
			speak(to, "indulgence")
		end
	elseif card:inherits("Slash") and to:hasSkill("baoguo") and (to:getHp()<=1) then
		speak(to,"lujunyi_weak")
	elseif card:inherits("SavageAssault") and (to:hasSkill("fuhu") or to:hasSkill("shalu")) then
		speak(to,"tiger")
	elseif card:inherits("FireAttack") then
		if from:hasSkill("shenhuo") then
			speak(from,"shenhuo")
		elseif to:hasSkill("shenhuo") then
			speak(to,"shenhuo_forb")
		end
	elseif card:inherits("Ecstasy") then
		speak(from,"ecstasy_source")
		speak(to,"ecstasy")
	elseif card:inherits("GanlinCard") then
		speak(to,"friendly")
	elseif card:inherits("CujuCard") then
		speak(from,"cuju")
	elseif card:inherits("DaleiCard") then
		speak(from,"dalei")
		speak(to,"dalei_target")
	elseif card:inherits("QimenCard") then
		speak(from,"qimen_source")
		speak(to,"qimen")
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

-- snatch dismantlement
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
-- duel
sgs.ai_chat.duel_female=
{
"哼哼哼，怕了吧~"
}
sgs.ai_chat.duel=
{
"来吧！像男人一样决斗吧！"
}
-- ex_nihilo
sgs.ai_chat.lucky=
{
"哎哟运气好",
"哈哈哈哈哈"
}
-- collateral
sgs.ai_chat.collateral_female=
{
"别以为这样就算赢了！"
}
sgs.ai_chat.collateral=
{
"你妹啊，我的刀！"
}
-- assassinate
sgs.ai_chat.assassinate_female=
{
" やめて ~  いや……"
}
sgs.ai_chat.assassinate=
{
"啊！有刺客！"
}
-- ecstasy
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
-- indulgence
sgs.ai_chat.indulgence_source=
{
"我画个圈圈诅咒你！",
"其实这是乐不思蜀哦~"
}
sgs.ai_chat.indulgence=
{
"擦，动我",
"呜呜呜，招你惹你了……",
"放我出去！"
}
-- bishangliangshan
sgs.ai_chat.drivolt=
{
"逼，逼你妹啊逼！",
"擦，我的肉！",
"我不要上梁山！"
}
-- salvageassault
sgs.ai_chat.tiger=
{
"好一只吊睛白额的大虫！",
"难道……这就是传说中的景阳冈花瓣太岁么- -",
"看什么，俺可是打虎英雄！",
"好大只，压力好大……"
}

--songjiang
sgs.ai_chat.friendly=
{
"。。。",
"你有何企图。。。",
"别以为这样就能收买我了。",
"这……不太好吧- -",
"谢了。。。",
}
--qingzhang
sgs.ai_chat.qiongtu={
"擦，还我菊花！",
"内牛满面了",
"哎哟我去"
}
--sun2niang
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
--pangwanchun
sgs.ai_chat.lianzhu=
{
"放箭！",
"看我射不死你们！",
}
--qiaodaoqing
sgs.ai_chat.huanshu=
{
"竟敢伤我，你活够了？",
"挣扎吧，在火与雷的地狱中！",
"老子最牛逼，不服单挑"
}
--yanqing
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
--gaoqiu
sgs.ai_chat.cuju=
{
"好球！",
"踢个球玩玩~",
"想和我一起组队踢世界杯吗？",
"看什么看，踢的就是你！"
}
--zhangqing
sgs.ai_chat.yinyu=
{
"靠石头吃饭不容易，兄弟们担待点~",
"看我一石激起千层浪！"
}
--gongsunsheng
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
--andaoquan
sgs.ai_chat.jishi=
{
"这盒金疮药接住了~",
"医生真苦逼，各种疗伤……",
"掌仙术！",
}
--weidingguo
sgs.ai_chat.shenhuo=
{
"本将没事就爱玩个火~",
"谁说不抽烟就不能带打火机了？",
"烧死你，哇哈哈哈哈~~",
"神火漫天，万物尽毁！圣水不在，谁可破得？"
}
sgs.ai_chat.shenhuo_forb=
{
"放火放到老子头上来了？",
"火神头上也敢动火？"
}
--luzhishen
sgs.ai_chat.zuohua_death=
{
"本来无一物，何处惹尘埃……"
}
--lujunyi
sgs.ai_chat.lujunyi_weak=
{
"擦，再卖血会卖死的",
"不敢再卖了诶诶诶诶"
}
--baixiuying
sgs.ai_chat.zhangshi_female=
{
"别指望下次我会帮你哦"
}
sgs.ai_chat.zhangshi=
{
"美人儿莫怕，我来啦~"
}

--gift
sgs.ai_chat.zongzi=
{
"哇，有粽子吃耶！"
}
sgs.ai_chat.moonpie=
{
"杀光小日本！活捉苍井空！",
"日本省是我大天朝固有领土！",
"这什么东西，难吃的要死~"
}
