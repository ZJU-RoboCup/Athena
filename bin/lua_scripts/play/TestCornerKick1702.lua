
local Radius = 25
local PassPos = CGeoPoint:new_local(360, 170)
local FinalShootPos = CGeoPoint:new_local(360, 170)
local RunPos = ball.refAntiYPos(CGeoPoint:new_local(-450, 300))
local ShootPos = ball.refAntiYPos(FinalShootPos)

local PosForAssister = function ()
	local tmpX
	local tmpY

	local UpdatePos = function ()
		tmpX = ball.posX()
		tmpY = ball.posY()
		tmpos = ball.pos()
	end

	local pos_BallChange = function ()
		UpdatePos()
		if ( tmpY > 0 ) then
			return  tmpos + Utils.Polar2Vector(Radius, math.pi*4/3)
		else
			return  tmpos + Utils.Polar2Vector(Radius, math.pi*2/3)
		end
	end

	return pos_BallChange
end

local PosForAssister2 = function ()
	local tmpX
	local tmpY

	local UpdatePos = function ()
		tmpX = ball.posX()
		tmpY = ball.posY()
		tmpos = ball.pos()
	end

	local pos_BallChange = function ()
		UpdatePos()
		if ( tmpY > 0 ) then
			return  tmpos + Utils.Polar2Vector(Radius, math.pi*5/3)
		else
			return  tmpos + Utils.Polar2Vector(Radius, math.pi*1/3)
		end
	end

	return pos_BallChange
end

local PosForKicker = function ()
	local tmpX
	local tmpY

	local UpdatePos = function ()
		tmpX = ball.posX() - Radius
		tmpY = ball.posY()
	end

	local pos_BallChange = function ()
		UpdatePos()
		return CGeoPoint:new_local(tmpX,tmpY)
	end

	return pos_BallChange
end

-- pos for halffield mode
local OTHER_POS = {
  CGeoPoint:new_local(100,70),
  CGeoPoint:new_local(100,120),
  CGeoPoint:new_local(80,0),
  CGeoPoint:new_local(80,-80),
  CGeoPoint:new_local(80,80)
}
local BACK_POS = CGeoPoint:new_local(80,0)

local HALF = gOppoConfig.IfHalfField

gPlayTable.CreatePlay{

firstState = "readyState",

["readyState"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Kicker") < 10 , 30, 180) then
			return "FirstMove"
		end
	end,
	Kicker = task.goCmuRush(PosForAssister(), player.toBallDir),
	Tier   = task.stop(),
	match    = ""
},

["FirstMove"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Kicker") < 20 and
			      player.toTargetDist("Tier") < 10, 30, 180) then
			return "SecondMove"
		end
	end,
	Kicker = task.goCmuRush(PosForAssister(), player.toBallDir),
	Tier   = task.goCmuRush(PosForKicker(), player.toBallDir),
	match    = ""
},

["SecondMove"] = {
	switch = function()
		if bufcnt(true,30) then
			return "GoToFinalPos"
		end
	end,
	Kicker = task.goCmuRush(PosForAssister2(), player.toBallDir),
	Tier   = task.stop(),--task.goCmuRush(RunPos),
	match    = ""
},

["GoToFinalPos"] = {
	switch = function()
		if bufcnt(true,70) then
			return "Dribble"
		end
	end,
	Kicker = task.staticGetBall(PassPos),
	Tier   = task.stop(),
	match    = ""
},

["Dribble"] = {
	switch = function()
		if bufcnt(true,70) then
			return "Go1"
		end
	end,
	Kicker = task.slowGetBall(PassPos),
	Tier   = task.stop(),
	match    = ""
},

["Go1"] = {
	switch = function()
		if bufcnt(true,30) then
			return "Go2"
		end
	end,
	Kicker = task.slowGetBall(PassPos),
	Tier   = task.goCmuRush(RunPos),
	match    = ""
},

["Go2"] = {
	switch = function()
		if bufcnt(true,75) then
			return "Chippass"
		end
	end,
	Kicker = task.slowGetBall(PassPos),
	Tier   = task.goCmuRush(ShootPos),
	match    = ""
},

["Chippass"] = {
	switch = function()
		if bufcnt(player.kickBall("Kicker") , 1, 180) then
			return "fix"
		end
	end,
	Kicker = task.chipPass(PassPos,190),
	Tier   = task.goCmuRush(ShootPos),
	match    = ""
},

["fix"] = {
	switch = function()
		if bufcnt(true, 45) then
			return "Shoot"
		end
	end,
	Kicker = task.stop(),
	Tier   = task.goCmuRush(ShootPos),
	match    = ""
},

["Shoot"] = {
	switch = function()
		if bufcnt(player.kickBall("Tier") , 1 , 240) then
			return "stop"
		end
	end,
	Kicker = task.stop(),
	Tier   = task.InterTouch(),
	match    = ""
},

["stop"] = {
    switch = function ()
        return "stop"
    end,
    Kicker = task.stop(),
    Tier   = task.stop(),
    match    = ""
  },
name = "TestCornerKick1702",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}