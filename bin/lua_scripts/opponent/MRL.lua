gOppoConfig = {
  ----------------------play---------------------------------------------------------------
    CornerKick  = {1,3,7,10,11,17},   --{1,2,3,4,5(三车冲),6,7,9,10(挡拆),11,12,13,14(挡板)} {16,17,18(守门员),19(测阵型),20,21(传中间),22(传中间,测阵型),23,30,31(对手少车时)},
	
	-- added 2016.3.16
	--FrontKick = {20},--11甩--{11 chasekick}
	
	FrontKick   = function() -- {yes "12 11 9" -2016.3.16}
		if math.abs(ball.refPosX()) < 150 then
			return {5,12}  --{3,5(挑对角),7(倒勾),9(甩),12(挑前)}
		else
			return {6}  --{1,4,5,6(挑对角),11(挑门)}
		end
	end,
	MiddleKick  = function()
		if math.abs(ball.refPosX()) < 150 then
			return {2,6} --{1,2(挑对角),3,4,5(immortal), 6(挑前)}
		else
			return {2}   --{4,5}
		end
	end,
	BackKick    = {"Ref_ImmortalKickV2"}, --{1,2,3, "Ref_ImmortalKickV1"}   {yes "2," --2016.3.16}
	
	CornerDef   = "Ref_CornerDefV1", -- 可能要用v4_3，防头球
	BackDef 	= "Ref_BackDefV1",
	--[[function()
		if math.abs(ball.refPosY()) > 100 then
			return "Ref_BackDefV5"  --保守用v5 --少一盯人车用v5_1
		else
			return "Ref_BackDefV8"--挑过中场犯规脚本
		end
	end,--]]
	MiddleDef   = "Ref_MiddleDefV10_1",--横向marktouch用v5，四车markfront用v10_1,保守打法用V2
	FrontDef    = "Ref_FrontDefV7", --四车markfront用v9,若挑就用V7，保守打法用Ref_FrontDefV2 or V1
	PenaltyKick = "Ref_PenaltyKickV3",
	PenaltyDef  = "Ref_PenaltyDefV2",
	KickOff		= "Ref_KickOffV2",
	KickOffDef  = "Ref_KickOffDefV1",
	NorPlay     = "NormalPlayMark"
}