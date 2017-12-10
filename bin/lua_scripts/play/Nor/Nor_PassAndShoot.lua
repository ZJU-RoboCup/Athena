-- 注意：此脚本中advance和shoot状态下用的都是抢球的skill
-- 目前使用两车盯人，打弱队时可以使用前场跑goPassPos

local isUsePass = false

gPlayTable.CreatePlay{
firstState = "shoot",

["shoot"] = {
	switch = function ()
		if bufcnt(ball.posX() < -50, "normal") then
			return "defend"
		elseif cond.bestPlayerChanged() then
			return "shoot"
		elseif bufcnt(not cond.canShootOnBallPos("Leader") and 
			ball.enemyDistMinusPlayerDist("Leader") < 0, "normal") then
			return "advance"
		elseif bufcnt(isUsePass and not cond.canShootOnBallPos("Leader") and
			player.toTargetDist("Special") < 20 , "normal") then
			return "pass"
		elseif player.kickBall("Leader") then
			return "finish"
		end
	end,
	Leader   = task.advance(),
	Special  = task.goPassPos("Leader"),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][DAM]"
},

["advance"] = {
	switch = function ()
		if bufcnt(ball.posX()<-50, "normal") then
			return "defend"
		elseif cond.bestPlayerChanged() then
			return "advance"
		elseif bufcnt(cond.canShootOnBallPos("Leader") and 
			ball.enemyDistMinusPlayerDist("Leader") > 10, "fast") then
			return "shoot"
		elseif bufcnt(isUsePass and not cond.canShootOnBallPos("Leader") and
			player.toTargetDist("Special") < 20 and
			ball.enemyDistMinusPlayerDist("Leader") > 20, "normal") then
			return "pass"
		end
	end,
	Leader   = task.advance(),
	Special  = task.goPassPos("Leader"),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][DAM]"
},

["pass"] = {
	switch = function ()
		if player.kickBall("Leader") or 
		   player.isBallPassed("Leader", "Special") then
			return "kick"
		elseif bufcnt(cond.canShootOnBallPos("Leader") and
			cond.currentBayes("attack"), "normal") then
			return "shoot"
		elseif bufcnt(not cond.currentBayes("attack"), "normal") then
			return "advance"
		end
	end,
	Leader   = task.pass("Special"),
	Special  = task.goPassPos("Leader"),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L](SDAM)"
},

["kick"] = {
	switch = function ()
		if bufcnt(player.kickBall("Special"), "fast", 120) then
			return "exit"
		end
	end,
	Leader   = task.goPassPos("Special"),
	Special  = task.shoot(),
	Middle   = task.defendMiddle("Special"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{SLDAM}"
},

["defend"] = {
	switch = function ()
		if bufcnt(ball.posX() > -20, "normal") then
			return "advance"
		end
	end,
	Leader   = task.advance(), -- task.marking("Second")
	Middle   = task.goPassPos("Leader"),
	Special  = task.sideBack(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][DAS][M]"
},

name = "Nor_PassAndShoot",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}