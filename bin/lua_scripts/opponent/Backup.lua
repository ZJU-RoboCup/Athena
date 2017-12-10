gOppoConfig = {
  ----------------------play---------------------------------------------------------------
	CornerKick  = {18,19,23}, --{1,2,3,7,13,14,15,16,18,19,23,24},
	FrontKick   = function()  --{1,2,3,5,6,19,20}, -{1,2,3,6}
		if math.abs(ball.refPosX()) < 130 then
			return {20} --{1,2,3,6}
		else
			return {20} --{7}
		end
	end,
	MiddleKick  = {12}, --{1,2,11,12},
	BackKick    = {"Ref_ImmortalKickV2"}, --{5,7, "Ref_ImmortalKickV1"}
	CornerDef   = "Ref_CornerDefV1",
	BackDef 	= function()
		if math.abs(ball.refPosY())>100 then
			return "Ref_BackDefV5"
		else 
			return "Ref_BackDefV8"--挑过中场犯规脚本
		end
	end,
	MiddleDef   = "Ref_MiddleDefV5",--若无效果用Ref_MiddleDefV2
	FrontDef    = "Ref_FrontDefV10", --若无效果用Ref_FrontDefV2
	PenaltyKick = "Ref_PenaltyKickV1",
	PenaltyDef  = "Ref_PenaltyDefV3",
	KickOff		= "Ref_KickOffV2",
	KickOffDef  = "Ref_KickOffDefV1",
	NorPlay     = "NormalPlayDefend"
}