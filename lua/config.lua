
-- this script to store the basic configuration for game program itself
-- and it is a little different from config.ini

config = {
	version = "20120931",
	version_name = "终结版F2.7",
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
--[[	"BWQZ",
		"QLFD",
		"ZCYN",
		"YBYT",
		"God",
		"Stanley",
		"InterChange",]]
		"SP",
		"Test",
--		"JoyGeneral",
	},

	scene_names = {
		"Dusong",
		"Couple",
--		"Changban",
--		"Contract",
--[[	"Zombie",
		"Legend",
		"Impasse",]]
		"Custom",
	},

	ai_names = {
		"Kenkic的机器女仆",
		"HyperX的基友",
		"天启的怨灵",
		"灵魂的⑨",
		"太阳神的苦工",
		"William大神",
		"安歧的小黑屋",
		"海泡叉",
		"歧姐家的猴子",
		"葱娘家的灵魂手办",
		"启姐家的小雏田",
		"KK家的充气女仆",
		"神上的化身",
		"donle的最后之作",
		"全宇宙最强的天霜雪舞",
		"导线的电阻",
		"QB的契约",
		"小A喜欢用的外挂",
		"吉祥物小萨",
		"Slob的杀虫剂",
		"克拉克的跑动投",
		"讨厌的核弹",
		"沾血的青苹果",
	},

	color_guan = "#547998",
	color_jiang = "#D0796C",
	color_min = "#4DB873",
	color_kou = "#8A807A",
	color_god = "#96943D",
}

for i=1, 20 do
	local scene_name = ("MiniScene_%02d"):format(i)
	table.insert(config.scene_names, scene_name)
end

