local KICKER_POS1 = CGeoPoint:new_local(100,-180)
local KICKER_POS2 = CGeoPoint:new_local(100,180)
local TIER_POS1   = CGeoPoint:new_local(180,-180)
local TIER_POS2   = CGeoPoint:new_local(180,180)

gPlayTable.CreatePlay{

firstState = "goto1",
["goto1"] = {
	switch = function()
		if  bufcnt(
			player.toTargetDist("Kicker") < 5 and
			player.toTargetDist("Tier") < 5, 20) then
			return "goto2"
		end
	end,
	Kicker = task.goCmuRush(KICKER_POS1,-1.57,800),
	Tier   = task.goCmuRush(TIER_POS1,-1.57,800),
	match  = ""
},

["goto2"] = {
	switch = function()
		if  bufcnt(
			player.toTargetDist("Kicker") < 5 and
			player.toTargetDist("Tier") < 5, 20) then
			return "goto1"
		end
	end,
	Kicker = task.goCmuRush(KICKER_POS2,-1.57,800),
	Tier   = task.goCmuRush(TIER_POS2,-1.57,800),
	match  = ""
},	


name = "TestTwoCtrMethod",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}