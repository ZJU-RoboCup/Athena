local targetdierction
local realdirection = 0
local ballspeed
local balldirection
local flag = true
local ReceiverPos = CGeoPoint:new_local(100,0)

local PasserPos = CGeoPoint:new_local(252.5,152.5)

local TargetPos = {CGeoPoint:new_local(295,195),
				   CGeoPoint:new_local(295,163.624),
				   CGeoPoint:new_local(295,136.54),
				   CGeoPoint:new_local(295,112.583),
				   CGeoPoint:new_local(295,90.93),
				   CGeoPoint:new_local(295,70.9742),
				   CGeoPoint:new_local(295,52.2501),
				   CGeoPoint:new_local(295,34.3838),
				   CGeoPoint:new_local(295,17.0603),
				   CGeoPoint:new_local(295,0),
				   CGeoPoint:new_local(295,-17.0603),
				   CGeoPoint:new_local(295,-34.3838),
				   CGeoPoint:new_local(295,-52.2501),
				   CGeoPoint:new_local(295,-70.9742),
				   CGeoPoint:new_local(295,-90.93),
				   CGeoPoint:new_local(295,-112.583),
				   CGeoPoint:new_local(295,-136.54),}

local Targetdir = {(TargetPos[1]-ReceiverPos):dir(),
                   (TargetPos[2]-ReceiverPos):dir(),
                   (TargetPos[3]-ReceiverPos):dir(),
                   (TargetPos[4]-ReceiverPos):dir(),
                   (TargetPos[5]-ReceiverPos):dir(),
                   (TargetPos[6]-ReceiverPos):dir(),
                   (TargetPos[7]-ReceiverPos):dir(),
                   (TargetPos[8]-ReceiverPos):dir(),
                   (TargetPos[9]-ReceiverPos):dir(),
                   (TargetPos[10]-ReceiverPos):dir(),
               	   (TargetPos[11]-ReceiverPos):dir(),
               	   (TargetPos[12]-ReceiverPos):dir(),
               	   (TargetPos[13]-ReceiverPos):dir(),
                   (TargetPos[14]-ReceiverPos):dir(),
           		   (TargetPos[15]-ReceiverPos):dir(),
                   (TargetPos[16]-ReceiverPos):dir(),
                   (TargetPos[17]-ReceiverPos):dir(),}

local isRecordMode = false
local TIER_TASK
if isRecordMode then
	TIER_TASK = task.goSpeciPos(ReceiverPos,Targetdir[13])
else
	TIER_TASK = task.goTouchPos(ReceiverPos, TargetPos[9])
end

gPlayTable.CreatePlay{

firstState = "goto",

["goto"] = {
	switch = function ()
		print("goto")
		if bufcnt(player.toTargetDist("Kicker")<30 and 
			player.toTargetDist("Tier")<30 and ball.toPlayerDist("Kicker")<15,  "fast")then
		    flag = true
			return "pass"
		end
	end,		
	Kicker  = task.goSpeciPos(PasserPos,player.toPlayerDir("Tier")),
	Tier   =  TIER_TASK,	
	match = ""
},

["pass"] = {
	switch = function ()
		print("pass",player.kickBall("Kicker"))
		if bufcnt(player.kickBall("Kicker")
			or player.toBallDist("Kicker")>30, 1)then			
			return "kick"
		end
	end,		
	Kicker  = task.compensatePass("Tier",500),
	Tier   = TIER_TASK,	
	match = ""
},	

["kick"] = {
    switch = function ()
    	print("kick")
		if ball.pos():dist(player.pos("Tier"))<50 and flag == true then
			flag = false		
			ballspeed = ball.velMod()
			print("ballspeed",ballspeed)
			balldirection = ball.velDir()			
			targetdirection = math.abs(Utils.Normalize(Utils.Normalize(ball.velDir() + math.pi)- 
				player.dir("Tier"))*180/math.pi)				
		end		
		if bufcnt(player.kickBall("Tier")
			or(player.toBallDist("Tier")>20 and ball.velX()>0), 1) then						
			return "recordfile"																			
		end	
	end,		
	Tier = task.touch(TargetPos[13]),	
    Kicker = task.stop(),	
	match = ""
},

["recordfile"] = { 
 switch = function ()
 		print("recordfile")
		if bufcnt(ball.posX()>200, 2) then					
				realdirection = math.abs((Utils.Normalize(balldirection + math.pi)- ball.velDir())*180/math.pi)
				local recordfile = io.open("CompensateRecordFile.txt","a")
				recordfile:write(ballspeed.."  "..realdirection.."  "..targetdirection.."\n")
				recordfile:close()
				return "goto"																			
		end					
	end,	
	Kicker = task.stop(),
	Tier = task.stop(),	
	match = ""
},	

name = "CompensateRecord",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

