-- FrontKickV4改版 前场靠前的地方打横传
-- 射门改成了InterTouch 提高成功率

local TMP_POS = ball.refSyntYPos(CGeoPoint:new_local(170, 100))
local SYNT_CORNER_POS = ball.refSyntYPos(CGeoPoint:new_local(370, 240))
local ANTI_CORNER_POS = ball.refAntiYPos(CGeoPoint:new_local(370, 240))
local TOUCH_POS = ball.refAntiYPos(CGeoPoint(param.pitchLength/2.0, 15))

local BACK_DIR = function(d)
	return function()
		return ball.antiY()*d
	end
end

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
	Defender = task.goCmuRush(ball.refAntiYPos(CGeoPoint:new_local(-240, 220)), _, 600, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{ALSDM}"
},

["continueGo"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Defender") < 100, 1, 180) then			
			return "continuePass"
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

["continuePass"] = {
	switch = function()
		if  player.kickBall("Assister") or player.isBallPassed("Assister","Defender") then
			return "continueShoot"
		elseif  bufcnt(true, 60) then--90) then
			return "exit"
		end
	end,
	Assister = task.passToPos(pos.passForTouch(ball.goRush()),550),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, 650, flag.allow_dss),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, 650, flag.allow_dss),
	Defender = task.goCmuRush(ball.goRush()),
	Leader   = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ASDLM}"
},

["continueShoot"] = {
	switch = function()
		if  bufcnt(player.kickBall("Defender"), 1, 90) then
			return "exit"
		end
	end,
	Defender = task.InterTouch(ball.goRush()),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, 600, flag.allow_dss),
	Middle   = task.defendMiddle(),
	Assister = task.leftBack(),
	Leader   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{SD}[LAM]"
},

name = "Ref_FrontKickV1604",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}
