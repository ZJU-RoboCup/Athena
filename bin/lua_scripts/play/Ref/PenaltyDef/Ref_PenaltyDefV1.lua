gPlayTable.CreatePlay{
firstState = "start",

["start"] = {
	switch = function ()
		if bufcnt(cond.isGameOn(), 20)then
			return "exit"
		end
	end,
	Leader   = task.goSpeciPos(CGeoPoint:new_local(150,0)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(-320,80)),
	Assister = task.goSpeciPos(CGeoPoint:new_local(-320,-80)),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(-320,105)),
	Defender = task.goSpeciPos(CGeoPoint:new_local(-320,-105)),
	Goalie   = task.penaltyGoalie(),
	match    = "{LASDM}"
},

name = "Ref_PenaltyDefV1",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
