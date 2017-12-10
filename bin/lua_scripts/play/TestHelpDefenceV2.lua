---模拟RoboDragon定位球

AtrPos_Leader_1 = ball.refAntiYPos(CGeoPoint:new_local(67,-57))
AtrPos_Leader_2 = ball.refAntiYPos(CGeoPoint:new_local(85,98))
AtrPos_Leader_3 = ball.refAntiYPos(CGeoPoint:new_local(199,115))
--AtrPos_Leader_4 = ball.refSyntYPos(CGeoPoint:new_local(180,-106))

local ChipDir = function ( p )  --第一次挑传传球方向
	return function (  )
		return (ball.refAntiYPos(p)() - ball.pos()):dir()
	end
end

AtrPos_Special_1 = ball.refAntiYPos(CGeoPoint:new_local(136,113)) 
AtrPos_Special_2 = ball.refAntiYPos(CGeoPoint:new_local(123,33)) 
AtrPos_Special_3 = ball.refAntiYPos(CGeoPoint:new_local(92,-36)) 
AtrPos_Special_4 = ball.refAntiYPos(CGeoPoint:new_local(93,37)) 



AtrPos3_Middle  = ball.refAntiYPos(CGeoPoint:new_local(18,121))




gPlayTable.CreatePlay{
	
	firstState = "Start",

	["Start"] = {
		switch = function ()
			if  bufcnt(player.toTargetDist("Leader") < 30 and player.toTargetDist("Special") < 30
						and player.toTargetDist("Middle") < 30 , 40 ,120) then	
				return "ChangePos_1"
			end
		end,
		Assister   = task.staticGetBall(ChipDir(CGeoPoint:new_local(199,-115))),
		Leader = task.goCmuRush(AtrPos_Leader_1,player.toPointDir(CGeoPoint:new_local(300,30))),
		Special  = task.goCmuRush(AtrPos_Special_1),
		Middle  = task.goCmuRush(AtrPos3_Middle),
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
		Assister   = task.staticGetBall(ChipDir(CGeoPoint:new_local(199,-115))),
		Leader = task.goCmuRush(AtrPos_Leader_2,player.toPointDir(CGeoPoint:new_local(300,30))),
		Special  = task.goCmuRush(AtrPos_Special_2),
		Middle  = task.goCmuRush(AtrPos3_Middle),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{MALDS}"
	},

	["ChangePos_2"] = {
		switch =function (  )

			-- if  bufcnt(player.toTargetDist("Leader") < 50 , 1,100) then
			-- 	return "FixGo"
			-- end
			if  bufcnt(player.kickBall("Assister") or player.toBallDist("Assister") > 20,"fast") then
				return "FixGo"
			end
		end,
		--Assister   = task.staticGetBall(CGeoPoint:new_local(262,-178)),
		--Assister   = task.goAndTurnKick(AtrPos_Leader_3),
		Assister = task.chipPass(ChipDir(CGeoPoint:new_local(199,115)),150),
		Leader = task.goCmuRush(AtrPos_Leader_3,player.toPointDir(CGeoPoint:new_local(300,30))),
		Special  = task.goCmuRush(AtrPos_Special_3,player.toBallDir),
		Middle  = task.goCmuRush(AtrPos3_Middle,player.toPointDir(CGeoPoint:new_local(300,30))),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{MALDS}"
	},
	-- ["FirstPass"] = {
	-- 	switch =function (  )

	-- 		if  bufcnt(player.kickBall("Assister") or player.toBallDist("Assister") > 20,"fast") then
	-- 			return "FixGo"
	-- 		end
	-- 	end,
	-- 	--Assister   = task.goAndTurnKick(AtrPos_Leader_3),
	-- 	Assister = task.stop(),
	-- 	Leader = task.goCmuRush(AtrPos_Leader_3,player.toBallDir),
	-- 	Special  = task.goCmuRush(AtrPos_Special_4,player.toBallDir),
	-- 	Middle  = task.goCmuRush(AtrPos3_Middle,player.toPointDir(CGeoPoint:new_local(300,0))),
	-- 	--Middle = task.leftBack(),
	-- 	Defender = task.singleBack(),
	-- 	Goalie   = task.goalie(),
	-- 	match    = "{MALDS}"
	-- },
	["FixGo"] = {
		switch = function() 
			if bufcnt(true, 30) then
			 	return "FinalShoot"
			 end 
		end,
		--Assister   = task.goCmuRush(ShootBallPos_Assister),
		Assister = task.stop(),
		Leader = task.goCmuRush(AtrPos_Leader_3,player.toPointDir(CGeoPoint:new_local(300,30))),
		Special  = task.goCmuRush(AtrPos_Special_4,player.toBallDir),
		Middle  = task.leftBack(),
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
		Assister = task.stop(),
		Leader = task.shootV2(),
		Special  = task.goCmuRush(AtrPos_Special_4,player.toBallDir),
		Middle  = task.leftBack(),
		Defender = task.rightBack(),
		Goalie   = task.goalie(),
		match    = "{MALDS}"
	},

	name 	   	= "TestHelpDefenceV2",
	applicable 	={
		exp = "a",
		a   = true
	},
	attribute 	= "attack",
	timeout 	= 99999
}
