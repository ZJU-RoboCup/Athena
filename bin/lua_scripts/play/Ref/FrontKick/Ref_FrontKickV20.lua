-- 针对CMU防守不保持的特点，使用goAndTurnChip，进行两传一射
-- by zhyaic 2014.7.14

local MAKE_BALL_POS = CGeoPoint:new_local(60,160)
-- pos for Leader
local LEADER_POS1 = ball.refSyntYPos(CGeoPoint:new_local(-150,100))
local LEADER_POS2 = ball.refAntiYPos(MAKE_BALL_POS)

-- pos for Special
local SPECIAL_POS1 = ball.refAntiYPos(CGeoPoint:new_local(278,163))

-- pos for Middle
local MIDDLE_POS1 = ball.refSyntYPos(CGeoPoint:new_local(268,119))

gPlayTable.CreatePlay{

firstState = "start",

["start"]={
	switch = function()
		if  bufcnt(player.toTargetDist("Leader")<15 and 
				   player.toTargetDist("Middle")<20, 30) then
			return "run"
		end
	end,
	Assister = task.staticGetBall(CGeoPoint:new_local(325,0)),
	Leader   = task.goCmuRush(LEADER_POS1),
	Middle   = task.goCmuRush(MIDDLE_POS1),
	Special  = task.goCmuRush(SPECIAL_POS1),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match = "{A}[LMSD]"
},

["run"] = {
	switch = function()
		if  player.toTargetDist("Leader") < 250 then
			return "pass"
		end
	end,
	Assister = task.slowGetBall(CGeoPoint:new_local(325,0)),--task.staticGetBall(CGeoPoint:new_local(325,0)),
	Leader   = task.goCmuRush(LEADER_POS2, player.toBallDir),
	Middle   = task.goCmuRush(MIDDLE_POS1),
	Special  = task.goCmuRush(SPECIAL_POS1),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match = "{ALMSD}"
},

["pass"]={
	switch=function ()
		if  bufcnt(player.kickBall("Assister") 
			or ball.velMod() > 50, "fast", 100) then
			return "fix"
		end
	end,
	Assister = task.goAndTurnChip(MAKE_BALL_POS,200),--270--task.goAndTurnKick(MAKE_BALL_POS,400),
	Leader   = task.goCmuRush(LEADER_POS2, player.toBallDir),
	Middle   = task.goCmuRush(MIDDLE_POS1),
	Special  = task.goCmuRush(SPECIAL_POS1),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match = "{ALMSD}"
},

["fix"]={
	switch=function ()
		if  bufcnt(true, 30) then
			return "secondPass"
		end
	end,
	Assister = task.leftBack(),
	Leader   = task.goCmuRush(LEADER_POS2, player.toBallDir),
	Middle   = task.goCmuRush(MIDDLE_POS1, player.toBallDir),
	Special  = task.goCmuRush(SPECIAL_POS1),
	Defender = task.rightBack(),
	Goalie   = task.goalie(),
	match = "{LMS}[AD]"
},

["secondPass"]={
	switch=function ()
		if  bufcnt(player.kickBall("Leader") or player.isBallPassed("Leader", "Middle"), "fast") then
			return "shoot"
		end

		if  bufcnt(true, 150) then
			return "exit"
		end
	end,
	Assister = task.leftBack(),
	Leader   = task.receivePass("Middle",600),
	Middle   = task.goCmuRush(MIDDLE_POS1, player.toBallDir),
	Special  = task.goCmuRush(SPECIAL_POS1),
	Defender = task.rightBack(),
	Goalie   = task.goalie(),
	match = "{LMS}[AD]"
},

["shoot"]={
	switch=function ()
		if  bufcnt(player.kickBall("Middle"), 1, 100) then
			return "exit"
		end
	end,
	Assister = task.leftBack(),
	Leader   = task.defendMiddle(),
	Middle   = task.waitTouch(),--InterNew(),
	Special  = task.goSupportPos("Middle"),
	Defender = task.rightBack(),
	Goalie   = task.goalie(),
	match = "{LMS}[AD]"
},

name = "Ref_FrontKickV20",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}