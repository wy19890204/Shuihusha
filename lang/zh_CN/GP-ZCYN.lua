-- ZhechongYannan Shuihusha part 5.

return {
	["ZCYN"] = "折冲厌难",

	["#_guansheng"] = "大刀",
	["guansheng"] = "关胜",
	["tongwu"] = "通武",
	[":tongwu"] = "当你使用的【杀】被【闪】抵消时，你可以获得该【闪】并交给除目标角色外的任一角色。",
	["~guansheng"] = "关某记下了！",

	["#_ruanxiaoer"] = "立地太岁",
	["ruanxiaoer"] = "阮小二",
	["fuji"] = "伏击",
	[":fuji"] = "其他角色的判定阶段开始时，若该角色的判定区里有牌，你可以弃置一张手牌，视为你对该角色使用了一张【行刺】（不能被【将计就计】或【无懈可击】响应）。",
	["@fuji"] = "%src 即将开始判定阶段，你可以弃一张手牌【伏击】之",
	["~ruanxiaoer"] = "斜阳影下空踏浪，休言村里一渔人。",

	["#_yangxiong"] = "病关索",
	["yangxiong"] = "杨雄",
	["designer:yangxiong"] = "裁之刃•散",
	["cv:yangxiong"] = "爪子",
	["guizi"] = "刽子",
	[":guizi"] = "每当其他角色进入濒死状态时，你可以弃置一张黑桃牌，令该角色立即死亡。若如此做，则视为你杀死该角色。",
	["#Guizi"] = "%from 发动了技能【%arg】，将 %to 拖出去宰掉了",
	["@guizi"] = "%src 正在死亡线上挣扎，你可以发动【刽子】，弃一张黑桃牌将其拖出去宰掉",
	["$guizi1"] = "午时已到，行刑！",
	["$guizi2"] = "斩！",
	["~yangxiong"] = "背疮疼痛，恨不能战死沙场～",

	["#_haosiwen"] = "井木犴",
	["haosiwen"] = "郝思文",
	["cv:haosiwen"] = "烨子",
	["sixiang"] = "四象",
	[":sixiang"] = "回合开始阶段，你可以弃置一张手牌，令至多X名角色（至少一名）依次将手牌调整至X张，X为场上现存势力数。若如此做，弃牌阶段，你至少须弃置等同于场上现存势力数的牌（不足则全弃）。每回合限一次。",
	["#SixiangBad"] = "%from 受到技能【%arg2】影响，必须至少弃掉 %arg 张牌",
	["#SixiangWorst"] = "%from 受到技能【%arg2】影响，弃掉了所有的装备和手牌（共 %arg 张）",
	["@sixiang"] = "你可以弃一张手牌发动技能【四象】",
	["$sixiang1"] = "青龙、白虎、朱雀、玄武！",
	["$sixiang2"] = "朱雀神鸟，为我先导！",
	["~haosiwen"] = "井宿八星，都陨落了。",

	["#_pengqi"] = "天目将",
	["pengqi"] = "彭玘",
	["tianyan"] = "天眼",
	[":tianyan"] = "其他角色的判定阶段开始时，若该角色的手牌数大于2，你可以观看牌堆顶的三张牌，并可以弃掉其中任意数量的牌。",

	["#_shiwengong"] = "大教师",
	["shiwengong"] = "史文恭",
	["dujian"] = "毒箭",
	[":dujian"] = "每当你使用【杀】对其他角色造成伤害时，若你不在该角色的攻击范围内，你可以防止该伤害，改为将该角色的武将牌翻面。",
	["~shiwengong"] = "非我不力，实乃毒誓害我。",

	["#_lingzhen"] = "轰天雷",
	["lingzhen"] = "凌振",
	["cv:lingzhen"] = "专业",
	["coder:lingzhen"] = "凌天翼",
	["paohong"] = "炮轰",
	[":paohong"] = "<b>锁定技</b>，你的黑色普通【杀】均视为具雷电属性的【杀】；你使用具雷电属性的【杀】时无距离限制。",
	["$paohong1"] = "一炮就送你归天！",
	["$paohong2"] = "不投降就去死吧！",
	["~lingzhen"] = "啊！我不会倒下～",

	["#_ligun"] = "飞天大圣",
	["ligun"] = "李衮",
	["hengchong"] = "横冲",
	[":hengchong"] = "当你使用的【杀】被【闪】抵消时，若你没装备武器，你可以弃置一张与该【杀】花色相同的牌，令该【杀】依然造成伤害，然后你对目标角色的上家或下家造成1点伤害。",
	["@hengchong"] = "你对 %src 使用的【杀】为 %arg 花色，可以弃置与其相同花色的手牌发动【横冲】",
	["#Hengchong"] = "%from 发动了【%arg】，弃置了 %arg2 令该【杀】依然对 %to 造成伤害",

	["caozheng"] = "曹正",
	["#_caozheng"] = "操刀鬼",
	["tuzai"] = "屠宰",
	[":tuzai"] = "你每使用【杀】对其他角色造成一次伤害，可以展示该角色的一张手牌：若该牌为红色，你弃掉该牌，然后摸一张牌。",

	["#_zourun"] = "独角龙",
	["zourun"] = "邹润",
	["longjiao"] = "龙角",
	[":longjiao"] = "每当你成为非延时类锦囊的目标时，你可以摸两张牌，然后将其中的一张置于牌堆顶。",
	["~zourun"] = "龙角已断，兄弟分离",

	["#_caifu"] = "铁臂膊",
	["caifu"] = "蔡福",
	["juesi"] = "诀死",
	[":juesi"] = "<b>锁定技</b>，每当你对其他角色造成伤害时，若该角色的当前体力值不大于1，则该伤害+1。",
	["#JuesiBuff"] = "%from 的锁定技【决死】被触发，对 %to 的伤害从 %arg 点上升至 %arg2 点",
	["~caifu"] = "最后一刀是留给我的吗",

	["#_gudasao"] = "母大虫",
	["gudasao"] = "顾大嫂",
	["cihu"] = "雌虎",
	[":cihu"] = "任一女性角色每受到一次男性角色使用【杀】造成的伤害，你可以弃置X张牌，X为该女性角色的当前体力值，对伤害来源造成1点伤害，然后令任一已受伤的女性角色回复1点体力。",
	["@cihu"] = "%src 对可怜的 %arg 造成了伤害，你可以发动【雌虎】来锄强扶弱！",
	["$cihu"] = "欺负女人, 你算什么好汉！",
	["~gudasao"] = "要记住，女人要得天下",
}

