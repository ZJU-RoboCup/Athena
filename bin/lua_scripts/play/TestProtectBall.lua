gPlayTable.CreatePlay{

firstState = "getball",
["getball"] = {
	switch = function ()
		if bufcnt(ball.valid() and player.toTargetDist("Kicker") < 30 , "slow") then
			
			return "protect"
		end
	end	,
	Kicker = task.testSlowGetBall(CGeoPoint:new_local(200,100)),
	match  = ""
	
},

["protect"] = {
	switch = function ()
		if bufcnt(true,10000) then
			return "getball"
		end
	end	,
	
	Kicker = task.protectBall(),
	match  = ""
},


name = "TestProtectBall",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}