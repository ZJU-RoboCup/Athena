local CHIPBALLPOS = ball.refSyntYPos(CGeoPoint:new_local(0,172))
local AtrPos = ball.refAntiYPos(CGeoPoint:new_local(190,172))
local GetBallPos = ball.refSyntYPos(CGeoPoint:new_local(150,29))
local ShootBallPos = ball.refSyntYPos(CGeoPoint:new_local(0,180))
local BlockPos = ball.refAntiYPos(CGeoPoint:new_local(200,170))

gPlayTable.CreatePlay{

	firstState = "Start",

	["Start"] = {
		switch = function()
			if  bufcnt(player.toTargetDist("Leader") < 30 and player.InfoControlled("Assister"), 40,120) then
				return "ChipBall"
			end
		end,
		Assister = task.slowGetBall("Leader"),
		Leader   = task.goCmuRush(GetBallPos,player.toPointDir(ShootBallPos)),
		Special  = task.goCmuRush(ShootBallPos),
		Middle   = task.goCmuRush(AtrPos),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
		match    = "{A}[LSDM]"
	},
    ["ChipBall"] = {
    	switch = function()
    		if  bufcnt(player.kickBall("Assister") or player.toBallDist("Assister") > 20, "fast") then
    			return "fixGo"
            elseif  bufcnt(true, 100) then
                return "exit"
    		end
    	end,
    	Assister = task.chipPass("Leader",350),
    	Leader	 = task.goCmuRush(GetBallPos,player.toPointDir(ShootBallPos)),
    	Special  = task.goCmuRush(ShootBallPos),
    	Middle   = task.goCmuRush(AtrPos),
		Defender = task.singleBack(),
		Goalie   = task.goalie(),
    	match    = "{ALS}[D][M]"
    },
    ["fixGo"] = {
    	switch = function()
    		if  bufcnt(true,20) then
    		 	return "SecondPass"
    		 end
    	end,

    	Assister = task.leftBack(),
    	Leader = task.goCmuRush(GetBallPos,player.toPointDir(ShootBallPos)),
    	Special  = task.goCmuRush(ShootBallPos),
    	Middle   = task.goCmuRush(AtrPos),
		Defender = task.rightBack(),
		Goalie   = task.goalie(),
    	match    = "{LS}[AD]{M}"
    },
    ["SecondPass"] = {
    	switch = function()
    		if  bufcnt(player.kickBall("Leader") or player.isBallPassed("Leader","Special"), "fast") then
    		 	return "Shoot"
            elseif  bufcnt(true, 120) then
                return "exit"
    		end
    	end,
    	Assister = task.leftBack(),
    	-- Leader   = task.touchPass(ShootBallPos),
        Leader = task.receivePass("Special"),
    	Special  = task.goCmuRush(ShootBallPos),
    	Middle   = task.goCmuRush(AtrPos),
		Defender = task.rightBack(),
		Goalie   = task.goalie(),
    	match    = "{LS}[AD]{M}"
    },
    ["Shoot"] = {
    	switch = function()
    		if  bufcnt(player.kickBall("Special"), "fast",70) then
				return "exit"
			end
    	end,
    	Leader   = task.goCmuRush(BlockPos),
    	Assister = task.leftBack(),
    	Special  = task.shootV2(),
    	--Middle   = task.goCmuRush(AtrPos),
        Middle = task.defendMiddle(),
		Defender = task.rightBack(),
		Goalie   = task.goalie(),
    	match    = "{S}[AD]{M}"
    },
	name 	   	= "Ref_BackKickV11",
	applicable 	={
		exp = "a",
		a   = true
	},
	attribute 	= "attack",
	timeout 	= 99999
}
