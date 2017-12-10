local targetdierction
local realdirection = 0
local ballspeed
local balldirection
local flag = true

gPlayTable.CreatePlay{

firstState = "goto",

["goto"] = {
	switch = function ()
	print("goto");
		if bufcnt(player.toTargetDist("Tier")<30,  150)then
		   flag = true
			return "pass"
		end
	end,
	Kicker  = task.goTouchPos(CGeoPoint:new_local(0,160),CGeoPoint:new_local(150,200)),
	Tier   =  task.slowGetBall("Kicker"),
	match = ""
},

["pass"] = {
	switch = function ()
	print("pass")
		if bufcnt(player.kickBall("Tier") or player.isBallPassed("Tier","Kicker"), "fast")then			
			return "fix"
		end
	end,		
	Kicker = task.goTouchPos(CGeoPoint:new_local(0,160),CGeoPoint:new_local(150,200)),
	Tier   = task.chipPass("Kicker"),
	match  = ""
},	

["fix"] = {
	switch = function ()
	print("fix")
		if bufcnt(true, 20) then			
			return "kick"
		end
	end,		
	Kicker = task.goTouchPos(CGeoPoint:new_local(0,160),CGeoPoint:new_local(150,200)),
	Tier   = task.stop(),
	match  = ""
},

["kick"] = {
    switch = function ()
    print("kick")
		if ball.pos():dist(player.pos("Kicker"))<50 and flag == true then
			flag = false		
			ballspeed = ball.velMod()
			balldirection = ball.velDir()			
			local target2me = pos.theirGoal()- player.pos("Kicker")
			targetdirection = Utils.Normalize(Utils.Normalize(ball.velDir() + math.pi)- player.dir("Kicker"))*180/math.pi
		end		
		if bufcnt(player.kickBall("Kicker") or player.kickBall("Goalie"), "fast") then						
			return "goto"																			
		end
	end,		
	Kicker = task.touch(CGeoPoint:new_local(150,200)),	
	Tier = task.stop(),	
	match = ""
},

["recordfile"] = { 
 switch = function ()
		if bufcnt(ball.posX()>200, 1) then					
				realdirection = (Utils.Normalize(balldirection + math.pi)- ball.velDir())*180/math.pi
				local recordfile = io.open("onepassshoot.txt","a")
				recordfile:write(ballspeed.."  "..realdirection.."  "..targetdirection.."\n")
				recordfile:close()
				return "goto"																			
		end					
	end,	
	Kicker = task.stop(),
	Tier = task.stop(),
	match = ""
},	

name = "TestChipPassShoot",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

