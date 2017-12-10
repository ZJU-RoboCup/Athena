local FreeKick_ImmortalStart_Pos = function ()
	local pos
	local ball2goal = CVector:new_local(CGeoPoint:new_local(450, 0) - ball.pos())
	pos = ball.pos() + Utils.Polar2Vector(100, ball2goal:dir())
	local tempPos = ball.pos()+ Utils.Polar2Vector(50, ball2goal:dir())
	for i = 1, 6 do
		if vision:TheirPlayer(i):Valid() then
			if vision:TheirPlayer(i):Pos():dist(tempPos) < 20 then
				local dir = (CGeoPoint:new_local(450, 30 * ball.antiY()) - vision:TheirPlayer(i):Pos()):dir()
				pos = vision:TheirPlayer(i):Pos() + Utils.Polar2Vector(20, dir)
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

PreGoPos = function (role)
	return function ()
		return player.pos(role) + Utils.Polar2Vector(5, player.dir(role))
	end
end

gPlayTable.CreatePlay{

firstState = "startball",

["startball"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Special") < 30 or
		          player.toTargetDist("Defender") < 30, 70, 120)then
			return "changepos"
		end
	end,
	Assister = task.staticGetBall(CGeoPoint:new_local(450, 0)),
	Leader   = task.defendMiddle(),
	Middle   = task.singleBack(),
	Special  = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(0, 150))),
	Defender = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(0, 100))),
	Goalie   = task.goalie(),
	match    = "{AL}[MSD]"
},

["changepos"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Leader") < 30, 30, 120)then
			return "getball"
		end
	end,
	Assister = task.slowGetBall(CGeoPoint:new_local(450, 0)),	
	Middle   = task.singleBack(),
	Special  = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(250, 150))),
	Defender = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(-200, 120))),
	Leader   = task.goSpeciPos(FREEKICKPOS),
	Goalie   = task.goalie(),
	match    = "{LMA}[SD]"
},

["getball"] = {
	switch = function ()
		if bufcnt(player.InfoControlled("Assister") and player.toTargetDist("Leader") < 30, 30, 120) then
			return "chipball"
		end
	end,
	Assister = task.slowGetBall(CGeoPoint:new_local(450, 0)),
	Special  = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(200, 150))),	
	Leader   = task.goSpeciPos(Field_RobotFoward_Pos("Leader")),	
	Defender = task.leftBack(),
	Middle   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LAS}[DM]"
},	

["chipball"] = {
    switch = function ()
		if bufcnt(player.kickBall("Assister") or
				  player.toBallDist("Assister") > 20, "fast") then
			return "shootball"
		elseif  bufcnt(true,60) then
			return "exit"
		end
	end,
	Assister = task.chipPass(CGeoPoint:new_local(450, 0), 8000), --踢球车
	Leader   = task.stop(),
	Special  = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(200, 150))),
	Defender = task.leftBack(),
	Middle   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{ALS}[DM]"
},
--[[
["waitball"] = {
    switch = function ()
		if bufcnt( math.abs(Utils.Normalize((player.dir("Leader") - player.toBallDir("Leader"))))< math.pi / 4, "normal") then
			return "shootball"
		elseif  bufcnt(true, 40) then
			return "exit"
		end
	end,
	Leader   = task.goSpeciPos(PreGoPos("Leader")),
	Assister = task.stop(),
	Special  = task.goSupportPos("Leader"),
	Defender = task.leftBack(),
	Middle   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{ALS}[DM]"
},
]]--
["shootball"] = {
    switch = function ()
		if bufcnt(false, 1, 60) then
			return "exit"
		end
	end,
	Leader   = task.chaseNew(),
	Special  = task.goSupportPos("Leader"),
	Assister = task.sideBack(),
	Defender = task.leftBack(),
	Middle   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LS}[ADM]"
},

name = "Ref_MiddleKickV4",
applicable ={
	exp = "a",
	a   = true
},
attribute = "attack",
timeout = 99999
}
