-- 两车站位，向后传球，一传一射
-- by zhyaic 2013.6.12
local BALL_POSX
local BALL_POSY
local BALL_ANTI

local SHOOT_POS = function()
	return CGeoPoint:new_local(BALL_POSX - 80, BALL_POSY + BALL_ANTI*(100))
end

local SHOOT_TARGET = function ()
	return CGeoPoint:new_local(302.5, -15*BALL_ANTI)
end

local SHOOT_DIR = function()
	local tpos = CGeoPoint:new_local(BALL_POSX - 80, BALL_POSY + BALL_ANTI*(100))
	return (CGeoPoint:new_local(302.5,-15*BALL_ANTI) - tpos):dir()
end

local PASS_DIR = function()
	return (CGeoPoint:new_local(BALL_POSX - 72, BALL_POSY + BALL_ANTI*(100)) - ball.pos()):dir()
end

gPlayTable.CreatePlay{

firstState = "startball",

["startball"] = {
	switch = function ()
		if  bufcnt(player.toTargetDist("Kicker") < 15 and
			player.toTargetDist("Leader") < 15, 20) then
			BALL_POSX = ball.posX()
			BALL_POSY = ball.posY()
			BALL_ANTI = ball.antiY()
			return "round"
		end
	end,
	Kicker = task.goBackBall(CGeoPoint:new_local(300,0),25),
	Leader   = task.goBackBall(0,50),
	match    = "{L}"
},

["round"] = {
	switch = function ()
		if bufcnt(true, 300) then
			return "shoot"
		elseif  bufcnt(player.canDirectShoot("Kicker"),60) then
			return "pass"
		end
	end,
	Kicker = task.goBackBall(PASS_DIR,30),
	Leader = task.goBackBall(CGeoPoint:new_local(300,0),25),
	match  = "{L}"
},	

["pass"] = {
    switch = function ()
		if  bufcnt(player.kickBall("Kicker"), 1, 50) then
			return "fixgoto"
		end
	end,
	Kicker = task.passToDir(PASS_DIR, 170),
	Leader = task.goCmuRush(SHOOT_POS, SHOOT_DIR, 1000),
	match  = "{L}"
},

["fixgoto"] = {
    switch = function ()
		if bufcnt(player.toTargetDist("Leader") < 35, "normal") then
			return "kick"
		elseif  bufcnt(true, 100) then
			return "exit"
		end
	end,
	Kicker = task.stop(),
	Leader = task.goCmuRush(SHOOT_POS, SHOOT_DIR, 1000),
	match  = "{L}"
},

["kick"] = {
    switch = function ()
		if bufcnt(player.kickBall("Leader"), 1, 120) then
			return "exit"
		end
	end,
	Kicker = task.stop(),
	Leader = task.touch(SHOOT_TARGET),
	match  = "{L}"
},

["shoot"] = {
    switch = function ()
		if bufcnt(player.kickBall("Leader"), 1, 150) then
			return "exit"
		end
	end,
	Leader   = task.shoot(),
	Kicker = task.stop(),
	match    = "{L}"
},

name = "TestShortPassAndShoot",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}