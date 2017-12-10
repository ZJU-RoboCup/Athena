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
local side = function(role)
	local myposX
	local myposY
	local ballposX
	local ballposY
	local getPos = function()
		myposX = player.posX(role)
		myposY = player.posY(role)
		ballposX = ball.refPosX()
		ballposY = ball.refPosY()
	end
	local compute = function()
		getPos()
		return CGeoPoint:new_local(ballposX-50,ballposY+100*ball.antiY())
	end
	return compute
end
local FrontPosX = 310
local Dist = 30
local WaitPos = {
	{
		ball.refAntiYPos(CGeoPoint:new_local(FrontPosX, -Dist*3)),
		ball.refAntiYPos(CGeoPoint:new_local(FrontPosX, -Dist*2)),
		ball.refAntiYPos(CGeoPoint:new_local(FrontPosX, -Dist*1)),
		ball.refAntiYPos(CGeoPoint:new_local(FrontPosX, Dist*0))
	},
	{
		ball.refAntiYPos(CGeoPoint:new_local(300, 0)),
		ball.refAntiYPos(CGeoPoint:new_local(250, 0)),
		ball.refAntiYPos(CGeoPoint:new_local(200, 0)),
		ball.refAntiYPos(CGeoPoint:new_local(150, 0))
	},
	{
		ball.refAntiYPos(CGeoPoint:new_local(270, -60)),
		ball.refAntiYPos(CGeoPoint:new_local(270, -30)),
		ball.refAntiYPos(CGeoPoint:new_local(270, 30)),
		ball.refAntiYPos(CGeoPoint:new_local(270, 60))
	}
}
local PassPos = {
	CGeoPoint:new_local(155,190),
	CGeoPoint:new_local(375,145)
}
local ShootPos = {
	ball.refAntiYPos(PassPos[1]),
	ball.refAntiYPos(CGeoPoint:new_local(375,140))
}
local RushPos = ball.refAntiYPos(CGeoPoint:new_local(440,170))
local SidePos = {
		ball.refAntiYPos(CGeoPoint:new_local(150,-param.pitchWidth/2+150)),
		ball.refAntiYPos(CGeoPoint:new_local(200,-param.pitchWidth/2+150)),
		ball.refAntiYPos(CGeoPoint:new_local(250,-param.pitchWidth/2+150))
}

local acc = 300
local randomNum
local StayPos = WaitPos[1]
local MiddlePos = ball.refAntiYPos(CGeoPoint:new_local(200,-65))
local ChipStatus = function()
	local BallPosX
	local BallPosY
	local BallRefPos
	local Dist
	local Power
	local ChipBuf
	local GetStatus = function()
		BallPosX = ball.refPosX()
		BallPosY = ball.refPosY()
		--print("BALLREF : ",BallPosX," ",BallPosY)
		--print("BALL: ",ball.posX()," ",ball.posY())
		BallRefPos = CGeoPoint:new_local(BallPosX,BallPosY)
		Dist = BallRefPos:dist(gRolePos["Leader"])
		Power = Dist * 0.63
		ChipBuf = (-0.0000023171*Power*Power + 0.0031183100*Power  + 0.1231603074)*60
	end
	local ComputeChipPower = function()
		GetStatus()
		print("cp: ",Power)
		return Power
	end
	local ComputeChipBuf = function()
		GetStatus()
		return ChipBuf
	end
	return ComputeChipPower,ComputeChipBuf
end
local ChipPower,Buf = ChipStatus()
local function randomPos(i)
	return function()
		return StayPos[i]()
	end
end

gPlayTable.CreatePlay{
firstState = "randomStayPos",
["randomStayPos"] = {
	switch = function()
		randomNum = 1--math.random(3)
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
		if bufcnt(true,25) then
			pos.detect(_,_,50)
			return "move"
		end
	end,
	Assister = task.staticGetBall(Pos1(),false),
	Leader   = task.goCmuRush(randomPos(1), _, _, flag.allow_dss),
	Special  = task.goCmuRush(randomPos(2), _, _, flag.allow_dss),
	Middle   = task.goCmuRush(randomPos(3), _, _, flag.allow_dss),
	Defender = task.goCmuRush(randomPos(4), _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}[L][SMD]"
},

["move"] = {
	switch = function()
		pos.sendVehicleNumber(gRoleNum["Leader"], gRoleNum["Special"], gRoleNum["Middle"], gRoleNum["Defender"], gRoleNum["Assister"], gRoleNum["Goalie"])
		pos.detectDefendKicked(2,40)
		if cond.isGameOn() then
			return "exit"
		elseif pos.detectDefendKickedOver()==true then
			return "pull"
		end
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(randomPos(1)),
	Special  = task.goCmuRush(randomPos(2)),
	Middle   = task.goCmuRush(randomPos(3)),
	Defender = task.goCmuRush(randomPos(4)),
	Goalie   = task.goalie(),
	match    = "{A}{L}{SMD}"
},

["pull"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Leader")<20,10,150) then
			return "detect"
		end
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(MiddlePos,_,200),
	Special  = task.goCmuRush(randomPos(2)),
	Middle   = task.goCmuRush(randomPos(3)),
	Defender = task.goCmuRush(randomPos(4)),
	Goalie   = task.goalie(),
	match    = "{A}{L}{SMD}"
},

["detect"] = {
	switch = function()
		pos.detect(_,80)
		if cond.isGameOn() then
			return "exit"
		elseif (pos.detectMarkedOver()==true or pos.detectAgain()==true) and pos.detectDefendKickedOver()==true then
				if pos.isMarked(gRoleNum["Leader"]) == false then
					if pos.isChip() or player.canFlatPassTo("Assister","Leader")==false then
						return "chipPassToWreck"--"chipPassToMiddle"
					else
						return "chipPassToWreck"--"chipPassToMiddle"
					end
				else
					return "chipPassToWreck"
				end
			end
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(ShootPos[1], _, 250),
	Special  = task.goCmuRush(randomPos(2)),
	Middle   = task.goCmuRush(randomPos(3)),
	Defender = task.goCmuRush(randomPos(4)),
	Goalie   = task.goalie(),
	match    = "{A}{L}{SMD}"
},	

-- nobody is calling this state
-- ["passToMiddle"] = {
-- 	switch = function()
-- 		if bufcnt(player.kickBall("Assister") ,1,100) then
-- 			return"shoot"
-- 		end
-- 	end,
-- 	Assister = task.goAndTurnKick(pos.passForTouch(ShootPos[1])),
-- 	Leader   = task.goCmuRush(ShootPos[1]),
-- 	Special  = task.goCmuRush(SidePos[1],_,200),
-- 	Middle   = task.goCmuRush(SidePos[2],_,200),
-- 	Defender = task.goCmuRush(SidePos[3],_,200),
-- 	Goalie   = task.goalie(),
-- 	match    = "{A}{L}[SMD]"
-- },

-- ["chipPassToMiddle"] = {
-- 	switch = function()
-- 		if cond.isGameOn() then
-- 			return "exit"
-- 		elseif bufcnt(true,80) then
-- 			return"chipPass1"
-- 		end
-- 	end,
-- 	Assister = task.slowGetBall(PassPos[1]),
-- 	Leader   = task.goCmuRush(ShootPos[1]),
-- 	Special  = task.goCmuRush(randomPos(2)),
-- 	Middle   = task.goCmuRush(randomPos(3)),
-- 	Defender = task.goCmuRush(randomPos(4)),
-- 	Goalie   = task.goalie(),
-- 	match    = "{A}{L}{SMD}"
-- },

["chipPass1"] = {
	switch = function()
		if bufcnt(player.kickBall("Assister") ,1,100) then
			return"fix1"
		end
	end,
	Assister = task.chipPass(pos.chipPassForTouch(PassPos[1]), ChipPower),
	Leader   = task.goCmuRush(ShootPos[1]),
	Special  = task.goCmuRush(side("Special")),
	Middle   = task.goCmuRush(side("Middle")),
	Defender = task.goCmuRush(side("Defender")),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

["fix1"] = {
	switch = function() 
		if bufcnt(true,Buf) then
			return"shoot"
		end
	end,
	Assister = task.stop(),
	Leader   = task.goCmuRush(ShootPos[1]),
	Special  = task.goCmuRush(side("Special")),
	Middle   = task.goCmuRush(side("Middle")),
	Defender = task.goCmuRush(side("Defender")),
	Goalie   = task.goalie(),
	match    = "{A}{L}{SMD}"
},

["chipPassToWreck"] = {
	switch = function()
		if cond.isGameOn() then
			return "exit"
		elseif bufcnt(true,40) then
			return"goToWreck"
		end
	end,
	Assister = task.slowGetBall(PassPos[2]),
	Leader   = task.goCmuRush(ShootPos[1]),
	Special  = task.goCmuRush(randomPos(2)),
	Middle   = task.goCmuRush(randomPos(3)),
	Defender = task.goCmuRush(randomPos(4)),
	Goalie   = task.goalie(),
	match    = "{A}{L}{SMD}"
},

["goToWreck"] = {
	switch = function()
		if cond.isGameOn() then
			return "exit"
		elseif bufcnt(player.toTargetDist("Leader") < 20 ,120 ,180) then
			return "chipPass2"
		end
	end,
	Assister = task.slowGetBall(PassPos[1]),
	Leader   = task.goCmuRush(RushPos),
	Special  = task.goCmuRush(randomPos(2)),
	Middle   = task.goCmuRush(randomPos(3)),
	Defender = task.goCmuRush(randomPos(4)),
	Goalie   = task.goalie(),
	match    = "{A}{L}{SMD}"
},	

["chipPass2"] = {
	switch = function()
		if bufcnt(player.kickBall("Assister") ,1,90) then
			return"fix2"
		end
	end,
	Assister = task.chipPass(PassPos[2], 180),
	Leader   = task.goCmuRush(ShootPos[2]),
	Special  = task.goCmuRush(side("Special")),
	Middle   = task.goCmuRush(side("Middle")),
	Defender = task.goCmuRush(side("Defender")),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

["fix2"] = {
	switch = function() 
		if bufcnt(true,Buf) then
			return"shoot"
		end
	end,
	Assister = task.stop(),
	Leader   = task.goCmuRush(ShootPos[2]),
	Special  = task.goCmuRush(side("Special")),
	Middle   = task.goCmuRush(side("Middle")),
	Defender = task.goCmuRush(side("Defender")),
	Goalie   = task.goalie(),
	match    = "{A}{L}{SMD}"
},

["shoot"] = {
	switch = function()
		if bufcnt(player.kickBall("Leader") ,1,120) then
			return"exit"
		end
	end,
	Assister = task.goSupportPos("Special"),
	Leader   = task.InterTouch(),
	Special  = task.leftBack(),
	Middle   = task.defendMiddle(),
	Defender = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{L}[S][MDA]"
},

name = "TestParsianKick",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}