local MOVE_TO_KICK_POINT_SPEED = 100
local KICK_PREPARE_DIST = 12
local FACING_ANGLE_ERROR=math.pi/30
local POS_ANGLE_ERROR=math.pi/30
local MAX_WAITING_COUNT = 30
local KICK_SPEED = 50
local Go_Kick_Open_Speed = 30
local Go_Kick_Open_Speed_Base = 30
local Go_Kick_Open_Speed_Step = 10

function PenaltyKick(task)
	local _lastcycle = 0

	local state={"PREPAIRING",
		   "WAITING",
		   "ROTATING",
		   "KICK"}
 	
	local _waitingCount = 0
	local currentstate = state[1]

 	execute = function(runner)
		local ballpos = ball.pos()
		local _target = pos.theirGoal()
		local mtask = {}
		if vision:Cycle()- _lastcycle > 10 then
			currentstate = state[1]
			_waitingCount = 0
		end			

		_lastcycle = vision:Cycle()

		if currentstate == state[1] then
			local self2ballDir = player.toBallDir(runner)
			local self2ballDist = player.toBallDist(runner)
			local targetDir = (_target - ballpos):dir()

			local startpoint = ballpos + Utils.Polar2Vector(-KICK_PREPARE_DIST,targetDir)
			local facingAngleDiff = Utils.Normalize( self2ballDir - player.dir(runner) )
			local absFacingAngleDIff = math.abs(facingAngleDiff)
			local posAngleDiff = Utils.Normalize(self2ballDir-targetDir)
			local absPosAngleDiff = math.abs(posAngleDiff)
			
			if (vision:gameState():canKickBall() == false) or(
				(absPosAngleDiff > POS_ANGLE_ERROR) or
				  (absFacingAngleDIff > FACING_ANGLE_ERROR) or
				  (self2ballDist > KICK_PREPARE_DIST+3) )then
				  mtask.pos = startpoint
				  mtask.dir = targetDir
				  currentstate = state[1]
					return SmartGoto(mtask)(runner)
				else
				currentstate = state[2]
			end
		end

		if currentstate == state[2] then
			_waitingCount = _waitingCount+1
			if  _waitingCount > MAX_WAITING_COUNT then
				currentstate = state[3]
			else
				return Stop()(runner)
			end
		end

		if currentstate == state[3] then
			local theirGoal = CGeoPoint:new_local(param.pitchLength/2,0)
			local sign
			local kickpoint
			local goalieNum = 0
			for i=1,1,6 do
				if vision:TheirPlayer(i):Pos():dist(theirGoal) < param.goalWidth/2 then
					goalieNum = i
					break
				end
			end
			if goalieNum ~= 0 then
				local oppGoalieY = vision:TheirPlayer(goalieNum):Y()
				if oppGoalieY>0 then
					sign = -1
				else
					sign = 1
				end
				_target = CGeoPoint:new_local(param.pitchLength/2+param.goalDepth,sign*param.goalWidth/2 -3)
			else

				sign = 1
				_target = CGeoPoint:new_local(param.pitchLength/2+param.goalDepth,sign*param.goalWidth/2 -3)
			end

			target2ball = ballpos - _target
			kickpoint = ballpos + Utils.Polar2Vector(param.playerRadius+4,target2ball:dir())
			
			if  player.pos(runner):dist(kickpoint) > 3 then
				mtask.pos = kickpoint
				mtask.dir = (_target - ballpos):dir()
				return SimpleGoto(mtask)(runner)
			else
				currentstate = state[4]
			end
		end
		
		if currentstate == state[4] then
			mtask.dir = (_target - ballpos):dir()
			mtask.pos = ballpos

			kickStatus:setKick(runner,1500)

			dribbleStatus:setDribbleCommand(runner,2)
			return SimpleGoto(mtask)(runner)
		end			
	end
	
	matchPos = function()
		return CGeoPoint:new_local(0,0)
	end
	
	return execute, matchPos
end

 gSkillTable.CreateSkill{
 	name = "PenaltyKick",
 	execute = function (self)
 		print("This is in skill"..self.name)
 	end
 }

