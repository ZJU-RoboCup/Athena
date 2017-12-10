-- 针对Immortal
-- by yys 2014-04-10
-- 2014-07-20 yys 改

local USE_FAR_POINT = gOppoConfig.CornerKickPosSwitch
local FINAL_SHOOT_POS = USE_FAR_POINT and CGeoPoint:new_local(345, 200) or CGeoPoint:new_local(365 ,105)
local CHIP_POWER = USE_FAR_POINT and 245 or 180
local BREAKER_POS1 = ball.refAntiYPos(CGeoPoint:new_local(265, 70))
local BREAKER_POS2 = ball.refAntiYPos(CGeoPoint:new_local(275, 90))
local BREAKER_DIR1 = ball.refAntiYDir(-1.57)
local BREAKER_DIR2 = ball.refAntiYDir(1.57)
local BREAKER_POS3 = ball.refAntiYPos(CGeoPoint:new_local(250, 10))

local END_POS1	   = ball.refAntiYPos(CGeoPoint:new_local(300, 50))
local END_POS2	   = ball.refAntiYPos(FINAL_SHOOT_POS)
local END_POS3	   = ball.refAntiYPos(CGeoPoint:new_local(250, 20))

local CHIP_POS	   = FINAL_SHOOT_POS

local READY_POS         = ball.refAntiYPos(CGeoPoint:new_local(-100, 40))
local ANTI_READY_POS    = ball.refAntiYPos(CGeoPoint:new_local(-30, 170))
local ANTI_CORNER_POS   = ball.refAntiYPos(CGeoPoint:new_local(290, 170))
local ANTI_CORNER_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(230, 180))

local USE_CHIP  = true



gPlayTable.CreatePlay{

firstState = "readyState",

["readyState"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Leader") < 20 and
			      player.toTargetDist("Special") < 20, 20, 120) then
			return "tmpState"
		end
	end,
	Assister = task.staticGetBall(CHIP_POS),
	Leader   = task.goCmuRush(BREAKER_POS1, BREAKER_DIR1, 600, flag.allow_dss),
	Special  = task.goCmuRush(BREAKER_POS2, BREAKER_DIR2, 600, flag.allow_dss),
	Middle   = task.goCmuRush(READY_POS),
	Defender = task.goCmuRush(ANTI_READY_POS),
	Goalie   = task.goalie(),
	match    = "{A}{SMLD}"
},

["tmpState"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Middle") < 90, "normal", 120) then
			return "gotoState"
		end
	end,
	Assister = USE_CHIP and task.staticGetBall(CHIP_POS) or task.staticGetBall(CHIP_POS),
	Leader   = task.goCmuRush(BREAKER_POS1, BREAKER_DIR1, 600, flag.allow_dss),
	Special  = task.goCmuRush(BREAKER_POS2, BREAKER_DIR2, 600, flag.allow_dss),
	Middle   = task.goCmuRush(BREAKER_POS3),
	Defender = task.goSimplePos(ANTI_CORNER_POS),
	Goalie   = task.goalie(),
	match    = "{ASMLD}"
},

["gotoState"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Middle") < 90, 60, 120) then
			return "chipBall"
		end
	end,
	Assister = USE_CHIP and task.slowGetBall(CHIP_POS) or task.staticGetBall(CHIP_POS),
	Leader   = task.goCmuRush(BREAKER_POS1, BREAKER_DIR1, 600, flag.allow_dss),
	Special  = task.goCmuRush(BREAKER_POS2, BREAKER_DIR2, 600, flag.allow_dss),
	Middle   = task.goCmuRush(BREAKER_POS3, _, _, flag.allow_dss),
	Defender = task.goSimplePos(ANTI_CORNER_POS),
	Goalie   = task.goalie(),
	match    = "{ASMLD}"
},

["chipBall"] = {
	switch = function()
		if player.kickBall("Assister") or player.toBallDist("Assister") > 30 then
			return "fixGoto"
		elseif  bufcnt(true, 60) then
			return "exit"
		end
	end,
	Assister = USE_CHIP and task.chipPass(CHIP_POS, CHIP_POWER) or task.goAndTurnKick(CHIP_POS, 400),
	Leader   = task.goCmuRush(END_POS1, _, 600, flag.allow_dss),
	Special  = task.goCmuRush(END_POS2, _, 600, flag.allow_dss),
	Middle   = task.goSimplePos(END_POS3),
	Defender = task.goCmuRush(ANTI_CORNER_POS, _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{ASMLD}"
},

["fixGoto"] = {
	switch = function()
		if bufcnt(true, 40) then
			return "kickBall"
		elseif  bufcnt(true, 70) then
			return "exit"
		end
	end,
	Leader   = task.goCmuRush(END_POS1, _, 600, flag.allow_dss),
	Special  = task.goCmuRush(END_POS2, _, 600, flag.allow_dss),
	Middle   = task.goSimplePos(END_POS3),
	Assister = task.singleBack(),
	Defender = task.goCmuRush(ANTI_CORNER_POS_2, _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{SMLDA}"
},

["kickBall"] = {
	switch = function()
		if player.kickBall("Special") then
			return "finish"
		elseif  bufcnt(true, 100) then
			return "exit"
		end
	end,
	Leader   = task.leftBack(),
	Special  = task.waitTouchNew(END_POS2),
	Middle   = task.rightBack(),
	Assister = task.goSupportPos("Special"),
	Defender = task.goCmuRush(ANTI_CORNER_POS_2, _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{SDA}[LM]"
},

name = "Ref_CornerKickV9",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}