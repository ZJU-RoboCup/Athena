--by hzy 3/28/2016
-- ball.x()<130时使用
local CHIP_POS = CGeoPoint:new_local(400,200)
local PASS_POS = ball.refSyntYPos(CHIP_POS)
local SHOOT_POS = ball.refSyntYPos(CGeoPoint:new_local(180,280))

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
local random = function()
	return os.time()%2==0
end

local debug = false

gPlayTable.CreatePlay{

firstState = "goto",

["goto"] = {
	switch = function ()
		if debug then print("1601 goto") end
		if bufcnt(player.toTargetDist("Leader") < 20,  "fast", 100)then
			return "dribble"
		end
	end,
	Assister = task.staticGetBall(PASS_POS),
	Leader   = task.goCmuRush(PASS_POS),
	Special  = task.runMultiPos(SOME_POS,false,30),
	Defender = task.runMultiPos(SOME_POS2,false,30),
	Middle   = task.defendMiddle(),
	match    = "[LSDM]"
},

["dribble"] = {
	switch = function ()
		if debug then print("dribble") end
		if bufcnt(true, 60) then			
			return "chippass"
		end
	end,		
	Kicker   = task.slowGetBall(PASS_POS,false),
	Leader   = task.goCmuRush(PASS_POS),
	Special  = task.continue(),
	Defender = task.continue(),
	Middle   = task.defendMiddle(),
	match    = "{LSDM}"
},	

["chippass"] = {
    switch = function ()
    	if debug then print("chippass") end
		if bufcnt(player.kickBall("Kicker") ,2,150) then						
			return "flatpass"
		end	
	end,		
	Kicker   = task.chipPass("Leader", 180),
	Leader   = task.goCmuRush(PASS_POS),
	Special  = task.goCmuRush(SHOOT_POS),
	Middle   = task.defendMiddle(),
	Defender = task.continue(),
	match    = "{LSDM}"
},

["flatpass"] = {
    switch = function ()
    	if debug then print("flatpass") end
    	if bufcnt(player.kickBall("Leader") ,2,150) then
    		return "shoot"
    	end
	end,		
	Kicker = task.runMultiPos(SOME_POS,false,30),
	Leader   = task.receivePass("Special",400),
	Special  = task.waitTouchNew(SHOOT_POS),
	Middle   = task.defendMiddle(),
	Defender = task.continue(),
	match    = "{SLMD}"
},
["shoot"] = {
	switch = function()
		if debug then print("shoot") end
		if bufcnt(true,80) then
			return "exit"
		end
	end,
	Kicker   = task.continue(),
	Leader   = task.rightBack(),
	Special  = task.waitTouchNew(SHOOT_POS),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	match   = "{S}[LDM]"
},

name = "Ref_FrontKickV1601",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

