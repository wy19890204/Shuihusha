module("extensions.speffects", package.seeall)
extension = sgs.Package("speffects")

animate = sgs.CreateTriggerSkill{
	name = "animate",
	events = sgs.ChoiceMade,
	on_trigger = function(self, event, player, data)
		local room = player:getRoom()
		local choice= data:toString():split(":")
		if choice[1]=="SkillInvoke" and choice[3]=="yes" then
			local skillname=choice[2]
			if (sgs.Sanguosha:translate("*"..skillname) ~="*"..skillname) then
				for _, p in sgs.qlist(room:getAllPlayers()) do
					p:unicast(string.format("animate lightbox:*%s:%s",skillname,2500))        
				end				
			end
			local img=string.format("image\\system\\emotion\\%s\\0.png",skillname)
			local fp = io.open(img, "rb")  
			if fp then 
				fp:close() 
				room:setEmotion(player,skillname)
			end			
		end
		return false
	end,
}

local skills = sgs.SkillList()
if not sgs.Sanguosha:getSkill("animate") then skills:append(animate) end
sgs.Sanguosha:addSkills(skills)

--[[
特效原则， 

文字:
在下面定义特效文字的，就会发动文字特效， 注意，锁定技和视为技(奇袭,国色等)无效， 要系统询问你是否发动的技能才有特效

图片： 
image\system\emotion\ 目录下存在以你技能为名字的目录且目录下有 0.png时，会出现图片特效



如果是需要图片特效，比如奸雄
需要在  image\system\emotion\ 下建立一个名为 jianxiong 的目录
然后做n个png , 文件命命名从 0.png 开始递加
类似
image\system\emotion\jianxiong\0.png
image\system\emotion\jianxiong\1.png
image\system\emotion\jianxiong\2.png
image\system\emotion\jianxiong\3.png
image\system\emotion\jianxiong\4.png
image\system\emotion\jianxiong\5.png

为了测试，你可以先把整个铁锁连环的目录 chain 拷贝一份，重命名为 jianxiong

]]

sgs.LoadTranslationTable {
	["speffects"] = "特效包",
}

