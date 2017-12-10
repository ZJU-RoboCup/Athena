gPlayTable.CreatePlay{

firstState = "goto",

["goto"] = {
	switch = function ()
		if cond.isNormalStart() then
			return "slowGoto"
		elseif cond.isGameOn() then
			return "exit"
		end
	end,
	Leader  = task.goSpeciPos(CGeoPoint:new_local(320,0)),
	Middle  = task.goSpeciPos(CGeoPoint:new_local(0,100)),
	Special = task.goSpeciPos(CGeoPoint:new_local(0,-100)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie  = task.goalie(),
	match = "{L}{ASDM}"
},

["slowGoto"] = {
	switch = function ()
		if bufcnt(ball.toPlayerDist("Leader") < 20, 60) then	
			return "kick"
		end
	end,
	Leader = {StaticGetBall{pos = ball.pos, dir = dir.specified(20)}},
	Middle  = task.goSpeciPos(CGeoPoint:new_local(0,100)),
	Special = task.goSpeciPos(CGeoPoint:new_local(0,-100)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie  = task.goalie(),
	match = "{LASDM}"
},

["kick"] = {
	switch = function ()
		if cond.isGameOn() or player.kickBall("Leader") then
			return "exit"
		end
	end,
	Leader  = task.penaltyKick(),
	Middle  = task.goSpeciPos(CGeoPoint:new_local(0,100)),
	Special = task.goSpeciPos(CGeoPoint:new_local(0,-100)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie  = task.goalie(),
	match = "{LASDM}"
},

name = "Ref_PenaltyKickV1",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
