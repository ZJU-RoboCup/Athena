-- 2014-07-20 yys 改
local FINAL_SHOOT_POS = CGeoPoint:new_local(335,80)
local READY_POS    = ball.refAntiYPos(CGeoPoint:new_local(-170,150))
local BREAKER_POS1 = ball.refAntiYPos(CGeoPoint:new_local(235,70))
local BREAKER_POS2 = ball.refAntiYPos(CGeoPoint:new_local(245,90))
local BREAKER_POS3 = ball.refAntiYPos(CGeoPoint:new_local(225,75))
local END_POS1	   = ball.refAntiYPos(CGeoPoint:new_local(240,20))
local END_POS2	   = ball.refAntiYPos(CGeoPoint:new_local(250,140))
local END_POS3	   = ball.refAntiYPos(FINAL_SHOOT_POS)
local CHIP_POS 	   = pos.passForTouch(FINAL_SHOOT_POS)
local BREAKER_DIR1 = ball.refAntiYDir(-1.57)
local BREAKER_DIR2 = ball.refAntiYDir(1.57)

local CHIP_DIR = player.toPointDir(CHIP_POS)

-- add by zhyaic 2013.6.28
local ANTI_CORNER_POS = ball.refAntiYPos(CGeoPoint:new_local(380,150))
local ANTI_READY_POS = ball.refAntiYPos(CGeoPoint:new_local(-50,170))


local DEBUG = true

gPlayTable.CreatePlay{

firstState = "readyState",

["readyState"] = {
	switch = function()
		if bufcnt(true, 20) then -- player.toTargetDist("Leader")<10
			return "tmpState"
		end
	end,
	Assister = task.staticGetBall(CHIP_POS),
	Leader   = task.goCmuRush(BREAKER_POS1, BREAKER_DIR1),
	Special  = task.goCmuRush(BREAKER_POS2, BREAKER_DIR2),
	Middle   = task.goCmuRush(READY_POS),
	Defender = task.goCmuRush(ANTI_READY_POS),
	Goalie   = task.goalie(),
	match    = "{A}{MLSD}"
},

["tmpState"] = {
	switch = function()
		-- 修改100，使得分开变快
		if bufcnt(player.toTargetDist("Middle") < 100 , "normal",1000) then -- and player.toTargetDist("Assister") < 15
			return "gotoState"
		end
	end,
	Assister = task.staticGetBall(CHIP_POS,false),
	Leader   = task.goCmuRush(BREAKER_POS1, BREAKER_DIR1),
	Special  = task.goCmuRush(BREAKER_POS2, BREAKER_DIR2),
	Middle   = task.goCmuRush(BREAKER_POS3),
	Defender = task.goSimplePos(ANTI_CORNER_POS),
	Goalie   = task.goalie(),
	match    = "{AMLSD}"
},

["gotoState"] = {
	switch = function()
		if DEBUG then print("gotoState") end
		-- 修改30，使得上前插入变快
		if bufcnt(player.toTargetDist("Middle") < 80, "fast", 180) then
			return "dribble"
		end
	end,
	Assister = task.staticGetBall(CHIP_POS,false),
	Leader   = task.goCmuRush(END_POS1),
	Special  = task.goCmuRush(END_POS2),
	Middle   = task.goCmuRush(BREAKER_POS3),
	Defender = task.goSimplePos(ANTI_CORNER_POS),
	Goalie   = task.goalie(),
	match    = "{AMLSD}"
},
["dribble"] = {
	switch = function()
		if DEBUG then print("dribble") end
		if bufcnt(true, 40) then
			return "chipBall"
		end
	end,
	Assister = task.staticGetBall(CHIP_POS,false),
	Leader   = task.goCmuRush(END_POS1),
	Special  = task.goCmuRush(END_POS2),
	Middle   = task.goCmuRush(BREAKER_POS3),
	Defender = task.goSimplePos(ANTI_CORNER_POS),
	Goalie   = task.goalie(),
	match    = "{AMLSD}"
},

["chipBall"] = {
	switch = function()
		if DEBUG then print("chipBall") end
		if player.kickBall("Assister") or
		   player.toBallDist("Assister") > 30 then
			return "fixGoto"
		elseif  bufcnt(true, 60) then
			return "exit"
		end
	end,
	Assister = task.chipPass(CHIP_POS, 150),
	Leader   = task.goCmuRush(END_POS1),
	Special  = task.goCmuRush(END_POS2),
	Middle   = task.goSimplePos(END_POS3),
	Defender = task.goCmuRush(ANTI_CORNER_POS),
	Goalie   = task.goalie(),
	match    = "{AMLSD}"
},

["fixGoto"] = {
	switch = function()
		if DEBUG then print("fixGoto") end
		if bufcnt(true, 30) then
			return "kickBall"
		end
	end,
	Leader   = task.goCmuRush(END_POS1),
	Special  = task.goCmuRush(END_POS2),
	Middle   = task.goSimplePos(END_POS3),
	Assister = task.goSupportPos("Middle"),
	Defender = task.goCmuRush(ANTI_CORNER_POS),
	Goalie   = task.goalie(),
	match    = "{MLSAD}"
},

["kickBall"] = {
	switch = function()
	if DEBUG then print("kickBall") end
		if player.kickBall("Middle") then
			return "finish"
		elseif  bufcnt(true, 90) then
			return "exit"
		end
	end,
	Leader   = task.leftBack(),
	Special  = task.rightBack(),
	Middle   = task.InterTouch(),
	Assister = task.goSupportPos("Middle"),
	Defender = task.goCmuRush(ANTI_CORNER_POS),
	Goalie   = task.goalie(),
	match    = "{MD}[LSA]"
},
-- ["halt"] = {
-- 	switch = function()
-- 		print("halt!!!")
-- 		if bufcnt(false,1) then
-- 			return "exit"
-- 		end
-- 	end,
-- 	Leader   = task.stop(),
-- 	Special  = task.stop(),
-- 	Middle   = task.stop(),
-- 	Assister = task.stop(),
-- 	Defender = task.stop(),
-- 	Goalie   = task.stop(),
-- 	match    = "{ASMDL}"
-- },
name = "Ref_CornerKickV3",
applicable = {
	exp = "a",
	a = true
},
score = 0,
attribute = "attack",
timeout   = 99999
}