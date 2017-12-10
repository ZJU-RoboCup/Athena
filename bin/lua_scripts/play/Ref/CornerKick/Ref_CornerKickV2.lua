-- 2014-07-20 yys 改
local FINAL_SHOOT_POS = CGeoPoint:new_local(360, 90)
local START_POS = ball.refAntiYPos(CGeoPoint:new_local(282,59))
local FAKE_POS	= ball.refAntiYPos(CGeoPoint:new_local(290,79))
local FAKE_POS_2= CGeoPoint:new_local(310, 79)
local END_POS	= ball.refAntiYPos(CGeoPoint:new_local(340, 110))
local END_POS_2	= ball.refAntiYPos(FAKE_POS_2)
local CHIP_POS 	= pos.passForTouch(FAKE_POS_2)
local SYNT_POS	= ball.refSyntYPos(CGeoPoint:new_local(50,100))
local START_DIR = ball.refAntiYDir(1.5)
local END_DIR	= ball.refAntiYDir(-0.987)
local ROUND_TO_PENALTY = task.testLearn(END_POS,END_DIR)
local ADD_POS   = ball.refAntiYPos(CGeoPoint:new_local(275,31))

gPlayTable.CreatePlay{

firstState = "gotoState",

["gotoState"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Leader")<10, 30) then
			return "dribbleState"
		end
	end,
	Assister = task.staticGetBall(CHIP_POS),
	Leader   = task.goCmuRush(START_POS, START_DIR),
	Special  = task.goCmuRush(FAKE_POS, START_DIR),
	Middle 	 = task.goCmuRush(SYNT_POS),
	Defender = task.goCmuRush(ADD_POS),
	Goalie   = task.goalie(),
	match = "{A}{LSMD}"
},

["dribbleState"] = {
	switch = function()
		if bufcnt(true, 60) then
			return "secondState"
		end
	end,
	Assister = task.slowGetBall(CHIP_POS),
	Leader   = task.goCmuRush(START_POS, START_DIR),
	Special  = task.goCmuRush(FAKE_POS, START_DIR),
	Middle 	 = task.goCmuRush(SYNT_POS),
	Defender = task.goCmuRush(ADD_POS),
	Goalie   = task.goalie(),
	match = "[ALSMD]"
},

["secondState"] = {
	switch = function()
		if bufcnt(true, 60) then
			return "chipBall"
		end
	end,
	Assister = task.slowGetBall(CHIP_POS),
	Leader   = ROUND_TO_PENALTY,
	Special  = task.goCmuRush(FAKE_POS, START_DIR),
	Middle 	 = task.goCmuRush(ball.refAntiYPos(CGeoPoint:new_local(320,230))),
	Defender = task.goCmuRush(ADD_POS),
	Goalie   = task.goalie(),
	match = "{ALSMD}"
},

["chipBall"] = {
	switch = function()
		if player.kickBall("Assister") or player.toBallDist("Assister") > 30 then
			return "fixGoto"
		end
	end,
	Assister = task.chipPass(CHIP_POS, 220),
	Leader   = ROUND_TO_PENALTY,
	Special  = task.goCmuRush(FAKE_POS),
	Middle 	 = task.continue(),
	Defender = task.goCmuRush(ADD_POS),
	Goalie   = task.goalie(),
	match = "{ALSMD}"
},

["fixGoto"] = {
	switch = function()
	if  bufcnt(true, 20) then
			return "kickBall"
		end
	end,
	Assister = task.rightBack(),
	Leader   = task.goCmuRush(END_POS),
	Special  = task.goCmuRush(END_POS_2),
	Middle 	 = task.continue(),
	Defender = task.leftBack(),
	Goalie   = task.goalie(),
	match = "{LSM}[DA]"
},

["kickBall"] = {
	switch = function()
		if player.kickBall("Leader") then
			return "finish"
		elseif  bufcnt(true, 100) then
			return "exit"
		end
	end,
	Assister = task.rightBack(),
	Leader   = task.waitTouch(END_POS, 0),
	Special  = task.waitTouch(END_POS_2, 0),
	Middle 	 = task.goSupportPos("Leader"),
	Defender = task.leftBack(),
	Goalie   = task.goalie(),
	match = "{ALSMD}"
},

name = "Ref_CornerKickV2",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
score     = 0,
timeout   = 99999
}