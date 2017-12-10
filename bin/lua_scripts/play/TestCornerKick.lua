local FINAL_SHOOT_POS = CGeoPoint:new_local(360, 100)
local ANTIPOS_3 = pos.passForTouch(FINAL_SHOOT_POS)
local ANTIPOS_3 = ball.refAntiYPos(FINAL_SHOOT_POS)

gPlayTable.CreatePlay{

firstState = "getball",

["getball"] = {
	switch   = function()
		if bufcnt(player.toBallDist("Kicker") < 20, 100) then
			return "dribble"
		end
	end,
	Kicker = task.staticGetBall(FINAL_SHOOT_POS),
	Tier   = task.goCmuRush(CGeoPoint:new_local(0,0)),
	match = ""
},

["dribble"] = {
	switch   = function()
		if bufcnt(player.toTargetDist("Tier") < 280, 1) then
			return "chip"
		end
	end,
	Kicker = task.slowGetBall(FINAL_SHOOT_POS),
	Tier = task.goCmuRush(ANTIPOS_3),
	match = ""
},

["chip"] = {
	switch   = function()
		if bufcnt(player.kickBall("Kicker"), 1, 120) then
			return "fix"
		end
	end,
	Kicker = task.chipPass(FINAL_SHOOT_POS, 180),
	Tier = task.goCmuRush(ANTIPOS_3),
	match = ""
},

["fix"] = {
	switch   = function()
		if bufcnt(true, 80) then
			return "kick"
		end
	end,
	Kicker = task.stop(),
	Tier = task.goSimplePos(ANTIPOS_3),
	match = ""
},

["kick"] = {
	switch   = function()
		if bufcnt(player.kickBall("Tier"), 1, 120) then
			return "exit"
		end
	end,
	Kicker = task.stop(),
	Tier = task.waitTouch(ANTIPOS_3, 0),
	match = ""
},


name = "TestCornerKick",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}
