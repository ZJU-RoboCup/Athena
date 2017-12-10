local FreeKick_ImmortalStart_Pos = function ()
	local pos
	local ball2goal = CVector:new_local(CGeoPoint:new_local(450,0) - ball.pos())
	pos = ball.pos() + Utils.Polar2Vector(60,ball2goal:dir())
	local tempPos = ball.pos()+ Utils.Polar2Vector(60, ball2goal:dir())
	for i = 1,6 do
		if vision:TheirPlayer(i):Valid() then
			if vision:TheirPlayer(i):Pos():dist(tempPos) < 20 then
				local dir = (CGeoPoint:new_local(300,30 * ball.antiY()) - vision:TheirPlayer(i):Pos()):dir()
				pos = vision:TheirPlayer(i):Pos() + Utils.Polar2Vector(20,dir)
				break
			end
		else
			pos = tempPos
		end
		
	end
	return pos
end

local Field_RobotFoward_Pos = function (role)
	local pos
	local UpdatePos = function ()
		local roleNum = player.num(role)
		if Utils.PlayerNumValid(roleNum) then
			local carPos = vision:OurPlayer(roleNum):Pos()
			pos = carPos + Utils.Polar2Vector(1, vision:OurPlayer(roleNum):Dir())
		end
		return pos
	end
	return UpdatePos
end

local FREEKICKPOS
local FIELDROBOTPOS
FREEKICKPOS = FreeKick_ImmortalStart_Pos

PSSS = function (role)
	return function (  )
		return player.pos(role) + Utils.Polar2Vector(5,player.dir(role))
	end
	
end

gPlayTable.CreatePlay{

firstState = "changepos",



["changepos"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Leader") < 30 , "normal")then
			return "getball"
		end
	end,
	Assister = task.slowGetBall(CGeoPoint:new_local(450,0)),	
	Leader   = task.goSpeciPos(FREEKICKPOS),
	match    = "{AL}"
},


["getball"] = {
	switch = function ()
		if bufcnt(player.InfoControlled("Assister") and player.toTargetDist("Leader") < 30, 150) then
			return "chipball"
		end
	end,
	Assister = task.slowGetBall(CGeoPoint:new_local(450,0)),
	Leader   = task.goSpeciPos(FREEKICKPOS),	
	--Leader = task.goSpeciPos(PSSS("Leader")),
	--Leader = task.stop(),
	match    = "{AL}"
},	

["chipball"] = {
    switch = function ()
		if bufcnt(player.kickBall("Assister"), "fast") then
			return "waitball"
		end
	end,
	Assister = task.chipPass(CGeoPoint:new_local(300,30),120),
	--Leader = task.stop(),
	Leader = task.goSpeciPos(PSSS("Leader")),
	--Leader = task.goSpeciPos(PSSS("Leader")),
	match    = "{AL}"
},

["waitball"] = {
    switch = function ()
		if bufcnt( math.abs(Utils.Normalize((player.dir("Leader") - player.toBallDir("Leader"))))< math.pi / 6, "normal") then
			return "shootball"
		end
	end,
	
	--Leader   = task.goSpeciPos(player.pos("Leader") + Utils.Polar2Vector(0.1,(CGeoPoint:new_local(300,0) - player.pos("Leader")):dir())),	
	Leader = task.goSpeciPos(PSSS("Leader")),
	Assister = task.stop(),
	match    = "{AL}"
},

["shootball"] = {
    switch = function ()
		if bufcnt(false,1,200) then
			return "exit"
		end
	end,
	
	Leader   = task.crazyPush(),
	Assister = task.stop(),
	match    = "{AL}"
},

name = "TestImmortalKick",
applicable ={
	exp = "a",
	a   = true
},
attribute = "attack",
timeout = 99999
}
