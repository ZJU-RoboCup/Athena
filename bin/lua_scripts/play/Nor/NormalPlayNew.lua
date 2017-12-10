local isUsePass = false
local gPassPos=CGeoPoint:new_local(100,100)
local gGuisePos=CGeoPoint:new_local(100,-100)
local gKickDir=CGeoPoint:new_local(ball.toTheirGoalDir(),0)


local function updateOurBallAction()
		if bufcnt(cond.canShootOnBallPos("Leader"),5) then
			return "Shoot"
		elseif bufcnt(cond.canPassOnBallPos("Leader",gPassPos,gGuisePos),5) then
			return "Pass"
		elseif bufcnt(cond.canKickAtEnemy("Leader",gKickDir),5) then
			return "KickAtEnemy"
		else
			return "None"
		end
end


local function passJumpCond(fieldArea)
	if  bufcnt(player.kickBall("Leader"), "fast",150) then
		gRoleNum["Leader"]=gRoleNum["Special"]
		return fieldArea.."ReceiveShoot"
	end

	if  bufcnt(player.isBallPassedNormalPlay("Leader","Special") and ball.valid(), 5,150) then
		gRoleNum["Leader"]=gRoleNum["Special"]
		return fieldArea.."ReceiveShoot"
	end

	if gCurrentBallStatus ~= "OurBall" then
		return gCurrentFieldArea..gCurrentBallStatus
	end	
end

local function receiveShootJumpCond()
	if  bufcnt(player.kickBall("Leader"), "fast",150) then
		return gCurrentFieldArea..gCurrentBallStatus
	end
	if  bufcnt(player.passIntercept("Leader"),8) then
		return gCurrentFieldArea..gCurrentBallStatus
	end
end

local function getPassPos()
	return gPassPos
end

local function getGuisePos()
	return gGuisePos
end

local function getKickDir()
	return gKickDir:x()
end


gPlayTable.CreatePlay{
firstState = "initState",

["initState"] = {
	switch = function ()
		return gCurrentFieldArea..gCurrentBallStatus
	end,
	Leader   = task.advance(),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},


["FrontFieldNone"]={
	switch=function ()
		if gCurrentFieldArea..gCurrentBallStatus ~= gLastState then
			return gCurrentFieldArea..gCurrentBallStatus
		end
	end,
	Leader   = task.advance(),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.goSecondPassPos("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},



["FrontFieldStandOff"]={
	switch=function ()
		if cond.bestPlayerChanged() or gCurrentBallStatus ~= "StandOff" then
			return gCurrentFieldArea..gCurrentBallStatus
		end
	end,
	Leader   = task.advance("Middle"),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.tandem(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},


["FrontFieldWaitAdvance"]={
	switch=function ()
		if gCurrentFieldArea..gCurrentBallStatus ~= gLastState then
			return gCurrentFieldArea..gCurrentBallStatus
		end
	end,
	Leader   = task.advance(),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},


["FrontFieldGiveUpAdvance"]={
	switch=function ()
		if  gCurrentFieldArea..gCurrentBallStatus ~= gLastState then
			return gCurrentFieldArea..gCurrentBallStatus
		end
	end,
	Leader   = task.protectBall(),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},



["FrontFieldOurBall"]={
	switch=function ()
		if gCurrentFieldArea..gCurrentBallStatus ~= gLastState  then
			return gCurrentFieldArea..gCurrentBallStatus
		else
			local state=updateOurBallAction()
			if state ~= "None" then
				return gCurrentFieldArea..state
			end
		end
	end,
	Leader   = task.advance(),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.goSecondPassPos("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][SM]"

},

["FrontFieldShoot"]={
	switch=function ()
		if bufcnt(player.kickBall("Leader"), "fast",150) then
			return gCurrentFieldArea..gCurrentBallStatus
		end
		if gCurrentBallStatus ~= "OurBall" then
			return gCurrentFieldArea..gCurrentBallStatus
		end
		if gCurrentFieldArea ~=gLastFieldArea then
			return gCurrentFieldArea.."Shoot"
		end
	end,
	Leader   = task.shoot("safe"),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.goSecondPassPos("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][SM]"
},


["FrontFieldPass"]={
	switch=function ()
		return passJumpCond("FrontField")
	end,
	Leader   = task.pass("Special"),
	Special  = task.goCmuRush(getPassPos,player.toShootOrRobot("Leader")),
	Middle   = task.goCmuRush(getGuisePos),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][S][M]"
},

["FrontFieldReceiveShoot"]={
	switch=function ()
		return receiveShootJumpCond()
	end,
	Leader   = task.shoot("safe"),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.goSecondPassPos("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LADM}[S]"
},


["FrontFieldKickAtEnemy"]={
	switch=function ()
		if  bufcnt(player.kickBall("Leader"), "fast",80) then
			return gCurrentFieldArea..gCurrentBallStatus
		end
		if gCurrentBallStatus ~= "OurBall" then
			return gCurrentFieldArea..gCurrentBallStatus
		end
		if gCurrentFieldArea ~=gLastFieldArea then
			return gCurrentFieldArea.."KickAtEnemy"
		end
	end,
	Leader   = task.shoot("safe"),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"

},


["MiddleFieldNone"]={
	switch=function ()
		if gCurrentFieldArea..gCurrentBallStatus ~= gLastState  then
			return gCurrentFieldArea..gCurrentBallStatus
		end
	end,
	Leader   = task.advance(),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},



["MiddleFieldStandOff"]={
	switch=function ()
		if cond.bestPlayerChanged() or gCurrentBallStatus ~= "StandOff" then
			return gCurrentFieldArea..gCurrentBallStatus
		end
	end,
	Leader   = task.advance("Middle"),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.tandem(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},


["MiddleFieldWaitAdvance"]={
	switch=function ()
		if gCurrentFieldArea..gCurrentBallStatus ~= gLastState then
			return gCurrentFieldArea..gCurrentBallStatus
		end
	end,
	Leader   = task.advance(),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},


["MiddleFieldGiveUpAdvance"]={
	switch=function ()
		if  gCurrentFieldArea..gCurrentBallStatus ~= gLastState then
			return gCurrentFieldArea..gCurrentBallStatus
		end
	end,
	Leader   = task.protectBall(),
	Special  = task.defendMiddle("Leader"),
	Middle   = task.goFirstPassPos("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADM][S]"
},


["MiddleFieldOurBall"]={
	switch=function ()
		if gCurrentFieldArea..gCurrentBallStatus ~= gLastState  then
			return gCurrentFieldArea..gCurrentBallStatus
		else
			local state=updateOurBallAction()
			if state ~= "None" then
				return gCurrentFieldArea..state
			end
		end
	end,
	Leader   = task.advance(),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.goSecondPassPos("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"

},

["MiddleFieldShoot"]={
	switch=function ()
		if  bufcnt(player.kickBall("Leader"), "fast",180) then
			return gCurrentFieldArea..gCurrentBallStatus
		end
		if gCurrentBallStatus ~= "OurBall" then
			return gCurrentFieldArea..gCurrentBallStatus
		end
		if gCurrentFieldArea ~=gLastFieldArea then
			return gCurrentFieldArea.."Shoot"
		end

	end,
	Leader   = task.shoot("safe"),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.goSecondPassPos("Leader"),
	Defender = task.leftBack("Leader"),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},


["MiddleFieldPass"]={
	switch=function ()
		return passJumpCond("MiddleField")
	end,
	Leader   = task.pass("Special"),
	Special  = task.goCmuRush(getPassPos,player.toShootOrRobot("Leader")),
	Middle   = task.goCmuRush(getGuisePos),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][S][M]"
},

["MiddleFieldReceiveShoot"]={
	switch=function ()
		return receiveShootJumpCond()
	end,
	Leader   = task.shoot("safe"),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.goSecondPassPos("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LADM}[S]"
},


["MiddleFieldKickAtEnemy"]={
	switch=function ()
		if  bufcnt(player.kickBall("Leader"), "fast",80) then
			return gCurrentFieldArea..gCurrentBallStatus
		end
		if gCurrentBallStatus ~= "OurBall" then
			return gCurrentFieldArea..gCurrentBallStatus
		end
		if gCurrentFieldArea ~=gLastFieldArea then
			return gCurrentFieldArea.."KickAtEnemy"
		end
	end,
	Leader   = task.shoot("safe"),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"

},


["BackFieldNone"]={
	switch=function ()
		if gCurrentFieldArea..gCurrentBallStatus ~= gLastState  then
			return gCurrentFieldArea..gCurrentBallStatus
		end
	end,
	Leader   = task.advance(),
	Special  = task.tandem(),
	Middle   = task.goFirstPassPos("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},



["BackFieldStandOff"]={
	switch=function ()
		if cond.bestPlayerChanged() or gCurrentBallStatus ~= "StandOff" then
			return gCurrentFieldArea..gCurrentBallStatus
		end
	end,
	Leader   = task.advance(),
	Special  = task.tandem(),
	Middle   = task.goFirstPassPos("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADM][S]"
},


["BackFieldWaitAdvance"]={
	switch=function ()
		if gCurrentFieldArea..gCurrentBallStatus ~= gLastState then
			return gCurrentFieldArea..gCurrentBallStatus
		end
	end,
	Leader   = task.advance(),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.goSecondPassPos("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},


["BackFieldGiveUpAdvance"]={
	switch=function ()
		if  gCurrentFieldArea..gCurrentBallStatus ~= gLastState then
			return gCurrentFieldArea..gCurrentBallStatus
		end
	end,
	Leader   = task.protectBall(),
	Special  = task.tandem(),
	Middle   = task.goFirstPassPos("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},


["BackFieldOurBall"]={
	switch=function ()
		if gCurrentFieldArea..gCurrentBallStatus ~= gLastState  then
			return gCurrentFieldArea..gCurrentBallStatus
		else
			local state=updateOurBallAction()
			if state ~= "None" then
				return gCurrentFieldArea..state
			end
		end
	end,
	Leader   = task.shoot("safe"),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.tandem(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"

},

["BackFieldShoot"]={
	switch=function ()
		if  bufcnt(player.kickBall("Leader"), "fast",180) then
			return "finish"
		end
		if gCurrentBallStatus ~= "OurBall"  then
			return gCurrentFieldArea..gCurrentBallStatus
		end
		if gCurrentFieldArea ~=gLastFieldArea then
			return gCurrentFieldArea.."Shoot"
		end
	end,
	Leader   = task.shoot("safe"),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.tandem(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},


["BackFieldPass"]={
	switch=function ()
		return passJumpCond("BackField")
	end,
	Leader   = task.passToPos(getPassPos),
	Special  = task.goCmuRush(getPassPos,player.toShootOrRobot("Leader")),
	Middle   = task.goCmuRush(getGuisePos),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][S][M]"
},

["BackFieldReceiveShoot"]={
	switch=function ()
		return receiveShootJumpCond()
	end,
	Leader   = task.shoot("safe"),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LADM}[S]"
},


["BackFieldKickAtEnemy"]={
	switch=function ()
	--print(getKickDir())
		if  bufcnt(player.kickBall("Leader"), "fast",80) then
			return gCurrentFieldArea..gCurrentBallStatus
		end
		if gCurrentFieldArea ~=gLastFieldArea then
			return gCurrentFieldArea.."KickAtEnemy"
		end
	end,
	Leader   = task.advance(),
	Special  = task.goFirstPassPos("Leader"),
	Middle   = task.tandem(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"

},

name = "NormalPlayNew",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}