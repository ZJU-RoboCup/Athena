local WaitPos = {
	CGeoPoint:new_local(225,-250),
	CGeoPoint:new_local(225,250)
}
gPlayTable.CreatePlay{

firstState = "getball",
["getball"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Assister") < 20 and player.toTargetDist("Leader") < 20, 20) then
			return "firstpass"
		end
	end,
	Assister   = task.staticGetBall("Leader"),
	Leader   = task.goCmuRush(WaitPos[1], player.toBallDir, 600, flag.allow_dss),
	match  = "[AL]"
},

["firstpass"] = {
	switch = function()
		if player.kickBall("Assister") then
			return "pass2"
		end
	end,
	Assister   = task.pass("Leader"),
	Leader   = task.goCmuRush(WaitPos[2], player.toBallDir, 600, flag.allow_dss),
	match  = "[AL]"
},

["pass1"] = {
	switch = function()
		if player.kickBall("Assister") then
			return "pass2"
		end
	end,
	Assister = task.receiveChip("Leader",180),
	Leader   = task.goCmuRush(WaitPos[2], player.toBallDir, 600, flag.allow_dss),
	match  = "[AL]"
},

["pass2"] = {
	switch = function()
		if player.kickBall("Leader") then
			return "pass1"
		end
	end,
	Assister = task.goCmuRush(WaitPos[1], player.toBallDir, 600, flag.allow_dss),
	Leader   = task.receiveChip("Assister",180),
	match  = "[AL]"
},

["stop"] = {
	switch = function()
		return "stop"
	end,
	Leader   = task.stop(),
	Middle   = task.stop(),
	Defender = task.stop(),
	match  = "[AL]"
},

name = "TestChipPassLoop",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}