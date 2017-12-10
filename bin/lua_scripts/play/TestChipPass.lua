--by hzy 3/28/2016
-- ball.x()<130时使用
local CHIP_POS = CGeoPoint:new_local(400,200)
local PASS_POS = ball.refSyntYPos(CHIP_POS)
local SHOOT_POS = ball.refSyntYPos(CGeoPoint:new_local(200,100))

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

local debug = true

gPlayTable.CreatePlay{

firstState = "goto",

["goto"] = {
	switch = function ()
		if debug then print("1601 goto") end
		if bufcnt(player.toTargetDist("Kicker") < 20,  10,300)then
			return "dribble"
		end
	end,
	Kicker = task.goSpeciPos(ball.refSyntYPos(CGeoPoint:new_local(-10,200))),
	Tier   = task.goCmuRush(PASS_POS),
	Goalie  = task.runMultiPos(SOME_POS,false,30),
	match    = ""
},

["dribble"] = {
	switch = function ()
		if debug then print("dribble") end
		if bufcnt(true, 300)then			
			return "chippass"
		end
	end,		
	Kicker = task.slowGetBall(PASS_POS,false,300),
	Tier   = task.goCmuRush(PASS_POS),
	Goalie  = task.continue(),
	match    = ""
},	

["chippass"] = {
    switch = function ()
    	if debug then print("chippass") end
		if bufcnt(player.kickBall("Kicker") ,1,1000) then						
			return "flatpass"
		end	
	end,		
	Kicker = task.chipPass("Tier", 180),
	Tier   = task.goCmuRush(PASS_POS),
	Goalie  = task.waitTouchNew(SHOOT_POS),
	match    = ""
},

["flatpass"] = {
    switch = function ()
    	if debug then print("flatpass") end
    	if bufcnt(player.kickBall("Tier") ,1,300) then
    		return "shoot"
    	end
	end,		
	Kicker = task.runMultiPos(SOME_POS,false,30),
	Tier   = task.receivePass("Goalie",500),
	Goalie  = task.waitTouchNew(SHOOT_POS),
	match    = ""
},
["shoot"] = {
	switch = function()
		if debug then print("shoot") end
		if bufcnt(true,600) then
			return "exit"
		end
	end,
	Kicker = task.continue(),
	Tier   = task.stop(),
	Goalie  = task.waitTouchNew(SHOOT_POS),
	match   = ""
},

name = "TestChipPass",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

