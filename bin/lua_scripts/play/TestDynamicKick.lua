local PassPos = pos.getShootPos()
local FakePos = pos.getAssistPos()
local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)

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
local WaitPosFront = {
	{
		ball.refAntiYPos(CGeoPoint:new_local(50, -100)),
		ball.refAntiYPos(CGeoPoint:new_local(120, 260)),
		ball.refAntiYPos(CGeoPoint:new_local(355, -140)),
		ball.refAntiYPos(CGeoPoint:new_local(190, 0))
	},
	{
		ball.refAntiYPos(CGeoPoint:new_local(280, 125)),
		ball.refAntiYPos(CGeoPoint:new_local(280, -125)),
		ball.refAntiYPos(CGeoPoint:new_local(125, 125)),
		ball.refAntiYPos(CGeoPoint:new_local(125, -125))
	},
	{
		ball.refAntiYPos(CGeoPoint:new_local(250, 260)),
		ball.refAntiYPos(CGeoPoint:new_local(190, 260)),
		ball.refAntiYPos(CGeoPoint:new_local(130, 260)),
		ball.refAntiYPos(CGeoPoint:new_local(70, 260))
	}
}
local sidePos = {
		ball.refAntiYPos(CGeoPoint:new_local(150,param.pitchWidth/2-100)),
		ball.refAntiYPos(CGeoPoint:new_local(200,param.pitchWidth/2-100)),
		ball.refAntiYPos(CGeoPoint:new_local(250,param.pitchWidth/2-100))
}
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
local acc = 300
local randomNum
local StayPos = WaitPosCorner[1]

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
		randomNum = math.random(3)
		local ballPosY = ball.refPosY()
		if math.abs(ballPosY) < 200 then
			randomNum = 3
		end
		if ball.refPosX() >= 360 then
			StayPos = WaitPosCorner[1]--WaitPosCorner[randomNum]
		else
			StayPos = WaitPosFront[randomNum]
		end
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
	Leader   = task.goCmuRush(randomPos(1), _, _,DSS_FLAG),
	Special  = task.goCmuRush(randomPos(2), _, _,DSS_FLAG),
	Middle   = task.goCmuRush(randomPos(3), _, _,DSS_FLAG),
	Defender = task.goCmuRush(randomPos(4), _, _,DSS_FLAG),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},

["move"] = {
	switch = function()
		pos.sendVehicleNumber(gRoleNum["Leader"], gRoleNum["Special"], gRoleNum["Middle"], gRoleNum["Defender"], gRoleNum["Assister"], gRoleNum["Goalie"])
		pos.detectDefendKicked(2,40)
		if cond.isGameOn() then
			return "exit"
		elseif bufcnt(player.toTargetDist("Leader") < 20 and player.toTargetDist("Special") < 20,10 ,180) then
			return "start"
		end
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(randomPos(1), _, _,DSS_FLAG),
	Special  = task.goCmuRush(randomPos(2), _, _,DSS_FLAG),
	Middle   = task.goCmuRush(randomPos(3), _, _,DSS_FLAG),
	Defender = task.goCmuRush(randomPos(4), _, _,DSS_FLAG),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},

["start"] = {
	switch = function()
		pos.detect(_,_,30)
		pos.detectDefendKicked(2,40)
		return "detectAgain"
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(randomPos(1), _, _,DSS_FLAG),
	Special  = task.goCmuRush(randomPos(2), _, _,DSS_FLAG),
	Middle   = task.goCmuRush(randomPos(3), _, _,DSS_FLAG),
	Defender = task.goCmuRush(randomPos(4), _, _,DSS_FLAG),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},

["detect"] = {
	switch = function()
		pos.sendVehicleNumber(gRoleNum["Leader"], gRoleNum["Special"], gRoleNum["Middle"], gRoleNum["Defender"], gRoleNum["Assister"], gRoleNum["Goalie"])
		pos.detect(_,60)
		pos.detectDefendKicked(2,40)
		if cond.isGameOn() then
			return "exit"
		elseif pos.detectMarkedOver()==true and pos.detectDefendKickedOver()==true then
			print('scan')
			return "scan"
		elseif pos.detectAgain()==true then
			local ballPosY = ball.refPosY()
			--print(ballPosY)
			if math.abs(ballPosY) < 200 then
				print('scan')
				return "scan"
			else
				return "detectAgain"
			end
		end
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.continue(),
	Special  = task.continue(),
	Middle   = task.continue(),
	Defender = task.continue(),
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
		elseif pos.scanOver()==true then
			return "wait"
		end
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(RunPos1,player.toBallDir,acc),
	Special  = task.goCmuRush(RunPos2,player.toBallDir,acc),
	Middle   = task.goCmuRush(RunPos3,player.toBallDir,acc),
	Defender = task.goCmuRush(RunPos4,player.toBallDir,acc),
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
	Leader   = task.goCmuRush(RunPos1,player.toBallDir,acc),
	Special  = task.goCmuRush(RunPos2,player.toBallDir,acc),
	Middle   = task.goCmuRush(RunPos3,player.toBallDir,acc),
	Defender = task.goCmuRush(RunPos4,player.toBallDir,acc),
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
	Leader   = task.goCmuRush(RunPos1,player.toBallDir,acc),
	Special  = task.goCmuRush(RunPos2,player.toBallDir,acc),
	Middle   = task.goCmuRush(RunPos3,player.toBallDir,acc),
	Defender = task.goCmuRush(RunPos4,player.toBallDir,acc),
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
	Leader   = task.goCmuRush(RunPos1),
	Special  = task.goCmuRush(RunPos2),
	Middle   = task.goCmuRush(RunPos3),
	Defender = task.goCmuRush(RunPos4),
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
	Assister = task.chipPass(pos.chipPassForTouch(PassPos), pos.getChipPower(),false,false),
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

name = "TestDynamicKick",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}