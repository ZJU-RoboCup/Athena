gOppoConfig = {
  ----------------------play---------------------------------------------------------------
	--CornerKick  = {9, 11, 12, 16, 1612, 666},-- {"FrontKickV1616"},MRL 
	CornerKick  = {11,666},
	--[[
		cornerkick:
		9 -> 两车朝向两边
		11-> 同侧平传
		12-> 四车并排,双回环
		1611-> 11挑传版
		1612-> 12站位,禁区抢两点
	--]]

	R4CornerKick  = {1},

	FrontKick   = function()
		if ball.refPosX() < 280 then 
			return {1702,1704}--{1604, 1613}
		else
			return {"Ref_CornerKickV1612","Ref_CornerKickV666"}--{1609, 1611, 1612, 1615, 1616}  --角球改 1614 1611 效果不好
		end
		-- 1616 ERROR!!!!!!!!!!!!!!!
		-- 1614 同侧回挑
		-- 
	end,

	MiddleKick  = {"TestDynamicBackKick",5},
	BackKick    = {"TestDynamicBackKick","Ref_ImmortalKickV2"},--1606
	CornerDef   = "Ref_CornerDefV4_3",   -- V4_3防头球 V5_1全盯人
	BackDef 	= function()
		if math.abs(ball.refPosY()) > 100 then
			return "Ref_BackDefV5_1"  			 --保守用v5
		else
			return "Ref_BackDefV8"  				 --挑过中场犯规脚本
		end
	end,

	MiddleDef   = "Ref_MiddleDefV10_1",  --横向marktouch用v5，四车markfront用v10_1,保守打法用V2
	FrontDef    = "Ref_FrontDefV2",      --四车markfront用v9,若挑就用V7，保守打法用Ref_FrontDefV2 or V1
	PenaltyKick = "Ref_PenaltyKickV3",
	PenaltyDef  = "Ref_PenaltyDefV3",
	KickOff     = "Ref_KickOffV1",       --V1挑门

	KickOffDef  = "Ref_KickOffDefV1",
	NorPlay     = "NormalPlayDefend",
	IfHalfField = false,
	CornerKickPosSwitch = true -- 开关用于修改所有挑弱侧禁区点，false为禁区前点，true为距离禁区100左右的点 --IranOpen2017 -- 希望以后不会再有这样的开关
}