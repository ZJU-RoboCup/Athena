-- 针对RoboDragons
-- by yys 2014-07-22
local FINAL_SHOOT_POS = CGeoPoint:new_local(270, 200)

local MIDDLE_POS_1 = CGeoPoint:new_local(50, 0)
local MIDDLE_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(230, -160))

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

local CHIP_POS = pos.passForTouch(FINAL_SHOOT_POS)

gPlayTable.CreatePlay{

firstState = "gotmp",

["gotmp"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Leader") < 15 and
				   player.toTargetDist("Special") < 15, 30, 180) then
			return "gotmp2"
		end
	end,
	Assister = task.staticGetBall(CHIP_POS),
	Leader   = task.goCmuRush(TMP_POS1),
	Special  = task.goCmuRush(TMP_POS2),
	Middle   = task.goCmuRush(ANTI_RUSH_POS),
	Defender = task.goCmuRush(MIDDLE_POS_1),
	Goalie   = task.goalie(),
	match    = "{A}[LSM][D]"
},

["gotmp2"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Defender") < 50, 1, 180) then
			return "sidegoto"
		end
	end,
	Assister = task.staticGetBall(CHIP_POS),
	Leader   = task.goCmuRush(TMP_POS1),
	Special  = task.goCmuRush(TMP_POS2),
	Middle   = task.goCmuRush(ANTI_RUSH_POS),
	Defender = task.goCmuRush(MIDDLE_POS_2, player.toBallDir, 600, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},


["sidegoto"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Middle") < 100, "fast", 150) then
			return "sidepass"
		end
	end,
	Assister = task.staticGetBall(CHIP_POS),
	Leader   = task.goCmuRush(TMP_POS1),
	Special  = task.goCmuRush(TMP_POS2),
	Middle   = task.goCmuRush(ANTI_CORNER_POS1),
	Defender = task.goCmuRush(MIDDLE_POS_2, player.toBallDir, 600, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

["sidepass"] = {
    switch = function ()
		if bufcnt(player.kickBall("Assister") or player.toBallDist("Assister") > 30, 1) then
			return "fixgoto"
		elseif bufcnt(true, 120) then
			return "exit"
		end
	end,
	Assister = task.goAndTurnKick(CHIP_POS, 600),
	Leader   = task.stop(),
	Special  = task.stop(),
	Middle   = task.goCmuRush(ANTI_CORNER_POS1),
	Defender = task.goCmuRush(MIDDLE_POS_2, player.toBallDir, 600, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{ALMSD}"
},

["fixgoto"] = {
    switch = function ()
		if bufcnt(true, 5) then
			return "sidekick"
		end
	end,
	Leader   = task.defendMiddle(),
	Special  = task.sideBack(),
	Middle   = task.goCmuRush(ANTI_CORNER_POS1, _, 400),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{M}[LSAD]"
},

["sidekick"] = {
    switch = function ()
		if bufcnt(player.kickBall("Leader"), 1, 100) then
			return "exit"
		end
	end,
	Leader   = task.defendMiddle(),
	Special  = task.sideBack(),
	Middle   = task.waitTouchNew(ANTI_CORNER_POS1),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{M}[LSAD]"
},

name = "Ref_FrontKickV10",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}