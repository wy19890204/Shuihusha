-- translation for TigerPackage

return {
	["tiger"] = "淫包",

	["$leiheng"] = "025",
	["#leiheng"] = "插翅虎", -- guan 4hp
	["leiheng"] = "雷横",
	["guzong"] = "故纵",
	[":guzong"] = "其他角色的弃牌阶段结束时，若该角色已受伤，则你可以弃置X张牌，将此阶段中弃置的X张牌从弃牌堆返回该角色手牌，然后你弃掉其等量的牌。",

	["$sunli"] = "039",
	["#sunli"] = "病尉迟", -- guan 4hp
	["sunli"] = "孙立",
	["neiying"] = "内应",
	[":neiying"] = "你可以将你的任意两张颜色相同的牌当【将计就计】使用；出牌阶段，你可以指定任意两名角色，令其互相观看一次对方的手牌。每阶段限一次。",

	["$wuyanguang"] = "115",
	["#wuyanguang"] = "金甲元帅", -- guan 4hp
	["wuyanguang"] = "兀颜光",
	["jintang"] = "金汤",
	[":jintang"] = "锁定技，当你受到伤害时，若你的当前体力值不大于2，则你最多受到1点伤害；当你受到无属性伤害时，若你的当前体力值为1，则防止该伤害；当你死亡时，你须将你装备区里的所有牌分别置于其他角色的装备区里。",
	["@jintang"] = "你死了，受到锁定技【金汤】影响，你须将你装备区的 %arg 置于其他角色装备区内",
	["#JintangForb"] = "%from 的锁定技【%arg】被触发，防止了 %arg2 点伤害",
	["#JintangCut"] = "%from 的锁定技【%arg】被触发，伤害由 %arg2 点减至 1 点",

	["$shixiu"] = "033",
	["#shixiu"] = "拼命三郎", -- jiang 4/6hp
	["shixiu"] = "石秀",
	["pinming"] = "拼命",
	[":pinming"] = "任意角色每受到一次伤害，若伤害来源不为你，则你可以扣减1点体力上限，对伤害来源造成相同的伤害；当其他角色受到你对其造成的伤害进入濒死状态时，你可以令其立即死亡，然后你立即死亡。",

	["$lvfang"] = "054",
	["#lvfang"] = "小温侯", -- jiang 4hp
	["lvfang"] = "吕方",
	["lieji"] = "烈戟",
	[":lieji"] = "你可以跳过你的出牌阶段并弃置一张基本牌，视为你对一至两名其他角色使用一张【杀】。",
	["@lieji"] = "你可以弃一张基本牌发动【烈戟】，视为对一至两名其他角色使用一张【杀】",

	["$tianhu"] = "113",
	["#tianhu"] = "晋王", -- jiang 4hp
	["tianhu"] = "田虎",
	["wuzhou"] = "五州",
	[":wuzhou"] = "出牌阶段，若你的装备区里有牌，则当你的手牌数小于X时，你可以将手牌补至X张（X为5与你装备区里的牌数之差）。",
	["huwei"] = "护卫",
	[":huwei"] = "主公技，若你的装备区里没有牌，则其他将势力角色可以在其出牌阶段将一张装备牌置于你的装备区里，然后其摸一张牌。",
	["huweiv"] = "護衛",
	[":huweiv"] = "出牌阶段，可以将一张装备牌置于主公田虎的装备区里，然后其摸一张牌。",

	["$zhangheng"] = "028",
	["#zhangheng"] = "船火儿", -- min 3hp
	["zhangheng"] = "张横",
	["jielue"] = "劫掠",
	[":jielue"] = "其他角色的判定牌生效后，你可以获得其一张手牌。",
	["fuhun"] = "附魂",
	[":fuhun"] = "<font color=green><b>觉醒技</b></font>，当任一其他角色死亡时，你须回复1点体力，永久获得技能“离魂”和该角色当前的所有技能（主公技、限定技和觉醒技除外）并失去技能“劫掠”。",

	["$xiebao"] = "035",
	["#xiebao"] = "双尾蝎", -- min 4hp (cgdk)
	["xiebao"] = "解宝",
	["cv:xiebao"] = "烨子【剪刀剧团】",
	["liehuo"] = "猎获",
	[":liehuo"] = "出牌阶段，当你使用【杀】对目标角色造成伤害时或该【杀】被【闪】抵消时，若目标角色的手牌数不小于你的手牌数，则你可以获得其一张手牌。",
	["$liehuo1"] = "没想到，竟有意外收获！",
	["$liehuo2"] = "你跑不了！",
	["~xiebao"] = "哥！",

	["$shien"] = "085",
	["#shien"] = "金眼彪", -- min 3hp (qjwm)
	["shien"] = "施恩",
	["cv:shien"] = "明哲【剪刀剧团】",
	["longluo"] = "笼络",
	[":longluo"] = "当你成为一张【杀】的目标后，你可以亮出牌堆顶的两张牌，然后将其中的基本牌交给任意角色并弃掉其余的牌。",
	["$Longluo1"] = "%from 亮出的 %card 不是基本牌，自动弃置",
	["$Longluo2"] = "%from 亮出的 %card 是基本牌，可以交给任意一名角色",
	["xiaozai"] = "消灾",
	[":xiaozai"] = "当你受到伤害时，你可以交给除伤害来源外的任一其他角色两张手牌，然后将该伤害转移给该角色。",
	["@xiaozai"] = "你可以交给除你和伤害来源外的一名角色两张手牌，将该伤害转移给该角色",
	["$longluo1"] = "小小心意，不成敬意。",
	["$longluo2"] = "好汉，且慢！",
	["$xiaozai1"] = "上下使得银两，可免三百杀威棒！",
	["$xiaozai2"] = "财可通神，只当破财消灾。",
	["~shien"] = "唉，散尽钱财，亦不能避祸，都不是等闲之辈啊！",

	["$yanshun"] = "050",
	["#yanshun"] = "锦毛虎", -- kou 4hp (xzdd)
	["yanshun"] = "燕顺",
	["cv:yanshun"] = "猎狐【声声melody】",
	["coder:yanshun"] = "凌天翼",
	["huxiao"] = "虎啸",
	[":huxiao"] = "出牌阶段，你可以将你的任一装备牌当【猛虎下山】使用。",
	["$huxiao1"] = "仰天一啸，百兽惊！",
	["$huxiao2"] = "小的们，统统给我抓起来！",
	["$huxiao3"] = "谁敢擅入清风寨？",
	["~yanshun"] = "虎落平阳被犬欺啊！",

	["$wangying"] = "058",
	["#wangying"] = "矮脚虎", -- kou 3hp
	["wangying"] = "王英",
	["tanse"] = "贪色",
	[":tanse"] = "当你成为女性角色使用的非延时类锦囊的目标后，你可以选择一项：交给其一张装备牌，或获得其装备区里的一张牌。",
	["#Tanse"] = "%from 发动了【%arg】，目标是 %to，选择%arg2",
	["tan1"] = "交给其一张装备牌",
	["se2"] = "获得其装备区里的一张牌",
	["@tanse"] = "你可以交给 %src 一张装备牌",
	["houfa"] = "后发",
	[":houfa"] = "当其他角色的【杀】因弃置进入弃牌堆时，你可以获得之；你可以将两张【杀】当一张【杀】使用（该【杀】无距离限制且不能被【闪】响应）。",

	["$lizhong"] = "086",
	["#lizhong"] = "打虎将", -- kou 3/4hp (xzdd)
	["lizhong"] = "李忠",
	["cv:lizhong"] = "刺客【怀旧配音联盟】",
	["coder:lizhong"] = "凌天翼",
	["linse"] = "吝啬",
	[":linse"] = "<b>锁定技</b>，你不能成为【顺手牵羊】和【过河拆桥】的目标；你的手牌上限始终等于你的体力上限。",
	["$linse1"] = "嗯？休要打吾主意！",
	["$linse2"] = "此地无银三百两。",
	["~lizhong"] = "只可惜了俺祖传的膏药啊！",

}
