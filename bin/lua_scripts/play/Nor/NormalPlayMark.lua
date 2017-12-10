
local isUsePass = true
local gPassPos=CGeoPoint:new_local(100,100)
local gGuisePos=CGeoPoint:new_local(100,-100)


local function haveSuitSider()
	return world:getSuitSider()>0
end


local function updateOurBallAction()
	if bufcnt(not cond.canShootOnBallPos("Leader") and 
			cond.canPassOnBallPos("Leader",gPassPos,gGuisePos) and isUsePass,3) then
		return "Pass"
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


local function frontFieldJumpCond()
	if gCurrentFieldArea..gCurrentBallStatus ~= gLastFieldArea..gLastBallStatus then
		return gCurrentFieldArea..gCurrentBallStatus
	end
	if bufcnt(Utils.InOurPenaltyArea(ball.pos(),5),5) then
		return "NoAdvance"
	end
end

local function backFieldJumpCond()
	if gCurrentFieldArea..gCurrentBallStatus ~= gLastFieldArea..gLastBallStatus then
		return gCurrentFieldArea..gCurrentBallStatus
	end
	if enemy.attackNum() < 3 then
		return "BackFieldNone"..enemy.attackNum()
	end
	if bufcnt(Utils.InOurPenaltyArea(ball.pos(),5),5) then
		return "NoAdvance"
	end
end

gPlayTable.CreatePlay{
firstState = "initState",

["initState"] = {
	switch = function ()
		return gCurrentFieldArea..gCurrentBallStatus
	end,
	Leader   = task.advance(),
	Special  = task.sideBack(),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADMS]"
},


["FrontFieldNone"]={
	switch=function ()
		return frontFieldJumpCond()
	end,
	Leader   = task.advance(),
	Special  = task.goSupportPos("Leader"),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADM][S]"
},



["FrontFieldStandOff"]={
	switch=function ()
		return frontFieldJumpCond()
	end,
	Leader   = task.advance(),
	Special  = task.goSupportPos("Leader"),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADM][S]"
},


["FrontFieldGiveUpAdvance"]={
	switch=function ()
		return frontFieldJumpCond()
	end,
	Leader   = task.protectBall(),
	Special  = task.goSupportPos("Leader"),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADM][S]"
},



["FrontFieldOurBall"]={
	switch=function ()
		if bufcnt(not cond.canShootOnBallPos("Leader") and 
			cond.canPassOnBallPos("Leader",gPassPos,gGuisePos) and isUsePass,3) then
			return "FrontFieldPass"
		end
		return frontFieldJumpCond()
	end,
	Leader   = task.shoot("safe"),
	Special  = task.goSupportPos("Leader"),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADM][S]"

},


["FrontFieldPass"]={
	switch=function ()
		return passJumpCond("FrontField")
	end,
	Leader   = task.passToPos(getPassPos),
	Special  = task.goCmuRush(getPassPos,player.toShootOrRobot("Leader")),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADM][S]"
},

["FrontFieldReceiveShoot"]={
	switch=function ()
		return receiveShootJumpCond()
	end,
	Leader   = task.shoot("safe"),
	Special  = task.goSupportPos("Leader"),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LADSM}"
},


["BackFieldNone"]={
	switch=function ()
		return backFieldJumpCond()
	end,
	Leader   = task.advance(),
	Special  = task.markingNormal("First"),
	Middle   = task.markingNormal("Second"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},

["BackFieldOurBall"]={
	switch=function ()
		if bufcnt(not cond.canShootOnBallPos("Leader") and 
			cond.canPassOnBallPos("Leader",gPassPos,gGuisePos) and isUsePass,3) then
			return "FrontFieldPass"
		end
		return backFieldJumpCond()
	end,
	Leader   = task.shoot("safe"),
	Special  = task.markingNormal("First"),
	Middle   = task.markingNormal("Second"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"

},

["BackFieldStandOff"]={
	switch=function ()
		return backFieldJumpCond()
	end,
	Leader   = task.advance(),
	Special  = task.markingNormal("First"),
	Middle   = task.markingNormal("Second"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},


["BackFieldGiveUpAdvance"]={
	switch=function ()
		return backFieldJumpCond()
	end,
	Leader   = task.protectBall(),
	Special  = task.markingNormal("First"),
	Middle   = task.markingNormal("Second"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},

["BackFieldNone0"]={
	switch=function ()
		if	enemy.attackNum() == 1 then
			return "BackFieldNone1"
		elseif	enemy.attackNum() == 2 then
			return "BackFieldNone2"
		elseif enemy.attackNum() >= 3 then
			return "BackFieldNone"
		end
		if bufcnt(Utils.InOurPenaltyArea(ball.pos(),5),5) then
			return "NoAdvance"
		end
	end,
	Leader   = task.advance(),
	Special  = task.goSecondPassPos("Leader"),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},

["BackFieldNone1"]={
	switch=function ()
		if	enemy.attackNum() == 2 then
			return "BackFieldNone2"
		elseif enemy.attackNum() >= 3 then
			return "BackFieldNone"
		end
		if bufcnt(Utils.InOurPenaltyArea(ball.pos(),5),5) then
			return "NoAdvance"
		end
	end,
	Leader   = task.advance(),
	Special  = task.goSecondPassPos("Leader"),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},

["BackFieldNone2"]={
	switch=function ()
		if	enemy.attackNum() == 1 then
			return "BackFieldNone1"
		elseif enemy.attackNum() >= 3 then
			return "BackFieldNone"
		end
		if bufcnt(Utils.InOurPenaltyArea(ball.pos(),5),5) then
			return "NoAdvance"
		end
	end,
	Leader   = task.advance(),
	Special  = task.markingNormal("First"),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][MS]"
},

["NoAdvance"]={
	switch=function ()
		if  bufcnt(not Utils.InOurPenaltyArea(ball.pos(),15),1) then
			if gRoleNum["Leader"]==bestPlayer:getOurBestPlayer() 
				or gRoleNum["Assister"]==bestPlayer:getOurBestPlayer()
				or gRoleNum["Defender"]==bestPlayer:getOurBestPlayer() 
				or gRoleNum["Special"]==bestPlayer:getOurBestPlayer() then
				return "NoMiddleDef"..gCurrentBallStatus
			else
				return gCurrentFieldArea..gCurrentBallStatus
			end
		end
	end,
	Leader   = task.defendMiddle(),
	Middle   = task.goSecondPassPos("Leader"),
	Special  = task.sideBack(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[ADL][S][M]"
},

name = "NormalPlayMark",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}