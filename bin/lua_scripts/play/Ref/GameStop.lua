-- change to double size by yys
--[[if ball.refPosX() < -360 and math.abs(ball.refPosY()) > 200 then
	gCurrentPlay = "Ref_Stop4CornerDef"
elseif ball.refPosX() > 360 and math.abs(ball.refPosY()) > 220 then
	gCurrentPlay = "Ref_StopV1701"
else
	gCurrentPlay = "Ref_StopV1701"
end
--gCurrentPlay = "Ref_StopV1701"
gCurrentPlay = "Ref_StopV2888"--]]
local HALF = gOppoConfig.IfHalfField
if ball.refPosX() < -360 and math.abs(ball.refPosY()) > 200 then
	gCurrentPlay = HALF and "Ref_StopV1701" or "Ref_Stop4CornerDef"
elseif ball.refPosX() > 360 and math.abs(ball.refPosY()) > 220 then
	gCurrentPlay = HALF and "Ref_Stop4CornerKickV1701" or "Ref_Stop4CornerKickV2"
else
	gCurrentPlay = HALF and "Ref_StopV1701" or "Ref_StopV2"
end
