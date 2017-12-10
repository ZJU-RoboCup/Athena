
local MIDDLE_POS = CGeoPoint:new_local(150,267)
local SHOOT_POS  = CGeoPoint:new_local(330,-220) --330,150
local PASS_POS  = CGeoPoint:new_local(332,-220) --330,150
local WAITPOS = CGeoPoint:new_local(352,-194)
local WAITDIR = function()
	return dir.playerToBall("Kicker")
end

local gotoball = function()
	return CGeoPoint:new_local(ball.posX()+20,ball.posY()+20)
end

local FINAL_SHOOT_POS = CGeoPoint:new_local(140, 80)
local ANTIPOS_3 = ball.refAntiYPos(FINAL_SHOOT_POS)

gPlayTable.CreatePlay{

firstState = "testState1",

["testState1"] = {
	switch   = function()
		if bufcnt( ball.valid() , 120 ,6000) then
			return "testState2"
		end
	end,
	Goalie = task.stop(),
	Tier   = task.goCmuRush(MIDDLE_POS),
	Kicker = task.goCmuRush(SHOOT_POS),
	match  = ""
},

["testState2"] = {
	switch   = function()
		if bufcnt(player.kickBall("Goalie"), 1,600) then
			return "testState3"
		end
	end,
	Goalie = task.goAndTurnKick("Tier",500),
	Tier   = task.goCmuRush(MIDDLE_POS),
 	Kicker = task.goCmuRush(SHOOT_POS),
	match  = ""
},

["testState3"] = {
	switch   = function()
		if bufcnt(player.kickBall("Tier"), 1,600) then
			return "testState4"
		end
	end,
	Goalie = task.stop(),
	Tier   = task.receivePass(PASS_POS,700),
	Kicker = task.goCmuRush(SHOOT_POS),
	match  = ""
},

["testState4"] = {
	switch   = function()
		if bufcnt(player.toTargetDist("Kicker") <10,2) then
			return "testState5"
		end
	end,
	Goalie = task.stop(),
	Tier   = task.stop(),
	Kicker = task.goCmuRush(SHOOT_POS),--task.waitTouch(WAITPOS,0),
	match  = ""
},

["testState5"] = {
	switch   = function()
		if bufcnt(player.kickBall("Kicker"), 2,300)then
			return "testState1"
		end
	end,
	Goalie = task.stop(),
	Tier   = task.stop(),
	Kicker = task.waitTouchNew(SHOOT_POS),--task.waitTouch(WAITPOS,0),
	match  = ""
},


name = "TestPassShoot",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}
