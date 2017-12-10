
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

local function jumpNoMiddleDefCond()
	if  bufcnt(ball.posX()<pos.defendMiddlePos():x()+20,5) then
		return "NoMiddleDef"..gCurrentBallStatus
	end
	if gRoleNum["Middle"]==bestPlayer:getOurBestPlayer() 
		or gRoleNum["Assister"]==bestPlayer:getOurBestPlayer()
		or gRoleNum["Defender"]==bestPlayer:getOurBestPlayer() then
			--gRoleNum["Leader"]=gRoleNum["Middle"]
		if gCurrentBallStatus=="OurBall" then
			return "NoMiddleDefOurBallTransit"
		else
			return "NoMiddleDefNoneTransit"
		end
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
	if gCurrentBallStatus ~= gLastBallStatus then
		return "FrontField"..gCurrentBallStatus
	end
	local temp = jumpNoMiddleDefCond()
	if  temp~= nil then
		return temp
	end
	if gCurrentFieldArea ~= "FrontField" and haveSuitSider()then
		if gCurrentBallStatus=="OurBall" then
			return gCurrentFieldArea.."OurBallTransit"
		else
			return gCurrentFieldArea.."NoneTransit"
		end	
	end
	if bufcnt(Utils.InOurPenaltyArea(ball.pos(),5),5) then
		return "NoAdvance"
	end
end

local function backFieldJumpCond()
	if gCurrentFieldArea..gCurrentBallStatus ~= gLastFieldArea..gLastBallStatus then
		return gCurrentFieldArea..gCurrentBallStatus
	end
	if bufcnt(Utils.InOurPenaltyArea(ball.pos(),5),5) then
		return "NoAdvance"
	end
	return jumpNoMiddleDefCond()
end

local function noMiddleDefJumpCond( ... )
	if gCurrentBallStatus ~= gLastBallStatus then
		if string.find(gCurrentState, "Transit") ~= nil then
			if gCurrentBallStatus == "OurBall" then
				return "NoMiddleDef"..gCurrentBallStatus.."Transit"
			else
				return "NoMiddleDef".."None".."Transit"
			end
		else
			return "NoMiddleDef"..gCurrentBallStatus
		end
	end
	if  bufcnt(player.posX("Leader")>-140 * param.lengthRatio and player.toTargetDist("Middle")<20,5)  then
		return gCurrentFieldArea..gCurrentBallStatus
	end
	if  bufcnt( not (math.abs(ball.velDir())>math.pi/1.5) and ball.velMod()>100 and gCurrentFieldArea=="FrontField",3) then
		return gCurrentFieldArea..gCurrentBallStatus
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
	Leader   = task.advance(),--task.waitTouchNew(),--task.advance(),
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
	Leader   = task.advance(),
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
	Special  = task.sideBack(),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADMS]"
},

["BackFieldOurBall"]={
	switch=function ()
		if bufcnt(not cond.canShootOnBallPos("Leader") and 
			cond.canPassOnBallPos("Leader",gPassPos,gGuisePos) and isUsePass,3) then
			return "FrontFieldPass"
		end
		return backFieldJumpCond()
	end,
	Leader   = task.advance(),--task.shoot("safe"),
	Special  = task.sideBack(),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADMS]"

},

["BackFieldStandOff"]={
	switch=function ()
		return backFieldJumpCond()
	end,
	Leader   = task.advance(),
	Special  = task.sideBack(),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADMS]"
},


["BackFieldGiveUpAdvance"]={
	switch=function ()
		return backFieldJumpCond()
	end,
	Leader   = task.protectBall(),
	Special  = task.sideBack(),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADMS]"
},


["NoMiddleDefNone"]={
	switch=function ()
		return noMiddleDefJumpCond()
	end,
	Leader   = task.advance(),
	Special  = task.goSecondPassPos("Leader"),
	Middle   = task.sideBack(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADM][S]"
},


["NoMiddleDefOurBall"]={
	switch=function ()
		return noMiddleDefJumpCond()
	end,
	Leader   = task.advance(),--task.shoot("safe"),
	Special  = task.goSecondPassPos("Leader"),
	Middle   = task.sideBack(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADM][S]"
},


["NoMiddleDefStandOff"]={
	switch=function ()
		return noMiddleDefJumpCond()
	end,
	Leader   = task.advance(),
	Special  = task.goSecondPassPos("Leader"),
	Middle   = task.sideBack(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADM][S]"
},


["NoMiddleDefGiveUpAdvance"]={
	switch=function ()
		return noMiddleDefJumpCond()
	end,
	Leader   = task.protectBall(),
	Special  = task.goSecondPassPos("Leader"),
	Middle   = task.sideBack(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADM][S]"
},


["BackFieldNoneTransit"]={
	switch=function ()
		if  bufcnt(player.posX("Special")<-210* param.lengthRatio,3) then
			return "BackFieldNone"
		end
		return backFieldJumpCond()
	end,
	Leader   = task.advance(),
	Special  = task.sideBack(),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADM][S]"
},

["BackFieldOurBallTransit"]={
	switch=function ()
		if  bufcnt(player.posX("Special")<-210* param.lengthRatio,3) then
			return "BackFieldOurBall"
		end
		return backFieldJumpCond()
	end,
	Leader   = task.advance(),--task.shoot("safe"),
	Special  = task.sideBack(),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADM][S]"

},


["NoMiddleDefNoneTransit"]={
	switch=function ()
		if  bufcnt(player.posX("Middle")<-210* param.lengthRatio,3) then
			return "NoMiddleDefNone"
		end
		return noMiddleDefJumpCond()
	end,
	Leader   = task.advance(),
	Special  = task.goSecondPassPos("Leader"),
	Middle   = task.sideBack(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][M][S]"
},


["NoMiddleDefOurBallTransit"]={
	switch=function ()
		if  bufcnt(player.posX("Middle")<-210* param.lengthRatio,3) then
			return "NoMiddleDefOurBall"
		end
		return noMiddleDefJumpCond()
	end,
	Leader   = task.advance(),
	Special  = task.goSecondPassPos("Leader"),
	Middle   = task.sideBack(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][M][S]"
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

name = "NormalPlayDefend",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}