function speak(to,type)
	if not sgs.GetConfig("AIChat", true) then return end
	if not to or to:getState() ~= "robot" then return end
	
	local i =math.random(1,#sgs.ai_chat[type])
	to:speak(sgs.ai_chat[type][i])
end

function speakTrigger(card,from,to,event)
	if event == sgs.SlashEffect and from:hasSkill("tongwu") then
		speak(to,"tongwu")
	elseif event == sgs.SlashProceed then
		if from:hasSkill("kaixian") then
			speak(from,"kaixian")
		end
	elseif event == sgs.SlashHit then
		if from:hasSkill("jingzhun") then
			speak(to,"jingzhun")
		end
	elseif event == sgs.Death and from:hasSkill("zuohua") then
		speak(from,"zuohua_death")
	elseif event == sgs.FinishJudge and from:hasFlag("CujuBad") then
		speak(from,"cuju_fail")
	elseif event == sgs.Pindian then
		speak(from,"pindian")
		speak(to,"pindian_target")
	end

	if not card then return end

	if card:inherits("Indulgence") then
		speak(from, "indulgence_source")
		if to:getHandcardNum() > to:getHp() then
			speak(to, "indulgence")
		end
	elseif card:inherits("SupplyShortage") then
		speak(from, "supply_shortage_source")
		if to:getHandcardNum() < to:getHp() then
			speak(to, "supply_shortage")
		end
	elseif card:inherits("Slash") then
		if to:hasSkill("baoguo") and to:getHp() <= 1 then
			speak(to,"lujunyi_weak")
		end
	elseif card:inherits("SavageAssault") then
		if to:hasSkill("fuhu") or to:hasSkill("shalu") then
			speak(to,"tiger")
		end
	elseif card:inherits("FireAttack") then
		if from:hasSkill("shenhuo") then
			speak(from,"shenhuo")
		elseif to:hasSkill("shenhuo") then
			speak(to,"shenhuo_forb")
		else
			speak(from,"fire_attack")
		end
	elseif card:inherits("Ecstasy") then
		speak(from,"ecstasy_source")
		speak(to,"ecstasy")
	elseif card:inherits("GanlinCard") then
		speak(from, "ganlin")
		speak(to,"friendly")
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
--pindian
sgs.ai_chat.pindian=
{
"出大的！",
"来来来拼点了",
"哟，拼点吧"
}
sgs.ai_chat.pindian_target=
{
"老子怕你不成！",
"暂且看看，鹿死谁手吧！",
}
-- duel
sgs.ai_chat.duel_female=
{
"哼哼哼，怕了吧~"
}
sgs.ai_chat.duel=
{
"来吧！像男人一样决斗吧！",
"生活压力大，只有靠欺负同事来缓解压力……"
}
-- ex_nihilo
sgs.ai_chat.lucky=
{
"哎哟运气好",
"马猴烧酒，参上！",
"哈哈哈哈哈"
}
-- amazing_grace
sgs.ai_chat.amazing_grace=
{
"一人一口，分而食之。",
"药药药，切克劳，煎饼果子来一套~"
}
-- god_salvation
sgs.ai_chat.god_salvation=
{
"有肉一起吃，有血一起回。",
"妈妈没回来不开门。"
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
-- fire_attack
sgs.ai_chat.fire_attack=
{
"抓住那对异性恋，烧死他们！",
"火攻是一件具有技术含量的活动。"
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
-- wiretap
sgs.ai_chat.wiretap_female=
{
"讨厌，人家都被你看光了啦~"
}
sgs.ai_chat.wiretap=
{
"再看，再看就把你喝掉！"
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
-- supply_shortage
sgs.ai_chat.supply_shortage_source=
{
"喝西北风去吧！",
"其实这是兵粮寸断哦~"
}
sgs.ai_chat.supply_shortage=
{
"本大仙运气好，下次一定是梅花~",
"饿肚子的感觉，你知道吗……"
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
sgs.ai_chat.ganlin=
{
"这是您的~",
"这个给您~"
}
sgs.ai_chat.friendly=
{
"。。。",
"你有何企图。。。",
"别以为这样就能收买我了。",
"这……不太好吧- -",
"谢了。。。",
}
--lujunyi
sgs.ai_chat.baoguo=
{
"别怕，有兄弟给你顶着~",
"为朋友两肋插刀，为小猫插朋友两刀！"
}
sgs.ai_chat.lujunyi_weak=
{
"擦，再卖血会卖死的",
"不敢再卖了诶诶诶诶"
}
--wuyong
sgs.ai_chat.huace=
{
"主公，你还记得大明湖畔的吴学究么……",
"其实，我一直深爱着你！"
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
sgs.ai_chat.yixing=
{
"谁、谁刚才摸了我一下？",
"啊！"
}
--guansheng
sgs.ai_chat.tongwu=
{
"我闪不闪呢……",
"闪、还是不闪，这是个问题= ="
}
--linchong
sgs.ai_chat.duijue=
{
"寒星夺魄刺，冷月索命舞！",
"哦不如搞基，就真的不如搞基~ 嘿咻~"
}
--huarong
sgs.ai_chat.kaixian=
{
"我这一箭即可取汝狗头！元芳你怎么看？"
}
sgs.ai_chat.jingzhun=
{
"看看看，看你麻痹看！"
}
--chaijin
sgs.ai_chat.danshu=
{
"哈哈哈，傻眼了吧，爷也有个叫刘备的爸爸！",
"小子，牌不多就别妄动哦~"
}
sgs.ai_chat.haoshen=
{
"幸福是什么？幸福就是有个有钱的哥们啊！",
"求安慰求包养各种求。。"
}
--zhutong
sgs.ai_chat.sijiu=
{
"跟爷混，有肉吃。"
}
--luzhishen
sgs.ai_chat.liba=
{
"这JB鸟树，真TNND沉！",
"你插的再深，洒家也能拔出来~"
}
--wusong
sgs.ai_chat.fuhu=
{
"一点一点亮晶晶，满眼都是小星星。。",
"老虎，老鼠，喝醉了分不清楚……"
}
--yangzhi
sgs.ai_chat.maidao=
{
"谁TM敢动我的刀我和谁玩命！",
"卖刀，其实是一种态度，一种人生信仰，你懂的。"
}
sgs.ai_chat.buydao=
{
"哎呦，不错，这个屌！",
"两个大子回家耍去，这刀归我了~ ",
"城管执法！没收管制刀具！"
}
--xuning
sgs.ai_chat.goulian=
{
"要不是被偷了宝甲，谁愿意上这破逼梁山？！",
"狗链枪？我在行！"
}
--daizong
sgs.ai_chat.jibao=
{
"让领导先走！",
"戴跑跑，我跑啊跑啊跑~ ",
"不好意思，还是我在跑~ "
}
--yangxiong
sgs.ai_chat.xingxing=
{
"兄弟，一路走好。",
"其实，这全是导演安排的……"
}
--andaoquan
sgs.ai_chat.jishi=
{
"这盒金疮药接住了~",
"医生真苦逼，各种疗伤……",
"掌仙术！"
}
--husanniang
sgs.ai_chat.wuji=
{
"天然美貌蒲小喵~ ",
"喵~ 喵~ "
}
--sun2niang
sgs.ai_chat.heidian=
{
"二娘饶命……",
"没留意竟然进了黑店！"
}
--gaoqiu
sgs.ai_chat.cuju=
{
"好球！",
"踢个球玩玩~",
"想和我一起组队踢世界杯吗？",
"看什么看，踢的就是你！"
}
sgs.ai_chat.cuju_fail=
{
"操！臭球！",
"竟然，踢空了？",
"世界杯什么的，呵呵，说着玩呢。",
}
--caijing
sgs.ai_chat.jiashu=
{
"来，给大爷乐一个~",
"要记住，你爸是李刚。",
"去歧姐家把虫妹叫来~ "
}
--fangla
sgs.ai_chat.yongle={
"谁有肉？快交出来！",
"交保护费了！",
"坑爹啊，你除了初音手办就没有别的了么？"
}
--lishishi
sgs.ai_chat.yinjian={
"捂脸。",
"切了你！",
"你才带孩子呢，你带四五个孩子！"
}
--yanxijiao
sgs.ai_chat.suocai={
"你们都嫌弃我……",
"MUA~",
"嘤嘤……好桑心- -"
}
sgs.ai_chat.huakui={
"才不是胆小，只是太谨慎。",
"工口网站什么的最讨厌了！",
"花花乖，花花不哭~"
}
--qingzhang
sgs.ai_chat.qiongtu={
"擦，还我菊花！",
"内牛满面了",
"哎哟我去"
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
--zhangqing
sgs.ai_chat.yinyu=
{
"靠石头吃饭不容易，兄弟们担待点~",
"看我一石激起千层浪！"
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
