local updateFlag = true
local updateFlag2 = true
local tempShootPos
local tempChipPos
local tempPower
local randomNum
local dir = 120*math.pi/180
local VERBOSE = true
local WaitPos = {
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
		ball.refAntiYPos(CGeoPoint:new_local(270, -60)),
		ball.refAntiYPos(CGeoPoint:new_local(270, -30)),
		ball.refAntiYPos(CGeoPoint:new_local(270, 30)),
		ball.refAntiYPos(CGeoPoint:new_local(270, 60))
	}
}
local StayPos = WaitPos[1]

local sidePos = {
		ball.refAntiYPos(CGeoPoint:new_local(150,param.pitchWidth/2-100)),
		ball.refAntiYPos(CGeoPoint:new_local(200,param.pitchWidth/2-100)),
		ball.refAntiYPos(CGeoPoint:new_local(250,param.pitchWidth/2-100))
}

local Left_POS     = ball.refAntiYPos(CGeoPoint:new_local(300, 40))
local Middle_POS   = ball.refAntiYPos(CGeoPoint:new_local(295, 15))
local Right_POS    = ball.refAntiYPos(CGeoPoint:new_local(295, -15))
local Other_POS    = ball.refAntiYPos(CGeoPoint:new_local(300, -40))

local Block_POS_1  = ball.refAntiYPos(CGeoPoint:new_local(300, 15))
local Block_POS_2  = ball.refAntiYPos(CGeoPoint:new_local(300, -15))

local RUN_POS_1    = {
  ball.refAntiYPos(CGeoPoint:new_local(210, 15)),
  ball.refAntiYPos(CGeoPoint:new_local(80, 120)),
  ball.refAntiYPos(CGeoPoint:new_local(365, 100)),
}

local RUN_POS_2    = {
  ball.refAntiYPos(CGeoPoint:new_local(210, -15)),
  ball.refAntiYPos(CGeoPoint:new_local(30, -120)),
  ball.refAntiYPos(CGeoPoint:new_local(220, -200)),
}

local PassPos = function()
	return tempShootPos
end
local ChipPassPos = function()
	return tempChipPos
end
local ShootPos = PassPos
local function ShootPosCalculate()
	if updateFlag then
		tempShootPos = pos.bestShootPointForFreeKicks()
	end
end

local ChipPower = function()
	return tempPower
end

local function ChipPosCalculate()
	if updateFlag2 then
		tempChipPos = pos.bestShootPointForFreeKicks(true)
		tempPower = pos.getChipPower()
	end
end
local function randomPos(i)
	return function()
		return StayPos[i]()
	end
end
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
		return CGeoPoint:new_local(-0, tmpY-600*ball.antiY())
	end
	return pos_BallChange
end

local FacePos = CGeoPoint:new_local(0,0) + Utils.Polar2Vector(20, 0.75*math.pi)

gPlayTable.CreatePlay{
firstState = "randomStayPos",

["test"] = {
	switch = function()
	pos.sendVehicleNumber(gRoleNum["Leader"], gRoleNum["Special"], gRoleNum["Middle"], gRoleNum["Defender"], gRoleNum["Assister"], gRoleNum["Goalie"])
		pos.bestShootPointForFreeKicks()
		return "test"
	end,
	Assister = task.staticGetBall(Pos1()),
	Leader   = task.stop(),
	Special  = task.stop(),
	Middle   = task.stop(),
	Defender = task.stop(),
	Goalie   = task.stop(),
	match    = "{AL}{SMD}"
	},

["randomStayPos"] = {
	switch = function()
		randomNum = math.random(3)
		StayPos = WaitPos[randomNum]
		return "ready"
	end,
	Assister = task.staticGetBall(Pos1()),
	Leader   = task.stop(),
	Special  = task.stop(),
	Middle   = task.stop(),
	Defender = task.stop(),
	Goalie   = task.goalie(),
	match    = "{AL}{SMD}"
	},
["ready"] = {
	switch = function()
		updateFlag = true
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
		updateFlag = true
		if bufcnt(player.toTargetDist("Leader") < 20 and player.toTargetDist("Special") < 20,30 ,180) then
			return "wait"
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

["wait"] = {
	switch = function()
		ShootPosCalculate()
		if pos.isChip() == false then
			if VERBOSE then print("this is FLATPASS") end
			return "goToFlatPassPos"
		else
			if VERBOSE then print("this is CHIPPASS!!!!!") end
			ChipPosCalculate()
			return "FaceToTheField"
		end
	end,
	Assister = task.staticGetBall(Pos2(),false),
	Leader   = task.goCmuRush(randomPos(1), _, _, flag.allow_dss),
	Special  = task.goCmuRush(randomPos(2), _, _, flag.allow_dss),
	Middle   = task.goCmuRush(randomPos(3), _, _, flag.allow_dss),
	Defender = task.goCmuRush(randomPos(4), _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},

["goToFlatPassPos"] = {
	switch = function()
		ShootPosCalculate()
		if bufcnt(player.toTargetDist("Leader") < 40 ,1) then
			updateFlag = false
			return "flatPass"
		elseif bufcnt(true ,180) then
			ChipPosCalculate()
			return "FaceToTheField"
		end
	end,
	Assister = task.staticGetBall(Pos2(),false),
	Leader   = task.goCmuRush(ShootPos),
	Special  = task.goCmuRush(sidePos[1], _, _, flag.allow_dss),
	Middle   = task.goCmuRush(sidePos[2], _, _, flag.allow_dss),
	Defender = task.goCmuRush(sidePos[3], _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match  	 = "{A}[L][SMD]"
},

["FaceToTheField"] = {
	switch = function()
	    ChipPosCalculate()
		if bufcnt(true ,60) then
			updateFlag2 = false
			return "goToChipPassPos"
		end
	end,
	Assister = task.staticGetBall(FacePos,false),
	Leader   = task.stop(),
	Special  = task.continue(),
	Middle   = task.continue(),
	Defender = task.continue(),
	Goalie   = task.goalie(),
	match  	 = "{A}[L][SMD]"
},

["goToChipPassPos"] = {
	switch = function()
		ChipPosCalculate()
		if bufcnt(player.toTargetDist("Leader") < 40 ,1,180) then
			updateFlag2 = false
			return "chipPass"
		end
	end,
	Assister = task.staticGetBall(FacePos,false),
	Leader   = task.goCmuRush(ChipPassPos),
	Special  = task.goCmuRush(sidePos[1], _, _, flag.allow_dss),
	Middle   = task.goCmuRush(sidePos[2], _, _, flag.allow_dss),
	Defender = task.goCmuRush(sidePos[3], _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match  	 = "{A}[L][SMD]"
},

["flatPass"] = {
	switch = function()
		if bufcnt(player.kickBall("Assister"), 1, 90) then
			return "shoot"
		end
	end,
	Assister = task.goAndTurnKick(pos.passForTouch(PassPos)),
	Leader   = task.goCmuRush(ShootPos),
	Special  = task.continue(),
	Middle   = task.continue(),
	Defender = task.continue(),
	Goalie   = task.goalie(),
	match    = "{A}[L][SMD]"
},

["chipPass"] = {
	switch = function()
		if bufcnt(player.kickBall("Assister"), 1, 90) then
			return "shoot"
		end
	end,
	Assister = task.chipPass(pos.passForTouch(ChipPassPos), pos.getChipPower(),false,false),
	Leader   = task.goCmuRush(ChipPassPos),
	Special  = task.continue(),
	Middle   = task.continue(),
	Defender = task.continue(),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
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
	Special  = task.goCmuRush(RunPos2, _, _, flag.allow_dss),
	Middle   = task.goCmuRush(RunPos3, _, _, flag.allow_dss),
	Defender = task.goCmuRush(RunPos4, _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

["shoot"] = {
	switch = function()
		if bufcnt(player.kickBall("Leader"), 1, 90) then
			return "exit"
		end
	end,
	Assister = task.goSupportPos("Leader"),
	Leader   = task.InterTouch(),
	Special  = task.leftBack(),
	Middle   = task.defendMiddle(),
	Defender = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{AL}[SMD]"
},

name = "Ref_CornerKickV666",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}