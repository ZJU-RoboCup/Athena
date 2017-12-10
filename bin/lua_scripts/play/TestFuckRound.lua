local FOUR_POS = {
CGeoPoint:new(250,200),
CGeoPoint:new(250,-200),
CGeoPoint:new(-250,-200),
CGeoPoint:new(-250,200)
}

gPlayTable.CreatePlay{

firstState = "state1",

["state0"] = {
	switch = function()
		if bufcnt(true,1200) then
			return "state1"
		end
	end,
	Kicker  = task.runMultiPos(FOUR_POS),
	match = ""
},

["state1"] = {
	switch = function()
		if bufcnt(true,1200) then
			return "state0"
		end
	end,
	Kicker  = task.runMultiPos(FOUR_POS),
	match = ""
},


name = "TestFuckRound",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
