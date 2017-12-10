local  ballBackPos_1 = function (  )

	local  pos = function (  )
		return ball.pos() + Utils.Polar2Vector(30, (ball.pos() - CGeoPoint:new_local(300,0)):dir())
	end
	return pos

end

local ballBackPos_2 =  function (  )

	local  pos = function (  )
		return ball.pos() + Utils.Polar2Vector(150, (ball.pos() - CGeoPoint:new_local(300,0)):dir())
	end
	return pos

end

local  BALLBACKPOS_1 = ballBackPos_1()
local  BALLBACKPOS_2 = ballBackPos_2()

gPlayTable.CreatePlay{
	

firstState = "goto",

["goto"] = {
	switch = function()
		--if  bufcnt(player.toBallDist("Leader") < 15 and player.toTargetDist("Assister") < 20, "normal") then
		if  bufcnt(player.toBallDist("Leader") < 15 , "normal") then
			return "backPass"
		end
	end,
	Leader = task.staticGetBall(CGeoPoint:new_local(300,0)),
	Assister = task.goSpeciPos(BALLBACKPOS_2),
	Middle =task.goSpeciPos(CGeoPoint:new_local(0,0)),
	
	match = "[LAM]"
},

["backPass"] = {
	switch = function (  )
		if  bufcnt(math.abs(Utils.Normalize((player.toBallDir("Leader") - player.dir("Leader"))))> math.pi / 3,"normal" ) then
			return "kick"
		end
	end,
	Leader  = task.dribbleBackPass("Middle"),
	Middle = task.stop(),
	
	Assister = task.stop(),
	
	match    = "{LAM}"
},
["kick"] = {
	switch =function (  )
		if  bufcnt(player.kickBall("Assister"), "normal") then
			return "exit"
		end
	end,
	Leader   = task.stop(),
	Middle =task.stop(),
	Assister = task.stop(),
	
	match    = "{LAM}"
},


name 	   	= "TestDribbleBackPass",
applicable 	={
	exp = "a",
	a   = true
},
attribute 	= "attack",
timeout 	= 99999,
}
