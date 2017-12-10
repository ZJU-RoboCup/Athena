-- 角球超必杀，三车对禁区进行围堵攻击
-- 后卫冲进禁区射门 by zhyaic 2013.5.23
-- 2014-07-20 yys 改
local AVOID_KEEPER_POS = CGeoPoint:new_local(0,0)
local FINAL_SHOOT_POS = CGeoPoint:new_local(350, 90)
local READY_POS1  = ball.refAntiYPos(CGeoPoint:new_local(-50,0))
local READY_POS2  = ball.refAntiYPos(CGeoPoint:new_local(-50,50))
local READY_POS3  = ball.refAntiYPos(CGeoPoint:new_local(-150,150))
local MIDDLE_POS1 = ball.refAntiYPos(CGeoPoint:new_local(150,80))
local MIDDLE_POS2 = ball.refAntiYPos(CGeoPoint:new_local(240,150))
local MIDDLE_POS3 = ball.refAntiYPos(FINAL_SHOOT_POS)
local CHIP_POS	  = pos.passForTouch(FINAL_SHOOT_POS)
local GOALIE_POS  = ball.refSyntYPos(CGeoPoint:new_local(0,50))
-- 以下的两个点与车的超调情况有关，可以适当地往前及向后
-- 同时，此参数需要与goopen中的player.toTargetDist(...)<20
-- 中20这个量一起进行调节
local RUSH_POS1   = ball.refAntiYPos(CGeoPoint:new_local(215,15))
local RUSH_POS2   = ball.refAntiYPos(CGeoPoint:new_local(270,65))
local BREAKER_POS1 = ball.refAntiYPos(CGeoPoint:new_local(230,100))
local BREAKER_POS2 = ball.refAntiYPos(CGeoPoint:new_local(305,110))
local BREAKER_POS3 = ball.refAntiYPos(CGeoPoint:new_local(310,35))

local ball_Change_Pos = function ()
	local tmpX
	local tmpY

	local UpdatePos = function ()
		tmpX = ball.posX() - 100
		tmpY = ball.posY() - 30
		if tmpX > 180 then
			tmpX = 180
		end
		if tmpY > 0 then
			tmpY = ball.posY() - 30
		else
			tmpY = ball.posY() + 30
		end
		if tmpY > 160 then
			tmpY = 160
		end
		if tmpY < -160 then
			tmpY = -160
		end
	end

	local pos_BallChange = function ()
		UpdatePos()
		return CGeoPoint:new_local(tmpX,tmpY) , CGeoPoint:new_local(tmpX, tmpY*(-1))
	end

	return pos_BallChange
end

local BALLCHANGE_POS_SYNT,BALLCHANGE_POS_ANTI = ball_Change_Pos()

gPlayTable.CreatePlay{

firstState = "start",

["start"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Special")<30 and
		   player.toTargetDist("Leader")<30 and
		   player.toTargetDist("Middle")<30,  "normal") then
			return "goalone"
		end
	end,
	Assister = task.staticGetBall(CHIP_POS),
	Special  = task.goCmuRush(ball.refAntiYPos(CGeoPoint:new_local(180, 160)),_,_,flag.allow_dss),
	Middle   = task.goCmuRush(AVOID_KEEPER_POS,_,_,flag.allow_dss),
	Defender = task.goCmuRush(READY_POS2,_,_,flag.allow_dss),
	Leader   = task.goCmuRush(READY_POS3,_,_,flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},

["goalone"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Leader") < 30, "fast") then
			return "goget"
		end
	end,
	Assister = task.slowGetBall(CHIP_POS),
	Special  = task.goSpeciPos(BALLCHANGE_POS_SYNT),
	Middle   = task.goCmuRush(READY_POS1),
	Defender = task.goCmuRush(READY_POS2),
	Leader   = task.goCmuRush(MIDDLE_POS1),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

["goget"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Leader") < 30, "fast") then
			return "gopass"
		end
	end,
	Assister = task.slowGetBall(CHIP_POS),
	Special  = task.goCmuRush(BALLCHANGE_POS_SYNT),
	Middle   = task.goCmuRush(RUSH_POS1),
	Defender = task.goCmuRush(RUSH_POS2),
	Leader   = task.goCmuRush(MIDDLE_POS2),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

["gopass"] = {
	switch = function ()
		if player.kickBall("Assister") or
		   player.toBallDist("Assister") > 30 then
			return "goopen"
		elseif  bufcnt(true, 120) then
			score.minus(4)
			return "exit"
		end
	end,
	Leader   = task.goCmuRush(MIDDLE_POS3),
	Assister = task.chipPass(CHIP_POS, 190),
	Middle   = task.goCmuRush(RUSH_POS1),
	Defender = task.goCmuRush(RUSH_POS2),
	Special  = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

["goopen"] = {
    switch = function ()
    	if bufcnt(true, 100) then
    		return "gokick"
    	end
	end,

	Leader   = task.goCmuRush(MIDDLE_POS3),
	Middle   = task.goCmuRush(RUSH_POS1),
	Defender = task.goCmuRush(RUSH_POS2),
	Assister = task.rightBack(),
	Special  = task.leftBack(),
	Goalie   = task.goalie(),
	match    = "{LMD}(AS)"
},

["gokick"] = {
    switch = function ()
		if player.kickBall("Leader") then
			return "finish"
		elseif bufcnt(true, 100) then
			return "exit"
		end
	end,
	Defender = task.touchBetweenPos(BREAKER_POS2,RUSH_POS2),
	Middle   = task.touchBetweenPos(RUSH_POS1,BREAKER_POS1),
	Leader   = task.waitTouch(MIDDLE_POS3,0),
	Assister = task.rightBack(),
	Special  = task.leftBack(),
	Goalie   = task.goalie(),
	match    = "{LMD}[AS]"
},

name = "Ref_CornerKickV4",
applicable = {
	exp = "a",
	a = true
},
score = 0,
attribute = "attack",
timeout = 99999
}