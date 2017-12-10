
local SHOOT_POS = ball.refSyntYPos(CGeoPoint:new_local(90,-130))

gPlayTable.CreatePlay{

firstState = "wait",

["wait"] = {
	switch   = function()
		print("wait")
		if bufcnt( ball.valid() , 60 ,6000) then
			return "getball"
		end
	end,
	Tier   = task.stop(),
	Kicker = task.goCmuRush(SHOOT_POS),
	match  = ""
},
["getball"] = {
	switch   = function()
		print("getball")
		if bufcnt( player.toTargetDist("Tier") < 20 and player.toTargetDist("Tier") < 20 , 20 ,100) then
			return "dribble"
		end
	end,
	Tier   = task.staticGetBall(SHOOT_POS),
	Kicker = task.goCmuRush(SHOOT_POS),
	match  = ""
},
["dribble"] = {
	switch   = function()
		print("dribble")
		if bufcnt(true ,40) then
			return "chippass"
		end
	end,
	Tier   = task.slowGetBall(SHOOT_POS,false),
	Kicker = task.goCmuRush(SHOOT_POS),
	match  = ""
},
["chippass"] = {
	switch   = function()
		print("chippass")
		if bufcnt(player.kickBall("Tier"),1,120) then
			return "shoot"
		end
	end,
	Tier   = task.chipPass("Kicker"),
	Kicker = task.goCmuRush(SHOOT_POS),
	match  = ""
},
["shoot"] = {
	switch = function()
		print("shoot")
		if bufcnt(player.kickBall("Kicker"),60,120) then
			return "wait"
		end
	end,
	Tier   = task.stop(),
	Kicker = task.waitTouchNew(SHOOT_POS),
	match  = ""
},
name = "TestWaitChip",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}
