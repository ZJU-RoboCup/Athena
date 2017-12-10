local CHIP_POS   = CGeoPoint:new_local(200,-140)
local SHOOT_POS  = CGeoPoint:new_local(0, 200)

gPlayTable.CreatePlay{

firstState = "goto",

["goto"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Kicker")<30,  15)then
			return "dribble"
		end
	end,		
	Kicker  = task.goSpeciPos(SHOOT_POS),
	Tier   =  task.staticGetBall(CGeoPoint:new_local(200,0)),	
	match = ""
},

["dribble"] = {
	switch = function ()
		if bufcnt(player.toBallDist("Tier")<15, 20)then			
			return "chipBall"
		end
	end,		
	Kicker  = task.goSpeciPos(SHOOT_POS),
	Tier   = task.slowGetBall(CHIP_POS),	
	match = ""
},	

["chipBall"] = {
    switch = function ()
		if bufcnt(player.kickBall("Tier"), 1) then						
			return "fixGoto"																			
		end	
	end,		
	Kicker = task.goSpeciPos(SHOOT_POS),
	Tier = task.touchPass(SHOOT_POS,500),	
	match = ""
},

["fixGoto"] = { 
 switch = function ()
		if bufcnt(ball.posX()>200 and player.kickBall("Kicker"), 1) then					
			return "goto"																			
		end					
	end,	
	Kicker = task.waitTouchNew(SHOOT_POS),
	Tier = task.stop(),	
	match = ""
},	

name = "TestWaitTouch",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

