---针对Skuba
local AtrPos1  = ball.refSyntYPos(CGeoPoint:new_local(175,85))    --拉人位置一
local AtrPos2  = ball.refSyntYPos(CGeoPoint:new_local(167,64))    --拉人位置二
local RUSHPOS1 = ball.refSyntYPos(CGeoPoint:new_local(215,63))
local RUSHPOS2 = ball.refSyntYPos(CGeoPoint:new_local(206,36))

local KICKPOS = ball.refAntiYPos(CGeoPoint:new_local(0,150))

local AFTER_PASS_POS_1_START = ball.refSyntYPos(CGeoPoint:new_local(275,135))
local AFTER_PASS_POS_1_END 	 = ball.refSyntYPos(CGeoPoint:new_local(165,65))

local AFTER_PASS_POS_2_START = ball.refSyntYPos(CGeoPoint:new_local(105,-10))
local AFTER_PASS_POS_2_END 	 = ball.refSyntYPos(CGeoPoint:new_local(105,150))

gPlayTable.CreatePlay{

	firstState = "Start",

	["Start"] = {
		switch = function ()
			if  bufcnt(player.toTargetDist("Middle") < 50 , "normal") then
				return "GotoEmptyPos"
			end
		end,
		Assister = task.staticGetBall(CGeoPoint:new_local(0,150)),
		Leader   = task.goCmuRush(AtrPos1),
		Special  = task.goCmuRush(AtrPos2),
		Middle   = task.goCmuRush(KICKPOS,player.toBallDir),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{A}[LMDS]"
	},

	["GotoEmptyPos"] = {
		switch = function ()
			if  bufcnt(player.toTargetDist("Middle") < 10 , 120,120) then
				return "FirstChipPass"
			end
		end,
		Assister = task.slowGetBall(CGeoPoint:new_local(0,150)),
		Leader   = task.goCmuRush(AtrPos1),
		Special  = task.goCmuRush(AtrPos2),
		Middle   = task.goCmuRush(KICKPOS,player.toBallDir),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{A}[MLDS]"
	},

	["FirstChipPass"] = {
		switch =function (  )
			if  bufcnt(player.kickBall("Assister") or player.toBallDist("Assister") > 20 , "fast") then
				return "FixGoto"
			elseif bufcnt(true, 120) then
				return "exit"
			end
		end,
		Assister = task.chipPass(CGeoPoint:new_local(0, 150), 9999),
		Leader   = task.goCmuRush(RUSHPOS1),
		Special  = task.goCmuRush(RUSHPOS2),
		Middle   = task.goCmuRush(KICKPOS,player.toBallDir),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{MALSD}"
	},
	["FixGoto"] = {
		switch = function (  )
			if  bufcnt(true, 40) then
				return "Kick"
			end
		end,
		Leader   = task.touchBetweenPos(AFTER_PASS_POS_1_START,AFTER_PASS_POS_1_END),
		Special  = task.touchBetweenPos(AFTER_PASS_POS_2_START,AFTER_PASS_POS_2_END),
		Middle   = task.goCmuRush(KICKPOS, player.toBallDir),
		Assister = task.leftBack(),
		Defender = task.rightBack(),
		Goalie   = task.goalie(),
		match    = "{MLS}[AD]"
	},
	["Kick"] = {
		switch = function ()
			if  bufcnt(player.kickBall("Middle"), "fast", 100) then
				return "exit"
			end
		end,
		Leader   = task.goSupportPos("Middle"),
		Special  = task.touchBetweenPos(AFTER_PASS_POS_2_START,AFTER_PASS_POS_2_END),
		Middle   = task.InterNew(),
		Assister = task.leftBack(),
		Defender = task.rightBack(),
		Goalie   = task.goalie(),
		match    = "{MLS}[AD]"
	},

	name 	   	= "Ref_BackKickV1",
	applicable 	= {
		exp = "a",
		a   = true
	},
	attribute 	= "attack",
	timeout 	= 99999
}
