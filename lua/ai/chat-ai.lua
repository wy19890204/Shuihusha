function SmartAI:speak(typpe, to)  -- self:speak("baoguo", lujunyi)
	if not sgs.GetConfig("AIChat", true) then return end
	typpe = typpe or "no_type"
	to = to or self.player
	if to == nil or to:getState() ~= "robot" then return end
	local isFemale = to:getGeneral():isFemale()
	if isFemale and sgs.ai_chat[typpe .. "_female"] then
		typpe = typpe .. "_female"
	end
	local i = math.random(1, #sgs.ai_chat[typpe])
	to:speak(sgs.ai_chat[typpe][i])
end

function SmartAI:speakTrigger(card, from, to, event)
	from = from or self.player
	to = to or self.player
	if event == sgs.SlashEffect and from:hasSkill("tongwu") then
		self:speak("tongwu", to)
	elseif event == sgs.SlashProceed then
		if from:hasSkill("kaixian") then
			self:speak("kaixian", from)
		end
	elseif event == sgs.SlashHit then
		if from:hasSkill("jingzhun") then
			self:speak("jingzhun", to)
		end
	elseif event == sgs.Death and from:hasSkill("zuohua") then
		self:speak("zuohua_death", from)
	elseif event == sgs.FinishJudge and from:hasFlag("CujuBad") then
		self:speak("cuju_fail", from)
	elseif event == sgs.Pindian then
		self:speak("pindian", from)
		self:speak("pindian_target", to)
	end

	if not card then return end

	if card:isKindOf("Indulgence") then
		self:speak("indulgence_source", from)
		if to:getHandcardNum() > to:getHp() then
			self:speak("indulgence", to)
		end
	elseif card:isKindOf("SupplyShortage") then
		self:speak("supply_shortage_source", from)
		if to:getHandcardNum() < to:getHp() then
			self:speak("supply_shortage", to)
		end
	elseif card:isKindOf("Slash") then
		if to:hasSkill("baoguo") and to:getHp() <= 1 then
			self:speak("lujunyi_weak", to)
		end
	elseif card:isKindOf("SavageAssault") then
		if to:hasSkill("fuhu") or to:hasSkill("shalu") then
			self:speak("tiger", to)
		end
	elseif card:isKindOf("FireAttack") then
		if from:hasSkill("shenhuo") then
			self:speak("shenhuo", from)
		elseif to:hasSkill("shenhuo") then
			self:speak("shenhuo_forb", to)
		else
			self:speak("fire_attack", from)
		end
	elseif card:isKindOf("Ecstasy") then
		self:speak("ecstasy_source", from)
		self:speak("ecstasy", to)
	elseif card:isKindOf("GanlinCard") then
		self:speak("ganlin", from)
		self:speak("friendly", to)
	end
end

sgs.ai_chat={}

sgs.ai_chat.no_type=
{
"我无话可说。"
}
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
--zhangshun
sgs.ai_chat.lihun=
{
"不要欺人太甚……",
"我做鬼也不会放过你！"
}
--zhuwu
sgs.ai_chat.fangzhen=
{
"此阵可挡精兵十万！",
"你们知道行列式和矩阵的区别是什么吗？"
}
--shiwengong
sgs.ai_chat.dujian=
{
"咱俩换个姿势再继续干！",
"不打你，不杀你，我用感情折磨你→_→"
}

--gaolian
sgs.ai_chat.guibing=
{
"魑魅魍魉怎么他就这么多……",
"一佛出世，二鬼升天！",
}
--tongguan
sgs.ai_chat.zhengfa=
{
"哇哈哈，哭喊吧，哀求吧，然后死吧！",
"杀杀杀，杀出一个天下~",
}
sgs.ai_chat.zhengfa_female=
{
"本宫定要弄死你们这帮逆贼！",
"是男是女？都拖出来弄死~",
}
--huyanzhuo
sgs.ai_chat.lianma=
{
"水浒杀最白的白板又出来打酱油了~",
"我的技能是什么？",
}
--pangwanchun
sgs.ai_chat.lianzhu=
{
"放箭！",
"看我射不死你们！",
}
--dong&xue
sgs.ai_chat.sheru=
{
"真没干劲儿。",
}
--lili
sgs.ai_chat.duomingmoucai=
{
"搞偷袭什么的我最拿手~",
"看我也没用，有本事就放马过来吧！",
}
--shijin
sgs.ai_chat.wubang=
{
"职业捡破烂捡一送二！",
"刀枪棍棒什么的都给我保管吧~",
}
sgs.ai_chat.xiagu=
{
"多谢！",
"帮了大忙了~",
}
--xiezhen
sgs.ai_chat.xunlie=
{
"我最擅长拍华南虎了~",
"我且带上相机，说不定能拍到华南虎~",
}
--linniangzi
sgs.ai_chat.shouwang=
{
"我就这样一直一直默默注视你。",
"喜欢你的头发眼睛耳朵肚脐眼。",
"我会含着他的嘴唇做一个好梦。",
}

--shixiu
sgs.ai_chat.pinming=
{
"其实我真不想拼命……都是导演安排的，泪目啊！",
"体力无下限，乱搞死得早。",
}
--lvfang
sgs.ai_chat.lieji=
{
"小温侯什么的，说起来都感觉丢人……",
"古有夏侯二刀，今有口口二刀！",
}
--xiebao
sgs.ai_chat.liehuo=
{
"哥！我抓到华南虎了！",
"哥！赶紧过来拍华南虎照片！",
}
--shien
sgs.ai_chat.xiaozai=
{
"替我挡着！",
"接着哦~",
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
--haosiwen
sgs.ai_chat.sixiang=
{
"两仪生四象，四象生八卦。",
"生的伟大，死的凄惨！",
}
--peixuan
sgs.ai_chat.binggong=
{
"嗯，元芳你怎么看？",
"啊？元芳挂了？",
}
--songqing
sgs.ai_chat.jiayao=
{
"人是铁，饭是碳，吃多了就变生铁了。",
"人是铁，饭是碳，想成钢就别吃多。",
}
--songwan
sgs.ai_chat.yijie=
{
"哦~我不是黄盖~哦~我人畜无害~",
"其实，这个世界也挺可爱的。",
}
--zhoutong
sgs.ai_chat.huatiancuo=
{
"花田里犯了错~哦破晓前忘掉。",
"花田里犯了错~哦拥抱变成了煎熬。",
}
sgs.ai_chat.huatianai=
{
"琥珀色的月结成了霜。",
"手里握着蝴蝶杯，不醉的是乌龟。",
}
--zhaoji
sgs.ai_chat.shemi=
{
"嗯……嗯……",
"哦……哦……",
}

--suochao
sgs.ai_chat.chongfeng=
{
"还不够，再来一发！",
"不要停，继续！",
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
sgs.ai_chat.riceball=
{
"哟，什么风把您老吹来了？",
"一边去，别打扰我思考！",
"不要乱碰人家啦……"
}
