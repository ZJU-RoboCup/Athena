local ShootPos = CGeoPoint:new_local(450, 46)

gPlayTable.CreatePlay{

firstState = "start",

["start"] = {
 	switch = function ()
    	if bufcnt(cond.isNormalStart(), 1) then --cond.isNormalStart(), 1
     		return "advance"
    	end
 	end,
 	Leader   = task.goSpeciPos(CGeoPoint:new_local(-170, 0)),
 	Special  = task.goSpeciPos(CGeoPoint:new_local(-320, 80)),
 	Assister = task.goSpeciPos(CGeoPoint:new_local(-320, -80)),
 	Middle   = task.goSpeciPos(CGeoPoint:new_local(-320, 105)),
 	Defender = task.goSpeciPos(CGeoPoint:new_local(-320, -105)),
 	Goalie   = task.goalie(),--Goalie   = task.penaltyGoalie2017V1(),
 	match    = "{LASMD}"
},

["advance"] = {
 	switch = function ()
   		if bufcnt(true, 600) then
     		return "exit"
   		end
 	end,
	Leader   = task.penaltyKick2017V2(), --{SlowGetBall{pos = ball.pos, dir = player.toPointDir(CGeoPoint:new_local(param.pitchLength/2,45))}},
	Special  = task.goSpeciPos(CGeoPoint:new_local(-320, 80)),
 	Assister = task.goSpeciPos(CGeoPoint:new_local(-320, -80)),
 	Middle   = task.goSpeciPos(CGeoPoint:new_local(-320, 105)),
 	Defender = task.goSpeciPos(CGeoPoint:new_local(-320, -105)),
 	Goalie   = task.goalie(),
 	match    = "{LASMD}"
},

name = "Ref_PenaltyKick2017V2",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
