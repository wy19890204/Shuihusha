-- AI卖萌聊天的说明兼DIY文档

--[[
种类1：function SmartAI:speak(typpe, to)
示例：self:speak("jishi", player)
标准函数，执行时先确定player参数，若无player则令player=self.player
然后看如果player为女性且有相应的"_female"内容，则切换到女性输出模式
最后调用sgs.ai_chat.jishi或sgs.ai_chat.jishi_female里的聊天语句并输出，视为player说出

种类2：function SmartAI:speakTrigger(card,from,to,event)
示例：self:speakTrigger(effect.card,effect.from,effect.to,sgs.CardEffect)
复杂情况的实现。一般在实卡或技能卡使用时，根据不同的使用者和目标，输出不同的语句
即“当from对to使用卡牌card时”，所调用输出的聊天语句
同时需要编辑self:speakTrigger函数
当from或to为空时取self.player
]]

--[[参考聊天条目]]

--分男女，当使用杀、过拆或顺牵时说
sgs.ai_chat.hostile_female=
{
"啧啧啧，来帮你解决点手牌吧",
"叫你欺负人!" ,
"手牌什么的最讨厌了"
}
sgs.ai_chat.hostile={
"yoooo少年，不来一发么",
"果然还是看你不爽",
"我看你霸气侧漏，不得不防啊"
}

--分男女，决斗时说
sgs.ai_chat.duel_female=
{
"哼哼哼，怕了吧~"
}
sgs.ai_chat.duel=
{
"来吧！像男人一样决斗吧！"
}

--使用无中生有时说
sgs.ai_chat.lucky=
{
"哎哟运气好",
"哈哈哈哈哈"
}

--分男女，被借刀杀人指定出杀者且没杀的时候说
sgs.ai_chat.collateral_female=
{
"别以为这样就算赢了！"
}
sgs.ai_chat.collateral=
{
"你妹啊，我的刀！"
}

--分男女，被行刺的时候说
sgs.ai_chat.assassinate_female=
{
" やめて ~  いや……"
}
sgs.ai_chat.assassinate=
{
"啊！有刺客！"
}

--对别人使用迷的时候说
sgs.ai_chat.ecstasy_source=
{
"嘿嘿，看我耍点阴招……",
"躺下吧！"
}

--被别人迷倒的时候说
sgs.ai_chat.ecstasy=
{
"我中了迷……药……",
"额……身体……不听使唤了……"
}

--被人使用画地为牢的时候说
sgs.ai_chat.indulgence=
{
"我画个圈圈诅咒你！",
"擦，动我",
"放我出去！"
}

--被人使用逼上梁山的时候说
sgs.ai_chat.drivolt=
{
"逼，逼你妹啊逼！",
"擦，我的肉！",
"我不要上梁山！"
}

--使用猛虎下山时，李逵和武松说
sgs.ai_chat.tiger=
{
"好一只吊睛白额的大虫！",
"难道……这就是传说中的景阳冈花瓣太岁么- -",
"看什么，俺可是打虎英雄！",
"好大只，压力好大……"
}

--张青发动穷途时，被穷途的人说
sgs.ai_chat.qiongtu={
"擦，还我菊花！",
"内牛满面了",
"哎哟我去"
}

--触发孙二娘的黑店，需要给一张装备牌的时候说
sgs.ai_chat.heidian=
{
"二娘饶命……",
"没留意竟然进了黑店！"
}

--庞万春使用连珠的时候说
sgs.ai_chat.lianzhu=
{
"放箭！",
"看我射不死你们！",
}

--乔道清发动幻术的时候说
sgs.ai_chat.huanshu=
{
"竟敢伤我，你活够了？",
"挣扎吧，在火与雷的地狱中！",
"老子最牛逼，不服单挑"
}

--燕青打擂拼点的时候说
sgs.ai_chat.dalei=
{
"出大的！",
"来来来拼点了",
"哟，拼点吧"
}
--燕青打擂时，拼点对方说
sgs.ai_chat.dalei_target=
{
"老子怕你不成！",
"暂且看看，鹿死谁手吧！",
}

--高俅发动蹴鞠的时候说
sgs.ai_chat.cuju=
{
"好球！",
"踢个球玩玩~",
"想和我一起组队踢世界杯吗？",
"看什么看，踢的就是你！"
}

--张清发动饮羽的时候说
sgs.ai_chat.yinyu=
{
"靠石头吃饭不容易，兄弟们担待点~",
"看我一石激起千层浪！"
}

--公孙胜发动奇门的时候说
sgs.ai_chat.qimen_source=
{
"封印术·八卦封印！",
"封印术·五行封印！",
"封印术·四象封印！",
"封印术·三才封印！",
"封印术·两仪封印！",
}
--公孙胜奇门时，对方说
sgs.ai_chat.qimen=
{
"咦？我的技能哪去了？",
"公孙老道，你丫给我等着！",
"被、被封印了么- -"
}

--安道全发动济世时说
sgs.ai_chat.jishi=
{
"这盒金疮药接住了~",
"医生真苦逼，各种疗伤……",
"掌仙术！",
}

--卢俊义报国虚弱时说
sgs.ai_chat.lujunyi_weak=
{
"擦，再卖血会卖死的",
"不敢再卖了诶诶诶诶"
}

--吃粽子时说
sgs.ai_chat.zongzi=
{
"哇，有粽子吃耶！"
}

--鲁智深死亡触发坐化时说
sgs.ai_chat.zuohua_death=
{
"本来无一物，何处惹尘埃……"
}

