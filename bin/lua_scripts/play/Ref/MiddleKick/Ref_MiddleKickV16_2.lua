-- 使用两车开球一传一后卫上前中场射门
-- by GTY 2016.3.30

local TMP_POS = ball.refSyntYPos(CGeoPoint:new_local(150,0))
local ANTI_CORNER_POS = ball.refAntiYPos(CGeoPoint:new_local(250,150))
local SYNT_CORNER_POS2 = ball.refSyntYPos(CGeoPoint:new_local(230,150))
local KICK_POS = ball.refAntiYPos(CGeoPoint:new_local(-120,260))
local WAIT_POS = ball.refAntiYPos(CGeoPoint:new_local(-300,200))
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
	Special  = task.goCmuRush(WAIT_POS),
	Middle   = task.goCmuRush(TMP_POS),
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
	Special  = task.goCmuRush(WAIT_POS),
	Middle   = task.goCmuRush(ANTI_CORNER_POS),
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
	Leader   = task.goBackBall(CGeoPoint:new_local(325,0), 20),
	Special  = task.goCmuRush(WAIT_POS),
	Middle   = task.goCmuRush(ANTI_CORNER_POS),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

["chooseKicker"] = {
	switch = function()
		if bufcnt(true,40) then
			print ("passball")
			return "passBall"
		end
	end,
	Assister = task.goBackBall("Special", 25),
	Leader   = task.goBackBall(CGeoPoint:new_local(325,0), 20),
	Special  = task.goCmuRush(KICK_POS),
	Middle   = task.goCmuRush(ANTI_CORNER_POS),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

["passBall"] = {
	switch = function()
		if bufcnt(player.kickBall("Assister"),1,600) then
			print ("kickball")
			return "kickBall"
		end
	end,
	Assister = task.touchPass(KICK_POS,500),
	Special  = task.goCmuRush(KICK_POS),
	Middle   = task.goCmuRush(ANTI_CORNER_POS),
	Defender = task.leftBack(),
	Leader   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

["kickBall"] = {
	switch = function()
		if  bufcnt(player.kickBall("Special"),1,600) then
			print("exit")
			return "exit"
		end
	end,
	Assister = task.defendMiddle(),
	Special  = task.waitTouchNew(),
	Middle   = task.goCmuRush(ANTI_CORNER_POS),
	Defender = task.leftBack(),
	Leader   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

name = "Ref_MiddleKickV16_2",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}
