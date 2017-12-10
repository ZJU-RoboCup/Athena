--by hzy 3/28/2016
-- ball.x()<130时使用
local CHIP_POS = CGeoPoint:new_local(250,240)
local PASS_POS = ball.refSyntYPos(CHIP_POS)
local SHOOT_POS = ball.refSyntYPos(CGeoPoint:new_local(0,-160))

local SOME_POS = {
	ball.refSyntYPos(CGeoPoint:new_local(170,190)),
	ball.refSyntYPos(CGeoPoint:new_local(280,150)),
	ball.refSyntYPos(CGeoPoint:new_local(320,-140)),
	ball.refSyntYPos(CGeoPoint:new_local(30,130)),
	SHOOT_POS
}
local SOME_POS2 = {
	ball.refSyntYPos(CGeoPoint:new_local(230,100)),
	ball.refSyntYPos(CGeoPoint:new_local(330,60)),
	ball.refSyntYPos(CGeoPoint:new_local(320,-140)),
	ball.refSyntYPos(CGeoPoint:new_local(30,130)),
	SHOOT_POS
}

local T_DIR = player.toPlayerHeadDir("Assister")

local random = function()
	return os.time()%2==0
end

local debug = false

gPlayTable.CreatePlay{

firstState = "goto",

["goto"] = {
	switch = function ()
		if debug then print("1601 goto") end
		if bufcnt(player.toTargetDist("Leader") < 20,  "fast")then
			return "dribble"
		end
	end,
	Assister = task.staticGetBall(PASS_POS),
	Leader   = task.goCmuRush(PASS_POS,T_DIR),
	Special  = task.runMultiPos(SOME_POS,false,30),
	Defender = task.runMultiPos(SOME_POS2,false,30),
	Middle   = task.defendMiddle(),
	match    = "{A}[LSDM]"
},

["dribble"] = {
	switch = function ()
		if debug then print("dribble") end
		if bufcnt(true, 50)then			
			return "chippass"
		end
	end,		
	Assister = task.slowGetBall(PASS_POS,false),
	Leader   = task.goCmuRush(PASS_POS,T_DIR),
	Special  = task.continue(),
	Defender = task.continue(),
	Middle   = task.defendMiddle(),
	match    = "{ALSDM}"
},	

["chippass"] = {
    switch = function ()
    	if debug then print("chippass") end
		if bufcnt(player.kickBall("Assister") ,1,100) then						
			return "flatpass"
		end	
	end,		
	Assister = task.chipPass("Leader", 90),
	Leader   = task.goCmuRush(PASS_POS,T_DIR),
	Special  = task.goCmuRush(SHOOT_POS),
	Middle   = task.defendMiddle(),
	Defender = task.continue(),
	match    = "{ALSDM}"
},

["flatpass"] = {
    switch = function ()
    	if debug then print("flatpass") end
    	if bufcnt(player.kickBall("Leader") ,1,180) then
    		return "shoot"
    	end
	end,		
	Assister = task.runMultiPos(SOME_POS,true,30),
	Leader   = task.receivePass("Special",500),
	Special  = task.waitTouchNew(SHOOT_POS),
	Middle   = task.defendMiddle(),
	Defender = task.continue(),
	match    = "{ASLMD}"
},
["shoot"] = {
	switch = function()
		if debug then print("shoot") end
		if bufcnt(true,80) then
			return "exit"
		end
	end,
	Assister = task.continue(),
	Leader   = task.rightBack(),
	Special  = task.waitTouchNew(SHOOT_POS),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	match   = "{AS}[LDM]"
},

name = "Ref_MiddleKickV1606",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

