-- translation for DragonPackage
-- Fifth of the twelve

return {
	["dragon"] = "辰龙",

	["$liutang"] = "021",
	["#liutang"] = "赤发鬼", -- kou 4hp
	["liutang"] = "刘唐",
	["coder:liutang"] = "Slob",

	["$hantao"] = "042",
	["#hantao"] = "百胜将", -- guan 4hp (qjwm)
	["hantao"] = "韩滔",
	["taolue"] = "韬略",
	["cv:hantao"] = "猎狐【声声melody】",
	[":taolue"] = "出牌阶段，你可以和一名角色拼点。若你赢，你可以将该角色判定区或装备区里的一张牌移到另一个合理的位置。若你没赢，你须弃置一张牌。每回合限一次。",
	["changsheng"] = "常胜",
	[":changsheng"] = "<b>锁定技</b>，你拼点的黑桃牌点数都视为K。",
	["#Changsheng"] = "%from 的锁定技【%arg】被触发，%from 拼点牌的点数视为K",
	["$changsheng1"] = "进可攻，退可守！",
	["$changsheng2"] = "流水无形而无不形。",
	["$taolue1"] = "斩杀敌将，犹如儿戏！",
	["$taolue2"] = "人上有人，天外有天。",
	["~hantao"] = "终究还是败了。",

	["$shibao"] = "117",
	["#shibao"] = "南离大将军", -- jiang 4hp
	["shibao"] = "石宝",
	["coder:shibao"] = "Slob",

	["$ruanxiaowu"] = "029",
	["#ruanxiaowu"] = "短命二郎", -- min 3/4hp
	["ruanxiaowu"] = "阮小五",
	["anxi"] = "暗袭",
	["shuilao"] = "水牢",

	["$zhengtianshou"] = "074",
	["#zhengtianshou"] = "白面郎君", -- kou 3hp (cgdk)
	["zhengtianshou"] = "郑天寿",
	["cv:zhengtianshou"] = "烨子【剪刀剧团】",
	["coder:zhengtianshou"] = "凌天翼",
	["wugou"] = "吴钩",
	[":wugou"] = "出牌阶段，你可以将两张相同颜色的基本牌当一张【行刺】使用。",
	["qiaojiang"] = "巧匠",
	[":qiaojiang"] = "你可以将你的任一黑色锦囊牌当【杀】、红色锦囊牌当【闪】使用或打出。",
	["$wugou"] = "男儿何不带吴钩，收取关山五十州",
	["$wugou1"] = "突击！",
	["$wugou2"] = "机会来了，动手！",
	["$qiaojiang1"] = "以动制静，以静制动。",
	["$qiaojiang2"] = "全力以赴，一举拿下！",
	["~zhengtianshou"] = "有勇无谋，难以取胜！",

	["$gaoyanei"] = "159",
	["#gaoyanei"] = "花花太岁", -- guan 3hp
	["gaoyanei"] = "高衙内",

	["$shantinggui"] = "044",
	["#shantinggui"] = "圣水将", -- jiang 3hp (bwqz)
	["shantinggui"] = "单廷珪",
	["designer:shantinggui"] = "烨子&宇文天启",
	["xiaofang"] = "消防",
	[":xiaofang"] = "绝密技能，效果不详",
	["#Xiaofang"] = "%from 发动了技能【%arg】，消除了 %to 受到伤害的火焰属性",
	["shuizhan"] = "水战",
	[":shuizhan"] = "锁定技，其他角色计算相互距离时，跳过你",
	["~shantinggui"] = "土克水……吾命休矣……",

	["$lizhu"] = "123",
	["#lizhu"] = "金剑先生", -- min 3hp
	["lizhu"] = "李助",
	["coder:lizhu"] = "Slob",

	["$yangchun"] = "073",
	["#yangchun"] = "白花蛇", -- kou 4hp (ybyt)
	["yangchun"] = "杨春",
	["cv:yangchun"] = "倔强的小红军【剪刀剧团】",
	["coder:yangchun"] = "战栗贵公子",
	["shexin"] = "蛇信",
	[":shexin"] = "出牌阶段，你可以弃置一张非延时类锦囊或装备牌，展示任一其他角色的手牌并弃掉其中除基本牌外的所有牌。每回合限一次。",
	["$shexin1"] = "此毒，无药可解。",
	["$shexin2"] = "辣手摧花！",
	["~yangchun"] = "居然……比我还狠……",

	["$qiongyaonayan"] = "164",
	["#qiongyaonayan"] = "狼先锋", -- guan 4hp
	["qiongyaonayan"] = "琼妖纳延",
	["jiaozhen"] = "叫阵",

	["$suochao"] = "019",
	["#suochao"] = "急先锋", -- jiang 4hp
	["suochao"] = "索超",
	["chongfeng"] = "冲锋",

	["$wangpo"] = "147",
	["#wangpo"] = "枯藤蔓", -- min 3hp (qlfd)
	["wangpo"] = "王婆",
	["cv:wangpo"] = "九辨【重华剧社】",
	["qianxian"] = "牵线",
	[":qianxian"] = "出牌阶段，你可以弃置一张黑色非延时锦囊，指定两名体力上限不相等的其他角色。若其交给你一张梅花手牌，则将其武将牌翻至正面向上，并重置之，否则将其武将牌翻至背面向上，并横置之。每回合限一次。",
	["@qianxian"] = "%src 对你发动了【牵线】，请给她一张梅花手牌",
	["meicha"] = "梅茶",
	[":meicha"] = "你可以将任一梅花手牌当【酒】使用。",
	["$qianxian1"] = "吃个‘和合汤’如何？",
	["$qianxian2"] = "这事交给干娘我了。",
	["$meicha1"] = "我这茶别有风味。",
	["$meicha2"] = "好个“宽煎叶儿茶”。",
	["~wangpo"] = "死到眼前，犹做发财梦～",

---------
	["#mengkang"] = "玉幡竿", -- kou 4hp (bwqz)
	["mengkang"] = "孟康",
	["cv:mengkang"] = "烨子【剪刀剧团】",
	["zaochuan"] = "造船",
	[":zaochuan"] = "出牌阶段，你可以将你的任一锦囊牌当【铁索连环】使用或重铸。",
	["mengchong"] = "艨艟",
	[":mengchong"] = "<b>锁定技</b>，武将牌未处于横置状态的角色计算与武将牌处于横置状态的角色的距离时，始终+1.",
	["$zaochuan1"] = "能攀强弩冲头阵，善造艨艟越大江！",
	["$zaochuan2"] = "楼船林立，可挡朝廷千军万马！",
	["~mengkang"] = "火炮突袭，快撤！",

	["#jiaoting"] = "没面目", -- kou 4hp (bwqz)
	["jiaoting"] = "焦挺",
	["designer:jiaoting"] = "宇文天启",
	["cv:jiaoting"] = "小虎尔赤【影音同画】",
	["qinlong"] = "擒龙",
	[":qinlong"] = "若你的装备区没牌：你使用【杀】时可以额外指定一名其他角色；出牌阶段可以使用任意数量的【杀】。",
	["$qinlong1"] = "都出局吧！",
	["$qinlong2"] = "看吾三十六路擒龙手！",
	["~jiaoting"] = "绝技不复代代相传矣！",

	["#kongliang"] = "独火星", -- kou 3hp (bwqz)
	["kongliang"] = "孔亮",
	["designer:kongliang"] = "烨子&凌天翼",
	["nusha"] = "怒杀",
	["cv:kongliang"] = "烨子【剪刀剧团】",
	[":nusha"] = "出牌阶段，你可以弃置一张【杀】，对除你以外手牌数最多的一名角色造成1点伤害。每回合限一次。",
	["wanku"] = "纨绔",
	[":wanku"] = "回合结束阶段，你可以将手牌补至你当前体力值的张数。每回合限一次。",
	["$nusha1"] = "你这厮是吃了熊心豹子胆了！",
	["$nusha2"] = "家财万贯安能保住你性命？！",
	["$wanku1"] = "吾出身富家，性喜玩乐。",
	["$wanku2"] = "鼠目寸光，如何了却我的心思？！",
	["~kongliang"] = "吾不识水性啊！",

	["#wangdingliu"] = "活闪婆", -- kou 3hp (bwqz)
	["wangdingliu"] = "王定六",
	["designer:wangdingliu"] = "烨子&宇文天启",
	["cv:wangdingliu"] = "芭小乐【声声melody】",
	["kongying"] = "空影",
	[":kongying"] = "当你使用或打出一张【闪】时，（在结算前）可以令任一其他角色打出一张【闪】，若该角色无法如此做，你对其造成1点伤害。",
	["@kongying"] = "%src 要求你打出一张【闪】，否则你会受到1点伤害",
	["jibu"] = "疾步",
	[":jibu"] = "<b>锁定技</b>，当你计算与其他角色的距离时，始终-1；当其他角色计算与你的距离时，始终+1。",
	["$kongying1"] = "哈哈哈哈，扑个空吧！",
	["$kongying2"] = "此步，汝当以何抵对？",
	["~wangdingliu"] = "吾，太大意了！",
}
