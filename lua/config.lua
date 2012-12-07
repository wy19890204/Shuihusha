
-- this script to store the basic configuration for game program itself
-- and it is a little different from config.ini

config = {
	version = "20121205",
	version_name = "百步飞剑①",
	mod_name = "QinsMoon",
	kingdoms = { "di", "xia", "wang", "free", "god"},
	package_names = {

	"StandardCard",
	"Maneuvering",
	"Plough",
--	"ExCard",
--	"Gift",
--	"Kuso",
--	"Joy",

	"Standard",
--	"Rat",
--	"Ox",
--	"Tiger",
--	"Hare",
--	"SP",
	"Test",

--	"JoyGeneral",
	},

	scene_names = {
--[[	"Impasse",
	"Couple",
	"Contract",
	"Zombie",
	"Legend",
	"Custom",]]
	},

	color_di = "#547998",
	color_xia = "#D0796C",
	color_wang = "#4DB873",
	color_free = "#8A807A",
	color_god = "#96943D",
}

--[[
for i=1, 20 do
	local scene_name = ("MiniScene_%02d"):format(i)
	table.insert(config.scene_names, scene_name)
end]]

