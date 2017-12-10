if vision:Cycle() - gOurIndirectTable.lastRefCycle > 6 then
	if cond.validNum() <= 3 then
		gCurrentPlay = "Ref4_FrontKickV1"
	elseif math.abs(ball.refPosY()) < 100 then
		gCurrentPlay = "Ref_FrontKickV8"
	else
		if type(gOppoConfig.MiddleKick) == "function" then
			gCurrentPlay = cond.getOpponentScript("Ref_MiddleKickV", gOppoConfig.MiddleKick(), 4)
		else
			gCurrentPlay = cond.getOpponentScript("Ref_MiddleKickV", gOppoConfig.MiddleKick, 4)
		end
	end
end