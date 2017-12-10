gPlayTable.CreatePlay{

firstState = "kick1",

["start"] = {
	switch = function ()
		if bufcnt(false, "fast", 60) then
			return "get"
		end
	end,
	Assister = task.stop(),
	Leader   = task.stop(),	
	match    = "[LA]"
},

["get"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Assister") < 10 , "normal")then
			return "pass"
		end
	end,
	Assister = task.testGoPassPos("Leader"),
	Leader   = task.slowGetBall(CGeoPoint:new_local(-300,0)),
	match    = "[LA]"
},


["pass"] = {
	switch = function ()
		if bufcnt(player.kickBall("Leader")  , "fast") then
			return "kick1"
		end
	end,
	Assister = task.testGoPassPos("Leader"),
	Leader   = task.pass("Assister"),		
	match    = "{LA}"
},	

["kick1"] = {
    switch = function ()
		if bufcnt(player.kickBall("Assister")  or player.isBallPassed("Assister","Leader"),"fast") then
			local tmp
			tmp = gRoleNum["Leader"]
			gRoleNum["Leader"] = gRoleNum["Assister"]
			gRoleNum["Assister"] = tmp
			return "kick1"
		end
	end,
	Leader   = task.testGoPassPos("Assister"),
	Assister = task.receivePass("Leader"),	
	match    = "{LA}"
},


name = "TestPassRecv",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}