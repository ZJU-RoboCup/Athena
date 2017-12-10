local function jumpNoMiddleDefCond()
	if gRoleNum["Middle"]==bestPlayer:getOurBestPlayer() 
		or gRoleNum["Assister"]==bestPlayer:getOurBestPlayer()
		or gRoleNum["Defender"]==bestPlayer:getOurBestPlayer() 
		or gRoleNum["Special"]==bestPlayer:getOurBestPlayer() then
		if player.toTargetDist("Middle")<20 then
			--gRoleNum["Leader"]=gRoleNum["Middle"]
		end
		return "NoMiddleDef"..gCurrentBallStatus
	end
	if  bufcnt(ball.posX()<pos.defendMiddlePos():x()+20,5) then
		return "NoMiddleDef"..gCurrentBallStatus
	end
end

local function noMiddleDefJumpCond()
	if  bufcnt(player.posX("Leader")>-140*param.lengthRatio,5)  then
		return gCurrentBallStatus
	end

	if  bufcnt( not (math.abs(ball.velDir())>math.pi/1.5) and ball.velMod()>100 and gCurrentFieldArea=="FrontField",3) then
		return gCurrentBallStatus
	end
end


local  function stateJumpCond()
	if gCurrentBallStatus ~= gLastBallStatus then
		return gCurrentBallStatus
	end
	if bufcnt(Utils.InOurPenaltyArea(ball.pos(),5),5) then
		return "NoAdvance"
	end
	local jump = jumpNoMiddleDefCond()
	if jump ~= nil then
		return jump
	end
end


local  function noMiddleDefStateJumpCond()
	if gCurrentBallStatus ~= gLastBallStatus then
		return "NoMiddleDef"..gCurrentBallStatus
	end
	if bufcnt(Utils.InOurPenaltyArea(ball.pos(),5),5) then
		return "NoAdvance"
	end
	return noMiddleDefJumpCond()	
end


gPlayTable.CreatePlay{
firstState = "initState",

["initState"] = {
	switch = function ()
		return gCurrentBallStatus
	end,
	Leader   = task.advance(),
	Special  = task.sideBack(),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADMS]"
},


["None"]={
	switch=function ()
		return stateJumpCond()
	end,
	Leader   = task.advance(),
	Special  = task.sideBack(),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADMS]"
},



["StandOff"]={
	switch=function ()
		return stateJumpCond()
	end,
	Leader   = task.advance(),
	Special  = task.sideBack(),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADMS]"
},


["GiveUpAdvance"]={
	switch=function ()
		return stateJumpCond()
	end,
	Leader   = task.protectBall(),
	Special  = task.sideBack(),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADMS]"
},



["OurBall"]={
	switch=function ()
		return stateJumpCond()
	end,
	Leader   = task.shoot("safe"),
	Special  = task.sideBack(),
	Middle   = task.defendMiddle("Leader"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADMS]"

},


["NoMiddleDefNone"]={
	switch=function ()
		return noMiddleDefStateJumpCond()
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
		return noMiddleDefStateJumpCond()
	end,
	Leader   = task.shoot("safe"),
	Special  = task.goSecondPassPos("Leader"),
	Middle   = task.sideBack(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADM][S]"
},


["NoMiddleDefStandOff"]={
	switch=function ()
		return noMiddleDefStateJumpCond()
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
		return noMiddleDefStateJumpCond()
	end,
	Leader   = task.protectBall(),
	Special  = task.goSecondPassPos("Leader"),
	Middle   = task.sideBack(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ADM][S]"
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
				return gCurrentBallStatus
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



name = "NormalPlayOneState",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}