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
local isDefendedKick
local WaitPosCorner = {
	{
		ball.refAntiYPos(CGeoPoint:new_local(275, -200)),
		ball.refAntiYPos(CGeoPoint:new_local(210, -25)),
		ball.refAntiYPos(CGeoPoint:new_local(225, 145)),
		ball.refAntiYPos(CGeoPoint:new_local(330, 240))
	},
	{
		ball.refAntiYPos(CGeoPoint:new_local(300, 0)),
		ball.refAntiYPos(CGeoPoint:new_local(250, 0)),
		ball.refAntiYPos(CGeoPoint:new_local(200, 0)),
		ball.refAntiYPos(CGeoPoint:new_local(150, 0))
	},
	{
		ball.refAntiYPos(CGeoPoint:new_local(225, -60)),
		ball.refAntiYPos(CGeoPoint:new_local(225, -30)),
		ball.refAntiYPos(CGeoPoint:new_local(225, 30)),
		ball.refAntiYPos(CGeoPoint:new_local(225, 60))
	}
}
--挡拆到弱侧
local StartPos = CGeoPoint:new_local(80, -200)
local Dist = 30
local Dir = math.pi/2
local ConvergePos = {
	ball.refAntiYPos(StartPos),
	ball.refAntiYPos(StartPos + Utils.Polar2Vector(Dist, Dir)),
	ball.refAntiYPos(StartPos + Utils.Polar2Vector(Dist*2, Dir)),
	ball.refAntiYPos(StartPos + Utils.Polar2Vector(Dist*3, Dir))
}
local BackDist1 = 50
local BackDist2 = 25
local PullPos = {
	ball.refAntiYPos(StartPos + Utils.Polar2Vector(BackDist1, math.pi)),
	ball.refAntiYPos(StartPos + Utils.Polar2Vector(Dist*3, Dir) + Utils.Polar2Vector(BackDist2, math.pi))
}
local BroadenDist1 = 400
local BroadenDist2 = 200
local PullPos2 = {
	ball.refAntiYPos(StartPos + Utils.Polar2Vector(BackDist1, math.pi) + Utils.Polar2Vector(BroadenDist1, math.pi/2)),
	ball.refAntiYPos(StartPos + Utils.Polar2Vector(Dist*3, Dir) + Utils.Polar2Vector(BackDist2, math.pi) + Utils.Polar2Vector(BroadenDist2, -math.pi/2))
}
local WreckPassPos = CGeoPoint:new_local(350, 70)
local WreckPos = ball.refAntiYPos(WreckPassPos)

--
--挡拆到内侧
local StartPos2 = CGeoPoint:new_local(50, 230)
local Dist2 = 30
local LDir = -math.pi/4
local RDir = math.pi/4
local ConvergePos2 = {
	ball.refAntiYPos(StartPos2 + Utils.Polar2Vector(Dist2, RDir)),
	ball.refAntiYPos(StartPos2 + Utils.Polar2Vector(Dist2, LDir)),
	ball.refAntiYPos(StartPos2 + Utils.Polar2Vector(Dist2*2, LDir)),
	ball.refAntiYPos(StartPos2),
}
local BackDist = 400
local BackPos = ball.refAntiYPos(StartPos2 + Utils.Polar2Vector(Dist2, RDir) + Utils.Polar2Vector(BackDist, math.pi))
local RollPassPos = CGeoPoint:new_local(190,-215)
local RollPos1 = ball.refAntiYPos(StartPos2 + Utils.Polar2Vector(30, math.pi) + Utils.Polar2Vector(400, -math.pi/2))
local RollPos2 = ball.refAntiYPos(RollPassPos)
local ProtectPos = ball.refAntiYPos(StartPos2 + Utils.Polar2Vector(Dist2*2.5, LDir))
--
local side = function(role)
	local myposX
	local myposY
	local getPos = function()
		myposX = player.posX(role)
		myposY = player.posY(role)
	end
	local compute = function()
		getPos()
		if myposY<0 then
			return CGeoPoint:new_local(50,-270)
		else
			return CGeoPoint:new_local(50,270)
		end
	end
	return compute
end
local acc = 300
local randomNum
local StayPos = WaitPosCorner[1]

local RunPos1,RunPos2,RunPos3,RunPos4 = pos.getRunPos()

local function randomPos(i)
	return function()
		return StayPos[i]()
	end
end

local side = function(role)
	local myposX
	local myposY
	local ballposX
	local getPos = function()
		myposX = player.posX(role)
		myposY = player.posY(role)
		ballposX = ball.refPosX()
	end
	local compute = function()
		getPos()
		if ballposX >= 360 then
			if myposY<0 then
				return CGeoPoint:new_local(50,-270)
			else
				return CGeoPoint:new_local(50,270)
			end
		else
			return CGeoPoint:new_local(myposX,270*ball.antiY())
		end
	end
	return compute
end

gPlayTable.CreatePlay{
firstState = "randomStayPos",
["randomStayPos"] = {
	switch = function()
		randomNum = 1--math.random(3)
		StayPos = WaitPosCorner[randomNum]
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
		if bufcnt(true,45) then
			return "move"
		end
	end,
	Assister = task.staticGetBall(Pos1(),false),
	Leader   = task.goCmuRush(randomPos(1), _, _, flag.allow_dss),
	Special  = task.goCmuRush(randomPos(2), _, _, flag.allow_dss),
	Middle   = task.goCmuRush(randomPos(3), _, _, flag.allow_dss),
	Defender = task.goCmuRush(randomPos(4), _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},

["move"] = {
	switch = function()
		pos.sendVehicleNumber(gRoleNum["Leader"], gRoleNum["Special"], gRoleNum["Middle"], gRoleNum["Defender"], gRoleNum["Assister"], gRoleNum["Goalie"])
		pos.detectDefendKicked(2,45)
		if cond.isGameOn() then
			return "exit"
		elseif bufcnt(player.toTargetDist("Leader") < 20 and player.toTargetDist("Special") < 20,10 ,180) then
			return "start"
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

["start"] = {
	switch = function()
		pos.detect(_,_,30)
		pos.detectDefendKicked(2,45)
		return "detectAgain"
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(RunPos1,player.toBallDir,acc),
	Special  = task.goCmuRush(RunPos2,player.toBallDir,acc),
	Middle   = task.goCmuRush(RunPos3,player.toBallDir,acc),
	Defender = task.goCmuRush(RunPos4,player.toBallDir,acc),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},

["detect"] = {
	switch = function()
		pos.sendVehicleNumber(gRoleNum["Leader"], gRoleNum["Special"], gRoleNum["Middle"], gRoleNum["Defender"], gRoleNum["Assister"], gRoleNum["Goalie"])
		pos.detect(_,60)
		pos.detectDefendKicked(2,45)
		if cond.isGameOn() then
			return "exit"
		elseif pos.detectMarkedOver()==true and pos.detectDefendKickedOver()==true then
			isDefendedKick = pos.isChip()
			print('scan')
			return "scan"
		elseif pos.detectAgain()==true then
			return "detectAgain"
		end
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(RunPos1,player.toBallDir,acc),
	Special  = task.goCmuRush(RunPos2,player.toBallDir,acc),
	Middle   = task.goCmuRush(RunPos3,player.toBallDir,acc),
	Defender = task.goCmuRush(RunPos4,player.toBallDir,acc),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},	

["detectAgain"] = {
	switch = function()
		print('detectagain')
		pos.detect(true,_,70)
		return "detect"
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(RunPos1,player.toBallDir,acc),
	Special  = task.goCmuRush(RunPos2,player.toBallDir,acc),
	Middle   = task.goCmuRush(RunPos3,player.toBallDir,acc),
	Defender = task.goCmuRush(RunPos4,player.toBallDir,acc),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},

["scan"] = {
	switch = function()
		pos.scan()
		if pos.detectAgain()==true then
			return "detectAgain"
		elseif pos.allMarked()==true then
				if isDefendedKick==false and pos.isDefendedHead()==true then
					return "identicalSidePick"
				else
					return "normalPick"
				end
		elseif pos.scanOver()==true then
			return "wait"
		end
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(RunPos1,player.toBallDir),
	Special  = task.goCmuRush(RunPos2,player.toBallDir),
	Middle   = task.goCmuRush(RunPos3,player.toBallDir),
	Defender = task.goCmuRush(RunPos4,player.toBallDir),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},

["wait"] = {
	switch = function()
		if bufcnt(true,2) then
			return "confirm"
		end
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(RunPos1,player.toBallDir),
	Special  = task.goCmuRush(RunPos2,player.toBallDir),
	Middle   = task.goCmuRush(RunPos3,player.toBallDir),
	Defender = task.goCmuRush(RunPos4,player.toBallDir),
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
	Leader   = task.goCmuRush(RunPos1,player.toBallDir),
	Special  = task.goCmuRush(RunPos2,player.toBallDir),
	Middle   = task.goCmuRush(RunPos3,player.toBallDir),
	Defender = task.goCmuRush(RunPos4,player.toBallDir),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},

["dribble"] = {
	switch = function()
		if cond.isGameOn() then
			return "exit"
		elseif bufcnt(true,80) then
			return"fake2"
		end
	end,
	Assister = task.slowGetBall(PassPos,false,false),
	Leader   = task.goCmuRush(RunPos1,player.toBallDir),
	Special  = task.goCmuRush(RunPos2,player.toBallDir),
	Middle   = task.goCmuRush(RunPos3,player.toBallDir),
	Defender = task.goCmuRush(RunPos4,player.toBallDir),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

["fake"] = {
	switch = function()
		pos.sendVehicleNumber(gRoleNum["Leader"], gRoleNum["Special"], gRoleNum["Middle"], gRoleNum["Defender"], gRoleNum["Assister"], gRoleNum["Goalie"])
		pos.fakePosCompute()
		if pos.canPass()==true then
			return"pass"
		end
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(PassPos),
	Special  = task.goCmuRush(side("Special")),--task.goCmuRush(RunPos2),
	Middle   = task.goCmuRush(side("Middle")),--task.goCmuRush(RunPos3),
	Defender = task.goCmuRush(side("Defender")),--task.goCmuRush(RunPos4),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

["fake2"] = {
	switch = function()
		pos.sendVehicleNumber(gRoleNum["Leader"], gRoleNum["Special"], gRoleNum["Middle"], gRoleNum["Defender"], gRoleNum["Assister"], gRoleNum["Goalie"])
		pos.fakePosCompute()
		if pos.canPass()==true then
			return"chippass"
		end
	end,
	Assister = task.slowGetBall(PassPos,false,false),
	Leader   = task.goCmuRush(PassPos),
	Special  = task.goCmuRush(side("Special")),--task.goCmuRush(RunPos2),
	Middle   = task.goCmuRush(side("Middle")),--task.goCmuRush(RunPos3),
	Defender = task.goCmuRush(side("Defender")),--task.goCmuRush(RunPos4),
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
	Special  = task.goCmuRush(side("Special")),--task.goCmuRush(RunPos2),
	Middle   = task.goCmuRush(side("Middle")),--task.goCmuRush(RunPos3),
	Defender = task.goCmuRush(side("Defender")),--task.goCmuRush(RunPos4),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

["fix"] = {
	switch = function()
		local buf = pos.getFixBuf() 
		if bufcnt(true,buf) then
			return"shoot"
		end
	end,
	Assister = task.stop(),
	Leader   = task.goCmuRush(PassPos),
	Special  = task.goCmuRush(side("Special")),--task.goCmuRush(RunPos2),
	Middle   = task.goCmuRush(side("Middle")),--task.goCmuRush(RunPos3),
	Defender = task.goCmuRush(side("Defender")),--task.goCmuRush(RunPos4),
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
	Special  = task.goCmuRush(side("Special")),--task.goCmuRush(RunPos2),
	Middle   = task.goCmuRush(side("Middle")),--task.goCmuRush(RunPos3),
	Defender = task.goCmuRush(side("Defender")),--task.goCmuRush(RunPos4),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

["shoot"] = {
	switch = function()
		if bufcnt(player.kickBall("Leader") ,1,120) then
			return"exit"
		end
	end,
	Assister = task.goSupportPos("Leader"),
	Leader   = task.InterTouch(),
	Special  = task.leftBack(),
	Middle   = task.defendMiddle(),
	Defender = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{L}[ASMD]"
},

["normalPick"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Leader") < 20 and player.toTargetDist("Special") < 20,40 ,180) then
			return "pull"
		end
	end,
	Assister = task.staticGetBall(WreckPassPos),
	Leader   = task.goCmuRush(ConvergePos[1]),
	Special  = task.goCmuRush(ConvergePos[2]),
	Middle   = task.goCmuRush(ConvergePos[3]),
	Defender = task.goCmuRush(ConvergePos[4]),
	Goalie   = task.goalie(),
	match    = "{A}[L][D][SM]"
},

["pull"] = {
	switch = function()
		if bufcnt(true,20) then
			return "pull2"
		end
	end,
	Assister = task.staticGetBall(WreckPassPos),
	Leader   = task.goCmuRush(PullPos[1]),
	Special  = task.goCmuRush(ConvergePos[2]),
	Middle   = task.goCmuRush(ConvergePos[3]),
	Defender = task.goCmuRush(PullPos[2]),
	Goalie   = task.goalie(),
	match    = "{A}{LD}{SM}"
},

["pull2"] = {
	switch = function()
		if bufcnt(true,20) then
			return "rush"
		end
	end,
	Assister = task.slowGetBall(WreckPassPos),
	Leader   = task.goCmuRush(PullPos2[1]),
	Special  = task.goCmuRush(ConvergePos[2]),
	Middle   = task.goCmuRush(ConvergePos[3]),
	Defender = task.goCmuRush(PullPos2[2]),
	Goalie   = task.goalie(),
	match    = "{A}{LD}{SM}"
},

["rush"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Leader") < 170,2 ,180) then
			return "passToWreckPos"
		end
	end,
	Assister = task.slowGetBall(WreckPassPos),
	Leader   = task.goCmuRush(WreckPos),
	Special  = task.goCmuRush(ConvergePos[2]),
	Middle   = task.goCmuRush(ConvergePos[3]),
	Defender = task.goCmuRush(PullPos2[2]),
	Goalie   = task.goalie(),
	match    = "{A}{LD}{SM}"
},

["passToWreckPos"] = {
	switch = function()
		if bufcnt(player.kickBall("Assister") ,1,80) then
			return "fixgoto"
		end
	end,
	Assister = task.chipPass(WreckPassPos,250),
	Leader   = task.goCmuRush(WreckPos),
	Special  = task.goCmuRush(ConvergePos[2]),
	Middle   = task.goCmuRush(ConvergePos[3]),
	Defender = task.goCmuRush(PullPos2[2]),
	Goalie   = task.goalie(),
	match    = "{A}{LD}{SM}"
},

["fixgoto"] = {
	switch = function()
		if bufcnt(true,30) then
			return "shoot"
		end
	end,
	Assister = task.stop(),
	Leader   = task.goCmuRush(WreckPos),
	Special  = task.goCmuRush(ConvergePos[2]),
	Middle   = task.goCmuRush(ConvergePos[3]),
	Defender = task.goCmuRush(PullPos2[2]),
	Goalie   = task.goalie(),
	match    = "{A}{LD}{SM}"
},

["identicalSidePick"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Defender") < 20 and player.toTargetDist("Leader") < 20,30 ,180) then
			return "back"
		end
	end,
	Assister = task.staticGetBall(RollPassPos),
	Leader   = task.goCmuRush(ConvergePos2[1]),
	Special  = task.goCmuRush(ConvergePos2[2]),
	Middle   = task.goCmuRush(ConvergePos2[3]),
	Defender = task.goCmuRush(ConvergePos2[4]),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},

["back"] = {
	switch = function()
		if bufcnt(true,15) then
			return "rush2"
		end
	end,
	Assister = task.staticGetBall(RollPassPos),
	Leader   = task.goCmuRush(BackPos),
	Special  = task.goCmuRush(ConvergePos2[2]),
	Middle   = task.goCmuRush(ConvergePos2[3]),
	Defender = task.goCmuRush(ConvergePos2[4]),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},

["rush2"] = {
	switch = function()
		if bufcnt(true,15) then
			return "rush22"
		end
	end,
	Assister = task.staticGetBall(RollPassPos),
	Leader   = task.goCmuRush(RollPos1),
	Special  = task.goCmuRush(ConvergePos2[2]),
	Middle   = task.goCmuRush(ConvergePos2[3]),
	Defender = task.goCmuRush(ConvergePos2[4]),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},

["rush22"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Leader") < 40,2 ,180) then
			return "passToRollPos"
		end
	end,
	Assister = task.staticGetBall(RollPassPos),
	Leader   = task.goCmuRush(RollPos2),
	Special  = task.goCmuRush(ConvergePos2[2]),
	Middle   = task.goCmuRush(ProtectPos),
	Defender = task.goCmuRush(ConvergePos2[4]),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},

["passToRollPos"] = {
	switch = function()
		if bufcnt(player.kickBall("Assister") ,1,80) then
			return "shoot"
		end
	end,
	Assister = task.goAndTurnKick(pos.passForTouch(RollPos2)),
	Leader   = task.goCmuRush(RollPos2),
	Special  = task.goCmuRush(ConvergePos2[2]),
	Middle   = task.goCmuRush(ProtectPos),
	Defender = task.goCmuRush(ConvergePos2[4]),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},


name = "TestDynamicKickPickVersion",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}