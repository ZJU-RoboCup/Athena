local SHOOT_POS = ball.refAntiYPos(CGeoPoint:new_local(150, -265))
local PASS_POS  = pos.passForTouch(SHOOT_POS)

gPlayTable.CreatePlay{

firstState = "getball",

["getball"] = {
	switch = function ()
		if bufcnt(player.toBallDist("Kicker") < 20, 120) then
			return "prepare"
		end
	end,
	Kicker = task.staticGetBall(CGeoPoint:new_local(450, -280)),
	Tier   = task.goCmuRush(CGeoPoint:new_local(-300, 0)),
	match  = ""
},

["prepare"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Tier") < 350, 1) then    -- flat 350   chip 320
			return "turnpass"
		end
	end,
	Kicker = task.staticGetBall(CGeoPoint:new_local(450, -280)),
	Tier   = task.goCmuRush(SHOOT_POS),
	match  = ""
},

["turnpass"] = {
	switch = function ()
		if bufcnt(player.kickBall("Kicker"), "fast") then
			return "shoot"
		end
	end,
	Kicker = task.goAndTurnKickQuick(PASS_POS, 500, "flat"),  -- flat 500   chip 130
	Tier   = task.continue(),
	match  = ""	
},

["shoot"] = {
	switch = function ()
		if bufcnt(player.kickBall("Tier"), 1, 60) then    -- flat 60    chip 90
			return "finish"
		end
	end,
	Kicker = task.stop(),
	Tier   = task.waitTouch(SHOOT_POS, 1.57),
	match  = ""	
},

name = "TestTurnPassAndShoot",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}