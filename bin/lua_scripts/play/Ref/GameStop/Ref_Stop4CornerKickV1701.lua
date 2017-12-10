-- 半场模式角球定位球在一开始就进行站位
-- by Alan 2016-12-01

local STOP_FLAG  = bit:_or(flag.slowly, flag.dodge_ball)
local STOP_DSS   = bit:_or(STOP_FLAG, flag.allow_dss)
local KICK_POS   = function ()
	return ball.syntYPos(CGeoPoint:new_local(ball.posX() - 59, 300))()
end

local KICK_DIR  = ball.antiYDir(1.57)

local MIDDLE_POS = ball.antiYPos(CGeoPoint:new_local(190, -25))

local FRONT_POS1 = ball.antiYPos(CGeoPoint:new_local(345, 150))
local FRONT_POS2 = ball.antiYPos(CGeoPoint:new_local(325, 125))

local OTHER_POS = {
  CGeoPoint:new_local(150,-120),
  CGeoPoint:new_local(150,120)
}
local BACK_POS=CGeoPoint:new_local(80,0)

local ACC = 400

gPlayTable.CreatePlay{

firstState = "start",

["start"] = {
	switch = function()
		if cond.isGameOn() then
			return "exit"
		end	  
	end,
	Kicker   = task.goCmuRush(KICK_POS, KICK_DIR, ACC, STOP_DSS),
	Special  = task.goCmuRush(FRONT_POS1, _, ACC, STOP_DSS),
	Leader   = task.goCmuRush(FRONT_POS2, _, ACC, STOP_DSS),
	Middle   = task.goCmuRush(MIDDLE_POS, _, ACC, STOP_DSS),
	Defender = task.goCmuRush(OTHER_POS[1], _, ACC, STOP_DSS),
	Goalie   = task.goCmuRush(BACK_POS, _, ACC, STOP_DSS),
	match    = "[MLSD]"
},

name = "Ref_Stop4CornerKickV1701",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}