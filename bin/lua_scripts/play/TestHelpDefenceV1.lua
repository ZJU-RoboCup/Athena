---模拟RoboDragon定位球

AtrPos_Leader_1 = ball.refSyntYPos(CGeoPoint:new_local(-59,-99))
AtrPos_Leader_2 = ball.refSyntYPos(CGeoPoint:new_local(0,-73))
AtrPos_Leader_3 = ball.refSyntYPos(CGeoPoint:new_local(153,-106))
AtrPos_Leader_4 = ball.refSyntYPos(CGeoPoint:new_local(180,-106))

AtrPos_Special_1 = ball.refSyntYPos(CGeoPoint:new_local(88,-132)) 
AtrPos_Special_2 = ball.refSyntYPos(CGeoPoint:new_local(0,-130)) 
AtrPos_Special_3 = ball.refSyntYPos(CGeoPoint:new_local(-33,-68))
AtrPos_Special_4 = ball.refSyntYPos(CGeoPoint:new_local(120,-106))

AtrPos3_Middle_1  = ball.refSyntYPos(CGeoPoint:new_local(187,124))
AtrPos3_Middle_2  = ball.refSyntYPos(CGeoPoint:new_local(-10,124))
ShootBallPos_Assister = ball.refSyntYPos(CGeoPoint:new_local(0,50))

local ChipDir = function ( p )  --第一次挑传传球方向
	return function (  )
		return (ball.refAntiYPos(p)() - ball.pos()):dir()
	end
end

gPlayTable.CreatePlay{
	
	firstState = "Start",

	["Start"] = {
		switch = function ()
			if  bufcnt(player.toTargetDist("Leader") < 30 and player.toTargetDist("Special") < 30
						and player.toTargetDist("Middle") < 30 , 40 ,120) then	
				return "ChangePos_1"
			end
		end,
		Assister   = task.staticGetBall(ChipDir(CGeoPoint:new_local(180,106))),
		Leader = task.goCmuRush(AtrPos_Leader_1),
		Special  = task.goCmuRush(AtrPos_Special_1),
		Middle  = task.goCmuRush(AtrPos3_Middle_1),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "[ALMDS]"
	},
	["ChangePos_1"] = {
		switch = function ()
			if  bufcnt(player.toTargetDist("Leader") < 50 and player.toTargetDist("Special") < 50 ,1,100) then
				return "ChangePos_2" 
			end
		end,
		Assister   = task.staticGetBall(ChipDir(CGeoPoint:new_local(180,106))),
		Leader = task.goCmuRush(AtrPos_Leader_2),
		Special  = task.goCmuRush(AtrPos_Special_2),
		Middle  = task.goCmuRush(AtrPos3_Middle_1),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{MALDS}"
	},

	["ChangePos_2"] = {
		switch =function (  )

			if  bufcnt(player.toTargetDist("Leader") < 200 , 1,100) then
				return "FirstPass"
			end
		end,
		Assister   = task.staticGetBall(ChipDir(CGeoPoint:new_local(180,106))),
		Leader = task.goCmuRush(AtrPos_Leader_3,player.toBallDir),
		Special  = task.goCmuRush(AtrPos_Special_3,player.toBallDir),
		Middle  = task.goCmuRush(AtrPos3_Middle_1,player.toPointDir(CGeoPoint:new_local(300,0))),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{MALDS}"
	},
	["FirstPass"] = {
		switch =function (  )

			if  bufcnt(player.kickBall("Assister") or player.toBallDist("Assister") > 20,"fast") then
				return "FixGo_1"
			end
		end,
		Assister   = task.chipPass(ChipDir(CGeoPoint:new_local(180,106)),150),
		Leader = task.goCmuRush(AtrPos_Leader_3,player.toBallDir),
		Special  = task.goCmuRush(AtrPos_Special_3,player.toBallDir),
		Middle  = task.goCmuRush(AtrPos3_Middle_1,player.toPointDir(CGeoPoint:new_local(300,0))),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{MALDS}"
	},
	["FixGo_1"] = {
		switch = function() 
			if bufcnt(true, 10) then
			 	return "SecondPass"
			 end 
		end,
		Assister   = task.goCmuRush(ShootBallPos_Assister),
		Leader = task.goCmuRush(AtrPos_Leader_3,player.toBallDir),
		Special  = task.goCmuRush(AtrPos_Special_4,player.toBallDir),
		Middle  = task.goCmuRush(AtrPos3_Middle_1,player.toPointDir(CGeoPoint:new_local(300,0))),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{MALDS}"
	},
	["SecondPass"] = {
		switch = function (  )
			if  bufcnt(player.kickBall("Leader"), "fast") then
				return "FixGo_2"
			elseif bufcnt(true, 100) then
				return "exit"
			end	
		end,
		Assister   = task.goCmuRush(ShootBallPos_Assister),
		Leader = task.touchPass(ShootBallPos_Assister),
		Special  = task.goCmuRush(AtrPos_Special_4,player.toBallDir),
		Middle  = task.goCmuRush(AtrPos3_Middle_2,player.toPointDir(CGeoPoint:new_local(300,0))),
		--Middle = task.leftBack(),
		Defender = task.rightBack(),
		Goalie   = task.goalie(),
		match    = "{MALDS}"
	},
	["FixGo_2"] = {
		switch = function() 
			if  bufcnt(true, 10) then
				return "FinalShoot"
			end
		end,
		Assister   = task.goCmuRush(ShootBallPos_Assister),
		Leader = task.goCmuRush(AtrPos_Leader_4),
		Special  = task.goCmuRush(AtrPos_Special_4,player.toBallDir),
		--Middle  = task.goCmuRush(AtrPos3_Middle_2,player.toPointDir(CGeoPoint:new_local(300,0))),
		Middle = task.leftBack(),
		Defender = task.rightBack(),
		Goalie   = task.goalie(),
		match    = "{MALDS}"
	},
	["FinalShoot"] = {
		switch = function() 
			if  bufcnt(player.kickBall("Assister"), "fast",120) then
			 	return "exit"
			 end 
		end,
		Assister   = task.shootV2(),
		Leader = task.goCmuRush(AtrPos_Leader_4),
		Special  = task.goCmuRush(AtrPos_Special_4,player.toBallDir),
		--Middle  = task.goCmuRush(AtrPos3_Middle_2,player.toPointDir(CGeoPoint:new_local(300,0))),
		Middle = task.leftBack(),
		Defender = task.rightBack(),
		Goalie   = task.goalie(),
		match    = "{MALDS}"
	},

	name 	   	= "TestHelpDefenceV1",
	applicable 	={
		exp = "a",
		a   = true
	},
	attribute 	= "attack",
	timeout 	= 99999
}
