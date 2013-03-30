-- this script to store the basic configuration for game program itself
-- and it is a little different from config.ini

config = {
	version = "20130401",
	version_name = "终结版F4.5 愚人节特供",
	mod_name = "Shuihusha",
	kingdoms = { "guan", "jiang", "min", "kou", "god"},
	package_names = {
		"StandardCard",
		"Plough",
		"Maneuvering",
		"ExCard",
		"Events",
		"Gift",
		"Kuso",
		"Joy",

		"Standard",
		"Rat",
		"Ox",
		"Tiger",
		"Hare",
		"Dragon",
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
		"Warlords",
		"ArthurFerris",
		"Contract",
		"Impasse",
--		"Changban",
--[[	"Zombie",
		"Legend",]]
		"Custom",
	},

	ai_names = {
		"太阳神的三国杀",
		"启姐的小雏田",
		"海泡叉的乱码",
		"威廉古堡",
		"葱娘家的灵魂手办",
		"donle的最后之作",
		"安歧的小黑屋",
		"战栗的乐谱",
		"中国的钓鱼岛",
		"科比挂的柯南",
		"贝克街的亡灵",
		"导线的电阻",
		"QB的契约",
		"吉祥物小萨",
		"克拉克的跑动投",
		"早苗的假面",
		"自来也的黄书",
		"墨韵的诅咒",
		"被和谐的XX生",
		"沾血的青苹果",
		"海南的椰子",
		"喵一夏的幸福",
		"单身的活宝",
		"超级无敌小柴进",
		"卖萌的豚纸",
		"江西安义的雷海",
		"中条老道的大头贴",
		"超级塞克洛",
		"TLK的EH",
		"爱上小衣的殃",
		"纠结的金田一君",
		"肉酱茧",
	},

	color_guan = "#547998",
	color_jiang = "#D0796C",
	color_min = "#4DB873",
	color_kou = "#8A807A",
	color_god = "#96943D",

	mini_max = 30 -- 此处以Config.S_MINI_MAX_COUNT为准
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
		"gongsunsheng",
		"yanxijiao",
		"zhuwu",
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
-- 1 ~ 10
--		"qiongyaonayan+gongsunsheng",
--		"qiongyaonayan+peixuan",
--		"qiongyaonayan+luozhenren",
--		"qiongyaonayan+tianhu",
--		"yangchun+zhugui",
--		"suochao+luzhishen",
--		"suochao+daizong",
--		"suochao+luda",
--		"yanqing+zhoutong",
		"tianhu+songjiang",
-- 11 ~ 20
		"suochao+lujunyi",
--		"tianhu+ruanxiaowu",
		"shijin+ruanxiaowu",
--		"yangzhi+shijin",
--		"wuyong+weidingguo",
		"likui+peixuan",
		"lujunyi+yanqing",
--		"daizong+lishishi",
		"wuyong+shiqian",
		"peixuan+gaoqiu",
-- 21 ~ 30
		"peixuan+qiaodaoqing",
--		"fangla+sunerniang",
		"wangpo+lujunyi",
--		"lujunyi+zhoutong",
		"lujunyi+gaoyanei",
--		"suochao+pangwanchun",
--		"suochao+yanshun",
		"wangying+zhengtianshou",
--		"wangying+zhangqing",
--		"wangying+qiongying",
-- 31 ~ 40
--		"tianhu+yanshun",
		"husanniang+yanshun",
--		"husanniang+zhaoji",
--		"qingzhang+sunerniang",
--		"shien+andaoquan",
		"yanxijiao+guansheng",
--		"lujunyi+shien",
--		"likui+luozhenren",
--		"dingdesun+wangqing",
		"shijin+yanshun",
-- 41 ~ 50
		"shijin+leiheng",
		"zhaoji+pangwanchun",
		"zhaoji+andaoquan",
--		"pangwanchun+tora",
		"tongguan+suochao",

		"fangjie+songjiang",
		"liruilan+shijin",
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

