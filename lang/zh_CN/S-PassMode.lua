local pass_t = {
	["pass_mode"] = "闯关模式",
-- basic
	["pass"] = "闯关",
	["study"] = "请选择要学习的技能",
	["@exp"] = "经验",
	["exp"] = "经验值",
	["damage"] = "伤害",
--kingdom
	["evil"] = "炮灰",
	["evil_god"] = "猛将",
--louluo
	["#bubing_e"] = "",
	["bubing_e"] = "步兵",
	["shiqi_p"] = "士气",
	[":shiqi_p"] = "摸牌阶段开始时，你可以进行一次判定，若结果为红色你获得此牌。",

	["#nubing_e"] = "",
	["nubing_e"] = "弩兵",
	["qianggong_p"] = "强弓",
	[":qianggong_p"] = "当你使用【杀】指定一名角色为目标后，以下两种情况，你可以令此【杀】不可被【闪】响应：1、你的体力值小于或等于1。2、你的攻击范围大于4。",

	["#jianshi_e"] = "",
	["jianshi_e"] = "剑侍",
	["pojia_p"] = "破甲",
	[":pojia_p"] = "锁定技，你使用的黑色【杀】无视目标防具；红色【杀】命中时，随机弃置目标装备区的一张牌。",
	["zhanshang_p"] = "战觞",
	[":zhanshang_p"] = "当你成为【猛虎下山】或【万箭齐发】的目标时，可以摸一张牌。",

	["#qibing_e"] = "",
	["qibing_e"] = "骑兵",
	["qishu_p"] = "骑术",
	[":qishu_p"] = "锁定技，当你计算与其他角色的距离时，始终-1，当其他角色计算与你的距离时，始终+1（已装备的马匹无效果）。",
	["xunma_p"] = "驯马",
	[":xunma_p"] = "出牌阶段，你可以弃置一张马牌，然后摸两张牌并回复1点体力。",
	["xunmapass"] = "驯马",

	["#shoujiang_e"] = "",
	["shoujiang_e"] = "守将",
	["chenwen_p"] = "坚守",
	[":chenwen_p"] = "锁定技，当你没装备防具时，梅花的【杀】对你无效。",
	["zhongzhuang_p"] = "重装",
	[":zhongzhuang_p"] = "锁定技，当你受到一次伤害时，伤害值最多为1点。",

	["#paobing_e"] = "",
	["paobing_e"] = "炮兵",
	["dianji_p"] = "雷击",
	[":dianji_p"] = "出牌阶段，你可以弃置一张黑桃手牌，令一名角色进行判定，若结果为黑色，你对该角色造成1点雷电伤害。每阶段限一次。",
	["leiti_p"] = "避雷",
	[":leiti_p"] = "锁定技，当你受到一次雷电伤害前，摸X张牌（X为伤害点数），然后伤害为0。",

	["#kuangdaoke_e"] = "",
	["kuangdaoke_e"] = "狂刀客",
	["lianzhan_p"] = "连斩",
	[":lianzhan_p"] = "每当你使用的【杀】被目标的【闪】抵消时，你可以再对其使用一张【杀】，若此【杀】命中则伤害+1。",
	["douzhi_p"] = "斗志",
	[":douzhi_p"] = "回合结束时，你可以将手牌补至体力上限。",
--exp
	["nuhou_p"] = "怒吼",
	[":nuhou_p"] = "锁定技，每回合出杀次数+1。",
	["fenjin_p"] = "奋进",
	[":fenjin_p"] = "锁定技，每关开始时和击败敌人后额外摸一张牌。",
	["kezhi_p"] = "克制",
	[":kezhi_p"] = "锁定技，手牌上限+1。",
	["tipo_p"] = "体魄",
	[":tipo_p"] = "锁定技，体力上限+1。",
	["duanyan_p"] = "断言",
	[":duanyan_p"] = "出牌阶段，你可以选择一种类型并展示一名其他角色的1张手牌，若猜对则弃置此牌，否则你须弃置一张牌或摸一张牌并受到其对你造成的1点伤害，每阶段限一次。",
	["duanyan_p:slash"] = "杀",
	["duanyan_p:jink"] = "闪",
	["duanyan_p:peach_analeptic"] = "桃/酒",
	["duanyan_p:other"] = "其他",
	["xiongzi_p"] = "雄姿",
	[":xiongzi_p"] = "回合开始阶段开始时，你可以摸一张牌。",
	["quanheng_p"] = "权衡",
	[":quanheng_p"] = "出牌阶段，可以丢弃X张手牌，然后从牌堆摸取等量的牌（X为已损失的体力值且最大为4）。每阶段限一次。",
	["quanhengpass"] = "权衡",
	["qiangong_p"] = "谦恭",
	[":qiangong_p"] = "当你成为【过河拆桥】或【顺手牵羊】的目标时，可以选择自己的一张牌作为对象牌，成为【过河拆桥】的目标时，若装备区不为空选择的牌要交给对方。",
	["@qiangong_p-card"] = "请选择1张牌",
	["@xiuluo"] = "请弃置一张符合条件的手牌",
--system
	["savefile"] = "存档",
	["read"] = "读取存档",
	["deletesave"] = "删除存档",
	["cancel"] = "取消",
	["save"] = "保存",
	["notsave"] = "不保存",
	["wrong_skills"] = "数据错误！存在不合法的技能（可能由于version不同造成）",
	["wrond_exp"] = "数据错误！不合法的经验值",
	["wrong_hp"] = "数据错误！不合法的体力上限",

	["#TriggerDrawSkill"] = "%from 的【%arg】效果被触发，从牌堆摸了 %arg2 张牌",
	["#TriggerDamageUpSkill"] = "%from 的【%arg】效果被触发，对 %to 的伤害上升至 %arg2 点",
	["#TriggerDamageDownSkill"] = "%from 的【%arg】效果被触发，受到的伤害下降至 %arg2 点",
	
	["#DuanYanPassChooseType"] = "%from 选择了 %arg ，随机展示 %to 手中的1张牌",
	["#QiangongPassThrow"] = "%from 的【谦恭】效果被触发，弃置了1张牌以防止【%arg】的效果",
	["#QiangongPassGive"] = "%from 的【谦恭】效果被触发，将1张牌交给了 %to 以防止【%arg】的效果",
	["#JijiangPassLoseHp"] = "%from 的【激将】效果触发，%from 流失了1点体力",
	["#ZhaoliePass"] = "%from 使用了【昭烈】技能，舍弃 %arg 个仁义标记，回复1点体力后开始了一个新的回合",
	["#BaonuPass"] = "%from 的【暴怒】效果被触发，从牌堆摸了1张牌",
	["#JianhunPass1"] = "%from 使用了【箭魂】技能，对 %to 使用了 %arg 次相同的【杀】",
	["#JianhunPass2"] = "%from 使用了【箭魂】技能，对 %to 使用了 1 张相同的【杀】",
	
	["#XietianPass"] = "%from 的【挟天】效果被触发，获得了 %to 的1张手牌",
	["#Guimou"] = "由于 %from 的判定，%to 的【鬼谋】效果被触发，从牌堆摸了 1 张牌",
	["#QuwuPass"] = "%from 的【曲误】效果被触发 弃置了一张装备区的牌使伤害-1",
	["#JinguoPass"] = "%from 的【巾帼】效果被触发，将牌堆顶的牌加入手中",
	["#ZhanshangPass"] = "%from 的技能【战觞】效果被触发，从牌堆摸了1张牌",
	["#ZhongzhuangPass"] = "%from 的技能【重装】防止了 %arg 点伤害，锁定为 1 点",
	["#Jitian"] = "%from 的技能【祭天】防止了雷电属性伤害并摸了等同于伤害值的手牌数",
	["#WuhunPassThrow"] = "由于 %from 受到 %to 的伤害，技能【武魂】效果被触发，%to 随机丢弃了一张黑色手牌",
	["#WuhunPassThrowDamage"] = "由于 %from 受到 %to 的伤害，技能【武魂】效果被触发，%to 随机丢弃了一张黑色手牌并受到来源为自己的1点伤害",
	["#WuhunPassDraw"] = "由于 %from 死亡，%to 的技能【武魂】效果被触发，摸了 %arg 张牌",
	["#QingnangPassBuff"] = "由于上回合【青囊】的效果，%from 回复了1点体力",
	["#MafeiPass"] = "由于 %from 受到 %to 的伤害，技能【麻沸】效果被触发，%to 须在回合结束阶段弃置一张手牌",
	["#BuguaPass"] = "%from 使用了【卜卦】技能，将1张手牌与牌堆顶的牌交换",
	["#WeimuPass"] = "%from 使用了【帷幕.改】技能，弃置1张手牌无效了【%arg】的效果并摸了1张牌。",
	["#GainExp"] = "%from 成功击败了 %to，获得了 %arg 点经验值 ，当前经验值为 %arg2",
	
	["#NextStage"] = "%from 击败了所有敌人，来到了第【%arg】关",
	
	["#LoadNextStage"] = "读档成功！%from 来到了 <b>%arg2</b> 周目的第【%arg】关",
	["#ResetPlayer"] = "<font color='red'><b>进入了2周目，所有技能重置，并获得初始的50点经验值</b></font>",
}

for k, v in pairs(pass_t) do
	local ks = k:split("_")
	if not k:find(":") and #ks > 1 and ks[2] == "pass" then
		pass_t[ks[1]..ks[2]] = pass_t[k]
	end
end

return pass_t
