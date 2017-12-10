gOppoConfig = {
  ----------------------play---------------------------------------------------------------
	CornerKick  = {2,10,13,18}, --{5,6,7,9} --{20}让车消失,试一下 --{14,15,16,17}守门员上 --{21,22,23}绝招
	FrontKick   = {1,2,5}, --比较稳 --{3,10}, --{11}两传一射，大比分领先可用 --{1,2,6}若对手防开球车 --{17}拖时间
	MiddleKick  = {2,4,8},  --如果对手跟朝向就选{2,5} --{9}挡板
	BackKick    = {4,8}, --{8}类似Immortalkick --{7}直接射门 --Ref_ImmortalKickV1 --{12}挡板
	CornerDef   = "Ref_CornerDefV6",
	BackDef 	= "Ref_BackDefV5",--若横传，可使用middledefV7
	MiddleDef   = function ()
		if math.abs(ball.refPosY())>110 then
			return "Ref_MiddleDefV5"--若markingToch无效，使用老脚本middleDefV3
		else 
			return "Ref_BackDefV8"--挑过中场犯规脚本
		end
	end,
	FrontDef    = "Ref_FrontDefV7",
	PenaltyKick = "Ref_PenaltyKickV3",
	PenaltyDef  = "Ref_PenaltyDefV3",
	KickOff		= {1},
	KickOffDef  = "Ref_KickOffDefV1",
	NorPlay     = "NormalPlayDefend"
}