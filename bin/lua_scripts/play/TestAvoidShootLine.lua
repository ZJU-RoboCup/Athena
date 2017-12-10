local TWO_POS_1 = {
CGeoPoint:new(0,150),
CGeoPoint:new(0,-150)
}
local TWO_POS_2 = {
CGeoPoint:new(50,150),
CGeoPoint:new(50,-150)
}
local TWO_POS_3 = {
CGeoPoint:new(-50,150),
CGeoPoint:new(-50,-150)
}


gPlayTable.CreatePlay{

firstState = "start",

["start"] = {
	switch = function ()
		if bufcnt(false, "fast", 60) then
			return "get"
		end
	end,
	Assister = task.stop(),
	Leader   = task.stop(),	
	Special = {RunMultiPos{pos = TWO_POS_1}},
	Middle = {RunMultiPos{pos = TWO_POS_2}},
	Defender = {RunMultiPos{pos = TWO_POS_3}},
	match    = "[LA]{SMD}"
},

["get"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Assister") < 10 , "normal")then
			return "pass"
		end
	end,
	Assister = task.testGoPassPos("Leader"),
	Leader   = task.slowGetBall(CGeoPoint:new_local(300,0)),
	Special = {RunMultiPos{pos = TWO_POS_1,sender = "Leader"}},
	Middle = {RunMultiPos{pos = TWO_POS_2,sender = "Leader"}},
	Defender = {RunMultiPos{pos = TWO_POS_3,sender = "Leader"}},
	match    = "{LA}{SMD}"
},


["pass"] = {
	switch = function ()
		if bufcnt(player.kickBall("Leader") , "fast") then
			return "kick1"
		end
	end,
	Assister = task.testGoPassPos("Leader"),
	Leader   = task.pass("Assister"),		
	Special = {RunMultiPos{pos = TWO_POS_1,sender = "Leader"}},
	Middle = {RunMultiPos{pos = TWO_POS_2,sender = "Leader"}},
	Defender = {RunMultiPos{pos = TWO_POS_3,sender = "Leader"}},
	match    = "{LA}{SMD}"
},	

["kick1"] = {
    switch = function ()
		if bufcnt(player.kickBall("Assister") ,"fast") then
			local tmp
			tmp = gRoleNum["Leader"]
			gRoleNum["Leader"] = gRoleNum["Assister"]
			gRoleNum["Assister"] = tmp
			return "kick1"
		end
	end,
	Leader   = task.testGoPassPos("Assister"),
	Assister = task.receivePass("Leader"),	
	Special = {RunMultiPos{pos = TWO_POS_1,sender = "Assister"}},
	Middle = {RunMultiPos{pos = TWO_POS_2,sender = "Assister"}},
	Defender = {RunMultiPos{pos = TWO_POS_3,sender = "Assister"}},
	match    = "{LA}{SMD}"
},


name = "TestAvoidShootLine",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}