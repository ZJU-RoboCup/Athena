-- cornerkickV9修改
-- 针对Immortal
-- by yys 2014-04-10
-- 2014-07-20 yys 改
--改点
local FINAL_SHOOT_POS = CGeoPoint:new_local(250, 230)
local BREAKER_POS1 = ball.refAntiYPos(CGeoPoint:new_local(245, 70))
local BREAKER_POS2 = ball.refAntiYPos(CGeoPoint:new_local(255, 90))
local BREAKER_DIR1 = ball.refAntiYDir(-1.57)
local BREAKER_DIR2 = ball.refAntiYDir(1.57)
local BREAKER_POS3 = ball.refAntiYPos(CGeoPoint:new_local(320, -130))

local END_POS1	   = ball.refAntiYPos(CGeoPoint:new_local(350, -150))
local END_POS2	   = ball.refAntiYPos(FINAL_SHOOT_POS)
local END_POS3	   = ball.refAntiYPos(CGeoPoint:new_local(350, -120))

local CHIP_POS	   = FINAL_SHOOT_POS

local READY_POS         = ball.refAntiYPos(CGeoPoint:new_local(-120, 40))
local ANTI_READY_POS    = ball.refAntiYPos(CGeoPoint:new_local(-50, 170))
local ANTI_CORNER_POS   = ball.refAntiYPos(CGeoPoint:new_local(330, 180))
local ANTI_CORNER_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(390, 180))

local REACH_POS	 = ball.refAntiYPos(CGeoPoint:new_local(param.pitchLength/2.0, 40))

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
	Leader   = task.goCmuRush(END_POS1, _, 600, flag.allow_dss),
	Special  = task.goCmuRush(END_POS2, _, 600, flag.allow_dss),
	Middle   = task.goCmuRush(BREAKER_POS3, _, _, flag.allow_dss),
	Defender = task.goSimplePos(ANTI_CORNER_POS),
	Goalie   = task.goalie(),
	match    = "{ASMLD}"
},

["chipBall"] = {
	switch = function()
		if player.kickBall("Assister") or player.toBallDist("Assister") > 30 then
			if USE_CHIP then
				return "fixGoto"
			else
				return "kickBall"
			end
		elseif  bufcnt(true, 60) then
			return "exit"
		end
	end,
	Assister = USE_CHIP and task.chipPass(CHIP_POS, 230) or task.goAndTurnKick(CHIP_POS, 500),
	Leader   = task.goCmuRush(END_POS1, _, 600, flag.allow_dss),
	Special  = task.goCmuRush(END_POS2, _, 600, flag.allow_dss),
	Middle   = task.goSimplePos(END_POS3),
	Defender = task.goCmuRush(ANTI_CORNER_POS, _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{ASMLD}"
},

["fixGoto"] = {
	switch = function()
		if bufcnt(true, 45) then
			return "kickBall"
		elseif  bufcnt(true, 80) then
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
	Special  = task.waitTouchNew(END_POS2),--task.InterTouch(END_POS2,REACH_POS,900),
	Middle   = task.rightBack(),
	Assister = task.goSupportPos("Special"),
	Defender = task.goCmuRush(ANTI_CORNER_POS_2, _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{SDA}[LM]"
},

name = "Ref_FrontKickV1609",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}