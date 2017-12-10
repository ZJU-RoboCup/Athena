-- 2012年进SKUBA战术的加强版，前面跑位一模一样，后面的冲刺换成后卫前冲
-- by zhyaic 2013.6.16
-- 2014-07-20 yys 改

local FINAL_SHOOT_POS = CGeoPoint:new_local(360,90)
local TMP_POS   = ball.refAntiYPos(CGeoPoint:new_local(120,0))
local SYNT_POS  = ball.refSyntYPos(CGeoPoint:new_local(120,150))
local ANTIPOS_1 = ball.refAntiYPos(CGeoPoint:new_local(20,120))
local ANTIPOS_2 = ball.refAntiYPos(CGeoPoint:new_local(250,120))
local ANTIPOS_3 = ball.refAntiYPos(FINAL_SHOOT_POS)
local CHIP_POS  = pos.passForTouch(FINAL_SHOOT_POS)

local ANTI_FAKE_POS = ball.refAntiYPos(CGeoPoint:new_local(370,230))

gPlayTable.CreatePlay{

firstState = "start",

["start"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Special") < 60 and
		   player.toTargetDist("Leader") < 60,  "normal", 180) then
			return "goalone"
		end
	end,
	Assister = task.staticGetBall(CHIP_POS),
	Special  = task.goCmuRush(TMP_POS,_,_,flag.allow_dss),
	Leader   = task.goCmuRush(SYNT_POS,_,_,flag.allow_dss),
	Middle   = task.goCmuRush(CGeoPoint:new_local(0,0),_,_,flag.allow_dss),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{A}{DLSM}"
},

["goalone"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Defender") < 60, "fast", 180) then
			return "goget"
		end
	end,
	Assister = task.slowGetBall(CHIP_POS),
	Defender = task.goCmuRush(ANTIPOS_1),
	Special  = task.goCmuRush(SYNT_POS,_,_,flag.allow_dss),
	Middle   = task.goCmuRush(ANTI_FAKE_POS,_,_,flag.allow_dss),
	Leader   = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{DALSM}"
},

["goget"] = {
    switch = function ()
		if bufcnt(player.toTargetDist("Defender") < 140, "fast", 180) then
			return "gopass"
		end
	end,
	Defender = task.goCmuRush(ANTIPOS_2),
	Assister = task.slowGetBall(CHIP_POS),
	Special  = task.goCmuRush(SYNT_POS,_,_,flag.allow_dss),
	Middle   = task.goCmuRush(ANTI_FAKE_POS,_,_,flag.allow_dss),
	Leader   = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{DALSM}"
},

["gopass"] = {
    switch = function ()
		if bufcnt(player.kickBall("Assister") or
				  player.toBallDist("Assister") > 30,"fast", 180) then
			return "goopen"
		elseif  bufcnt(true, 120) then
			return "exit"
		end
	end,
	Defender = task.goCmuRush(ANTIPOS_3),
	Assister = task.chipPass(CHIP_POS, 180),
	Special  = task.goCmuRush(SYNT_POS,_,_,flag.allow_dss),
	Middle   = task.goCmuRush(ANTI_FAKE_POS,_,_,flag.allow_dss),
	Leader   = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{DALSM}"
},

["goopen"] = {
    switch = function ()
		if bufcnt(true, 40) then
			return "gokick"
		end
	end,
	Defender = task.goCmuRush(ANTIPOS_3),
	Assister = task.goSupportPos("Defender"),
	Special  = task.defendMiddle(),
	Middle   = task.goCmuRush(ANTI_FAKE_POS),
	Leader   = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{DALSM}"
},

["gokick"] = {
    switch = function ()
		if player.kickBall("Defender") then
			return "finish"
		elseif bufcnt(true, 90) then
			return "exit"
		end
	end,
	Defender = task.waitTouch(ANTIPOS_3, 0),
	Assister = task.goSupportPos("Defender"),
	Middle   = task.goCmuRush(ANTI_FAKE_POS),
	Special  = task.leftBack(),
	Leader   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{DM}{A}(LS)"
},

name = "Ref_CornerKickV6",
applicable ={
	exp = "a",
	a = true
},
score = 0,
attribute = "attack",
timeout = 99999
}