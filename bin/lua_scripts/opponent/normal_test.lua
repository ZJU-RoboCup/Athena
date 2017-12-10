gOppoConfig = {
  ----------------------play---------------------------------------------------------------\


    CornerKick  = {8,11,17,31,1601,1602,1603},--{8,11,16,17},   --{1,2,3,4,5(三车冲),6,7,9,10(挡拆),11,12,13,14(挡板)} {16,17,18(守门员),19(测阵型),20,21(传中间),22(传中间,测阵型),23,30,31(对手少车时)},
    --FrontKick   = {5},--{1,2,3,4,6}
	FrontKick   = function()
		if ball.refPosX() < 140 then 
			return {1601,1602,2,3,6,12}
		else
			return {2,4,6,10,11}
		end
	end,
	--菜鸡队伍 -- {11,12,20}
	MiddleKick  = {3},--{1,2,3,4,5,6}
	BackKick    = {"Ref_ImmortalKickV2"}, --{1,2,3, "Ref_ImmortalKickV1"}   {yes "2," --2016.3.16}
--BackKick    = {4},

	CornerDef   = "Ref_CornerDefV1", -- 可能要用v4_3，防头球
	BackDef 	= "Ref_BackDefV1",
	MiddleDef   = "Ref_MiddleDefV10_1",--横向marktouch用v5，四车markfront用v10_1,保守打法用V2
	FrontDef    = "Ref_FrontDefV7", --四车markfront用v9,若挑就用V7，保守打法用Ref_FrontDefV2 or V1
	PenaltyKick = "Ref_PenaltyKickV3",
	PenaltyDef  = "Ref_PenaltyDefV2",
	KickOff		= "Ref_KickOffV2",
	KickOffDef  = "Ref_KickOffDefV1",
	NorPlay     = "NormalPlayMark"
}