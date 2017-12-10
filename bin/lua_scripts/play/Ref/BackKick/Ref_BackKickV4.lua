-- 针对MRL，挑传球到同侧，转身射门
-- by Fantasy


local WAIT_SHOOT_POS = ball.refAntiYPos(CGeoPoint:new_local(-94, 160))
local WAIT_MIDDLE_POS =  ball.refAntiYPos(CGeoPoint:new_local(0, 0))
local WAIT_PASS_POS = ball.refAntiYPos(CGeoPoint:new_local(-76, -117))

local FINAL_SHOOT_POS_ORG = CGeoPoint:new_local(208, 208)
local FINAL_SHOOT_POS = ball.refAntiYPos(FINAL_SHOOT_POS_ORG)
local FINAL_MIDDLE_POS = ball.refAntiYPos(CGeoPoint:new_local(70, 200))
local FINAL_PASS_POS = ball.refAntiYPos(CGeoPoint:new_local(208, -240))
local PASS_POS = pos.passForTouch(CGeoPoint:new_local(208, -240))

local FAKE_PASS_POS = ball.refAntiYPos(CGeoPoint:new_local(100,223))
local USE_CHIP  = true

gPlayTable.CreatePlay{
	firstState = "start",

	["start"] = {
		switch = function ()
			if  bufcnt( player.toTargetDist("Special") < 30 and
						player.toTargetDist("Leader") < 30, 10, 120) then
				return "wait"
			end
		end,
		Assister = task.staticGetBall(FAKE_PASS_POS),
		Special  = task.goCmuRush(WAIT_PASS_POS, _, 600, flag.allow_dss),
		Leader	 = task.goCmuRush(WAIT_SHOOT_POS, _, 600, flag.allow_dss),
		Middle   = task.goCmuRush(WAIT_MIDDLE_POS, _, 600, flag.allow_dss),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{A}[SLMD]"
	},

	["wait"] = {
		switch = function ()
			if bufcnt(player.toTargetDist("Special") < 150, 1, 120) then
				return "firstPass"
			end
		end,
		Assister = USE_CHIP and task.slowGetBall(PASS_POS) or task.staticGetBall(FINAL_PASS_POS),
		Leader	 = task.goCmuRush(FINAL_SHOOT_POS, _, 600, flag.allow_dss),
		Middle   = task.goCmuRush(FINAL_MIDDLE_POS, _, 600, flag.allow_dss),
		Special  = task.goCmuRush(FINAL_PASS_POS, player.toBallDir, 600, flag.allow_dss),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{ASLMD}"
	},

	["firstPass"] = {
		switch = function ()
			if  bufcnt(player.kickBall("Assister") or player.toBallDist("Assister") > 20, "fast") then
    			return "shoot"
    		end
		end,
		Assister = USE_CHIP and task.chipPass(CGeoPoint:new_local(208, -240), 400),
		Leader	 = task.goCmuRush(FINAL_SHOOT_POS, _, 600, flag.allow_dss),
		Middle   = task.goCmuRush(FINAL_MIDDLE_POS, _, 600, flag.allow_dss),
		Special  = task.goCmuRush(FINAL_PASS_POS, _, 600, flag.allow_dss),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{ASLMD}"
	},

	["shoot"] = {
		switch = function ()
			if bufcnt(player.kickBall("Leader"), 20, 180) then
				return "exit"
			end
		end,
		Assister = task.leftBack(),
		Leader	 = task.goSupportPos("Special"),
		Middle   = task.defendMiddle(),
		Special  = task.InterceptNew(),
		Defender = task.rightBack(),
		Goalie   = task.goalie(),
		match    = "{LS}[MAD]"
	},

	name 	   	= "Ref_BackKickV4",
	applicable = {
		exp = "a",
		a   = true
	},
	attribute 	= "attack",
	timeout 	= 99999
}