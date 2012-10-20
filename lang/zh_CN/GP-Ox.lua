-- translation for OxPackage
-- Second of the twelve

return {
	["ox"] = "丑牛",

	["$gaolian"] = "116",
	["#gaolian"] = "高唐魔君", -- guan 3hp (ybyt)
	["gaolian"] = "高廉",
	["illustrator:gaolian"] = "封神榜",
	["cv:gaolian"] = "烨子风暴【天子会工作室】",
	["coder:gaolian"] = "战栗贵公子",
	["guibing"] = "鬼兵",
	[":guibing"] = "当你需要使用或打出一张【杀】时，你可以进行一次判定：若结果不为♥，则视为你使用或打出一张【杀】，否则你不能使用或打出【杀】，直到回合结束。",
	["heiwu"] = "黑雾",
	[":heiwu"] = "出牌阶段，你可以将任意数量的手牌以任意顺序置于牌堆顶。",
	["$guibing1"] = "嘿嘿～",
	["$guibing2"] = "看吾号令阴兵鬼将！",
	["$heiwu1"] = "毒雾弥漫，寸草不生！",
	["$heiwu2"] = "雾兴云法，罩他个天昏地暗。",
	["~gaolian"] = "是谁破了阵法？",

	["$tongguan"] = "111A",
	["#tongguan"] = "广阳郡王", -- guan 4hp (bwqz)
	["tongguan"] = "童贯",
	["illustrator:tongguan"] = "影枫",
	["designer:tongguan"] = "烨子&大Ｒ",
	["cv:tongguan"] = "北落师门【心水蓝音】",
	["aoxiang"] = "媪相",
	[":aoxiang"] = "<font color=green><b>觉醒技</b></font>，回合开始时，若你的当前体力值不大于2，则你须扣减1点体力上限，性别转变为女，并永久获得技能“王宦”（<b>锁定技</b>，当你使用【杀】指定一名男性角色为目标后，该角色须弃置一张手牌；当你成为男性角色使用【杀】的目标后，你摸一张牌）。",
	["zhengfa"] = "征伐",
	[":zhengfa"] = "回合结束阶段开始时，你可以和一名与你势力不同的角色拼点。若你赢，则视为你对至多X名其他角色使用一张【杀】（X为场上现存势力数）。若你没赢，则你将你的武将牌翻面。",
	["@zhengfa"] = "你可以和一名其他角色进行【征伐】拼点",
	["@zhengfa-effect"] = "征伐成功！你选择X名其他角色并视为对其使用一张【杀】，X为现存势力数",
	["$aoxiang"] = "为相何故宦，为宦亦为官。",
	["$zhengfa1"] = "臣愿效犬马之劳，以除心腹之患。", -- 正常童贯发动拼点时
	["$zhengfa3"] = "刻日扫清山寨，擒拿众贼。", -- 正常童贯拼点赢
	["$zhengfa5"] = "敌军势大，暂避其锋芒。", -- 正常童贯拼点输
	["~tongguan"] = "前有伏兵，后有追兵，似此如之奈何？", --正常童贯阵亡

	["$tongguanf"] = "111B",
	["#tongguanf"] = "广阴郡王",
	["tongguanf"] = "童公公",
	["illustrator:tongguanf"] = "Shadow.D",
	["designer:tongguanf"] = "烨子&大Ｒ",
	["cv:tongguanf"] = "北落师门【心水蓝音】",
	["wanghuan"] = "王宦",
	[":wanghuan"] = "<b>锁定技</b>，当你使用【杀】指定一名男性角色为目标后，该角色须弃置一张手牌；当你成为男性角色使用【杀】的目标后，你摸一张牌。",
	["yan"] = "阉",
	["$zhengfa2"] = "臣愿效犬马之劳，以除心腹之患。", -- 太监童贯发动拼点时
	["$zhengfa4"] = "区区草寇，何足为患？", -- 太监童贯拼点赢
	["$zhengfa6"] = "兵少将寡，如何可出战。", -- 太监童贯拼点输
	["$wanghuan1"] = "杀不死你，还玩不死你？", -- 1和2是1效果
	["$wanghuan2"] = "我军士气正盛，必当乘胜追击！",
	["$wanghuan3"] = "何人敢对本王造次？", -- 3和4是2效果
	["$wanghuan4"] = "众将士听令，屯驻前线，巡检御敌！",
	["~tongguanf"] = "前有伏兵，后有追兵，似此如之奈何？", --太监童贯阵亡

	["$huyanzhuo"] = "008",
	["#huyanzhuo"] = "双鞭", -- guan 4hp (fcdc)
	["huyanzhuo"] = "呼延灼",
	["illustrator:huyanzhuo"] = "刘彤&赵鑫",
	["cv:huyanzhuo"] = "泥马【影音同画】",
	["coder:huyanzhuo"] = "Slob",
	["lianma"] = "链马",
	[":lianma"] = "出牌阶段，你可以选择一项：横置所有装备区里有坐骑牌的角色的武将牌，或重置所有装备区里没有坐骑牌的角色的武将牌。每阶段限一次。",
	["lianma:lian"] = "链起来！",
	["lianma:ma"] = "解链！",
	["#LianmaOn"] = "%from 发动【%arg】将 %to 的武将牌横置",
	["#LianmaOff"] = "%from 发动【%arg】将 %to 的武将牌重置",
	["zhongjia"] = "重甲",
	[":zhongjia"] = "<b>锁定技</b>，你的手牌上限+X（X为处于连环状态的现存角色数）。",
	["$lianma1"] = "每三十匹一连，却把铁环连锁。", --1和2是横置效果
	["$lianma2"] = "三千连环马军，分作一百队锁定。",
	["$lianma3"] = "马无羁绊可奔袭。", -- 3和4是重置效果
	["$lianma4"] = "待敌军来时，分而袭之。",
	["$zhongjia1"] = "马覆重甲，人披铁铠。", -- 弃牌阶段结束时播放
	["$zhongjia2"] = "重装骑兵阵，声势若滔天。",
	["~huyanzhuo"] = "老当益壮报家国，敢叫金人丧破胆。",

	["$dongchaoxueba"] = "131",
	["#dongchaoxueba"] = "黑狱吏", -- jiang 4hp (yxqd)
	["dongchaoxueba"] = "董超薛霸",
	["illustrator:dongchaoxueba"] = "黑山老妖",
	["cv:dongchaoxueba"] = "秦奋&卡修【天子会工作室】",
	["coder:dongchaoxueba"] = "Slob",
	["sheru"] = "折辱",
	[":sheru"] = "出牌阶段，你可以弃置一张♠或♣基本牌，指定一名已受伤的其他角色并选择一项：令其摸X张牌，然后其失去1点体力，或弃掉其X张牌，然后令其回复1点体力（X为该角色已损失的体力值）。每阶段限一次。",
	["sheru:she"] = "摸X张牌并失去1点体力",
	["sheru:ru"] = "弃掉其X张牌并令其回复1点体力",
	["$sheru1"] = "林教头洗脚吧！",  -- 1和2是1效果
	["$sheru2"] = "说什么闲话！救你不得。",
	["$sheru3"] = "多出些钱物，也可保你一命。",  --3和4是2效果
	["$sheru4"] = "多是五站路，少便两程，便有分晓。",
	["~dongchaoxueba"] = "这黄泉路上你我相伴吧～",

	["$pangwanchun"] = "120",
	["#pangwanchun"] = "小养由基", -- jiang 4hp (fcdc)
	["pangwanchun"] = "庞万春",
	["illustrator:pangwanchun"] = "幻想世界",
	["designer:pangwanchun"] = "烨子&宇文天启",
	["cv:pangwanchun"] = "尘埃【一诺文化】",
	["lianzhu"] = "连珠",
	[":lianzhu"] = "出牌阶段，若你的武将牌正面向上，则你可以将你的武将牌翻面，视为你使用一张【万箭齐发】（目标角色需连续打出两张【闪】，否则受到你对其造成的1点火焰伤害）。",
	["#Lianzhu"] = "%from 的锁定技【%arg】被触发，%to 需要打出两张【闪】",
	["@lianzhu2jink"] = "%src 拥有技能【连珠】，你须再出一张【闪】",
	["$lianzhu1"] = "拈弓取箭，敢教汝等落荒而逃！",
	["$lianzhu2"] = "且看吾连珠鸣镝，统统乱箭射死！",
	["~pangwanchun"] = "高相公，吾悔矣！",

	["$huangxin"] = "038",
	["#huangxin"] = "镇三山", -- jiang 4hp (xzdd)
	["huangxin"] = "黄信",
	["illustrator:huangxin"] = "三国豪杰传",
	["designer:huangxin"] = "烨子&裁之刃·散",
	["cv:huangxin"] = "流岚【裔美声社】",
	["tongxia"] = "统辖",
	[":tongxia"] = "你可以跳过你的摸牌阶段，亮出牌堆顶的三张牌，然后将其中的装备牌分别置于任意角色的装备区里（替换原装备牌）并将其余的牌分别交给任意角色。",
	["$tongxia1"] = "穷兵黩武，动费万计。",
	["$tongxia2"] = "三地军需，由我调拨！",
	["~huangxin"] = "三山崛起，力不从心啊！",

	["$luozhenren"] = "124",
	["#luozhenren"] = "半仙", -- kou 3hp (qjwm)
	["luozhenren"] = "罗真人",
	["illustrator:luozhenren"] = "封神榜",
	["cv:luozhenren"] = "东方胤弘【天子会工作室】",
	["butian"] = "卜天",
	[":butian"] = "在任一角色的判定牌生效前，你可以弃置一张牌，观看牌堆顶的三张牌，然后用其中的任意一张牌替换该判定牌。",
	["@butian-card"] = "请弃置一张牌，发动技能【卜天】（更改 %src 的 %arg 判定结果）",
	["huaxian"] = "化仙",
	[":huaxian"] = "当你进入濒死状态时，你可以进行一次判定：若结果为♥，则你的当前体力值回复至1。",
	["$butian1"] = "掐指一算，万事尽知。",
	["$butian2"] = "天道？哈哈哈～",
	["$huaxian1"] = "脚著谢公屐，身登青云梯。",
	["$huaxian2"] = "天劫已度，上可登仙。",
	["~luozhenren"] = "灾祸易躲，天命难违。",

	["$lili"] = "096",
	["#lili"] = "催命判官", -- kou 3hp (cgdk)
	["lili"] = "李立",
	["illustrator:lili"] = "牛哞哞",
	["cv:lili"] = "莫名【忆昔端华工作室】",
	["duoming"] = "夺命",
	[":duoming"] = "其他角色的回合内，该角色每回复1点体力，你可以交给其两张黑色手牌，对其造成1点伤害。",
	["@duoming"] = "%src 回复了1点体力，你可以对其发动【夺命】技能",
	["moucai"] = "谋财",
	[":moucai"] = "其他角色每受到一次伤害，若其手牌数大于你的当前体力值，则你可以获得其一张手牌。",
	["$duoming1"] = "杀了你，脏了我案板！",
	["$duoming2"] = "休想走出这店门！",
	["$moucai1"] = "人为财死，鸟为食亡！",
	["$moucai2"] = "钱财乃身外之物，你留着也没用了！",
	["~lili"] = "生不带来，死不带去。",

	["$shijin"] = "023",
	["#shijin"] = "九纹龙", -- kou 4hp (qjwm)
	["shijin"] = "史进",
	["wubang"] = "舞棒",
	["illustrator:shijin"] = "热血水浒",
	["cv:shijin"] = "鸢飞【天子会工作室】",
	[":wubang"] = "当其他角色的武器牌进入弃牌堆时，你可以获得之。",
	["xiagu"] = "侠骨",
	[":xiagu"] = "当任一角色受到无属性伤害时，你可以弃置一张装备牌，令该伤害-1。",
	["@xiagu"] = "你可以弃置一张装备牌发动【侠骨】（令该伤害-1）",
	["$Xiagu"] = "%from 发动了【%arg】，弃置了 %card，令 %to 受到的伤害 -1",
	["$wubang1"] = "哪位教头再来点拨？",
	["$wubang2"] = "看吾耍枪弄棒！",
	["$xiagu1"] = "诸位哥哥，小弟来挡！",
	["$xiagu2"] = "大郎在此，这厮休得无礼！",
	["~shijin"] = "何以别离久，何以不得安。",

	["$lijun"] = "026",
	["#lijun"] = "混江龙", -- min 4hp (yxqd)
	["lijun"] = "李俊",
	["illustrator:lijun"] = "NF1",
	["designer:lijun"] = "扈成",
	["cv:lijun"] = "冥引【天子会工作室】",
	["nizhuan"] = "逆转",
	[":nizhuan"] = "当一张非延时类锦囊指定了多名目标角色时，（在结算前）你可以指定该锦囊的结算方向（顺时针或逆时针）。",
	["dingce"] = "定策",
	[":dingce"] = "出牌阶段，你可以交给任一其他角色一张锦囊牌，然后获得该角色的一张牌并展示之：若为锦囊牌，则你可以弃置该牌并回复1点体力。每阶段限一次。",
	["$nizhuan1"] = "海水无倒流，乾坤有倒转。",
	["$nizhuan2"] = "逆水行舟，不进则退。",
	["$dingce1"] = "小弟有一计，欲到卢先锋处商议。",
	["$dingce2"] = "功过智伯城三板，计胜淮阴沙几囊。",
	["~lijun"] = "兄弟们，备船出海，从头再来！",

	["$xiezhen"] = "034",
	["#xiezhen"] = "两头蛇", -- min 4hp (fcdc)
	["xiezhen"] = "解珍",
	["illustrator:xiezhen"] = "沧海",
	["cv:xiezhen"] = "鹏少",
	["xunlie"] = "巡猎",
	[":xunlie"] = "摸牌阶段，你可以放弃摸牌，然后分别获得一至X名其他角色的一张手牌（X为你装备区里的牌数且至少为1），或弃置一张♥或♦手牌并获得任一其他角色的两张手牌。",
	["@xunlie"] = "你可以跳过摸牌阶段，发动【巡猎】",
	["$xunlie1"] = "金蛇弓响，虎狼难逃。", --1和2是1效果
	["$xunlie2"] = "吾即刻去取讨大虫！",
	["$xunlie3"] = "这大虫，你给我吐出来！", -- 3和4是2效果
	["$xunlie4"] = "山林珍馐，任我取舍。",
	["~xiezhen"] = "顾不了那么多了！",

	["$linniangzi"] = "148",
	["#linniangzi"] = "腊梅傲雪", -- min 3hp (qlfd)
	["linniangzi"] = "林娘子",
	["illustrator:linniangzi"] = "小李飞刀",
	["designer:linniangzi"] = "烨子&大Ｒ",
	["cv:linniangzi"] = "蒲小猫【天子会工作室】",
	["shouwang"] = "守望",
	[":shouwang"] = "出牌阶段，你可以将一张【杀】交给任一男性角色，然后选择一项：摸一张牌，或令其摸一张牌。每阶段限一次。",
	["shouwang:tian"] = "对方摸1张牌",
	["shouwang:zi"] = "自己摸1张牌",
	["ziyi"] = "自缢",
	[":ziyi"] = "<font color=purple><b>限定技</b></font>，出牌阶段，若你已受伤，则你可以令任一已受伤的男性角色回复2点体力。若如此做，则回合结束后，你立即死亡。",
	["@rope"] = "麻绳",
	["zhongzhen"] = "忠贞",
	[":zhongzhen"] = "当你受到其他角色造成的伤害时，你可以和该角色拼点。若你赢，则防止该伤害。",
	["#Zhongzhen"] = "%from 对 %to 造成的 %arg 点伤害被抵消",
	["$shouwang1"] = "官人（哭声）。",
	["$shouwang2"] = "待来年春时，与君一叙。",
	["$ziyi"] = "清平世界，却如此方保贞洁，官人，莫念！",
	["$Ziyi"] = "清平世界，却如此方保贞洁，\
官人，莫念！",
	["$zhongzhen1"] = "怎可这般无礼？",
	["$zhongzhen2"] = "你若再上前一步，我便跳下去。",
	["~linniangzi"] = "官人，就此～别过！",

}
