local ballPos = ball.pos()
gPlayTable.CreatePlay{

firstState = "run",

["run"] = {
	switch = function()
		if player.toTargetDist("Kicker")<50 then
			return "run"
		end
	end,
	Kicker = task.staticGetBall(),
	match = ""
},

name = "TestStaticGetBall",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

