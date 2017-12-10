TOW_POS1 = CGeoPoint:new(-200,0)
			
TOW_POS2 = 	CGeoPoint:new(200,0)
	--CGeoPoint:new(-107,107)

POS3 = CGeoPoint:new(0,100)
POS4 =CGeoPoint:new(0,-100)

gPlayTable.CreatePlay{

firstState = "state1",

["state1"] = {
	switch = function()
		if bufcnt(false, 1 , 200) then
			return "state2"
		end
	end,
	Special  = {SmartGoto{pos = TOW_POS1}},
	--[[Middle = {RunMultiPos{pos = TOW_POS1}},
	Kicker = {RunMultiPos{pos = TOW_POS2}},	--]]
	Kicker = {SmartGoto{pos = TOW_POS1}},
	Assister = {SmartGoto{pos = POS3}},
	match = "{AS}"
},

["state2"] = {
	switch = function()
		if bufcnt(false ,1 , 200) then
			return "state1"
		end
	end,
	Special  = {SmartGoto{pos = TOW_POS2}},	
	--[[Middle = {Stop()}	,	
	Kicker = {Stop()},--]]
	Kicker  = {SmartGoto{pos = TOW_POS2}},	
	Assister = {SmartGoto{pos = POS4}},
	match = "{AS}"	
},

name = "TestPathPlanning",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}