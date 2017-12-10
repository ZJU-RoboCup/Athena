
local  gInitKickDirection = 1
local  gKickDirection = 1
local  stateKeepNum0=0
local  stateKeepNum1=0
local  targetPosX = 45
local  initY = -targetPosX
local function getKickDir()
	local targetPos = CGeoPoint:new_local(param.pitchLength/2.0,-gKickDirection*targetPosX)
	return (targetPos-ball.pos()):dir()
end 

local function getAntiKickDir()
	local targetPos = CGeoPoint:new_local(param.pitchLength/2.0,gKickDirection*targetPosX)
	return (targetPos-ball.pos()):dir()
end 

-- local  function enemyKickStyle()
-- 	local theirGoalieNum = bestPlayer:getTheirGoalie()
-- 	if enemy.velMod(theirGoalieNum)<15 then
-- 		return "still"
-- 	else 
-- 		return "move"
-- 	end
-- end
local function calPredictFactor(goalieVelY)
	local gPredictFactor =10/60
	if goalieVelY>100 then
		gPredictFactor = 10/60
	elseif goalieVelY>60 then
		gPredictFactor = 18/60
	elseif goalieVelY>30 then
		gPredictFactor = 6/60
	elseif goalieVelY>10 then
		gPredictFactor = 4/60
	end
	return gPredictFactor
end

local function generateKickDirection()
	local kickDirection=0
	local theirGoalieNum = bestPlayer:getTheirGoalie()
	local goalieY=enemy.posY(theirGoalieNum)
	--local goalieVelY=enemy.vel(theirGoalieNum):y()*2
	local lastGoalieY = vision:TheirPlayer(vision:LastCycle(),theirGoalieNum):Y()
	local lastLastGoalieY = vision:TheirPlayer(vision:LastCycle()-1,theirGoalieNum):Y()
	local lastGoalieVelY=goalieY-lastGoalieY
	local lastLastGoalieVelY=lastGoalieY-lastLastGoalieY
	local acc = lastGoalieVelY-lastLastGoalieVelY
	local goalieVelY=lastGoalieVelY+acc
	--local predictFactor=calPredictFactor(goalieVelY)
	predictFactor=1
	if goalieVelY>-10 then
		local predictGoalPosY=goalieY+goalieVelY*predictFactor
		if predictGoalPosY<-5 then
			kickDirection=-1
		end
		if predictGoalPosY>0 then
			kickDirection=1
		end
	else
		local predictGoalPosY=goalieY+goalieVelY*predictFactor
		if predictGoalPosY<-12 then
			kickDirection=-1
		end
		if predictGoalPosY>5 then
			kickDirection=1
		end
	end
	return kickDirection
	
end




gPlayTable.CreatePlay{

firstState = "goto",

["goto"] = {
	switch = function ()
		math.randomseed(os.time()) 
		if cond.isNormalStart() then
			stateKeepNum0=math.random(0,30)
			return "getBall"
		elseif cond.isGameOn() then
			return "exit"
		end
	end,
	Leader   = task.goSpeciPos(CGeoPoint:new_local(320,0),0),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,100)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-100)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][ASDM]"
},


["getBall"] = {
	switch = function ()
		if  bufcnt(true,15+stateKeepNum0) then
			stateKeepNum1=math.random(0,40)
			return "prepare"
		end
	end,
	Leader   = {SlowGetBall{pos = ball.pos, dir = player.toPointDir(CGeoPoint:new_local(param.pitchLength/2.0,initY))}},
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,100)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-100)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LASDM}"
},


["prepare"] = {
	switch = function ()
		if  bufcnt(true,60+stateKeepNum1) then
			return "slowGoto"
		elseif  bufcnt(generateKickDirection()==1,3) then
			return "direct"
		end
	end,
	Leader   = {SlowGetBall{pos = ball.pos, dir = player.toPointDir(CGeoPoint:new_local(param.pitchLength/2.0,initY))}},
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,100)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-100)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LASDM}"
},


["slowGoto"] = {
	switch = function ()
		if  bufcnt(generateKickDirection()==1,3,10) then
			return "direct"
		elseif bufcnt(generateKickDirection()==-1,3,10) then
			return "turn"
		elseif  bufcnt(generateKickDirection()==0,3,10) then
			return "direct"
		end
	end,
	Leader   = {SlowGetBall{pos = ball.pos, dir = player.toPointDir(CGeoPoint:new_local(param.pitchLength/2.0,initY))}},
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,100)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-100)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LASDM}"
},

["direct"] = {
	switch = function ()
		if  bufcnt( player.kickBall("Leader"), "normal", 10) then
			return "exit"
		end
	end,
	Leader   = task.penaltyChase(getKickDir),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,100)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-100)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LASDM}"
},

["turn"] = {
	switch = function ()
		if  bufcnt(true, 30) then
			return "kick"
		end
	end,
	Leader   = task.penaltyTurnShoot(getAntiKickDir),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,100)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-100)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LASDM}"
},

["kick"] = {
	switch = function ()
		if  bufcnt( player.kickBall("Leader"), "normal", 10) then
			return "exit"
		end
	end,
	Leader   = task.penaltyChase(getAntiKickDir),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,100)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-100)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LASDM}"
},

name = "Ref_PenaltyKickV4",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
