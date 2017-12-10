-- 使用两车开球，可直接骗定位球也可以打一传一射
-- by zhyaic 2013.6.28

local TMP_POS = ball.refSyntYPos(CGeoPoint:new_local(150,0))
local ANTI_CORNER_POS = ball.refAntiYPos(CGeoPoint:new_local(250,150))
local SYNT_CORNER_POS2 = ball.refSyntYPos(CGeoPoint:new_local(230,150))

local BACK_DIR = function(d)
	return function()
		return ball.antiY()*d
	end
end

gPlayTable.CreatePlay{
 
firstState = "tmpState",

["tmpState"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Middle") < 20, "normal") then
			return "oneReady"
		end
	end,
	Assister = task.goBackBall(CGeoPoint:new_local(325,0), 20),
	Leader   = task.goBackBall(0,50),
	Special  = task.goCmuRush(ball.goRush()),
	Middle   = task.goCmuRush(TMP_POS),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},

["oneReady"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Assister") < 5, "normal") then
			return "twoReady"
		end
	end,
	Assister = task.goBackBall(CGeoPoint:new_local(325,0), 20),
	Leader   = task.goBackBall(0,50),
	Special  = task.goCmuRush(ball.goRush()),
	Middle   = task.goCmuRush(ANTI_CORNER_POS),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},

["twoReady"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Leader") < 5, "normal") then
			return "chooseKicker"
		end
	end,
	Assister = task.goBackBall("Special", 25),
	Leader   = task.goBackBall(CGeoPoint:new_local(325,0), 20),
	Special  = task.goCmuRush(ball.goRush()),
	Middle   = task.goCmuRush(ANTI_CORNER_POS),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

["chooseKicker"] = {
	switch = function()
		if  bufcnt(player.canDirectShoot("Leader",100), 40) then
			return "justShoot"
		elseif  bufcnt(not player.isMarked("Special") and
			player.canFlatPassTo("Assister","Special"), 30) then
			return "passBall"
		elseif  bufcnt(true, 180) then
			return "justShoot"
		end
	end,
	Assister = task.goBackBall("Special", 25),
	Leader   = task.goBackBall(CGeoPoint:new_local(325,0), 20),
	Special  = task.goCmuRush(ball.goRush()),
	Middle   = task.goCmuRush(ANTI_CORNER_POS),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

["passBall"] = {
	switch = function()
		if  player.kickBall("Assister") or 
			player.isBallPassed("Assister","Special") then
			-- CSetPassDir("Assister")
			return "kickBall"
		elseif  bufcnt(true, 90) then
			return "exit"
		end
	end,
	Assister = task.pass("Special", 350),
	Special  = task.goCmuRush(ball.goRush()),
	Middle   = task.goCmuRush(ANTI_CORNER_POS),
	Defender = task.leftBack(),
	Leader   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{ASM}(LD)"
},

["kickBall"] = {
	switch = function()
		if  bufcnt(player.kickBall("Special"),1,60) then
			return "exit"
		end
	end,
	Assister = task.defendMiddle(),
	Special  = task.touch(),
	Middle   = task.goCmuRush(ANTI_CORNER_POS),
	Defender = task.leftBack(),
	Leader   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{SMLAD}"
},

["justShoot"] = {
	switch = function()
		if  bufcnt(player.kickBall("Assister"),1,120) then
			return "exit"
		end
	end,
	Leader   = task.shoot(),
	Assister = task.goCmuRush(SYNT_CORNER_POS2),
	Middle   = task.goCmuRush(ANTI_CORNER_POS),
	Special  = task.rightBack(),
	Defender = task.leftBack(),
	Goalie   = task.goalie(),
	match    = "{LAM}(SD)"
},

["justChip"] = {
	switch = function()
		if  bufcnt(player.kickBall("Leader") or player.toBallDist("Leader") > 50 ,1,120) then
			return "exit"
		end
	end,
	Leader   = task.chipPass(dir.shoot(),250, true),
	Assister = task.goCmuRush(SYNT_CORNER_POS2),
	Middle   = task.goCmuRush(ANTI_CORNER_POS),
	Special  = task.rightBack(),
	Defender = task.leftBack(),
	Goalie   = task.goalie(),
	match    = "{LAM}(SD)"	
},

name = "Ref_MiddleKickV16_1",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}
