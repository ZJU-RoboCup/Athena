gOppoConfig = {
	CornerKick  = "Ref_CornerKickDelay",
	FrontKick   = {17},
	MiddleKick  = {2},
	BackKick    = {8},
	CornerDef   = "Ref_CornerDefV5",
	BackDef 	= function()
		if math.abs(ball.refPosY())>100 then
			return "Ref_BackDefV5"
		else
			return "Ref_BackDefV8"--挑过中场犯规脚本
		end
	end,
	MiddleDef   = "Ref_FrontDefV9",--若无效果用middleDefV2
	FrontDef    = "Ref_FrontDefV9",
	PenaltyKick = "Ref_PenaltyKickV3",
	PenaltyDef  = "Ref_PenaltyDefV3",
	KickOff		= {1},
	KickOffDef  = "Ref_KickOffDefV1",
	NorPlay     = "NormalPlayDefend"
}