local pos  = {
	CGeoPoint:new_local(240,200),
	CGeoPoint:new_local(-240,200),
	CGeoPoint:new_local(-240,-200),
	CGeoPoint:new_local(240,-200)
}
local maxvel=0
local time = 300
local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)
gPlayTable.CreatePlay{

firstState = "run1",

["run1"] = {
	switch = function()
		if bufcnt(true,time) and player.toTargetDist("Kicker")<50 then
			return "run"..2--math.random(4)
		end
	end,
	Kicker = task.goCmuRush(pos[1],0, _, DSS_FLAG),
	match = ""
},
["run2"] = {
	switch = function()
		if bufcnt(true,time) and player.toTargetDist("Kicker")<50 then
			return "run"..3
		end
	end,
	Kicker = task.goCmuRush(pos[2],math.pi, _, DSS_FLAG),
	match = ""
},
["run3"] = {
	switch = function()
		if bufcnt(true,time) and player.toTargetDist("Kicker")<50 then
			return "run"..4
		end
	end,
	Kicker = task.goCmuRush(pos[3],0, _, DSS_FLAG),
	match = ""
},
["run4"] = {
	switch = function()
		if bufcnt(true,time) and player.toTargetDist("Kicker")<50 then
			return "run"..1--math.random(4)
		end
	end,
	Kicker = task.goCmuRush(pos[4],math.pi, _, DSS_FLAG),
	match = ""
},

name = "TestRun",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

