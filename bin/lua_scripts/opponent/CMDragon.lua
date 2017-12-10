gOppoConfig = {
	----------------------play---------------------------------------------------------------
	CornerKick  = {7}, -- 默认使用{7}, 如果{7}不好用则使用向后传的两个{12,14,23},角球区门将射门加打挡板{21}
	FrontKick   = function ()
		if ball.refPosX() > 140 then
			return {10}
		else
			return {17} --{18}
		end
	end,
	MiddleKick  = {6}, -- 默认使用{2}, 如果{2}不管用, 那么就使用{6}进行自动判断 
	BackKick    = {8}, -- 默认使用{8}, --{10}
	CornerDef   = "Ref_CornerDefV1",
	BackDef 	= function()
		if math.abs(ball.refPosY())>100 then
			return "Ref_BackDefV5"
		else 
			return "Ref_BackDefV8"--挑过中场犯规脚本
		end
	end,
	MiddleDef   = "Ref_MiddleDefV5",--若无效果用Ref_MiddleDefV2
	FrontDef    = "Ref_FrontDefV7",--若无效果用Ref_FrontDefV2
	PenaltyKick = "Ref_PenaltyKickV3",
	PenaltyDef  = "Ref_PenaltyDefV3",
	KickOff		= "Ref4_KickOffV1",
	KickOffDef  = "Ref_KickOffDefV1",
	NorPlay     = "NormalPlayOneState"
}