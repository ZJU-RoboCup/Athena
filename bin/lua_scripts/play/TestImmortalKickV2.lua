--by patrick
local chipangle = -20---与球门中心的距离(-150~250)
--local chipangle = 220
--local FAKE_PASS_POS = ball.refAntiYPos(CGeoPoint:new_local(46,-248))
local posDistance = 80-- 初始时球与车的距离
local movDistance_one = 5--
local movDistance_two = 10
local movDistance_three = 10
local  buff = 1--重要参数，要调
local chippower = 200--挑球力度
--local factor = chippower/10000  --buff时间参数

local FreeKick_ImmortalStart_Pos = function ()--------初始车位置
	local pos
	local  tempoint = CGeoPoint:new_local(450, chipangle)
	local ball2goal = CVector:new_local(ball.refAntiYPos(tempoint)()- ball.pos())
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

local Field_RobotFoward_Pos = function (role, movDistance)
	local pos
	local UpdatePos = function ()
		local roleNum = player.num(role)
		if Utils.PlayerNumValid(roleNum) then
			local carPos = vision:OurPlayer(roleNum):Pos()
			local tempointforward = CGeoPoint:new_local(450, chipangle)	
			player2goal = CVector:new_local(ball.refAntiYPos(tempointforward)()- carPos)	
			--pos = carPos + Utils.Polar2Vector(movDistance, vision:OurPlayer(roleNum):Dir())
            pos = carPos + Utils.Polar2Vector(movDistance, player2goal:dir())
		end
		return pos
	end
	return UpdatePos
end

local FREEKICKPOS
local FIELDROBOTPOS
FREEKICKPOS = FreeKick_ImmortalStart_Pos

local TEST = false

gPlayTable.CreatePlay{

firstState = "startball",

["startball"] = {
	switch = function ()
		if TEST then print("startball") end
		if bufcnt(true, 70, 120)then
			return "changepos"
		end
	end,
	Kicker = task.slowGetBall(CGeoPoint:new_local(450, chipangle)),
	Tier   = task.leftBack(),
	match    = "{A}[L]"
},

["changepos"] = {
	switch = function ()
		if TEST then print("changepos") end
		if bufcnt(player.toTargetDist("Tier") < 30, 30, 120)then
			return "getball"
		end
	end,
	Kicker = task.slowGetBall(CGeoPoint:new_local(450, chipangle)),	
	Tier   = task.goSpeciPos(FREEKICKPOS),
	match    = "{LA}"
},

["getball"] = {
	switch = function ()
		if TEST then print("getball") end
		if bufcnt(player.InfoControlled("Kicker") and player.toTargetDist("Tier") < 30, 30, 120) then
			return "chipball"
		end
	end,
	Kicker = task.slowGetBall(CGeoPoint:new_local(450, chipangle)),
	Tier   = task.goSpeciPos(Field_RobotFoward_Pos("Tier", movDistance_one)),	
	match    = "{LAS}[DM]"
},	

["chipball"] = {
    switch = function ()
    	if TEST then print("chipball") end
		if bufcnt(player.kickBall("Kicker") or
				  player.toBallDist("Kicker") > 20, "fast") then
			return "waitball"
		elseif  bufcnt(true,60) then
			return "exit"
		end
	end,
	Kicker = task.chipPass(CGeoPoint:new_local(450, chipangle), chippower),--chip的角度力度
	Tier   = task.goSpeciPos(Field_RobotFoward_Pos("Tier", movDistance_two)),
	match    = "{AL}"
},

["waitball"] = {
    switch = function ()
    	if TEST then print("waitball") end
		if bufcnt( math.abs(Utils.Normalize((player.dir("Tier") - player.toBallDir("Tier"))))< math.pi / 2, buff, 10) then
			return "shootball"
		elseif  bufcnt(true, 40) then
			return "exit"
		end
	end,
	Tier   = task.goSpeciPos(Field_RobotFoward_Pos("Tier", movDistance_three)),
	Kicker = task.stop(),
	match    = "{AL}"
},

["shootball"] = {
    switch = function ()
    	if TEST then print("shoottball") end
		if bufcnt(false, 1, 120) then
			return "exit"
		end
	end,
	Tier   = task.chaseNew(),
	Kicker = task.sideBack(),
	match    = "{L}[A]"
},

name = "TestImmortalKickV2",
applicable ={
	exp = "a",
	a   = true
},
attribute = "attack",
timeout = 99999
}
