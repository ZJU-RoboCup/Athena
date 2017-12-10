local Many_POS = {
	CGeoPoint:new_local(0,0),
	CGeoPoint:new_local(-30,43),
	CGeoPoint:new_local(0,85),
	CGeoPoint:new_local(50,67),
	CGeoPoint:new_local(100,0),
	CGeoPoint:new_local(50,-67),
	CGeoPoint:new_local(0,-85),
	CGeoPoint:new_local(-30,-43)
	}


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
	Special  = {RunMultiPos{pos = Many_POS}},
	Middle   = {RunMultiPos{pos = Many_POS}},
	Assister = {RunMultiPos{pos = Many_POS}},	
	match    = "{SMA}"
},

["state2"] = {
	switch = function()
		if vision:Ball():X() > 20 then
			return "state1"
		else
			return "state2"
		end
	end,
	Special  = {Stop()},
	Middle   = {Stop()},
	Assister = {Stop()},
	match    = "{SMA}"
},

name = "RunHeartShape",
applicable ={
	exp = "a",
	a   = true
},
attribute = "attack",
timeout   = 99999
}