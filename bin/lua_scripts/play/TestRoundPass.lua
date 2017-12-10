-- 此脚本可以用来进行学习型转身传球的参数调整
-- 需要结合脚本RoundBallTest来进行调整

gPlayTable.CreatePlay{

firstState = "gotoState",

["gotoState"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Goalie") < 6 and
				player.toTargetDist("Kicker") < 6, 60) then
			return "roundState"
		end
	end,
	Goalie = task.goBackBall(0, 20),
	Kicker = task.goSpeciPos(ball.antiYPos(CGeoPoint:new_local(240,150))),
	match  = ""
},

["roundState"] = {
	switch = function()
		if player.kickBall("Goalie") then
			return "kickState"	
		end
	end,
	Goalie = task.testRound("Kicker"),
	Kicker = task.goSpeciPos(ball.antiYPos(CGeoPoint:new_local(240,150))),
	match  = ""
},

["kickState"] = {
	switch = function()
		if player.kickBall("Kicker") then
			return "gotoState"
		end		
	end,
	Goalie = task.stop(),
	Kicker = task.waitTouch(CGeoPoint:new_local(240,-150),0),
	match  = ""
},

name = "TestRoundPass",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}