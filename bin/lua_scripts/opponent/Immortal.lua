gOppoConfig = {
  ----------------------play---------------------------------------------------------------
	CornerKick  = {11,16,19}, --{5,11,22}, --{10,12,13,16,18} --{4,6,7,14}
	FrontKick   = {10},--function () --若防开球车不跟着转{3,5,10} --{11} --{17}
		--if ball.refPosX() < 160 then
		--	return {1,2,6}
		--elseif ball.refPosX() > 160 and ball.refPosX() < 200 then
		--	return {1,6}
		--else
		--	return {6}
		--end
	--end,
	MiddleKick  = {2}, --{1,4}
	BackKick    = "Ref_ImmortalKickV2", --{4} --{10}边线两传一射 --{1,2,3,5}两传一射
	CornerDef   = "Ref_CornerDefV4",
	BackDef 	= "Ref_BackDefV4",--V4防头球，如果有5车进攻，迅速站默认点，调V5
	-- BackDef= function()
	-- 	if ball.refPosX()>-70 and math.abs(ball.refPosY())>110 then
	-- 		return "Ref_MiddleDefV7"
	-- 	else 
	-- 		return "Ref_BackDefV4" 
	-- 	end
	-- end, --若immortal会在前场平传可以使用middledefv7
	MiddleDef   = function()
		if ball.posX()<0 then
			return "Ref_BackDefV4"--往前场挑
		else
			return "Ref_MiddleDefV9"--markingTouch
		end
	end,
	--MiddleDef = "Ref_MiddleDefV9",--若对着门无效使用V5
	FrontDef    = "Ref_FrontDefV8",
	PenaltyKick = "Ref_PenaltyKickV3",
	PenaltyDef  = "Ref_PenaltyDefV3",
	KickOff		= {1},
	KickOffDef  = "Ref_KickOffDefV1",
	NorPlay     = "NormalPlayDefend"
}