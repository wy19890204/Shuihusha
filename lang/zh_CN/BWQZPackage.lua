-- TitianXingDao Shuihusha part 4.

return {
	["BWQZ"] = "博闻强识",

	["#_houjian"] = "通臂猿",
	["houjian"] = "侯健",
	["designer:houjian"] = "宇文天启",
	["yuanyin"] = "援引",
	[":yuanyin"] = "你可以将其他角色装备区里的武器当【杀】、非武器当【闪】使用或打出",
	["yuanyin:slash"] = "你想发动技能【援引·杀】吗？",
	["yuanyin:jink"] = "你想发动技能【援引·闪】吗？",

	["#_mengkang"] = "玉幡竿",
	["mengkang"] = "孟康",
	["zaochuan"] = "造船",
	[":zaochuan"] = "出牌阶段，你可以将你的任一锦囊牌当【铁索连环】使用或重铸。",
	["mengchong"] = "艨艟",
	[":mengchong"] = "<b>锁定技</b>，武将牌未处于横置状态的角色计算与武将牌处于横置状态的角色的距离时，始终+1。",
	["$zaochuan1"] = "能攀强弩冲头阵，善造艨艟越大江。",
	["$zaochuan2"] = "楼船林立，可挡朝廷千军万马。",

	["#_jiaoting"] = "没面目",
	["jiaoting"] = "焦挺",
	["designer:jiaoting"] = "宇文天启",
	["qinlong"] = "擒龙",
	[":qinlong"] = "若你的装备区没牌：你使用【杀】时可以额外指定一名其他角色；出牌阶段可以使用任意数量的【杀】",
	["$qinlong1"] = "都出局吧！",
	["$qinlong2"] = "三十六路擒龙手！",

	["#_shantinggui"] = "圣水将",
	["shantinggui"] = "单廷珪",
	["designer:shantinggui"] = "宇文天启",
	["xiaofang"] = "消防",
	[":xiaofang"] = "当场上出现火焰伤害时，你可以弃掉一张手牌，将其改为无属性伤害",
	["#Xiaofang"] = "%from 发动了技能【%arg】，消除了 %to 受到伤害的火焰属性",

	["#_qingzhang"] = "张青",
	["qingzhang"] = "菜园子",
	["shouge"] = "收割",
	[":shouge"] = "出牌阶段，你可以将一张【桃】或【酒】置于你的武将牌上，称为“菜”；你的回合外，你每失去一张手牌或流失1点体力，可以将一张“菜”移入弃牌堆，然后摸三张牌。",
	["vege"] = "菜",
	["qiongtu"] = "穷途",
	[":qiongtu"] = "其他角色的回合结束时，若该角色的手牌数不大于1，你可以获得该角色的一张牌。",
	["$shouge1"] = "没有耕耘，哪来收获？",
	["$shouge2"] = "一粒种子，就是一个春天。",
	["$shouge3"] = "又是一年秋收时节。",
	["$shouge4"] = "好一片麦田！", 
	["$qiongtu1"] = "哼，你这穷鬼，还要这些作甚？",
	["$qiongtu2"] = "这里就是张家店，客官，里边请！",

	["#_jiashi"] = "毒蔷薇",
	["jiashi"] = "贾氏",
	["cv:jiashi"] = "呼呼",
	["coder:jiashi"] = "凌天翼",
	["banzhuang"] = "半妆",
	[":banzhuang"] = "出牌阶段，你可以将你的任一红桃手牌当【无中生有】使用",
	["$banzhuang1"] = "一顾倾人城，再顾倾人国。",
	["$banzhuang2"] = "虚事难入公门，实事难以抵对。",
	["zhuying"] = "朱樱",
	[":zhuying"] = "<b>锁定技</b>，你的【酒】均视为【桃】。",
	["$zhuying1"] = "奴家这厢有礼了～",
	["$zhuying2"] = "奴家不胜酒力，浅饮一杯，聊表敬意。",

-- last words
	["~mengkang"] = "火炮突袭，快撤！",
	["~jiaoting"] = "绝技不复代代相传矣！",
	["~jiashi"] = "员外，饶了奴家吧～",
	["~qingzhang"] = "日头落了。",

--
	["gongsunzan"] = "公孙瓒", 
	["yicong"] = "义从", 
	[":yicong"] = "锁定技，只要你的体力值大于2点，你计算与其他角色的距离时，始终-1；只要你的体力值为2点或更低，其他角色计算与你的距离时，始终+1。",
	
	["yuanshu"] = "袁术",
	["yongsi"] = "庸肆",
	[":yongsi"] = "锁定技，摸牌阶段，你额外摸X张牌，X为场上现存势力数。弃牌阶段，你至少弃掉等同于场上现存势力数的牌（不足则全弃）",
	["weidi"] = "伪帝",
	[":weidi"] = "锁定技，你视为拥有当前主公的主公技。",
	
	["#YongsiGood"] = "%from 的锁定技【庸肆】被触发，额外摸了 %arg 张牌",
	["#YongsiBad"] = "%from 的锁定技【庸肆】被触发，必须至少弃掉 %arg 张牌",
	["#YongsiWorst"] = "%from 的锁定技【庸肆】被触发，弃掉了所有的装备和手牌（共 %arg 张）",
	
	["taichen"] = "抬榇",
	[":taichen"] = "出牌阶段，你可以自减1点体力或弃一张武器牌，弃掉你攻击范围内的一名角色处（手牌、装备区、判定区）的两张牌，每回合中，你可以多次使用抬榇",
	["$taichen"] = "良将不惧死以苟免，烈士不毁节以求生",
	["~sp_pangde"] = "吾宁死于刀下，岂降汝乎",
	
	["cv:gongsunzan"] = "",
	["cv:yuanshu"] = "名将三国",
	["cv:sp_sunshangxiang"] = "",
	["cv:sp_diaochan"] = "",
	["cv:sp_pangde"] = "Glory",
	
--sp_card	
	["sp_cards"] = "SP卡牌包",
	["sp_moonspear"] = "SP银月枪", 
	[":sp_moonspear"] = "你的回合外，若打出了一张黑色的牌，你可以立即指定攻击范围内的一名角色打出一张【闪】，否则受到你对其造成的1点伤害", 
	["@moon-spear-jink"] = "受到SP银月枪技能的影响，你必须打出一张【闪】", 
}
