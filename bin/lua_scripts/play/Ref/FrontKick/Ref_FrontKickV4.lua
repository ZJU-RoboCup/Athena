-- 修改为强行传球 by zhyaic 2014.07.21
local TMP_POS = ball.refSyntYPos(CGeoPoint:new_local(170, 100))
local SYNT_CORNER_POS = ball.refSyntYPos(CGeoPoint:new_local(370, 240))
local ANTI_CORNER_POS = ball.refAntiYPos(CGeoPoint:new_local(370, 240))

local BACK_DIR = function(d)
	return function()
		return ball.antiY()*d
	end
end

local CHIP_PASS = false

gPlayTable.CreatePlay{

firstState = "tmpState",

["tmpState"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Special") < 20, 20, 180) then
			return "oneReady"
		end
	end,
	Assister = task.goBackBall(CGeoPoint:new_local(450, -100), 20),
	Leader   = task.goBackBall(0, 50),
	Special  = task.goCmuRush(ball.goRush(), _, 600, flag.allow_dss),
	Middle   = task.goCmuRush(TMP_POS, _, 600, flag.allow_dss),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{A}{LSDM}"
},

["oneReady"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Assister") < 5, "normal", 180) then
			return "twoReady"
		end
	end,
	Assister = task.goBackBall(CGeoPoint:new_local(450, -100), 20),
	Leader   = task.goBackBall(0, 50),
	Special  = task.goCmuRush(ball.goRush(), _, 600, flag.allow_dss),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, 600, flag.allow_dss),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{A}{LSDM}"
},

["twoReady"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Leader") < 5, "normal", 180) then
			return "chooseKicker"
		end
	end,
	Assister = task.goBackBall("Special", 25),
	Leader   = task.goBackBall(CGeoPoint:new_local(450, -100), 20),
	Special  = task.goCmuRush(ball.goRush(), _, 600, flag.allow_dss),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, 600, flag.allow_dss),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALSDM}"
},

["chooseKicker"] = {
	switch = function()
		-- 由于CMU三车盯人，因此坚定地跑掉一辆车
		if  bufcnt(true, 60) then
			return "continueGo"
		end
	end,
	Assister = task.goBackBall("Special", 25),
	Leader   = task.goBackBall(CGeoPoint:new_local(450, -100), 20),
	Special  = task.goCmuRush(ball.goRush(), _, 600, flag.allow_dss),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, 600, flag.allow_dss),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALSDM}"
},

["continueGo"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Defender") < 50, 1, 180) then			
			if CHIP_PASS then
				return "dribble"
			else
				return "continuePass"
			end
		end
	end,
	Assister = task.stop(),
	Leader   = task.goBackBall(CGeoPoint:new_local(450, -100), 20),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, 600, flag.allow_dss),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, 600, flag.allow_dss),
	Defender = task.goCmuRush(ball.goRush(), _, 600, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{ALSDM}"
},

["dribble"] = {
	switch = function()
		if  bufcnt(true, 30) then
			return "continuePass"
		end
	end,
	Assister = task.slowGetBall(ball.goRush(),false),
	Leader   = task.goBackBall(CGeoPoint:new_local(450, -100), 20),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, 600, flag.allow_dss),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, 600, flag.allow_dss),
	Defender = task.goCmuRush(ball.goRush(), _, 600, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{ALSDM}"
},

["continuePass"] = {
	switch = function()
		if  player.kickBall("Assister") or 
			player.isBallPassed("Assister","Defender") then
			if CHIP_PASS then
				return "waitForShoot"
			else 
				return "continueShoot"
			end
		elseif  bufcnt(true, 90) then
			return "exit"
		end
	end,
	Assister = CHIP_PASS and task.chipPass(pos.passForTouch(ball.goRush()),300,false) or task.passToPos(pos.passForTouch(ball.goRush()),550),--task.passToPos(pos.passForTouch(ball.goRush())),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, 650, flag.allow_dss),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, 650, flag.allow_dss),
	Defender = task.goCmuRush(ball.goRush()),
	Leader   = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ASDLM}"
},

["waitForShoot"] = {
	switch = function()
		if  bufcnt(true,40) then
			return "continueShoot"
		end
	end,
	Assister = task.stop(),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, 650, flag.allow_dss),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, 650, flag.allow_dss),
	Defender = task.goCmuRush(ball.goRush()),
	Leader   = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ASDLM}"
},

["continueShoot"] = {
	switch = function()
		if  bufcnt(player.kickBall("Defender"), 1, 120) then
			return "exit"
		end
	end,
	Defender = task.InterTouch(),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, 600, flag.allow_dss),
	Middle   = task.defendMiddle(),
	Assister = task.leftBack(),
	Leader   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{SD}[LAM]"
},

name = "Ref_FrontKickV4",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}
