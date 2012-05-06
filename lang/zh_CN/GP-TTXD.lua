-- TitianXingDao Shuihusha part 3.

local tt = {
	["TTXD"] = "替天行道",
	["coder:TTXD"] = "roxiel",

	["#zhangqing"] = "没羽箭",
	["zhangqing"] = "张清",
	["cv:zhangqing"] = "烨子【剪刀剧团】",
	["yinyu"] = "饮羽",
	[":yinyu"] = "回合开始阶段，你可以进行一次判定，获得与判定结果对应的一项技能直到回合结束：\
	红桃：攻击范围无限；\
	方块：使用的【杀】不可被闪避；\
	黑桃：可使用任意数量的【杀】；\
	梅花：无视其他角色的防具。",
	["#Yinyu1"] = "%from 本回合攻击范围无限",
	["#Yinyu2"] = "%from 本回合使用的【杀】不可被闪避",
	["#Yinyu4"] = "%from 本回合可以使用任意数量的【杀】",
	["#Yinyu8"] = "%from 本回合无视其他角色的防具",
	["$yinyu1"] = "飞蝗如雨，看尔等翻成画饼！",
	["$yinyu2"] = "飞石连伤，休想逃跑！",
	["$yinyu3"] = "叫汝等饮羽沙场吧！",
	["$yinyu4"] = "此等破铜烂铁岂能挡我！",
	["$yinyu5"] = "看你马快，还是我飞石快！",

	["#yuehe"] = "铁叫子",
	["yuehe"] = "乐和",
	["cv:yuehe"] = "烨子【剪刀剧团】",
	["yueli"] = "乐理",
	[":yueli"] = "若你的判定牌为基本牌，在其生效后可以获得之。",
	["yueli:yes"] = "拿屎",
	["yueli:no"] = "不拿屎",
	["taohui"] = "韬晦",
	[":taohui"] = "回合结束阶段，你可以进行一次判定：若结果不为基本牌，你可以令任一角色摸一张牌，并可以再次使用“韬晦”，如此反复，直到判定结果为基本牌为止。",
	["$yueli1"] = "呵呵～",
	["$yueli2"] = "且慢，音律有误。",
	["$taohui1"] = "白云起，郁披香；离复合，曲未央。",
	["$taohui2"] = "此曲只应天上有，人间能得几回闻。",

	["#muhong"] = "没遮拦",
	["muhong"] = "穆弘",
	["cv:muhong"] = "流岚【裔美声社】",
	["wuzu"] = "无阻",
	[":wuzu"] = "<b>锁定技</b>，你始终无视其他角色的防具。",
	["$IgnoreArmor"] = "%to 装备着 %card，但 %from 貌似没有看见",
	["huqi"] = "虎骑",
	[":huqi"] = "<b>锁定技</b>，当你计算与其他角色的距离时，始终-1.",
	["$wuzu1"] = "谁敢拦我？",
	["$wuzu2"] = "游击部，冲！",

	["#zhoutong"] = "小霸王",
	["zhoutong"] = "周通",
	["cv:zhoutong"] = "烨子【剪刀剧团】",
	["qiangqu"] = "强娶",
	[":qiangqu"] = "当你使用【杀】对已受伤的女性角色造成伤害时，你可以防止此伤害，改为获得该角色的一张牌，然后你和她各回复1点体力。",
	["#Qiangqu"] = "%from 硬是把 %to 拉入了洞房",
	["huatian"] = "花田",
	[":huatian"] = "你每受到1点伤害，可以令任一已受伤的其他角色回复1点体力；你每回复1点体力，可以对任一其他角色造成1点伤害。",
	["$qiangqu1"] = "小娘子，春宵一刻值千金啊！",
	["$qiangqu2"] = "今夜，本大王定要做新郎！",
	["$huatian1"] = "无妨，只当为汝披嫁纱！",
	["$huatian2"] = "只要娘子开心，怎样都好！",
	["$huatian3"] = "破晓之前，忘了此错。",
	["$huatian4"] = "无心插柳，岂是花田之错？",

	["#qiaodaoqing"] = "幻魔君",
	["qiaodaoqing"] = "乔道清",
	["cv:qiaodaoqing"] = "烨子【剪刀剧团】",
	["huanshu"] = "幻术",
	[":huanshu"] = "你每受到1点伤害，可以令任一其他角色连续进行两次判定：若均为红色，你对其造成2点火焰伤害；若均为黑色，你对其造成2点雷电伤害。",
	["@huanshu"] = "请指定一个目标以便于发动【幻术】",
	["huanshu1"] = "幻术·第一次判定",
	["huanshu2"] = "幻术·第二次判定",
	["mozhang"] = "魔障",
	[":mozhang"] = "<b>锁定技</b>，你的回合结束时，若你未处于横置状态，你须横置你的武将牌。",
	["#Mozhang"] = "%from 的锁定技【%arg】被触发，将自己的武将牌横置",
	["$huanshu1"] = "沙石一起，真假莫辨！",
	["$huanshu2"] = "五行幻化，破！",
	["$huanshu3"] = "五雷天心，五雷天心，缘何不灵？",
	["$mozhang"] = "外道之法，也可乱心？",

-- last words
	["~zhangqing"] = "一技之长，不足傍身啊！",
	["~yuehe"] = "叫子也难吹奏了。",
	["~muhong"] = "弟，兄先去矣！",
	["~zhoutong"] = "虽有霸王相，奈无霸王功啊！",
	["~qiaodaoqing"] = "这，就是五雷轰顶的滋味吗？",
}

local gege = {"lujunyi", "zhangqing", "yuehe", "muhong", "zhoutong",
		"qiaodaoqing", "andaoquan", "gongsunsheng", "husanniang"}

for _, player in ipairs(gege) do
	tt["coder:" .. player] = tt["coder:TTXD"]
end

return tt
