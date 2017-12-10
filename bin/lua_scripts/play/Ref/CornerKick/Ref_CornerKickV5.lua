-- 2014-07-20 yys 改
local FINAL_SHOOT_POS = CGeoPoint:new_local(370, 110)
local LEFT_FRONT_POS = {
	ball.refAntiYPos(CGeoPoint:new_local(-50, 60)),
	ball.refAntiYPos(CGeoPoint:new_local(320, 60))
}

local RIGHT_FRONT_POS = {
	ball.refAntiYPos(CGeoPoint:new_local(-50,140)),
	ball.refAntiYPos(FINAL_SHOOT_POS)
}

local BACK_POS = {
	ball.refAntiYPos(CGeoPoint:new_local(-80, 100)),
	ball.refAntiYPos(CGeoPoint:new_local(340, 85))
}

local FAKE_ANTI_POS    = ball.refAntiYPos(CGeoPoint:new_local(300, 80))
local FAKE_SYNT_POS    = ball.refSyntYPos(CGeoPoint:new_local(280, 150))
local CHIP_POS         = pos.passForTouch(FINAL_SHOOT_POS)

local RUN_LEFT_POS  = task.runMultiPos(LEFT_FRONT_POS)
local RUN_RIGHT_POS = task.runMultiPos(RIGHT_FRONT_POS)
local RUN_BACK_POS  = task.runMultiPos(BACK_POS)

local WAIT_TOUCH_DIR = ball.refSyntYDir(0.5)
local WAIT_START_POS = ball.refAntiYPos(CGeoPoint:new_local(240,65))
local WAIT_END_POS   = ball.refAntiYPos(CGeoPoint:new_local(325,30))
gPlayTable.CreatePlay{

firstState = "gotoState",

["gotoState"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Leader") < 15, 30) then
			return "dribbleState"
		end
	end,
	Assister = task.staticGetBall(CHIP_POS),
	Leader   = task.goCmuRush(BACK_POS[1],_,_,flag.allow_dss),
	Special  = task.goCmuRush(LEFT_FRONT_POS[1],_,_,flag.allow_dss),
	Middle   = task.goCmuRush(RIGHT_FRONT_POS[1],_,_,flag.allow_dss),
	Defender = task.goCmuRush(FAKE_ANTI_POS,_,_,flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{MLSD}"
},

["dribbleState"] = {
	switch = function()
		if bufcnt(player.toPointDist("Leader",BACK_POS[2]()) < 140, "fast", 180) then
			return "chipBall"
		end
	end,
	Assister = task.slowGetBall(CHIP_POS),
	Leader   = RUN_BACK_POS,
	Special  = RUN_LEFT_POS,
	Middle   = RUN_RIGHT_POS,
	Defender = task.goCmuRush(FAKE_SYNT_POS,_,_,flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{AMLSD}"
},

["chipBall"] = {
	switch = function()
		if player.kickBall("Assister") or
		   player.toBallDist("Assister") > 30 then
			return "fixGoto"
		end
	end,
	Assister = task.chipPass(CHIP_POS, 180),
	Leader   = RUN_BACK_POS,
	Special  = RUN_LEFT_POS,
	Middle   = RUN_RIGHT_POS,
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{AMLSD}"
},

["fixGoto"] = {
	switch = function()
		if bufcnt(true, 40) then
			return "kickBall"
		end
	end,
	Leader   = RUN_BACK_POS,
	Special  = RUN_LEFT_POS,
	Middle   = RUN_RIGHT_POS,
	Assister = task.rightBack(),
	Defender = task.leftBack(),
	Goalie   = task.goalie(),
	match = "{LMS}(AD)"
},

["kickBall"] = {
	switch = function()
		if player.kickBall("Middle") then
			return "finish"
		elseif bufcnt(true, 100) then
			return "exit"
		end
	end,
	Leader   = task.touchBetweenPos(BACK_POS[2], LEFT_FRONT_POS[2]),
	Special  = task.defendMiddle(),
	Middle 	 = task.waitTouch(RIGHT_FRONT_POS[2], 0),
	Assister = task.rightBack(),
	Defender = task.leftBack(),
	Goalie   = task.goalie(),
	match = "{AMLSD}"
},

name = "Ref_CornerKickV5",
applicable ={
	exp = "a",
	a = true
},
score = 0,
attribute = "attack",
timeout   = 99999
}
