module("extensions.customcards", package.seeall)
extension = sgs.Package("customcards")

local cards = {}
local file = io.open("./etc/custom-cards.txt", "r")
for line in file:lines() do
	local t = line:split(" ")
	table.insert(cards, sgs.Sanguosha:cloneCard(t[1], t[2], t[3]))
end
file:close()

for _, card in ipairs(cards) do
	card:setParent(extension)
end

sgs.LoadTranslationTable{
	["customcards"] = "自定义卡牌包",
}
