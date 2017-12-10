local WAIT_POS = CGeoPoint:new_local(200,0)

gPlayTable.CreatePlay{

firstState = "wait",

["wait"] = {
	switch   = function()
		if bufcnt(ball.valid() and player.toTargetDist("Kicker")<20,15,10000) and ball.velMod() > 50 then
			print(ball.velMod())
			return "shoot"
		end
	end,
	Kicker = task.goCmuRush(WAIT_POS),
	match  = ""
},

["shoot"] = {
	switch   = function()
		if player.kickBall("Kicker") or bufcnt(player.toBallDist("Kicker") > 60, 120,10000) then
			return "end"
		end
	end,
	Kicker = task.chaseNew(),
	match  = ""
},

["end"] = {
	switch   = function()
		if bufcnt( ball.valid() and player.toTargetDist("Kicker")<20, 60 ,10000) then
			return "exit"
		end
	end,
	Kicker = task.goCmuRush(WAIT_POS),
	match  = ""
},

name = "TestAhua",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}