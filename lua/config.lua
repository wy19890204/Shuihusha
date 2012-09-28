
-- this script to store the basic configuration for game program itself
-- and it is a little different from config.ini

config = {
	version = "20120929",
	version_name = "终结版F2.4ζ",
	mod_name = "Shuihusha",
	kingdoms = { "guan", "jiang", "min", "kou", "god"},
	package_names = {

	"StandardCard",
	"Plough",
	"Maneuvering",
	"ExCard",
	"Events",
	"Gift",
--	"Kuso",
--	"Joy",

	"Standard",
	"Rat",
	"Ox",
	"Tiger",
--	"Hare",
	"SP",
	"Test",

--	"JoyGeneral",
	},

	scene_names = {
	"Dusong",
	"Couple",
--	"Changban",
	"Custom",
	},

	color_guan = "#547998",
	color_jiang = "#D0796C",
	color_min = "#4DB873",
	color_kou = "#8A807A",
	color_god = "#96943D",
}

for i=1, 5 do
	local scene_name = ("MiniScene_%02d"):format(i)
	table.insert(config.scene_names, scene_name)
end

