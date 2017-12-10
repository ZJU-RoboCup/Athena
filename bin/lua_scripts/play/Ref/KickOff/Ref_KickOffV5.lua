local PASS_POS = pos.chipPassForTouch(CGeoPoint:new_local(245, 265))

gPlayTable.CreatePlay{
firstState = "start",

["start"] = {
	switch = function ()
		if cond.isNormalStart() then
			return "temp"
		end
	end,
	Leader   = task.goSpeciPos(CGeoPoint:new_local(-20, -5)),
	Assister = task.goSpeciPos(CGeoPoint:new_local(-15, -240)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(-15, 240)),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(-170, 280)),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{LASMD}"
},

["temp"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Middle") < 340, 30) then
			return "pass"
		end
	end,
	Leader   = task.slowGetBall(PASS_POS, _, false),
	Assister = task.goSpeciPos(CGeoPoint:new_local(-50, -57)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(-50, 57)),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(240, 280), dir.evaluateTouch(CGeoPoint:new_local(300, 0)), flag.not_avoid_our_vehicle),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{LASMD}"
},

["pass"] = {
	switch = function ()
		if bufcnt(player.kickBall("Leader"), 1, 60) then
			return "fix"
		end
	end,
	Leader   = task.chipPass(PASS_POS, 150, _, false), --120
	Middle   = task.goSpeciPos(CGeoPoint:new_local(240, 280), dir.evaluateTouch(CGeoPoint:new_local(300, 0)), flag.not_avoid_our_vehicle),
	Special  = task.goSpeciPos(CGeoPoint:new_local(-50, 57)),
	Defender = task.rightBack(),
	Assister = task.leftBack(),
	Goalie   = task.goalie(),
	match    = "{LASDM}"
},

["fix"] = {
	switch = function ()
		if bufcnt(true, 30) then
			return "exit"
		end
	end,
	Leader   = task.stop(), --120
	Middle   = task.goSpeciPos(CGeoPoint:new_local(240, 280), dir.evaluateTouch(CGeoPoint:new_local(300, 0)), flag.not_avoid_our_vehicle),
	Special  = task.goSpeciPos(CGeoPoint:new_local(-50, 57)),
	Defender = task.rightBack(),
	Assister = task.leftBack(),
	Goalie   = task.goalie(),
	match    = "{LASDM}"
},

-- TODO
["kick"] = {
    switch = function ()
		if bufcnt(player.kickBall("Middle"), 1, 60) then
			return "exit"
		end
	end,
	Leader   = task.goPassPos("Middle"),
	Special  = task.defendMiddle(),
	Middle   = task.InterTouch(),--task.waitTouchNew(),
	Defender = task.rightBack(),
	Assister = task.leftBack(),
	Goalie   = task.goalie(),
	match    = "{LASDM}"
},

name = "Ref_KickOffV5",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
