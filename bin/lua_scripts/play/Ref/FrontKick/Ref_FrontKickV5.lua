-- 两车拉同边，另一车对侧打门

local FINAL_SHOOT_POS  = CGeoPoint:new_local(260,190)
local SYNT_CORNER_POS1 = ball.refSyntYPos(CGeoPoint:new_local(320,220))
local SYNT_CORNER_POS2 = ball.refSyntYPos(CGeoPoint:new_local(320,200))
local ANTI_RUSH_POS    = ball.refAntiYPos(CGeoPoint:new_local(-150,120))

local ANTI_CORNER_POS  = ball.refAntiYPos(FINAL_SHOOT_POS)
local CHIP_POS         = pos.passForTouch(FINAL_SHOOT_POS)
local TMP_POS1         = ball.refSyntYPos(CGeoPoint:new_local(150, 100))
local TMP_POS2         = ball.refSyntYPos(CGeoPoint:new_local(200, 100))

gPlayTable.CreatePlay{

firstState = "gotmp",

["gotmp"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Leader") < 40 and
			      player.toTargetDist("Special") < 40, "normal", 120) then
			return "startball"
		end
	end,
	Assister = task.staticGetBall(CHIP_POS),
	Leader   = task.goCmuRush(TMP_POS1, _, _, flag.allow_dss),
	Special  = task.goCmuRush(TMP_POS2, _, _, flag.allow_dss),
	Middle   = task.goCmuRush(ANTI_RUSH_POS, _, _, flag.allow_dss),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{A}[LS][MD]"
},

["startball"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Leader") < 40 and
			      player.toTargetDist("Special") < 40, "normal", 120) then
			return "sidegoto"
		end
	end,
	Assister = task.staticGetBall(CHIP_POS),
	Leader   = task.goCmuRush(SYNT_CORNER_POS1, _, _, flag.allow_dss),
	Special  = task.goCmuRush(SYNT_CORNER_POS2, _, _, flag.allow_dss),
	Middle   = task.goCmuRush(ANTI_RUSH_POS, _, _, flag.allow_dss),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{A}[LS][MD]"
},

["sidegoto"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Middle") < 200, "normal", 150) then
			return "sidepass"
		end
	end,
	Assister = task.slowGetBall(CHIP_POS),
	Leader   = task.goCmuRush(SYNT_CORNER_POS1, _, _, flag.allow_dss),
	Special  = task.goCmuRush(SYNT_CORNER_POS2, _, _, flag.allow_dss),
	Middle   = task.goCmuRush(ANTI_CORNER_POS, _, _, flag.allow_dss),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALSMD}"
},

["sidepass"] = {
    switch = function ()
		if bufcnt(player.kickBall("Assister") or
				  player.toBallDist("Assister") > 20, 1, 180) then
			return "fixgoto"
		elseif bufcnt(true, 120) then
			return "exit"
		end
	end,
	Assister = task.chipPass(CHIP_POS, 250),
	Leader   = task.goCmuRush(SYNT_CORNER_POS1, _, _, flag.allow_dss),
	Special  = task.goCmuRush(SYNT_CORNER_POS2, _, _, flag.allow_dss),
	Middle   = task.goCmuRush(ANTI_CORNER_POS, _, _, flag.allow_dss),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALMSD}"
},

["fixgoto"] = {
    switch = function ()
		if bufcnt(true, 90) then
			return "sidekick"
		end
	end,
	Leader   = task.goCmuRush(SYNT_CORNER_POS1, _, _, flag.allow_dss),
	Special  = task.goCmuRush(SYNT_CORNER_POS2, _, _, flag.allow_dss),
	Middle   = task.goCmuRush(ANTI_CORNER_POS, _, _, flag.allow_dss),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{MLS}(AD)"
},

["sidekick"] = {
    switch = function ()
		if bufcnt(player.kickBall("Middle"), 1, 120) then
			return "exit"
		end
	end,
	Leader   = task.goCmuRush(SYNT_CORNER_POS1, _, _, flag.allow_dss),
	Special  = task.goCmuRush(SYNT_CORNER_POS2, _, _, flag.allow_dss),
	Middle   = task.waitTouch(ANTI_CORNER_POS, 0),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{MLS}(AD)"
},

name = "Ref_FrontKickV5",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}