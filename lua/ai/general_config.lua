sgs.ai_keep_value = {
	Shit = 6,

	Peach = 5,

	Analeptic = 4.5,
	Jink = 4,

	Nullification = 3,

	Slash = 2,
	ThunderSlash = 2.5,
	FireSlash = 2.6,

	ExNihilo=4.6,

	AmazingGrace=-1,
	Lightning=-1,

	Ecstasy = 1.5,
	Drivolt = 1,
	Counterplot = 3.2,
	Wiretap = -1,
	Assassinate = 1.8,
	Provistore = 0,
	Treasury = -1,
	Tsunami = -1,
}

sgs.ai_use_value =
{

--skill cards
	XinzhanCard = 4.4,
	TianyiCard = 8.5,
	XianzhenCard = 9.2,
	XianzhenSlashCard = 9.2,
	HuangtianCard = 8.5,
	JijiangCard=8.5,
	DimengCard=3.5,
	JujianCard=6.7,
	QiangxiCard=2.5,
	LijianCard=8.5,
	RendeCard=8.5,
	MinceCard=5.9,
	ZhihengCard = 9,

	ShenfenCard = 8,
	GreatYeyanCard = 8,
	MediumYeyanCard = 5.6,
--normal cards
	ExNihilo=10,

	Snatch=9,
	Collateral=8.8,


	Indulgence=8,
	SupplyShortage=7,

	Peach = 6,
	Dismantlement=5.6,
	IronChain = 5.4,

	--retain_value=5

	FireAttack=4.8,


	FireSlash = 4.4,
	ThunderSlash = 4.5,
	Slash = 4.6,

	ArcheryAttack=3.8,
	SavageAssault=3.9,
	Duel=3.7,

	AmazingGrace=3,

	--special
	Analeptic = 5.98,
	Jink=8.9,
	Shit=-10,

	Ecstasy = 5,
	Drivolt = 5.2,
--	Counterplot = NULL,
	Wiretap = 9,
	Assassinate = 8.8,
	Provistore = 6.7,
	Treasury = 2,
	Tsunami = 2,
	Events = 5,
}

sgs.ai_use_priority = {
--priority of using an active card

--skill cards
	XinzhanCard = 9.2,
	GuhuoCard = 10,
	TianyiCard = 4,
	JieyinCard = 2.5,
	HuangtianCard = 10,
	XianzhenCard = 9.2,
	XianzhenSlashCard = 2.6,
	JijiangCard = 2.4,
	DimengCard=2.3,
	LijianCard = 4,
	QingnangCard=4.2,
	RendeCard= 5.8,
	MingceCard=4,
	JujianCard = 4.5,

	ShenfenCard = 2.3,
	GreatYeyanCard = 9,
	MediumYeyanCard = 6,
	SmallYeyanCard = 2.3,
	JilveCard = 0.4,
--

	Peach = 4.1,

	Dismantlement=4.4,
	Snatch=4.3,
	ExNihilo=4.6,

	GodSalvation=3.9,

	ArcheryAttack=3.5,
	SavageAssault=3.5,


	Duel=2.9,
	IronChain = 2.8,

	Collateral=2.75,

	Analeptic = 2.7,

	OffensiveHorse = 2.69,
	Halberd=2.685,
	KylinBow=2.68,
	Blade = 2.675,
	GudingBlade=2.67,
	DoubleSword =2.665,
	Spear = 2.66,
	Fan=2.655,
	IceSword=2.65,
	QinggangSword=2.645,
	Axe=2.64,
	MoonSpear=2.635,
	Crossbow = 2.63,


	FireSlash = 2.6,
	ThunderSlash = 2.5,
	Slash = 2.4,

	FireAttack=2,
	AmazingGrace=1.0,


	SilverLion=0.9,
	EightDiagram=0.8,
	RenwangShield=0.7,
	Vine=0.6,
	GaleShell=0.9,

	DefensiveHorse = 0,

	Ecstasy = 2.7,
	Drivolt = 3,
--	Counterplot = NULL,
	Wiretap = 4,
	Assassinate = 2.8,
	Provistore = 4.2,
	Treasury = 1,
	Tsunami = 1,
	Events = 3,

	DoubleWhip = 2.64,
	MeteorSword = 2.668,
	SunBow = 2.567,
	GoldArmor = 2.555,
	--god_salvation
	--deluge
	--supply_shortage
	--earthquake
	--indulgence
	--mudslide
	--lightning
	--typhoon
}


-- this table stores the chaofeng value for some generals
-- all other generals' chaofeng value should be 0
sgs.ai_chaofeng = {
	andaoquan = 7,
	shenwuyong = 7,

	shijin = 6,
	shien = 6,
	yuehe = 6,
	gongsunsheng = 6,

	yanqing = 5,
	hantao = 5,
	luozhenren = 5,
	songjiang = 5,
	lujunyi = 5,
	zhoutong = 5,

	zhuwu = 4,
	chaijin = 4,
	peixuan = 4,
	shiqian = 4,
	panjinlian = 4,

	haosiwen = 3,
	huangxin = 3,
	houjian = 3,
	qingzhang = 3,
	kongliang = 3,
	linniangzi = 3,
	baixiuying = 3,
	pengqi = 3,

	wusong = 2,
	gaoqiu = 2,
	mengkang = 2,
	jiaoting = 2,
	wangdingliu = 2,
	panqiaoyun = 2,
	duansanniang = 2,
	jiangsong = 2,

	taozongwang = 1,
	weidingguo = 1,

	oupeng = 0,
	wangqing = 0,
	zhutong = 0,
	yangzhi = 0,
	jiashi = 0,

	baisheng = -1,
	husanniang = -1,
	zhaoji = -1,

	luzhishen = -2,
	shenzhangqing = -2,
	likui = -2,
	yanshun = -2,
	tongguan = -2,
	qiaodaoqing = -2,

	huarong = -3,
	zhangqing = -3,
	muhong = -3,
	lizhong = -3,
	dingdesun = -3,

	liying = -4,
	linchong = -4,
	shiwengong = -4,

	guansheng = -5,
}

