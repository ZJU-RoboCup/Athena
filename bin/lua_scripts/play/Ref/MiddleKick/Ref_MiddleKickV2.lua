-- 两个车同边拉位，一个车冲反边打截球射门
local FINAL_SHOOT_POS  = CGeoPoint:new_local(210,200)

local SYNT_CORNER_POS1 = ball.refSyntYPos(CGeoPoint:new_local(250,150))
local SYNT_CORNER_POS2 = ball.refSyntYPos(CGeoPoint:new_local(250,120))
local ANTI_RUSH_POS    = ball.refAntiYPos(CGeoPoint:new_local(-150,120))
local ANTI_CORNER_POS1 = ball.refAntiYPos(FINAL_SHOOT_POS)
local TMP_POS1         = ball.refAntiYPos(CGeoPoint:new_local(120,0))
local TMP_POS2         = ball.refAntiYPos(CGeoPoint:new_local(160,0))

local SEC_TMP_POS1     = ball.refSyntYPos(CGeoPoint:new_local(120,100))
local SEC_TMP_POS2     = ball.refSyntYPos(CGeoPoint:new_local(160,100))
local GATE_POS		   = CGeoPoint:new_local(450,0)
local CHIP_POS         = FINAL_SHOOT_POS--pos.passForTouch(FINAL_SHOOT_POS)

local USE_CHIP = false

gPlayTable.CreatePlay{

firstState = "gotmp",

["gotmp"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Leader") < 15 and
			      player.toTargetDist("Special") < 15, 2, 180) then
			return "gotmp2"
		end
	end,
	Assister = task.staticGetBall(CHIP_POS),
	Leader   = task.goCmuRush(TMP_POS1),
	Special  = task.goCmuRush(TMP_POS2),
	Middle   = task.goCmuRush(ANTI_RUSH_POS),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{A}[LSM][D]"
},

["gotmp2"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Leader") < 15 and
			      player.toTargetDist("Special") < 15, 1, 180) then
			return "startball"
		end
	end,
	Assister = task.staticGetBall(CHIP_POS),
	Leader   = task.goCmuRush(SEC_TMP_POS1),
	Special  = task.goCmuRush(SEC_TMP_POS2),
	Middle   = task.goCmuRush(ANTI_RUSH_POS),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{A}[LSM][D]"
},


["startball"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Leader") < 40 and
			      player.toTargetDist("Special") < 40, "normal", 180) then
			return "sidegoto"
		end
	end,
	Assister = task.staticGetBall(CHIP_POS),
	Leader   = task.goCmuRush(SYNT_CORNER_POS1),
	Special  = task.goCmuRush(SYNT_CORNER_POS2),
	Middle   = task.goCmuRush(ANTI_RUSH_POS),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{A}[LSM][D]"
},

["sidegoto"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Middle") < 140, "fast", 150) then
			return "sidepass"
		end
	end,
	Assister = USE_CHIP and task.slowGetBall(CHIP_POS) or task.staticGetBall(CHIP_POS),
	Leader   = task.goCmuRush(SYNT_CORNER_POS1),
	Special  = task.goCmuRush(SYNT_CORNER_POS2),
	Middle   = task.goCmuRush(ANTI_CORNER_POS1),
	Defender = task.singleBack(),
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
	Assister = USE_CHIP and task.chipPass(CHIP_POS, 400) or task.goAndTurnKick(CHIP_POS,500),
	Leader   = task.goCmuRush(SYNT_CORNER_POS1),
	Special  = task.goCmuRush(SYNT_CORNER_POS2),
	Middle   = task.goCmuRush(ANTI_CORNER_POS1),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALMSD}"
},

["fixgoto"] = {
    switch = function ()
		if bufcnt(true, 1) then
			return "sidekick"
		end
	end,
	Leader   = task.goCmuRush(SYNT_CORNER_POS1),
	Special  = task.goCmuRush(SYNT_CORNER_POS2),
	Middle   = task.goCmuRush(ANTI_CORNER_POS1),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{MLS}(AD)"
},

["sidekick"] = {
    switch = function ()
		if bufcnt(player.kickBall("Leader"), 1, 150) then
			return "exit"
		end
	end,
	Leader   = task.defendMiddle(),
	Special  = task.goSupportPos("Middle"),
	Middle   = task.receivePass(GATE_POS,700),--task.InterceptNew(_,1,700),--task.InterNew(),--Intercept(),--
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{MLS}(AD)"
},

name = "Ref_MiddleKickV2",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}