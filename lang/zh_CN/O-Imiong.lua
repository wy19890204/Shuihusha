-- translation for Imiong

return {
	["imiong"] = "阴阳",
	["conjuring"] = "咒术",
	
	["moon"] = "阴",
	["sun"] = "阳",
	["multiply"] = "×",

	["poison_jur"] = "中毒",
	[":poison_jur"] = "目标角色回合开始时，随机弃置一张牌；第4回合转变为“剧毒”（目标角色回合开始时，该角色需选择一项：随机弃置两张牌，或失去1点体力）。共持续5个回合。",
	["sleep_jur"] = "昏睡",
	[":sleep_jur"] = "目标角色于其回合外不能使用或打出牌，直到其受到一次伤害。共持续2个回合。",
	["dizzy_jur"] = "晕眩",
	[":dizzy_jur"] = "目标角色不能发动其当前的所有技能，直到其进入濒死状态。共持续2个回合。",
	["petro_jur"] = "石化",
	["chaos_jur"] = "混乱",
	["weak_jur"] = "虚弱",
	["mildew_jur"] = "霉运",
	["reflex_jur"] = "反弹",
	["cure_jur"] = "回复",
	["revolt_jur"] = "抵抗",
	[":revolt_jur"] = "目标角色附加所有咒术状态的几率-50%。共持续5个回合。",
	["violent_jur"] = "狂暴",
	["lucky_jur"] = "幸运",
	[":lucky_jur"] = "目标角色摸牌阶段，该角色有75%的几率额外摸一张牌。共持续2个回合。",
	["invalid_jur"] = "无效",

	["#GainJur"] = "%from 进入了 %arg 状态",
	["#RemoveJur"] = "%from 解除了 %arg 状态",
	["#Poison"] = "%from 毒发！剧痛难忍，将自己抓出了条条血痕",
	["poison_jur:cd"] = "丢两张牌",
	["poison_jur:hp"] = "掉一点体力",
	["#Sleep"] = "%from 在昏睡，无法使用或打出牌！",
}
