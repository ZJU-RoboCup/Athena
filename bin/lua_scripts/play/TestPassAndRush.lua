local RUSH_POS = CGeoPoint:new_local(250, 60)
local RUSH = CGeoPoint:new_local(250, -60)
local WAIT_POS = CGeoPoint:new_local(100,-120)
local CHIP_POS = pos.passForTouch(RUSH_POS)

gPlayTable.CreatePlay{

firstState = "goto",
["goto"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Kicker") < 13 and
			         player.toTargetDist("Tier") < 10, 90) then
			return "rush"
		end
	end,
	Kicker = task.slowGetBall(CHIP_POS),
	Tier   = task.goCmuRush(WAIT_POS),
	match  = ""
},

["rush"] = {
	switch = function()
		if  bufcnt(player.toTheirPenaltyDist("Tier") < 30, 5) then
			return "pass"
		end
	end,
	Kicker = task.slowGetBall(CHIP_POS),
	Tier   = task.goCmuRush(RUSH),
	match  = ""
},

["pass"] = {
	switch = function()
		if  player.kickBall("Kicker") or player.toBallDist("Kicker") > 20 then
			return "fix"
		end
	end,
	Kicker = task.chipPass(CHIP_POS, 170),
	Tier   = task.goCmuRush(RUSH),
	match  = ""
},

["fix"] = {
	switch = function()
		if  bufcnt(true, 50) then
			return "kick"
		end
	end,
	Kicker = task.stop(),
	Tier   = task.goCmuRush(RUSH),
	match  = ""
},

["kick"] = {
	switch = function()
		if  player.kickBall("Tier") then
			return "exit"
		end
	end,
	Kicker = task.stop(),
	Tier   = task.waitTouch(RUSH, 0),
	match  = ""
},

name = "TestPassAndRush",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}