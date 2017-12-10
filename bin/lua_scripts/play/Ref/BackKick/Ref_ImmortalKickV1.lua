local posDistance = 120-- 两车距离
local FreeKick_ImmortalStart_Pos = function ()
	local pos
	local ball2goal = CVector:new_local(CGeoPoint:new_local(450, 0) - ball.pos())
	pos = ball.pos() + Utils.Polar2Vector(posDistance, ball2goal:dir()) --距离
	local tempPos = ball.pos()+ Utils.Polar2Vector(posDistance, ball2goal:dir())
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
			pos = carPos + Utils.Polar2Vector(2, vision:OurPlayer(roleNum):Dir())
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
	Tier   = task.leftBack(),
	Middle   = task.rightBack(),
	Special  = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(0, 150))),
	Defender = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(0, 100))),
	Goalie   = task.goalie(),
	match    = "{A}[MSD]"
},

["changepos"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Tier") < 30, 30, 120)then
			return "getball"
		end
	end,
	Assister = task.slowGetBall(CGeoPoint:new_local(450, 0)),	
	Middle   = task.singleBack(),
	Special  = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(250, 150))),
	Defender = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(-200, 120))),
	Tier   = task.goSpeciPos(FREEKICKPOS),
	Goalie   = task.goalie(),
	match    = "{A}{M}[SD]"
},

["getball"] = {
	switch = function ()
		if bufcnt(player.InfoControlled("Assister") and player.toTargetDist("Tier") < 30, 30, 120) then
			return "chipball"
		end
	end,
	Assister = task.slowGetBall(CGeoPoint:new_local(450, 0)),
	Special  = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(200, 150))),	
	Tier   = task.goSpeciPos(Field_RobotFoward_Pos("Tier")),	
	Defender = task.leftBack(),
	Middle   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{A}{S}[DM]"
},	

["chipball"] = {
    switch = function ()
		if bufcnt(player.kickBall("Assister") or
				  player.toBallDist("Assister") > 20, "fast") then
			return "waitball"
		elseif  bufcnt(true,60) then
			return "exit"
		end
	end,
	Assister = task.chipPass(CGeoPoint:new_local(450, 20), 11000),--chip的力度
	Tier   = task.stop(),
	Special  = task.goSpeciPos(ball.refAntiYPos(CGeoPoint:new_local(200, 170))),
	Defender = task.leftBack(),
	Middle   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{A}{S}[DM]"
},

["waitball"] = {
    switch = function ()
		if bufcnt( math.abs(Utils.Normalize((player.dir("Tier") - player.toBallDir("Tier"))))< math.pi / 4, "normal") then
			return "shootball"
		elseif  bufcnt(true, 40) then
			return "exit"
		end
	end,
	Tier   = task.goSpeciPos(PreGoPos("Tier")),
	Assister = task.stop(),
	Special  = task.goSupportPos("Tier"),
	Defender = task.leftBack(),
	Middle   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{A}{S}[DM]"
},

["shootball"] = {
    switch = function ()
		if bufcnt(false, 1, 60) then
			return "exit"
		end
	end,
	Tier     = task.chaseNew(),
	Special  = task.goSupportPos("Tier"),
	Assister = task.sideBack(),
	Defender = task.leftBack(),
	Middle   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{S}[ADM]"
},

name = "Ref_ImmortalKickV1",
applicable ={
	exp = "a",
	a   = true
},
attribute = "attack",
timeout = 99999
}
