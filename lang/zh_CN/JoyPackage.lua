-- translation for JoyPackage

return {
	["joy"] = "重口味包",

	["shit"] = "屎",
	[":shit"] = "当此牌在<font color='red'><b>你的回合</b></font>内从你的<font color='red'>手牌</font>进入<font color='red'>弃牌堆</font>时，\
	你将受到自己对自己的1点伤害（黑桃为流失1点体力），其中方块为无属性伤害、梅花为雷电伤害、红桃为火焰伤害\
	造成伤害的牌为此牌，在你的回合内，你可多次食用",
	["$ShitLostHp"] = "%from 吃了 %card, 将流失1点体力",
	["$ShitDamage"] = "%from 吃了 %card, 将受到自己对自己的1点伤害",

-- equips
	["joy_equip"] = "欢乐包",

	["monkey"] = "猴子",
	[":monkey"] = "装备后，当场上有其他角色使用【桃】时，你可以弃掉【猴子】，阻止【桃】的结算并将其收为手牌",
	["grab_peach"] = "偷桃",

	["gale-shell"] = "狂风甲",
	[":gale-shell"] = "锁定技，每次受到火焰伤害时，该伤害+1；你可以将狂风甲装备和你距离为1以内的一名角色的装备区内",
	["#GaleShellDamage"] = "%from 装备【狂风甲】的负面技能被触发，由 %arg 点火焰伤害增加到 %arg2 点火焰伤害",
}
