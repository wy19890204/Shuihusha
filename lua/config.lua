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
		"Dragon",
--		"Snake",
		"Mustang",
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
		"Warlords",
--		"Changban",
--		"Contract",
		"Impasse",
--[[	"Zombie",
		"Legend",]]
		"Custom",
	},

	ai_names = {
		"太阳神的三国杀",
		"启姐的小雏田",
		"KenKic的充气女仆",
		"海泡叉的乱码",
		"威廉古堡",
		"逮捕麻麻的手铐",
		"安歧的小黑屋",
		"妙妙的思绪",
		"氢弹的狂风甲",
		"葱娘家的灵魂手办",
		"donle的最后之作",
		"天霜雪舞的烤萝莉",
		"七爷的觉醒",
		"中国的钓鱼岛",
		"科比挂的柯南",
		"贝克街的亡灵",
		"导线的电阻",
		"QB的契约",
		"吉祥物小萨",
		"Slob的杀虫剂",
		"克拉克的跑动投",
		"早苗的假面",
		"自来也的黄书",
		"墨韵的诅咒",
		"被和谐的XX生",
		"沾血的青苹果",
		"海南的椰子",
		"卖萌的小猫",
		"江西安义的雷海",
		"中条老道的大头贴",
		"超级塞克洛",
		"TLK的EH",
	},

	color_guan = "#547998",
	color_jiang = "#D0796C",
	color_min = "#4DB873",
	color_kou = "#8A807A",
	color_god = "#96943D",

	mini_max = 25 -- 此处以Config.S_MINI_MAX_COUNT为准
}

scenario = { -- 非身份局模式下的一些设置
	savsa_packages = { -- 3v3模式：设置加载的扩展包
		"standard",
		"rat",
	},
	dusong_packages = { -- 独松关模式：设置加载的扩展包
		"standard",
		"rat",
	},
	couple_lord = "zhoutong", -- 花田错模式：设置主公
	couple_spouse = { -- 花田错模式：设置新的配对
--		"xiezhen+xiebao",
	},
}

ban_list = { -- 初始禁表设置
	roles_ban = { -- 身份局单禁
		"gongsunsheng",
	},
	kof_ban = { -- 1v1模式
		"andaoquan",
		"shixiu",
		"zhaoji",
--		"shenwuyong",
--		"wangdingliu",
	},
	savsa_ban = { -- 3v3模式
		"yanxijiao",
	},
	basara_ban = { -- 暗将模式
		"dingdesun",
--		"houjian",
--		"shenwusong",
--		"shenwuyong",
--		"shenzhangqing",
		"lili",
	},
	hegemony_ban = { -- 国战模式（注意所有的神势力也会被禁用）
		"dingdesun",
--		"houjian",
		"lili",
		"gongsunsheng";
	},
	pairs_ban = { -- 双将
		"caijing", -- 双将全禁
		"zhangheng",
		"+tongguan", -- 副将禁用
		"+tora",
		"tianhu+songjiang", -- 特定禁用
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
		"liruilan+shijin",
--		"shenwuyong",
--		"lujunyi+shenzhangqing",
--		"luozhenren+yuehe",
--		"husanniang+jiashi",
--		"jiashi+shenzhangqing"
	},
	forbid_packages = { -- 灰色不可选的包
		"test",
	},
}

mini_max = sgs.GetConfig("S_MINI_MAX_COUNT", config.mini_max)
for i=1, mini_max do
	local scene_name = ("MiniScene_%02d"):format(i)
	table.insert(config.scene_names, scene_name)
end

