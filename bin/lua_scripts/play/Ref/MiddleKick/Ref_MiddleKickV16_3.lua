-- by GTY 2016.3.30

local MIDDLE_POS1 = ball.refSyntYPos(CGeoPoint:new_local(370,110))
local REC_POS = ball.refSyntYPos(CGeoPoint:new_local(-300,220))
local KICK_POS1 = ball.refSyntYPos(CGeoPoint:new_local(298,200))
local KICK_POS2 = ball.refAntiYPos(CGeoPoint:new_local(430,270))
local PASS_POS = ball.refAntiYPos(CGeoPoint:new_local(430,270))
local WAIT_POS = ball.refAntiYPos(CGeoPoint:new_local(-300,200))
local A_DEFEND_POS = ball.refSyntYPos(CGeoPoint:new_local(250,200))
local BACK_DIR = function(d)
	return function()
		return ball.antiY()*d
	end
end

gPlayTable.CreatePlay{

firstState = "tmpState",

["tmpState"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Middle") < 20, "normal") then
			print("oneReady")
			return "oneReady"
		end
	end,
	Assister = task.goBackBall(CGeoPoint:new_local(325,0), 20),
	Leader   = task.goBackBall(0,50),
	Special  = task.goCmuRush(CGeoPoint:new_local(0,0)),
	Middle   = task.goCmuRush(CGeoPoint:new_local(0,20)),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

["oneReady"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Assister") < 5, "normal") then
			print("twoReady")
			return "twoReady"
		end
	end,
	Assister = task.goBackBall(CGeoPoint:new_local(325,0), 20),
	Leader   = task.goBackBall(0,50),
	Special  = task.goCmuRush(CGeoPoint:new_local(0,0)),
	Middle   = task.goCmuRush(CGeoPoint:new_local(0,20)),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

["twoReady"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Leader") < 5, "normal") then
			print ("chooseKicker")
			return "chooseKicker"
		end
	end,
	Assister = task.goBackBall("Special", 25),
	Leader   = task.goBackBall("Assister", 20),
	Special  = task.goCmuRush(CGeoPoint:new_local(0,0)),
	Middle   = task.goCmuRush(CGeoPoint:new_local(0,20)),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

["chooseKicker"] = {
	switch = function()
		if bufcnt(true,100) then
			print ("passball")
			return "passBall"
		end
	end,
	Assister = task.goBackBall("Special", 25),
	Leader   = task.goCmuRush(KICK_POS1),
	Special  = task.goCmuRush(REC_POS),
	Middle   = task.goCmuRush(MIDDLE_POS1),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

["passBall"] = {
	switch = function()
		if bufcnt(player.kickBall("Assister"),1,600) then
			print ("touchpass")
			return "touchpass"
		end
	end,
	Assister = task.receivePass(REC_POS),
	Leader   = task.goCmuRush(KICK_POS2),
	Special  = task.goCmuRush(REC_POS),
	Middle   = task.goCmuRush(MIDDLE_POS1),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

["touchpass"] = {
	switch = function()
		if  bufcnt(player.kickBall("Special"),1,600) then
			print("gotoball")
			return "gotoball"
		end
	end,
	Assister = task.goCmuRush(A_DEFEND_POS),
	Leader   = task.goCmuRush(KICK_POS2),
	Special  = task.receivePass(PASS_POS,800),
	Middle   = task.goCmuRush(MIDDLE_POS1),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

["gotoball"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Leader") <20,1,120) then
			print("kickball")
			return "kickball"
		end
	end,
	Assister = task.goCmuRush(A_DEFEND_POS),
	Leader   = task.goCmuRush(KICK_POS2),
	Special  = task.leftBack(),
	Middle   = task.goCmuRush(MIDDLE_POS1),
	Defender = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},
["kickball"] = {
	switch = function()
		if  bufcnt(player.kickBall("Leader"),1,240) then
			print("exit")
			return "exit"
		end
	end,
	Assister = task.defendMiddle(),
	Leader   = task.receiveShoot(),
	Special  = task.leftBack(),
	Middle   = task.goCmuRush(MIDDLE_POS1),
	Defender = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},
name = "Ref_MiddleKickV16_3",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}
