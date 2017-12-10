-- 直接挑门
local FINAL_SHOOT_POS = CGeoPoint:new_local(260, 200)

local MIDDLE_POS_1 = CGeoPoint:new_local(100, 0)
local MIDDLE_POS_2 = ball.jamPos(CGeoPoint:new_local(450, 0), 50, 10)

local ANTI_RUSH_POS    = ball.refAntiYPos(CGeoPoint:new_local(-70, 200))
local ANTI_CORNER_POS1 = ball.refAntiYPos(FINAL_SHOOT_POS)

local StopAroundBall = function ()
	local BLOCK_DIST = 40
	local AWAY_DIST = 2.5 + param.playerRadius
	local BLOCK_ANGLE = math.asin(AWAY_DIST / BLOCK_DIST) * 2
	local factor = ball.antiY

	local SIDE_POS = function()
		return ball.pos() + Utils.Polar2Vector(BLOCK_DIST, dir.ballToOurGoal() + factor() * BLOCK_ANGLE)
	end

	local INTER_POS = function()
		return ball.pos() + Utils.Polar2Vector(BLOCK_DIST, dir.ballToOurGoal() - factor() * BLOCK_ANGLE)
	end

	local MIDDLE_POS = function()
		return ball.pos() + Utils.Polar2Vector(BLOCK_DIST, dir.ballToOurGoal())
	end

	return SIDE_POS, MIDDLE_POS, INTER_POS
end

local TMP_POS1, TMP_POS2 = StopAroundBall()

local FACE_POS = CGeoPoint:new_local(-100, 0)

local CHIP_POS = ball.refAntiYPos(CGeoPoint:new_local(450, 0))

gPlayTable.CreatePlay{

firstState = "gotmp",

["gotmp"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Leader") < 20 and
				  player.toTargetDist("Special") < 20, 10, 180) then
			return "gotmp2"
		end
	end,
	Assister = task.staticGetBall(FACE_POS),
	Leader   = task.goCmuRush(TMP_POS1),
	Special  = task.goCmuRush(TMP_POS2),
	Middle   = task.goCmuRush(ANTI_RUSH_POS),
	Defender = task.goCmuRush(MIDDLE_POS_1, _, 600, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},

["gotmp2"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Defender") < 20, 2, 180) then
			return "chip"
		end
	end,
	Assister = task.staticGetBall(FACE_POS),
	Leader   = task.goCmuRush(TMP_POS1),
	Special  = task.goCmuRush(TMP_POS2),
	Middle   = task.goCmuRush(ANTI_RUSH_POS),
	Defender = task.goCmuRush(MIDDLE_POS_2, _, 600, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},


["chip"] = {
	switch = function ()
		if bufcnt(player.kickBall("Assister") or
			      player.toBallDist("Assister") > 30, "slow", 60) then
			return "finish"
		end
	end,
	Assister = task.goAndTurnKickQuick(CHIP_POS, 250, "chip"),
	Leader   = task.goCmuRush(TMP_POS1),
	Special  = task.goCmuRush(TMP_POS2),
	Middle   = task.goCmuRush(ANTI_CORNER_POS1),
	Defender = task.goCmuRush(MIDDLE_POS_2, _, 600, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

name = "Ref_FrontKickV11",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}