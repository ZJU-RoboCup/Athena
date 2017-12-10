--by hzy 3/28/2016
local CHIP_POS = CGeoPoint:new_local(120,80)
local TEMP_POS = CGeoPoint:new_local(250,-285)
local PASS_POS = ball.refSyntYPos(CHIP_POS)
local SHOOT_POS = ball.refSyntYPos(TEMP_POS)

local PASS_DIR = player.toPointDir(SHOOT_POS)

local SOME_POS = {
	ball.refSyntYPos(CGeoPoint:new_local(260,240)),
	ball.refSyntYPos(CGeoPoint:new_local(90,150)),
	ball.refSyntYPos(CGeoPoint:new_local(230,-100)),
	SHOOT_POS
}

local SOME_POS2 = {
	ball.refSyntYPos(CGeoPoint:new_local(330,-100)),
	ball.refSyntYPos(CGeoPoint:new_local(290,-220)),
	ball.refSyntYPos(CGeoPoint:new_local(270,-50)),
	ball.refSyntYPos(CGeoPoint:new_local(400,-160)),
	ball.refSyntYPos(CGeoPoint:new_local(330,-200)),
	ball.refSyntYPos(CGeoPoint:new_local(360,-220)),
	ball.refSyntYPos(CGeoPoint:new_local(300,-50)),
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
	Assister = task.staticGetBall(SHOOT_POS),
	Leader   = task.goCmuRush(CHIP_POS),
	Special  = task.goSupportPos("Leader"),
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
	Special  = task.runMultiPos(SOME_POS,false,40),
	Defender = task.continue(),
	match    = "{ALSD}"
},	

["chippass"] = {
    switch = function ()
    	if debug then print("chippass") end
		if bufcnt(player.kickBall("Assister") ,1,120) then
			return "flatpass"
		end	
	end,		
	Assister = task.chipPass("Leader", 170),
	Leader   = task.goCmuRush(PASS_POS),
	Special  = task.continue(),
	Defender = task.continue(),
	match    = "{ALSD}"
},

["flatpass"] = {
    switch = function ()
    	if debug then print("flatpass") end
    	if bufcnt(player.kickBall("Leader") ,1,150) then
    		return "shoot"
    	end
	end,
	Assister = task.goSupportPos("Defender"),
	Leader   = task.receivePass("Special",550),--task.receivePass("Special",600),
	Special  = task.goCmuRush(SHOOT_POS),
	Defender = task.continue(),
	match    = "{ALSD}"
},
["shoot"] = {
	switch = function()
		if debug then print("shoot") end
		if bufcnt(true,50) then
			return "exit"
		end
	end,
	Assister = task.stop(),
	Leader   = task.rightBack(),
	Special  = task.waitTouchNew(SHOOT_POS),--(SHOOT_POS,1.57),
	Defender = task.leftBack(),
	match   = "{ALSD}"
},

name = "Ref_CornerKickV1601",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

