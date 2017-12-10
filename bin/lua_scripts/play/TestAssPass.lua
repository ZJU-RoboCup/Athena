local MOVE_DIR = math.pi*3/4
local PASSER_STAND_POS = function()
	return ball.pos() + Utils.Polar2Vector(20, Utils.Normalize(MOVE_DIR*ball.antiY() + math.pi))
end
local lastP
local KICKER_STAND_POS = function()
	return ball.pos() + Utils.Polar2Vector(30, MOVE_DIR*ball.antiY()) + Utils.Polar2Vector(-4, ball.toTheirGoalDir())
end

gPlayTable.CreatePlay{

firstState = "gotoState",

["gotoState"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Goalie") < 6 and
				player.toTargetDist("Kicker") <6, "normal") then
			lastP = ball.pos()
			return "passState"
		end
	end,
	Kicker = task.goSpeciPos(PASSER_STAND_POS),
	Goalie = task.goSpeciPos(KICKER_STAND_POS),
	match = ""
},

["passState"] = {
	switch = function()
		if ball.toPointDist(lastP) > 4 then
			return "kickState"
		end
	end,

	Kicker = task.goSpeciPos(ball.pos),
	Goalie = task.goSpeciPos(KICKER_STAND_POS),
	match  = ""
},

["kickState"] = {
	switch = function()

	end,

	Kicker = task.stop(),
	Goalie = task.shoot(),
	match  = ""
},

name = "TestAssPass",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}