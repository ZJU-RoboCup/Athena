--by hzy 3/28/2016
local CHIP_POS = CGeoPoint:new_local(120,60)
local TEMP_POS = CGeoPoint:new_local(275,280)
local PASS_POS = ball.refSyntYPos(CHIP_POS)
local SHOOT_POS = ball.refSyntYPos(TEMP_POS)

local SOME_POS = {
	ball.refSyntYPos(CGeoPoint:new_local(370,150)),
	ball.refSyntYPos(CGeoPoint:new_local(325,240)),
	ball.refSyntYPos(CGeoPoint:new_local(270,200)),
	ball.refSyntYPos(CGeoPoint:new_local(330,0))
}

local SOME_POS2 = {
	ball.refSyntYPos(CGeoPoint:new_local(210,150)),
	ball.refSyntYPos(CGeoPoint:new_local(170,230)),
	ball.refSyntYPos(CGeoPoint:new_local(290,-50)),
	ball.refSyntYPos(CGeoPoint:new_local(200,-200))
}

local random = function()
	return true--os.time()%2==0
end

local debug = false

gPlayTable.CreatePlay{

firstState = "goto",

["goto"] = {
	switch = function ()
		if debug then print("goto") end
		if bufcnt(player.toTargetDist("Leader") < 30,  1,60)then
			return "dribble"
		end
	end,
	Assister = task.staticGetBall(),
	Leader   = task.goCmuRush(CHIP_POS),
	Special  = task.runMultiPos(SOME_POS,false,30),
	Defender = task.runMultiPos(SOME_POS2,false,30),
	match    = "{A}[LSD]"
},

["dribble"] = {
	switch = function ()
		if debug then print("dribble") end
		if bufcnt(true, 50)then			
			return "chippass"
		end
	end,		
	Assister = task.slowGetBall(PASS_POS,false),
	Leader   = task.goCmuRush(PASS_POS),
	Special  = task.continue(),
	Defender = task.continue(),
	match    = "{ALSD}"
},

["chippass"] = {
    switch = function ()
    	if debug then print("chippass") end
		if bufcnt(player.kickBall("Assister") ,1,60) then
			return "flatpass"
		end	
	end,		
	Assister = task.chipPass("Leader", 120),
	Leader   = task.goCmuRush(PASS_POS),
	Special  = task.continue(),
	Defender = task.continue(),
	match    = "{ALSD}"
},

["flatpass"] = {
    switch = function ()
    	if debug then print("flatpass") end
    	if bufcnt(player.kickBall("Leader"),1,140) then
    		return "shoot"
    	end
	end,
	Assister = task.goCmuRush(SHOOT_POS),
	Leader   = task.pass("Assister",450),
	Special  = task.continue(),
	Defender = task.continue(),
	match    = "{ALSD}"
},
["shoot"] = {
	switch = function()
		if debug then print("shoot") end
		if bufcnt(true,120) or player.kickBall("Assister") then
			return "exit"
		end
	end,
	Assister = task.waitTouchNew(SHOOT_POS),
	Leader   = task.rightBack(),
	Special  = task.continue(),--(SHOOT_POS,1.57),
	Defender = task.leftBack(),
	match   = "{ALSD}"
},

name = "Ref_CornerKickV1602",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

