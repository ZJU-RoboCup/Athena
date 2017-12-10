local updateFlag = true
local tempShootPos
local PassPos = function()
	return tempShootPos
end
local ChipPower = pos.getChipPower()
local function ShootPosCalculate()
	if updateFlag then
		tempShootPos = pos.bestShootPointForFreeKicks()
	end
end

local Change_Pos = function ()
  local tmpX
  local tmpY 

  local UpdatePos = function ()
    tmpX = player.posX("Leader")
    tmpY = player.posY("Leader")
  end

  local pos_Change = function ()
    UpdatePos()
    return CGeoPoint:new_local(tmpX, tmpY + ball.antiY()*50)
  end
  return pos_Change
end

local SupportPos = Change_Pos()
local ShootPos = PassPos
local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)

local WaitPos = {
	ball.refAntiYPos(CGeoPoint:new_local(60, 0)),
	ball.refAntiYPos(CGeoPoint:new_local(120, 180)),
	ball.refAntiYPos(CGeoPoint:new_local(120, -180))
}

local RushPos = {
	ball.refAntiYPos(CGeoPoint:new_local(255, 100)),
	ball.refAntiYPos(CGeoPoint:new_local(215, 0)),
	ball.refAntiYPos(CGeoPoint:new_local(300, 0))
}

local FixPos = ball.refAntiYPos(CGeoPoint:new_local(225, 300))
gPlayTable.CreatePlay{
firstState = "Ready",
["Ready"] = {
	switch = function()
		updateFlag = true
		if bufcnt(player.toTargetDist("Special") < 20 and player.toTargetDist("Special") < 20 ,30 ,180) then
			return "Wait"
		end
	end,
	Assister = task.staticGetBall(),
	Leader   = task.goCmuRush(WaitPos[1], _, 500, DSS_FLAG),
	Special  = task.goCmuRush(WaitPos[2], _, 500, DSS_FLAG),
	Middle   = task.goCmuRush(WaitPos[3], _, 500, DSS_FLAG),
	Defender = task.leftBack(),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},

["Wait"] = {
	switch = function()
		ShootPosCalculate()
		if bufcnt(true,20) then 
			return "Dribble"
		end
	end,
	Assister = task.staticGetBall(),
	Leader   = task.goCmuRush(WaitPos[1], _, 500,DSS_FLAG),
	Special  = task.goCmuRush(WaitPos[2], _, 500,DSS_FLAG),
	Middle   = task.goCmuRush(WaitPos[3], _, 500,DSS_FLAG),
	Defender = task.leftBack(),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},

["Dribble"] = {
	switch = function()
		if bufcnt(true,100) then 
			return "ChipPass"
		end
	end,
	Assister = task.slowGetBall(PassPos,false,false),
	Leader   = task.goCmuRush(WaitPos[1]),
	Special  = task.goCmuRush(WaitPos[2]),
	Middle   = task.goCmuRush(WaitPos[3]),
	Defender = task.rightBack(),
	Goalie   = task.goalie(),
	match  	 = "{A}{LSMD}"
},

["Rush"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Leader") < 150 ,15 ,180) then
			return "ChipPass"
		end
	end,
	Assister = task.slowGetBall(PassPos,false,false),
	Leader   = task.goCmuRush(ShootPos, _, 500,DSS_FLAG),
	Special  = task.goCmuRush(SupportPos, _, 500,DSS_FLAG),
	Middle   = task.leftBack(),
	Defender = task.rightBack(),
	Goalie   = task.goalie(),
	match  	 = "{A}[LS][DM]"
},

["ChipPass"] = {
	switch = function()
	ShootPosCalculate()
		if bufcnt(player.kickBall("Assister") ,1, 50) then
			return "fixposition"
		end
	end,
	Assister = task.chipPass(PassPos,ChipPower,false,false),
	Leader   = task.goCmuRush(ShootPos),
	Special  = task.goCmuRush(SupportPos),
	Middle   = task.leftBack(),
	Defender = task.rightBack(),
	Goalie   = task.goalie(),
	match  	 = "{A}{LSMD}"
},

["fixposition"] = {
	switch = function()
		if bufcnt(true,pos.getFixBuf()) then
			return "exit"
		end
	end,
	Assister = task.defendMiddle(),
	Leader   = task.goCmuRush(ShootPos),
	Special  = task.goCmuRush(SupportPos),
	Middle   = task.leftBack(),
	Defender = task.rightBack(),
	Goalie   = task.goalie(),
	match  	 = "{A}{LSMD}"
},

name = "TestDynamicBackKick",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}