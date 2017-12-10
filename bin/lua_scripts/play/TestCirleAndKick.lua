local GETBALLPOS = CGeoPoint:new_local(0,0)
local ballPos =function (  )
	return ball.pos()
end
local  BALLPOS = function (  )
	return ball.pos()+ Utils.Polar2Vector(15,Utils.Normalize(CVector:new_local(ballPos() - GETBALLPOS):dir()))
end

local  DIR = function (  )
	return Utils.Normalize(CVector:new_local(GETBALLPOS - ball.pos()):dir())
end
gPlayTable.CreatePlay{

firstState = "testState1",
["testState1"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Kicker") < 20, "normal") then
			return "testState2"
		end
	end,
	Defender = task.goAndTurn(),
	Kicker = task.goSpeciPos(GETBALLPOS),
	match 	= "{D}"
},	
["testState2"] = {
	switch =function (  )
		if  bufcnt(player.isMarked("Kicker") == false 
			and (math.abs(Utils.Normalize(CVector:new_local(ball.pos() - player.pos("Defender")):dir() 
				- CVector:new_local(GETBALLPOS - ball.pos()):dir()))< math.pi / 72), "fast") then
			return "Adjust"
		end
	end,
	
	Defender = task.goAndTurn(),
	Kicker = task.goSpeciPos(GETBALLPOS),
	match    =  "{D}"
},
["Adjust"] = {
	switch = function (  )
		if  bufcnt(player.toTargetDist("Defender") < 10 , "fast") then
			return "pass"
		end
	end,
	
	Defender = task.goSpeciPos(BALLPOS,DIR),
	Kicker = task.goSpeciPos(GETBALLPOS),
	match    =  "{D}"
},
["pass"] = {
	switch = function (  )
		if  bufcnt(player.kickBall("Defender"), "normal") then
			return "Kick"
		end
	end,
	
	Defender = task.pass("Kicker"),
	Kicker = task.goSpeciPos(GETBALLPOS),
	match    =  "{D}"
},
["Kick"] = {
	switch =function (  )
		if  bufcnt(player.kickBall("Kicker"), "normal") then
			return "exit"
		end
	end,
	
	Defender = task.stop(),
	Kicker   = task.shoot(),
	match    = "{D}"
},
name = "TestCirleAndKick",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}