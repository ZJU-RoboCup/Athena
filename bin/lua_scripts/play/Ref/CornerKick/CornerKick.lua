-- 根据不同的位置选择不同的定位球
-- 注意：此文件中只能定义一些常量，并且以时时更新的数据做判断
-- 使用gOurIndirectTable中的元素和本文件中的local变量进行控制切换
-- 其区别是gOurIndirectTable中的元素为状态保持量，而local变量为时时控制量

-- 在进入每一个定位球时，需要在第一次进时进行保持
if vision:Cycle() - gOurIndirectTable.lastRefCycle > 6 then
	if false and cond.validNum() <= 2 then
		-- if type(gOppoConfig.CornerKick) == "function" then
		-- 	gCurrentPlay = cond.getOpponentScript("Ref3_CornerKickV",gOppoConfig.CornerKick(),20)
		-- else
		-- 	gCurrentPlay = cond.getOpponentScript("Ref3_CornerKickV",gOppoConfig.CornerKick,20)
		-- end
	elseif (not gOppoConfig.FreeKickTest) and cond.validNum() <= 3 then
		if type(gOppoConfig.CornerKick) == "function" then
			gCurrentPlay = cond.getOpponentScript("Ref4_CornerKickV",gOppoConfig.R4CornerKick(),20)
		else
			gCurrentPlay = cond.getOpponentScript("Ref4_CornerKickV",gOppoConfig.R4CornerKick,20)
			print('r4select')
		end
	else
		if type(gOppoConfig.CornerKick) == "function" then
			gCurrentPlay = cond.getOpponentScript("Ref_CornerKickV",gOppoConfig.CornerKick(),20)
		else
			gCurrentPlay = cond.getOpponentScript("Ref_CornerKickV",gOppoConfig.CornerKick,20)
		end
	end
end

-- if type(gOppoConfig.CornerKick) == "table" then
-- 			local totalNum = table.getn(gOppoConfig.CornerKick)
-- 			local randNum = math.random(1,totalNum)
-- 			print("randNum","Ref_CornerKickV"..randNum)
-- 			gCurrentPlay = "Ref_CornerKickV"..gOppoConfig.CornerKick[randNum]
-- 		elseif type(gOppoConfig.CornerKick) == "string" then
-- 			if gOppoConfig.CornerKick == "random" then
-- 				local randNum = math.random(1,11)
-- 				print("randNum","Ref_CornerKickV"..randNum)
-- 				gCurrentPlay = "Ref_CornerKickV"..randNum
-- 			elseif gOppoConfig.CornerKick == "score" then
-- 				local maxScore = -1000
-- 				for playname, value in pairs(gPlayTable) do
-- 					local isCornerKick = string.find(playname, "Ref_CornerKickV")
-- 					if isCornerKick then
-- 						print("CornerKick in Score Mode: ", playname, gPlayTable[playname].score)
-- 						if gPlayTable[playname].score > maxScore then
-- 							gCurrentPlay = playname
-- 							maxScore = gPlayTable[gCurrentPlay].score
-- 						end
-- 					end
-- 				end
-- 			else
-- 			gCurrentPlay = gOppoConfig.CornerKick
-- 			end
-- 		else
-- 			print("Error in CornerKick.lua!!!!!!!!!!")
-- 		end

-- 		if recognizer:getResult() == "PassBack" then
-- 			gCurrentPlay = "Ref_CornerKickV1"
-- 		elseif recognizer:getResult() == "PassPenalty" then
-- 			gCurrentPlay = "Ref_CornerKickV11"
-- 		end