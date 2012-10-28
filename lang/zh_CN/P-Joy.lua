-- translation for JoyPackage

return {
	["kuso"] = "重口味包",

	["shit"] = "屎",
	[":shit"] = "当此牌在<font color='red'><b>你的回合</b></font>内从你的<font color='red'>手牌</font>进入<font color='red'>弃牌堆</font>时，\
	你将受到自己对自己的1点伤害（黑桃为流失1点体力），其中方块为无属性伤害、梅花为雷电伤害、红桃为火焰伤害\
	造成伤害的牌为此牌，在你的回合内，你可多次食用",
	["$ShitLostHp"] = "%from 吃了 %card, 将流失1点体力",
	["$ShitDamage"] = "%from 吃了 %card, 将受到自己对自己的1点伤害",

	["stink"] = "屁",
	[":stink"] = "出牌阶段对下家使用，除非目标角色打出一张【闪】或【行刺】，否则必须和自己的下家（你的下下家）交换位置；若目标打出了【行刺】，你受到其造成的一点伤害",
	["@haochou"] = "你的上家（%src）对你放了一个臭屁，你可以【闪】开，也可以用【行刺】千年杀之~ 否则，嘿嘿……",
	["#StinkSuccess"] = "实在是太臭了，%from 赶紧跑到了 %to 的后面",
	["#StinkHit"] = "%from 听声辨味忍无可忍，对 %to 使用了【行刺】，只听 %to 一声惨叫",
	["#StinkJink"] = "%from 察觉不对，打出了【闪】，逃之夭夭",

-- equips
	["joy"] = "欢乐包",

	["gale-shell"] = "狂风甲",
	[":gale-shell"] = "锁定技，每次受到火焰伤害时，该伤害+1；你可以将狂风甲装备和你距离为1以内的一名角色的装备区内",
	["#GaleShellDamage"] = "%from 装备【狂风甲】的负面技能被触发，由 %arg 点火焰伤害增加到 %arg2 点火焰伤害",

	["poison"] = "毒",
	[":poison"] = "出牌阶段，对和你距离为1以内的一名角色使用，令其进入中毒状态或解除中毒状态。当回复体力时，有20%的几率解除中毒状态。进入中毒状态的角色有以下负面效果：<br/>1、回合开始时随机丢失自己区域内的一张牌<br/>2、使用酒时失去一点体力",
	["#Poison_in"] = "%from 不幸中毒，脸色青紫，看起来情况不是很好",
	["#Poison_out"] = "%from 解毒成功，身体恢复了健康",
	["$Poison_lost"] = "%from 毒发，剧痛难忍，丢失了 %card",
	["#Poison_ana"] = "%from 毒发！剧痛难忍，将自己抓出了条条血痕",

-- zoo
	["zoo"] = "动物园",

	["monkey"] = "猴子",
	[":monkey"] = "装备后，当场上有其他角色使用【桃】时，你可以弃掉【猴子】，阻止【桃】的结算并将其收为手牌",
	["grab_peach"] = "偷桃",

-- generals
	["joyer"] = "满头包",

	["#maque"] = "国粹精品", -- hp12
	["maque"] = "麻雀",
	["designer:maque"] = "宇文天启",
	["cv:maque"] = "",
	["timer"] = "计时",
	[":timer"] = "锁定技，回合开始时，你失去1点体力上限。",
	["lingyu"] = "领域",
	[":lingyu"] = "锁定技，分发起始手牌时，共发你13张牌作为手牌。任何时候，你的牌（包括“碰”牌）只能有13张，除非满足游戏结束条件。",
	["zhuangche"] = "撞车",
	[":zhuangche"] = "若其他角色非弃牌阶段进入弃牌堆的牌正好和你手中对子的点数相同，你可以将其凑成一副置于你的武将牌上，称为“副”。\
★对子：两张相同点数的牌",
	["@zhuangche"] = "%src 的 %arg 进入弃牌堆，点数为 %dest，你可以发动【撞车】。",
	["zouma"] = "走马",
	[":zouma"] = "锁定技，你的判定、出牌、弃牌阶段自动跳过。摸牌阶段，你只能摸一张牌，之后须将满足一副规则的牌作为“副”置于你的武将牌上。\
★一副规则：三张点数相同或点数连续的牌",
	["fu1"] = "一副",
	["fu2"] = "二副",
	["fu3"] = "三副",
	["fu4"] = "四副",
	["@zouma"] = "你须将满足一副规则的牌作为“副”置于你的武将牌上。",
	["jizha"] = "叽喳",
	[":jizha"] = "锁定技，当你只有一个对子，其余全是“副”牌的时候（此时总牌数为14张），游戏结束。你所在的阵营获得游戏胜利。",
--	[":jizha"] = "锁定技，当你有七个对子，或只有一个对子，其余全是“一副”牌的时候（此时总牌数为14张），游戏结束。你所在的阵营获得游戏胜利。",

	["#chuiniu"] = "酒场饭局",
	["chuiniu"] = "吹牛",
	["designer:chuiniu"] = "宇文天启",
	["cv:chuiniu"] = "",
	[":chuiniu"] = "出牌阶段，你可以选择一名手牌数比你少的其他角色，执行以下操作：将你们的手牌移出游戏，分别摸牌，摸到牌的点数必须小于7，否则弃置，直到摸够三张，然后从你开始，说出类似“X个Y”这样的话，对方可以说出更大的数目或弃权，当某方弃权时，亮明双方手牌，若其牌数和点数符合最后说话者的结论，则算其获胜，否则算失败。若你获胜，弃掉双方移出游戏的牌，获得双方的手牌（一共六张），若你失败，则对方弃掉全部手牌，获得自己移出游戏的牌和你移出游戏的牌。每回合限一次。\
★吹牛的具体规则参见同名骰子游戏",
	["niu"] = "牛",
	["chuiniu_count"] = "我有X个Y，X=？",
	["chuiniu_num"] = "我有X个Y，Y=？",
	["chuiniu_count:pass"] = "放弃",
	["#Chuiniuing"] = "%from 说：“我有%arg个%arg2！”",
	["#ChuiniuEnd"] = "吹牛结束，统计结果为：%arg个%arg2",
	["#ChuiniuWin"] = "%from 在吹牛中胜出！",
	["~chuiniu"] = "再喝完这一杯……还有……三杯……",
}
