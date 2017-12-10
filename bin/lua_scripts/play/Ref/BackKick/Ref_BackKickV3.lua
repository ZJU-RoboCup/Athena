local AtrPos1  = ball.refAntiYPos(CGeoPoint:new_local(80,120))    --拉人位置一
local AtrPos2  = ball.refAntiYPos(CGeoPoint:new_local(200,180))    --拉人位置二
local TMP_DEF_POS1 = ball.refAntiYPos(CGeoPoint:new_local(-100,100))
local TMP_DEF_POS2 = ball.refAntiYPos(CGeoPoint:new_local(0,20))
local chipPos = CGeoPoint:new_local(325,-140)
local ChipPos = ball.refAntiYPos(chipPos)

local function ChasePos()
	local x = ball.posX()+60
	local y = ball.posY()+ball.antiY()*20
	-- local y = ball.syntY() * 160
	return CGeoPoint:new_local(x,y)
end

local delayTime = (OPPONENT_NAME == "delay") and 360 or 10

gPlayTable.CreatePlay{
	firstState = "start",

	["start"] = {
		switch = function ()
			if bufcnt(player.toTargetDist("Leader") < 100 and
					      player.toTargetDist("Assister") < 100, delayTime) then
				return "getBall"
			end
		end,
		Kicker   = task.staticGetBall(chipPos),
		Assister = task.goCmuRush(ChasePos),
		Leader   = task.goCmuRush(AtrPos1),
		Special  = task.goCmuRush(AtrPos2),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{A}[DLS]"
	},

	["getBall"] = {
		switch = function ()
			if bufcnt(player.toTargetDist("Assister") < 50 and
					      player.toTargetDist("Leader") < 50, 60, 100) then
				return "pass"
			end
		end,
		Kicker   = task.slowGetBall(chipPos),
		Assister = task.goCmuRush(ChasePos),
		Leader   = task.goCmuRush(AtrPos1),
		Special  = task.goCmuRush(AtrPos2),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{A}[DLS]"
	},

	["pass"] = {
		switch = function ()
			if bufcnt(player.kickBall("Kicker"), 1, 20) then
				return "approch"
			end
		end,
		Kicker   = task.chipPass(chipPos, 130),
		Assister = task.goCmuRush(ChipPos, dir.shoot(), 100),
		Leader   = task.goCmuRush(TMP_DEF_POS1),
		Special  = task.goCmuRush(TMP_DEF_POS2),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{A}[DLS]"
	},

	["approch"] = {
		switch = function ()
			if bufcnt(ball.posX() - player.posX("Assister") > 30, "fast", 60) then
				return "chase"
			end
		end,
		Kicker   = task.leftBack(),
		Assister = task.goCmuRush(ChipPos, dir.shoot(), 100),
		Leader   = task.goCmuRush(TMP_DEF_POS1),
		Special  = task.goCmuRush(TMP_DEF_POS2),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{A}[DLS]"
	},

	["chase"] = {
		switch = function ()
			if bufcnt(player.kickBall("Assister"), 1, 100) then
				return "exit"
			end
		end,
		Assister = task.shoot(),
		Leader   = task.goSupportPos("Assister"),
		Special  = task.goCmuRush(TMP_DEF_POS2),
		Defender = task.rightBack(),
		Kicker   = task.leftBack(),
		Goalie   = task.goalie(),
		match    = "{A}[DLS]"
	},

	name 	   	= "Ref_BackKickV3",
	applicable 	= {
		exp = "a",
		a   = true
	},
	attribute 	= "attack",
	timeout 	= 99999
}


