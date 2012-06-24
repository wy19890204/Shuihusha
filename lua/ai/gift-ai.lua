-- ai for gift-package

function SmartAI:useCardZongzi(card, use)
	if self.player:hasSkill("lisao") then return end
	use.card = card
end

sgs.ai_use_value.Zongzi = 5.5
sgs.ai_keep_value.Zongzi = 0
sgs.dynamic_value.benefit.Zongzi = true
