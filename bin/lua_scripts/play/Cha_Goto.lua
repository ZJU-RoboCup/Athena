local FOUR_POS = {
CGeoPoint:new(100,100),
CGeoPoint:new(100,-100),
CGeoPoint:new(-100,-100),
CGeoPoint:new(-100,100)
}

local TEST_POS1 = CGeoPoint:new(200,0)
local TEST_POS2 = CGeoPoint:new(100,0)
local TEST_POS3 = CGeoPoint:new(0,0)
local TEST_POS4 = CGeoPoint:new(-100,0)
local TEST_POS5 = CGeoPoint:new(-200,0)
local TEST_POS6 = CGeoPoint:new(-300,0)

gPlayTable.CreatePlay{

firstState = "state1",

["state1"] = {
	switch = function()
		if vision:Ball():X() < 0 then
			return "state2"
		else
			return "state1"
		end
	end,
	Special  = {SmartGoto{pos = TEST_POS1, pre = pre.high}},
	Leader   = {SmartGoto{pos = TEST_POS2}},
	Assister = {SmartGoto{pos = TEST_POS3}},
	Defender = {SmartGoto({pos = TEST_POS4})},
	Middle   = {SmartGoto{pos = TEST_POS5}},
	Goalie   = {SmartGoto{pos = TEST_POS6}},
	match = "[LS]{ADM}"
},

["state2"] = {
	switch = function()
		if vision:Ball():X() > 20 then
			return "state1"
		else
			return "state2"
		end
	end,
	Special  = {RunMultiPos{pos = FOUR_POS}},
	Leader   = {RunMultiPos{pos = FOUR_POS}},
	Assister = {RunMultiPos{pos = FOUR_POS}},
	Defender = {RunMultiPos{pos = FOUR_POS}},
	Middle   = {RunMultiPos{pos = FOUR_POS}},
	Goalie   = {RunMultiPos{pos = FOUR_POS}},
	match = "{ADMLS}"
},

name = "Cha_Goto",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
