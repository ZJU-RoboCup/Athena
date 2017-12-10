-- 角球定位球在一开始就进行站位
-- by zhyaic 2014-04-08
-- yys 2015-06-10

local STOP_FLAG  = bit:_or(flag.slowly, flag.dodge_ball)
local STOP_DSS   = bit:_or(STOP_FLAG, flag.allow_dss)
local KICK_POS   = function ()
	return ball.syntYPos(CGeoPoint:new_local(ball.posX() - 59, 300))()
end

local KICK_DIR  = ball.antiYDir(1.57)

local MIDDLE_POS = ball.antiYPos(CGeoPoint:new_local(190, -25))

local FRONT_POS1 = ball.antiYPos(CGeoPoint:new_local(345, 150))
local FRONT_POS2 = ball.antiYPos(CGeoPoint:new_local(325, 125))

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
	Defender = task.singleBack4Stop(),
	Goalie   = task.goalie(),
	match    = "[MLSD]"
},

name = "Ref_Stop4CornerKick",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}