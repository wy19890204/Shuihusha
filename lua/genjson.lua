--[[
	在 sanguosha.lua 的最后一行加入一行       dofile "lua/genjson.lua"
	然后重新启动一次游戏，即可重新生成 sgs.js, 
	建议下次游戏出新版后使用
]]
function genJson()	
	local trans,descs,packset,related,lordskill,packlist={},{},{},{},{},{}

	local generalnames=sgs.Sanguosha:getLimitedGeneralNames()
	local hidden={"zuocif","sp_diaochan","sp_sunshangxiang","sp_pangde","shenlvbu1","shenlvbu2","sp_caiwenji",
			"sp_machao","sp_jiaxu","luboyanf","zhibasunquan","wuxingzhuge","sujiang","sujiangf"}	
	table.insertTable(generalnames,hidden)
	
	for _, pack in ipairs(sgs.Sanguosha:getExtensions()) do	packset[pack]={} end
	
	for _, generalname in ipairs(generalnames) do	
		local general = sgs.Sanguosha:getGeneral(generalname)
		pack=general:getPackage()		
		for _, skill in sgs.qlist(general:getVisibleSkillList()) do			
			skillname=skill:objectName()

			local tran=string.format('"%s":"%s"',skillname,sgs.Sanguosha:translate(skillname))
			if not table.contains(trans,tran) then table.insert(trans,tran) end

			local desc=sgs.Sanguosha:translate(":"..skillname)
			desc=string.format('"%s":"%s"',skillname,string.gsub(desc,"\r*\n+","<br>"))
			if not table.contains(descs,desc) then table.insert(descs,desc) end			

			if skill:isLordSkill() then table.insert(lordskill, string.format('"%s":"1"',skillname)) end

			local item=string.format('"%s"',skillname)
			if not table.contains(packset[pack],item) then table.insert(packset[pack],item) end 	

			local rskill={}
			for _, skill in sgs.qlist(sgs.Sanguosha:getRelatedSkills(skillname)) do			
				table.insert(rskill,'"'..skill:objectName()..'"')			
			end
			rskilllist=string.format('"%s":[%s]',skillname,table.concat(rskill,","))
			if #rskill>0 and not table.contains(related,rskilllist) then table.insert(related,rskilllist) end

		end
	end

	for packname, skillname in pairs(packset) do	
		if #skillname>0 then 
			table.insert(packlist,string.format('"%s":[%s]',packname,table.concat(skillname,","))) 
			table.insert(trans,string.format('"%s":"%s"',packname,sgs.Sanguosha:translate(packname)))
		end		
	end

	local fp = io.open("lua/sgs.js","w")	
	fp:write(string.format("/* ban packages is: %s */\r\n",table.concat(sgs.Sanguosha:getBanPackages(),",")))	
	fp:write(string.format("var rskill={%s};\r\n",table.concat(related,","))) 
	fp:write(string.format("var lordskill={%s};\r\n",table.concat(lordskill,","))) 
	fp:write(string.format("var packs={\r\n%s};\r\n", table.concat(packlist,",\r\n")))
	fp:write(string.format("var trans={%s};\r\n",table.concat(trans,",")))
	fp:write(string.format("var descs={%s};\r\n",table.concat(descs,",")))	
	fp:close()
end

genJson()