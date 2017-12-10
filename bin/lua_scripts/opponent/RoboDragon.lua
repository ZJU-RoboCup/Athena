-- 可指定具体定位球，也可以以table的形式在几个定位球中随机选择

gOppoConfig = {
  ----------------------play---------------------------------------------------------------
	CornerKick  = {11,23}, -- {5,11,7}, --  守门员出来{6}-- 小招{2,6,7,9} 乱招{10,12}
	FrontKick   = function () --{1,2,3,5}, --如果前面的招都没效果{4}, --如果希望拖时间{6},
		if ball.refPosX() > 150 then
			return {10}
		else
			return {3,10}
		end
	end,
	MiddleKick  = {2}, --{4},打RoboDragon时有很好的效果 --如果没有领先且6被截{4}用来拖时间, 拖时间直接踢 {2},
	BackKick    = "Ref_BackKickV8",
	CornerDef   = "Ref_CornerDefV5",
	BackDef 	= function()
		if math.abs(ball.refPosY())>100 then
			return "Ref_BackDefV5"
		else 
			return "Ref_BackDefV8"--挑过中场犯规脚本
		end
	end,
	MiddleDef   = "Ref_FrontDefV9",--若无效果用Ref_MiddleDefV2
	FrontDef    = "Ref_FrontDefV9",--若无效果用Ref_FrontDefV2
	PenaltyKick = "Ref_PenaltyKickV3",
	PenaltyDef  = "Ref_PenaltyDefV3",
	KickOff		= {6},
	KickOffDef  = "Ref_KickOffDefV1",
	NorPlay     = "NormalPlayDefend"
}