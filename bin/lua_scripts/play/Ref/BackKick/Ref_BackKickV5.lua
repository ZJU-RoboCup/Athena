-- 两传一射，针对MRL
-- by Fantasy
-- motify by patrick 2016.4.7----------add crazy trick

local WAIT_MIDDLE_POS = ball.refAntiYPos(CGeoPoint:new_local(-94, 160))
local WAIT_SHOOT_POS =  ball.refAntiYPos(CGeoPoint:new_local(0, 0))
local WAIT_PASS_POS = ball.refAntiYPos(CGeoPoint:new_local(-76, -117))


--local FINAL_SHOOT_POS = ball.refAntiYPos(CGeoPoint:new_local(-70, -180))---此点能射进
local FINAL_SHOOT_POS = ball.refAntiYPos(CGeoPoint:new_local(-34, -180))---要调
local FINAL_MIDDLE_POS = ball.refAntiYPos(CGeoPoint:new_local(170, 200))
local FINAL_PASS_POS = ball.refAntiYPos(CGeoPoint:new_local(100, -240))
local PASS_POS = pos.passForTouch(CGeoPoint:new_local(108, -240))

local FINAL_TRICK_POS = ball.refAntiYPos(CGeoPoint:new_local(307, 210))
local TRICK_POS = ball.refAntiYPos(CGeoPoint:new_local(46, 120))------------trick shoot 
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
			if bufcnt(player.toTargetDist("Special") < 100 , 100, 180) then
				return "firstPass"
			end
		end,
		Assister = USE_CHIP and task.slowGetBall(PASS_POS) or task.staticGetBall(FINAL_PASS_POS),
		Leader	 = task.goCmuRush(TRICK_POS, _, 600, flag.allow_dss),
		Middle   = task.goCmuRush(FINAL_MIDDLE_POS, _, 600, flag.allow_dss),
		Special  = task.goCmuRush(FINAL_PASS_POS, player.toBallDir, 600, flag.allow_dss),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{ASLMD}"
	},

	["firstPass"] = {
		switch = function ()
			if  bufcnt(player.kickBall("Assister") or player.toBallDist("Assister") > 20, "fast") then
    			return "secondPass"
    		end
		end,
		Assister = USE_CHIP and task.chipPass(CGeoPoint:new_local(208, -240), 350),
		Leader	 = task.goCmuRush(TRICK_POS, _, 600, flag.allow_dss),
		Middle   = task.goCmuRush(FINAL_MIDDLE_POS, _, 600, flag.allow_dss),
		Special  = task.goCmuRush(FINAL_PASS_POS, _, 600, flag.allow_dss),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{ASLMD}"
	},

	["secondPass"] = {
		switch = function ()
			if bufcnt(player.kickBall("Special") or player.isBallPassed("Special", "Leader"), 1) then
				return "shoot"
			elseif bufcnt(true, 300) then
				return "exit"
			end
		end,
		Assister = task.leftBack(),
		Leader	 = task.goCmuRush(FINAL_SHOOT_POS, _, 600, flag.allow_dss),
		Middle   = task.goCmuRush(FINAL_MIDDLE_POS, _, 600, flag.allow_dss),
		Special  = task.receivePass("Leader"),
		Defender = task.rightBack(),
		Goalie   = task.goalie(),
		match    = "{ASLMD}"
	},
	["shoot"] = {
		switch = function ()
			if bufcnt(player.kickBall("Leader"), 1, 60) then
				return "exit"
			end
		end,
		Assister = task.leftBack(),
		Leader	 = task.waitTouchNew(),--task.touch(),--task.waitTouch(FINAL_SHOOT_POS, 0), --task.InterNew(),--
		Middle   = task.goCmuRush(FINAL_TRICK_POS, _, 600, flag.allow_dss),
		Special  = task.goSupportPos("Leader"),
		Defender = task.rightBack(),
		Goalie   = task.goalie(),
		match    = "{LS}[MAD]"
	},

	name 	   	= "Ref_BackKickV5",
	applicable = {
		exp = "a",
		a   = true
	},
	attribute 	= "attack",
	timeout 	= 99999
}