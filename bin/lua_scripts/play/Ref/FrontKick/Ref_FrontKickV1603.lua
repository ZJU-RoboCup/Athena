---- ball.x()>130 人墙战术

local WALL_POS1 = {
	ball.refSyntYPos(CGeoPoint:new_local(330,160)),
	ball.refSyntYPos(CGeoPoint:new_local(300,160)),
	ball.refSyntYPos(CGeoPoint:new_local(270,160))
}
local WALL_POS2 = {
	ball.refSyntYPos(CGeoPoint:new_local(320,50)),
	ball.refSyntYPos(CGeoPoint:new_local(290,50)),
	ball.refSyntYPos(CGeoPoint:new_local(260,50)),
	ball.refSyntYPos(CGeoPoint:new_local(270,130)),
	ball.refSyntYPos(CGeoPoint:new_local(315,130)),
}

local WAIT_POS = ball.refSyntYPos(CGeoPoint:new_local(0,-270))
local FACE_POS = ball.refSyntYPos(CGeoPoint:new_local(340,85))
local SHOOT_POS = ball.refSyntYPos(CGeoPoint:new_local(340,-85))
local debug = true

gPlayTable.CreatePlay{

firstState = "wait",

["wait"] = {
	switch = function ()
		if debug then print("wait") end
		if bufcnt(player.toTargetDist("Defender") < 30,  1, 120)then
			return "dribble"
		end
	end,
	Assister = task.staticGetBall(FACE_POS),
	Leader   = task.goCmuRush(WAIT_POS),
	Special  = task.goCmuRush(WALL_POS1[1]),
	Middle   = task.goCmuRush(WALL_POS1[2]),
	Defender = task.goCmuRush(WALL_POS1[3]),
	match    = "{A}{L}[SMD]"
},

["dribble"] = {
	switch = function ()
		if debug then print("dribble") end
		if bufcnt(true, 50)then
			return "chippass"
		end
	end,		
	Assister = task.slowGetBall(FACE_POS,false),
	Leader   = task.goCmuRush(SHOOT_POS),
	Special  = task.goCmuRush(WALL_POS1[1]),
	Middle   = task.goCmuRush(WALL_POS1[2]),
	Defender = task.goCmuRush(WALL_POS1[3]),
	match    = "{ALSMD}"
},	

["chippass"] = {
    switch = function ()
    	if debug then print("chippass") end
		if bufcnt(player.kickBall("Assister") ,5,80) then						
			return "gofix"
		end	
	end,		
	Assister = task.chipPass(FACE_POS,10000,false),
	Leader   = task.goCmuRush(SHOOT_POS),
	Special  = task.goCmuRush(WALL_POS2[1]),
	Middle   = task.goCmuRush(WALL_POS2[2]),
	Defender = task.goCmuRush(WALL_POS2[3]),
	match    = "{ALSMD}"
},
["gofix"] = {
    switch = function ()
    	if debug then print("gofix") end
		if bufcnt(player.toTargetDist("Leader") < 25 ,2,60) then
			return "shoot"
		end	
	end,
	Assister = task.goCmuRush(WALL_POS2[4]),
	Leader   = task.goCmuRush(SHOOT_POS),
	Special  = task.goCmuRush(WALL_POS2[5]),
	Middle   = task.goCmuRush(WALL_POS2[2]),
	Defender = task.goCmuRush(WALL_POS2[3]),
	match    = "{ALSMD}"
},
["shoot"] = {
    switch = function ()
    	if debug then print("shoot") end
		if bufcnt(player.kickBall("Leader"),2,120) then	
			return "exit"
		end	
	end,
	Assister = task.goCmuRush(WALL_POS2[4]),
	Leader   = task.waitTouchNew(SHOOT_POS,2),
	Special  = task.goCmuRush(WALL_POS2[5]),
	Middle   = task.goCmuRush(WALL_POS2[2]),
	Defender = task.goCmuRush(WALL_POS2[3]),
	match    = "{ALSMD}"
},

name = "Ref_FrontKickV1603",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

