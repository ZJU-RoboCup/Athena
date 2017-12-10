local FakePos    = ball.refAntiYPos(CGeoPoint:new_local(450,300))
local AssisPos   = ball.refAntiYPos(CGeoPoint:new_local(330,-270))
local ShootPos   = ball.refAntiYPos(CGeoPoint:new_local(330,145))
local SupportPos = ball.refAntiYPos(CGeoPoint:new_local(330,-25))

local WaitPos  = {
	ball.refAntiYPos(CGeoPoint:new_local(25, 0)),
	ball.refAntiYPos(CGeoPoint:new_local(25, 180)),
	ball.refAntiYPos(CGeoPoint:new_local(25, -180))
}

local PushPos  = {
	ball.refAntiYPos(CGeoPoint:new_local(50, 80)),
	ball.refAntiYPos(CGeoPoint:new_local(50, 130))
}

local PushPos2  = {
	ball.refAntiYPos(CGeoPoint:new_local(150, 50)),
	ball.refAntiYPos(CGeoPoint:new_local(150, 0))
}
gPlayTable.CreatePlay{

	firstState = "Start",

	["Start"] = {
		switch = function ()
			if  bufcnt(player.toTargetDist("Special") < 20 and player.toTargetDist("Middle") < 20 , "normal") then
				return "Push"
			end
		end,
		Assister = task.staticGetBall(),--task.staticGetBall(FakePos),
		Leader   = task.goCmuRush(WaitPos[1], _, _, flag.allow_dss),
		Special  = task.goCmuRush(WaitPos[2], _, _, flag.allow_dss),
		Middle   = task.goCmuRush(WaitPos[3], _, _, flag.allow_dss),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{A}{LSMD}"
	},

	["Push"] = {
		switch = function()
			if  bufcnt(player.toTargetDist("Middle") < 60 , "fast") then
				return "FistPass"
			elseif  bufcnt(true,120) then
			end
		end,
		Assister = task.staticGetBall(),
		Leader   = task.goCmuRush(PushPos[1], _, _, flag.allow_dss),
		Special  = task.goCmuRush(PushPos[2], _, _, flag.allow_dss),
		Middle   = task.goCmuRush(WaitPos[3], _, _, flag.allow_dss),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match  	 = "{A}{LSMD}"
	},

	["FistPass"] = {
		switch = function()
			if bufcnt(player.kickBall("Assister"),1 ,60) then
				return "SecondPass"
			end
		end,
		Assister = task.goAndTurnKick(AssisPos,700),
		Leader   = task.goCmuRush(PushPos2[1], _, _, flag.allow_dss),
		Special  = task.goCmuRush(PushPos2[2], _, _, flag.allow_dss),
		Middle   = task.goCmuRush(AssisPos),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match  	 = "{A}{LSMD}"
	},

	["SecondPass"] = {
		switch = function()
			if bufcnt(player.kickBall("Middle"),1) then
				return "SecondPass"
			elseif bufcnt(true,60) then
				return "exit"
			end
		end,
		Assister = task.rightBack(),
		Leader   = task.goCmuRush(ShootPos),
		Special  = task.goCmuRush(SupportPos, _, _, flag.allow_dss),
		Middle   = task.receiveChip(ShootPos,200,true),
		Defender = task.leftBack(),
		Goalie   = task.goalie(),
		match  	 = "{A}{LSMD}"
	},

	["fix"] = {
		switch = function()
			if bufcnt(true,30) then
				return "Shoot"
			end
		end,
		Assister = task.rightBack(),
		Leader   = task.goCmuRush(ShootPos),
		Special  = task.goCmuRush(SupportPos, _, _, flag.allow_dss),
		Middle   = task.stop(),
		Defender = task.leftBack(),
		Goalie   = task.goalie(),
		match  	 = "{A}{LSMD}"
	},

	["Shoot"] = {
		switch = function()
			if bufcnt(player.kickBall("Leader"),1,60) then
				return "exit"
			end
		end,
		Assister = task.rightBack(),
		Leader   = task.waitTouchNew(),
		Special  = task.task.goSupportPos("Leader"),
		Middle   = task.defendMiddle(),
		Defender = task.leftBack(),
		Goalie   = task.goalie(),
		match  	 = "{A}{LSMD}"
	},



	name 	   	= "Test_BackKickV233",
	applicable 	= {
		exp = "a",
		a   = true
	},
	attribute 	= "attack",
	timeout 	= 99999
}
