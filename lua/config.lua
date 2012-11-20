-- this script to store the basic configuration for game program itself
-- and it is a little different from config.ini

config = {
	version = "20121119",
	version_name = "终结版F3.5", -- Shuihusha's Anniversary
	mod_name = "Shuihusha",
	kingdoms = { "guan", "jiang", "min", "kou", "god"},
	package_names = {
		"StandardCard",
		"Plough",
		"Maneuvering",
		"ExCard",
		"Events",
		"Gift",
--		"Kuso",
--		"Joy",

		"Standard",
		"Rat",
		"Ox",
		"Tiger",
		"Hare",
--		"Dragon",
--		"Snake",
--		"Mustang",
--		"Sheep",
--		"Monkey",
--		"Cock",
--		"Boar",
		"SP",
		"Test",
--		"JoyGeneral",
	},

	scene_names = {
		"Dusong",
		"Couple",
		"Landlord",
		"WheelFight",
--		"Warlords",
--		"Changban",
--		"Contract",
--[[	"Zombie",
		"Legend",
		"Impasse",]]
		"Custom",
	},

	ai_names = {
		"KenKic的机器女仆",
		"HyperX的基友",
		"天启的怨灵",
		"逮捕麻麻的手铐",
		"太阳神的苦工",
		"威廉古堡",
		"安歧的小黑屋",
		"海泡叉的乱码",
		"妙妙的思绪",
		"氢弹的狂风甲",
		"歧姐家的猴子",
		"葱娘家的灵魂手办",
		"启姐家的小雏田",
		"KK家的充气女仆",
		"中国的钓鱼岛",
		"donle的最后之作",
		"天霜雪舞的烤萝莉",
		"七爷的觉醒",
		"科比挂的柯南",
		"贝克街的亡灵",
		"导线的电阻",
		"QB的契约",
		"小A喜欢用的外挂",
		"吉祥物小萨",
		"Slob的杀虫剂",
		"克拉克的跑动投",
		"讨厌的核弹",
		"早苗的假面",
		"Lisar的凯子",
		"自来也的黄书",
		"墨韵的诅咒",
		"被和谐的XX生",
		"沾血的青苹果",
		"海南的椰子",
		"卖萌的小猫",
		"江西安义的雷海",
		"中条老道的大头贴",
		"超级塞克洛",
	},

	color_guan = "#547998",
	color_jiang = "#D0796C",
	color_min = "#4DB873",
	color_kou = "#8A807A",
	color_god = "#96943D",

	mini_max = 25
}

scenario = {
	savsa_packages = {
		"standard",
		"rat",
	},
	dusong_packages = {
		"standard",
		"rat",
	},
	couple_lord = "zhoutong",
	couple_spouse = {
--		"xiezhen+xiebao",
	},
}

ban_list = {
	roles_ban = {
		"gongsunsheng",
	},
	kof_ban = {
		"andaoquan",
		"shixiu",
		"zhaoji",
--		"shenwuyong",
--		"wangdingliu",
	},
	savsa_ban = {
		"yanxijiao",
	},
	basara_ban = {
		"dingdesun",
--		"houjian",
--		"shenwusong",
--		"shenwuyong",
--		"shenzhangqing",
		"lili",
	},
	hegemony_ban = {
		"dingdesun",
--		"houjian",
		"lili",
		"gongsunsheng";
	},
	pairs_ban = {
		"caijing",
		"zhangheng",
		"+tongguan",
		"+tora",
		"tianhu+songjiang",
		"gaoqiu+luozhenren",
		"wangying+zhangqing",
		"wangying+qiongying",
		"tianhu+yanshun",
		"husanniang+yanshun",
		"husanniang+zhaoji",
		"qingzhang+sunerniang",
		"shien+andaoquan",
		"yanxijiao+guansheng",
		"lujunyi+shien",
		"likui+luozhenren",
		"dingdesun+wangqing",
		"shijin+yanshun",
		"shijin+leiheng",
		"zhaoji+pangwanchun",
		"zhaoji+andaoquan",
		"fangjie+songjiang",
--		"shenwuyong",
--		"liruilan+shijin",
--		"lujunyi+shenzhangqing",
--		"luozhenren+yuehe",
--		"husanniang+jiashi",
--		"oupeng+wangqing",
--		"jiashi+shenzhangqing"
	},
	forbid_packages = {
		"test",
	},
}

mini_max = sgs.GetConfig("S_MINI_MAX_COUNT", config.mini_max)
for i=1, mini_max do
	local scene_name = ("MiniScene_%02d"):format(i)
	table.insert(config.scene_names, scene_name)
end

