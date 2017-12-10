local PassPos = pos.getShootPos()
local FakePos = pos.getAssistPos()
local Pos1 = function()
	local tmpY
	local updatepos = function()
		tmpY = ball.posY()
	end 
	local pos_BallChange = function ()
		updatepos()
		return CGeoPoint:new_local(0, tmpY)
	end
	return pos_BallChange
end
local Pos2 = function()
	local tmpY
	local updatepos = function()
		tmpY = ball.posY()
	end 
	local pos_BallChange = function ()
		updatepos()
		return CGeoPoint:new_local(-0, -300*ball.antiY())
	end
	return pos_BallChange
end

local MiddlePosX = 250
local Dist1 = 90
local MiddlePos = {
	ball.refAntiYPos(CGeoPoint:new_local(MiddlePosX, -Dist1*1.5)),
	ball.refAntiYPos(CGeoPoint:new_local(MiddlePosX, -Dist1*0.5)),
	ball.refAntiYPos(CGeoPoint:new_local(MiddlePosX, Dist1*0.5)),
	ball.refAntiYPos(CGeoPoint:new_local(MiddlePosX, Dist1*1.5))
}
local StartPos = CGeoPoint:new_local(200, -230)
local StartPosX = 200
local Dist2 = 30
local Dir = 110*math.pi/180
local WaitPos = {
	{
		ball.refAntiYPos(StartPos),
		ball.refAntiYPos(StartPos + Utils.Polar2Vector(Dist2, Dir)),
		ball.refAntiYPos(StartPos + Utils.Polar2Vector(Dist2*2, Dir)),
		ball.refAntiYPos(StartPos + Utils.Polar2Vector(Dist2*3, Dir))
	},
	{
		ball.refAntiYPos(CGeoPoint:new_local(StartPosX, -Dist1*1.5)),
		ball.refAntiYPos(CGeoPoint:new_local(StartPosX, -Dist1*0.5)),
		ball.refAntiYPos(CGeoPoint:new_local(StartPosX, Dist1*0.5)),
		ball.refAntiYPos(CGeoPoint:new_local(StartPosX, Dist1*1.5))
	},
	{
		ball.refAntiYPos(CGeoPoint:new_local(270, -60)),
		ball.refAntiYPos(CGeoPoint:new_local(270, -30)),
		ball.refAntiYPos(CGeoPoint:new_local(270, 30)),
		ball.refAntiYPos(CGeoPoint:new_local(270, 60))
	}
}
local SidePos={
	ball.refAntiYPos(CGeoPoint:new_local(120, -200)),
	ball.refAntiYPos(CGeoPoint:new_local(150, -200)),
	ball.refAntiYPos(CGeoPoint:new_local(180, -200))
}
local randomNum
local StayPos = WaitPos[1]

local RunPos1,RunPos2,RunPos3,RunPos4 = pos.getRunPos()

local function randomPos(i)
	return function()
		return StayPos[i]()
	end
end

gPlayTable.CreatePlay{
firstState = "randomStayPos",
["randomStayPos"] = {
	switch = function()
		randomNum = 2--math.random(3)
		StayPos = WaitPos[randomNum]
		return "ready"
	end,
	Assister = task.staticGetBall(),
	Leader   = task.stop(),
	Special  = task.stop(),
	Middle   = task.stop(),
	Defender = task.stop(),
	Goalie   = task.goalie(),
	match    = "{AL}{SMD}"
},

["ready"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Leader") < 20 and player.toTargetDist("Special") < 20,30 ,180) then
			return "move"
		end
	end,
	Assister = task.staticGetBall(Pos1(),false),
	Leader   = task.goCmuRush(MiddlePos[1], _, _, flag.allow_dss),
	Special  = task.goCmuRush(MiddlePos[2], _, _, flag.allow_dss),
	Middle   = task.goCmuRush(MiddlePos[3], _, _, flag.allow_dss),
	Defender = task.goCmuRush(MiddlePos[4], _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},

["move"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Leader") < 20 and player.toTargetDist("Special") < 20,30 ,180) then
			return "start"
		end
	end,
	Assister = task.staticGetBall(Pos2(),false),
	Leader   = task.goCmuRush(randomPos(1), _, _, flag.allow_dss),
	Special  = task.goCmuRush(randomPos(2), _, _, flag.allow_dss),
	Middle   = task.goCmuRush(randomPos(3), _, _, flag.allow_dss),
	Defender = task.goCmuRush(randomPos(4), _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},

["start"] = {
	switch = function()
		pos.detect(false)
		return "detect"
	end,
	Assister = task.staticGetBall(Pos2(),false),
	Leader   = task.goCmuRush(randomPos(1), _, _, flag.allow_dss),
	Special  = task.goCmuRush(randomPos(2), _, _, flag.allow_dss),
	Middle   = task.goCmuRush(randomPos(3), _, _, flag.allow_dss),
	Defender = task.goCmuRush(randomPos(4), _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},

["detect"] = {
	switch = function()
		pos.sendVehicleNumber(gRoleNum["Leader"], gRoleNum["Special"], gRoleNum["Middle"], gRoleNum["Defender"], gRoleNum["Assister"], gRoleNum["Goalie"])
		pos.detect()
		pos.detectDefendKicked()
		if pos.detectMarkedOver()==true and pos.detectDefendKickedOver()==true then
			return "scan"
		elseif pos.detectAgain()==true then
			return "detectAgain"
		end
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(randomPos(1), _, _, flag.allow_dss),
	Special  = task.goCmuRush(randomPos(2), _, _, flag.allow_dss),
	Middle   = task.goCmuRush(randomPos(3), _, _, flag.allow_dss),
	Defender = task.goCmuRush(randomPos(4), _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},	

["detectAgain"] = {
	switch = function()
		print('detectagain')
		pos.detect(false)
		return "detect"
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(randomPos(1), _, _, flag.allow_dss),
	Special  = task.goCmuRush(randomPos(2), _, _, flag.allow_dss),
	Middle   = task.goCmuRush(randomPos(3), _, _, flag.allow_dss),
	Defender = task.goCmuRush(randomPos(4), _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},

["scan"] = {
	switch = function()
		pos.scan()
		if pos.scanOver()==true then
			return "confirm"
		end
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(randomPos(1), _, _, flag.allow_dss),
	Special  = task.goCmuRush(randomPos(2), _, _, flag.allow_dss),
	Middle   = task.goCmuRush(randomPos(3), _, _, flag.allow_dss),
	Defender = task.goCmuRush(randomPos(4), _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},

["confirm"] = {
	switch = function()
		if pos.isChip()==true then
			return"dribble"
		else
			return"fake"
		end
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(RunPos1, _, _, flag.allow_dss),
	Special  = task.goCmuRush(RunPos2, _, _, flag.allow_dss),
	Middle   = task.goCmuRush(RunPos3, _, _, flag.allow_dss),
	Defender = task.goCmuRush(RunPos4, _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}[L][SMD]"
},

["dribble"] = {
	switch = function()
		if bufcnt(true,80) then
			return"run"
		end
	end,
	Assister = task.slowGetBall(PassPos,false,false),
	Leader   = task.goCmuRush(RunPos1, _, _, flag.allow_dss),
	Special  = task.stop(),
	Middle   = task.stop(),
	Defender = task.stop(),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

["fake"] = {
	switch = function()
		pos.fakePosCompute()
		if pos.canPass()==true then
			return"pass"
		end
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(PassPos),
	Special  = task.goCmuRush(RunPos2, _, _, flag.allow_dss),
	Middle   = task.goCmuRush(RunPos3, _, _, flag.allow_dss),
	Defender = task.goCmuRush(RunPos4, _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

["run"] = {
	switch = function()
		if bufcnt(true,30) then
			return "fake2"
		end
	end,
	Assister = task.slowGetBall(PassPos,false,false),
	Leader   = task.goCmuRush(PassPos),
	Special  = task.continue(),
	Middle   = task.continue(),
	Defender = task.continue(),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

["fake2"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Leader") < 20 and player.toTargetDist("Special") < 20,30 ,180) then
			return "chippass"
		end
	end,
	Assister = task.slowGetBall(PassPos,false,false),
	Leader   = task.goCmuRush(PassPos),
	Special  = task.goCmuRush(SidePos[1], _, _, flag.allow_dss),
	Middle   = task.goCmuRush(SidePos[2], _, _, flag.allow_dss),
	Defender = task.goCmuRush(SidePos[3], _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},


["chippass"] = {
	switch = function()
		if bufcnt(player.kickBall("Assister") ,1,100) then
			return"fix"
		end
	end,
	Assister = task.chipPass(PassPos, pos.getChipPower(),false,false),
	Leader   = task.goCmuRush(PassPos),
	Special  = task.goCmuRush(SidePos[1], _, _, flag.allow_dss),
	Middle   = task.goCmuRush(SidePos[2], _, _, flag.allow_dss),
	Defender = task.goCmuRush(SidePos[3], _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

["fix"] = {
	switch = function()
		local buf = pos.getFixBuf() 
		if bufcnt(true,35) then
			return"shoot"
		end
	end,
	Assister = task.stop(),
	Leader   = task.goCmuRush(PassPos),
	Special  = task.goCmuRush(SidePos[1], _, _, flag.allow_dss),
	Middle   = task.goCmuRush(SidePos[2], _, _, flag.allow_dss),
	Defender = task.goCmuRush(SidePos[3], _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

["pass"] = {
	switch = function()
		if bufcnt(player.kickBall("Assister") ,1,100) then
			return"shoot"
		end
	end,
	Assister = task.goAndTurnKick(pos.passForTouch(PassPos)),
	Leader   = task.goCmuRush(PassPos),
	Special  = task.goCmuRush(RunPos2, _, _, flag.allow_dss),
	Middle   = task.goCmuRush(RunPos3, _, _, flag.allow_dss),
	Defender = task.goCmuRush(RunPos4, _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

["shoot"] = {
	switch = function()
		if bufcnt(player.kickBall("Leader") ,1,300) then
			return"exit"
		end
	end,
	Assister = task.stop(),
	Leader   = task.waitTouchNew(),
	Special  = task.stop(),
	Middle   = task.stop(),
	Defender = task.stop(),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

name = "TestDynamicMiddleKick",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}