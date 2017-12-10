local isUsePass = true
local PASS_CONDITION

gPlayTable.CreatePlay{

firstState = "advance",

["advance"] = {
	switch = function ()
		if cond.bestPlayerChanged() then
			return "advance"
		elseif bufcnt(ball.posX()<-50, "normal") then
			return "defend"
		elseif bufcnt(not player.isMarked("Special") and
			ball.enemyDistMinusPlayerDist("Leader") > 10 and
			not cond.canShootOnBallPos(), "normal") then
			return "pass"
		end
	end,
	Leader   = task.advance(),
	Special  = task.goPassPos("Leader"),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][S](DAM)"
},

["pass"] = {
	switch = function ()
		if player.kickBall("Leader") or 
		   player.isBallPassed("Leader", "Special") then
			return "kick"
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
	Leader   = task.advance(),
	Middle   = task.goPassPos("Leader"),
	Special  = task.marking("First"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][DA](S)(M)"
},

name = "Nor_MorePass",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
