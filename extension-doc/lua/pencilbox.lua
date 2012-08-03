-- 诸葛孔明，称号筋肉的卧龙，四血，蜀势力
module("extensions.pencilbox", package.seeall)
extension = sgs.Package("pencilbox")

LuapojiCard=sgs.CreateSkillCard
{
	name="LuapojiCard",
	target_fixed=true,
	-- will_throw=false,

	on_use = function(self, room, source, targets)
		if self:getSubcards():length() < 1 then
			room:loseHp(source)
		end
		room:throwCard(self)
	end,
}

luapoji=sgs.CreateViewAsSkill
{
	name="luapoji",
	n=1,

	view_filter = function(self, selected, to_select)
		if sgs.Self:getHandcardNum() < sgs.Self:getHp() then return false end
		if #selected > 0 then return false end
		return not to_select:isEquipped()
	end,

	view_as = function(self, cards)
		if #cards < 1 then
			if sgs.Self:getHandcardNum() < sgs.Self:getHp() then
				return LuapojiCard:clone()
			else
				return nil
			end
		end
		local view_as_card = LuapojiCard:clone()
		view_as_card:addSubcard(cards[1]:getId())
		view_as_card:setSkillName(self:objectName())
		return view_as_card
	end,

	enabled_at_play=function(self, player)
		return false
	end,

	enabled_at_response=function(self,player,pattern)
		return pattern == "nullification"
	end,

	enabled_at_nullification = function(self, player)
		return true
	end
}

zhugekongming = sgs.General(extension, "zhugekongming", "shu")
zhugekongming:addSkill(luapoji)

sgs.LoadTranslationTable{
	["zhugekongming"] = "诸葛孔明",
	["#zhugekongming"] = "筋肉的卧龙",
	["designer:zhugekongming"] = "二笔要打机",
	["luapoji"] = "破计",
	[":luapoji"] = "若你的手牌数小于你的体力值，你可以失去1点体力，视为你使用了一张【无懈可击】；若你的手牌数大于或等于你的体力值，你可以将一张手牌当【无懈可击】使用。",

	["LuapojiCard"] = "破计",
	["pencilbox"] = "笔筒",
}

