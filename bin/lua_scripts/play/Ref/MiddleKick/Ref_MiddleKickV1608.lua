--by hzy 6.10.2016
-- 挑禁区
local POS1 = {
	ball.refSyntYPos(CGeoPoint:new_local(260,0)),
	ball.refSyntYPos(CGeoPoint:new_local(390,-250)),
	ball.refSyntYPos(CGeoPoint:new_local(-80,-250)),
	ball.refSyntYPos(CGeoPoint:new_local(250,140)),
}
local POS2 = {
	ball.refSyntYPos(CGeoPoint:new_local(320,150)),
	ball.refSyntYPos(CGeoPoint:new_local(390,-180)),
	ball.refSyntYPos(CGeoPoint:new_local(390,-250)),
	ball.refSyntYPos(CGeoPoint:new_local(100,130)),
}
local POS3 = {
	ball.refSyntYPos(CGeoPoint:new_local(350,150))
}
local X = {250,300}
local Y = {-180,290}
local addX1 = {-50,-100,-150,-100}
local addX2 = {-50,-100,-150,-100}
local addY1 = {0,-100,-150,-100}
local addY2 = {0,-100,-150,-50}
local BALL_MIN_VALUE = -100
local MUL_POS1 = {
	ball.refAntiYPos(CGeoPoint:new_local(X[1]+addX1[1],Y[1]+addY1[1])),
	ball.refAntiYPos(CGeoPoint:new_local(X[1]+addX1[2],Y[1]+addY1[2])),
	--ball.refAntiYPos(CGeoPoint:new_local(X[1]+addX1[3],Y[1]+addY1[3])),
	ball.refAntiYPos(CGeoPoint:new_local(X[1]+addX1[4],Y[1]+addY1[4]))
}
local MUL_POS2 = {
	ball.refAntiYPos(CGeoPoint:new_local(X[2]+addX2[1],Y[2]+addY2[1])),
	ball.refAntiYPos(CGeoPoint:new_local(X[2]+addX2[2],Y[2]+addY2[2])),
	--ball.refAntiYPos(CGeoPoint:new_local(X[2]+addX2[3],Y[2]+addY2[3])),
	ball.refAntiYPos(CGeoPoint:new_local(X[2]+addX2[4],Y[2]+addY2[4]))
}

local chipPower = 190
local middleIsMarked = false

local pos2toPos1 = 0.6
local finalChipPos = CGeoPoint:new_local(X[2]+addX2[4],Y[2]+addY2[4])
local chipPos = ball.refAntiYPos(finalChipPos)
local shootPos1 = ball.refAntiYPos(finalChipPos)
local shootPos2 = function()
	local x1 = ball.posX()
	local x2 = shootPos1():x()
	local x = x1*(1 - pos2toPos1) + x2*pos2toPos1
	local y1 = ball.posY()
	local y2 = shootPos1():y()
	local y = y1*(1 - pos2toPos1) + y2*pos2toPos1
	return CGeoPoint:new_local(x,ball.syntY()*y)
end
local supportPos = ball.refSyntYPos(CGeoPoint:new_local(340,30))

local REACH_POS	 = ball.refAntiYPos(CGeoPoint:new_local(param.pitchLength/2.0, 20))

local waitchip = false
local TEST = false

gPlayTable.CreatePlay{

firstState = "wait",

["wait"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Leader") < 20,30,240) then
			return "run1"
		end
	end,
	Assister = task.staticGetBall(POS1[2],false),
	Leader	 = task.goCmuRush(POS1[1],dir.compensate(ball.pos())),
	Special	 = task.goCmuRush(POS1[2],dir.compensate(ball.pos())),
	Middle	 = task.goCmuRush(POS1[3]),
	Defender = task.goCmuRush(POS1[4]),
	Goalie	 = task.goalie(),
	match	 = "{A}{LSMD}"
},
["run1"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Middle") < 20,30,240) then
			return "run2"
		elseif bufcnt(true,180) then
			return "exit"
		end
	end,
	Assister = task.staticGetBall(POS1[2],false),
	Leader	 = task.goCmuRush(POS2[1],dir.compensate(ball.pos())),
	Special	 = task.goCmuRush(POS2[2],dir.compensate(ball.pos())),
	Middle	 = task.goCmuRush(POS2[3]),
	Defender = task.goCmuRush(POS2[4]),
	Goalie	 = task.goalie(),
	match	 = "{ALSMD}"
},
["run2"] = {
	switch = function()
		if bufcnt(true,75) then
			middleIsMarked = player.isMarked("Middle")
			return "chip"
		elseif ball.posX() < BALL_MIN_VALUE then
			return "exit"
		end
	end,
	Assister = task.slowGetBall(finalChipPos),
	Leader	 = task.goCmuRush(POS3[1]),
	Special	 = task.runMultiPos(MUL_POS1,false,30,dir.compensate(ball.pos())),--task.goCmuRush(POS2[2],dir.compensate(ball.pos())),--task.runMultiPos(MUL_POS1,false,20,dir.compensate(ball.pos())),
	Middle	 = task.runMultiPos(MUL_POS2,false,30),--task.goCmuRush(POS2[3],dir.compensate(ball.pos())),--task.runMultiPos(MUL_POS2,false,20,dir.compensate(ball.pos())),
	Defender = task.singleBack(),
	Goalie	 = task.goalie(),
	match	 = "{ALSMD}"
},
["chip"] = {
	switch = function()
		if TEST then print("chip") end
		if bufcnt(player.kickBall("Assister") or ball.toPlayerDist("Assister") > 30,2) then
			return "fix"
		elseif bufcnt(true,180) or ball.posX() < BALL_MIN_VALUE then
			return "exit"
		end
	end,
	Assister = task.chipPass(finalChipPos,chipPower),
	Leader	 = task.goCmuRush(POS3[1]),
	Special	 = task.continue(),--task.runMultiPos(MUL_POS1,false,30,dir.compensate(ball.pos())),--task.goCmuRush(shootPos2),
	Middle	 = task.continue(),--task.runMultiPos(MUL_POS2,false,40,dir.compensate(ball.pos())),--task.goCmuRush(shootPos1),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match	 = "{ALSMD}"
},
["fix"] = {
	switch = function()
		if TEST then print("fix") end
		if bufcnt(true,45) then
			return "touch"
		end
	end,
	Assister = task.rightBack(),
	Defender = task.leftBack(),
	Special	 = task.continue(),--function()
	-- 	if player.isMarked("Special") then
	-- 		waitchip = true
	-- 		return task.goCmuRush(shootPos2,dir.compensate(ball.pos()))
	-- 	else
	-- 		return task.goCmuRush(shootPos2)
	-- 	end
	-- end,

	Middle 	 = task.continue(),--function()
	-- 	if player.isMarked("Middle") then
	-- 		waitchip = true
	-- 		return task.goCmuRush(shootPos1,dir.compensate(ball.pos()))
	-- 	else
	-- 		return task.goCmuRush(shootPos1)
	-- 	end
	-- end,
	
	Leader	 = task.goCmuRush(POS3[1]),
	Goalie 	 = task.goalie(),
	match	 = "{ALSMD}"
},
["touch"] = {
	switch = function()
		if TEST then 
			print("touch")
			print("  waitchip",waitchip)
		end
		-- if bufcnt(not waitchip and player.toBallDist("Middle") < 10,10,240) then
		-- 	return "dribbleshoot"
		if bufcnt(not waitchip and (player.kickBall("Special") or player.kickBall("Middle")),2,240) then
			return "finish"
		elseif bufcnt(waitchip and (player.kickBall("Special") or player.kickBall("Middle")),2,240) then
			return "finaltouch"
		elseif bufcnt(true,120) then
			return "exit"
		end
	end,
	Assister = task.rightBack(),
	Defender = task.leftBack(),
	Special	 = task.goCmuRush(supportPos,dir.compensate(ball.pos())),
	Middle 	 = task.InterTouch(ball.refAntiYPos(CGeoPoint:new_local(X[2]+addX2[4],Y[2]+addY2[4])),REACH_POS,800),
	Leader	 = waitchip and task.goCmuRush(POS3[1]) or task.defendMiddle(),
	Goalie 	 = task.goalie(),
	match	 = "{ALSMD}"
},
["finaltouch"] = {
	switch = function()
		if player.kickBall("Leader") then
			return "finish"
		elseif bufcnt(true,120) then
			return "exit"
		end
	end,
	Assister = task.rightBack(),
	Defender = task.leftBack(),
	Special	 = task.defendMiddle(),
	Middle 	 = task.goCmuRush(MUL_POS1[4],dir.compensate(ball.pos())),
	Leader	 = task.waitTouchNew(),
	Goalie 	 = task.goalie(),
	match	 = "{ALSMD}"
},
["dribbleshoot"] = {
	switch = function()
		if bufcnt(player.kickBall("Middle"),1,60) then
			return "finish"
		end
	end,
	Assister = task.rightBack(),
	Defender = task.leftBack(),
	Special	 = task.defendMiddle(),
	Middle 	 = task.dribbleTurnShoot(ball.refAntiYPos(CGeoPoint:new_local(param.pitchLength/2.0,30))),
	Leader	 = task.goCmuRush(POS3[1]),
	Goalie 	 = task.goalie(),
	match	 = "{ALSMD}"
},
name = "Ref_MiddleKickV1608",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}