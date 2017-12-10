local updateFlag = true
local updateFlag2 = true
local tempShootPos
local tempChipPos
local tempPower
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
local Pos1 = function()
	local tmpY
	local updatepos = function()
		tmpY = ball.posY()
	end 
	local pos_BallChange = function ()
		updatepos()
		return CGeoPoint:new_local(-450, 300)
	end
	return pos_BallChange
end

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
local randomNum
local StayPos = WaitPos[1]

local RunPos = {
	ball.refAntiYPos(CGeoPoint:new_local(255, 35)),
	ball.refAntiYPos(CGeoPoint:new_local(215, 60)),
	ball.refAntiYPos(CGeoPoint:new_local(300, 85)),
	ball.refAntiYPos(CGeoPoint:new_local(170, -230))
}

local RushPos = {
	ball.refAntiYPos(CGeoPoint:new_local(255, 100)),
	ball.refAntiYPos(CGeoPoint:new_local(215, 0)),
	ball.refAntiYPos(CGeoPoint:new_local(300, 0))
}
local sidePos = {
		ball.refAntiYPos(CGeoPoint:new_local(150,param.pitchWidth/2-100)),
		ball.refAntiYPos(CGeoPoint:new_local(200,param.pitchWidth/2-100)),
		ball.refAntiYPos(CGeoPoint:new_local(250,param.pitchWidth/2-100))
}

local FakePos = CGeoPoint:new_local(325, 120)

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
		StayPos = WaitPos[randomNum]
		return "Ready"
	end,
	Assister = task.staticGetBall(Pos1()),
	Leader   = task.stop(),
	Special  = task.stop(),
	Middle   = task.stop(),
	Defender = task.stop(),
	Goalie   = task.goalie(),
	match    = "{AL}{SMD}"
	},
["Ready"] = {
	switch = function()
		updateFlag = true
		if bufcnt(player.toTargetDist("Leader") < 20 and player.toTargetDist("Special") < 20,30 ,180) then
			return "Wait"
		end
	end,
	Assister = task.staticGetBall(Pos1()),
	Leader   = task.goCmuRush(randomPos(1), _, 500, flag.allow_dss),
	Special  = task.goCmuRush(randomPos(2), _, 500, flag.allow_dss),
	Middle   = task.goCmuRush(randomPos(3), _, 500, flag.allow_dss),
	Defender = task.goCmuRush(randomPos(4), _, 500, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},

["Wait"] = {
	switch = function()
	ShootPosCalculate()
		if pos.isChip() == false then
			if VERBOSE then print("this is FLATPASS") end
			return "GoToFlatPassPos"
		else
			if VERBOSE then print("this is CHIPPASS!!!!!") end
			return "Dribble"
		end
	end,
	Assister = task.staticGetBall(Pos1()),
	Leader   = task.goCmuRush(randomPos(1), _, 500, flag.allow_dss),
	Special  = task.goCmuRush(randomPos(2), _, 500, flag.allow_dss),
	Middle   = task.goCmuRush(randomPos(3), _, 500, flag.allow_dss),
	Defender = task.goCmuRush(randomPos(4), _, 500, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},

["GoToFlatPassPos"] = {
	switch = function()
		ShootPosCalculate()
		if bufcnt(player.toTargetDist("Leader") < 50 ,10 ) then
			updateFlag = false
			return "FlatPass"
		elseif bufcnt(true ,180) then
			return "Dribble"
		end
	end,
	Assister = task.staticGetBall(Pos1()),
	Leader   = task.goCmuRush(ShootPos, _, 500, flag.allow_dss),
	Special  = task.goCmuRush(sidePos[1], _, 500, flag.allow_dss),
	Middle   = task.goCmuRush(sidePos[2], _, 500, flag.allow_dss),
	Defender = task.goCmuRush(sidePos[3], _, 500, flag.allow_dss),
	Goalie   = task.goalie(),
	match  	 = "{A}[L][SMD]"
},

["FlatPass"] = {
	switch = function()
		if bufcnt(player.kickBall("Assister") or ball.velMod() > 30 , 1, 180) then
			return "Shoot"
		end
	end,
	Assister = task.goAndTurnKick(PassPos),
	Leader   = task.goCmuRush(ShootPos, _, 500, flag.allow_dss),
	Special  = task.continue(),
	Middle   = task.continue(),
	Defender = task.continue(),
	Goalie   = task.goalie(),
	match    = "{A}[L][SMD]"
},

["Dribble"] = {
	switch = function()
			ChipPosCalculate()
		if bufcnt(true,80) then
			return "Push"
		end
	end,
	Assister = task.slowGetBall(ChipPassPos,false,false),
	Leader   = task.continue(),
	Special  = task.continue(),
	Middle   = task.continue(),
	Defender = task.continue(),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},

["Push"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Leader") < 25, 10, 180) then
			updateFlag2 = false
			return "Run"
		end
	end,
	Assister = task.slowGetBall(ChipPassPos,false,false),
	Leader   = task.goCmuRush(RunPos[1], _, 500, flag.allow_dss),
	Special  = task.goCmuRush(RunPos[2], _, 500, flag.allow_dss),
	Middle   = task.goCmuRush(RunPos[3], _, 500, flag.allow_dss),
	Defender = task.goCmuRush(RunPos[4], _, 500, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},

["Run"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Leader") < 15, 25, 60)  then
			return "GoToPosition"
		end
	end,
	Assister = task.slowGetBall(ChipPassPos,false,false),
	Leader   = task.goCmuRush(RushPos[1], _, 500, flag.allow_dss),
	Special  = task.goCmuRush(RushPos[2], _, 500, flag.allow_dss),
	Middle   = task.goCmuRush(RushPos[3], _, 500, flag.allow_dss),
	Defender = task.defendMiddle(),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},

["GoToPosition"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Leader") < 60, 5, 60)  then
			return "Chippass"
		end
	end,
	Assister = task.slowGetBall(ChipPassPos,false,false),
	Leader   = task.goCmuRush(ChipPassPos, _, 500, flag.allow_dss),
	Special  = task.leftBack(),
	Middle   = task.defendMiddle(),
	Defender = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},

["Chippass"] = {
	switch = function()
		if bufcnt(player.kickBall("Assister") or player.toBallDist("Assister") > 40 , 1, 180) then
			return "Shoot"--"fix"
		end
	end,
	Assister = task.chipPass(ChipPassPos, 180,false,false),
	Leader   = task.goCmuRush(ChipPassPos, _, 500, flag.allow_dss),
	Special  = task.leftBack(),
	Middle   = task.defendMiddle(),
	Defender = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
}, 

["fix"] = {
	switch = function()
		if bufcnt(true,35) then
			return "Shoot"
		end
	end,
	Assister = task.stop(),
	Leader   = task.goCmuRush(ChipPassPos, _, 500, flag.allow_dss),
	Special  = task.leftBack(),
	Middle   = task.defendMiddle(),
	Defender = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},

["Shoot"] = {
	switch = function()
		if bufcnt(player.kickBall("Leader"), 1, 60) then
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

name = "TestDynamicPass",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}