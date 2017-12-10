module(..., package.seeall)


--~ -----------------------------------------------
--~ referee box condition
--~ -----------------------------------------------

function isGameOn()
	--print("check game on")
	return vision:gameState():gameOn()
end

function isNormalStart()
	return vision:gameState():canEitherKickBall()
end

--~ -----------------------------------------------
--~ geometry condition
--~ -----------------------------------------------


--~ -----------------------------------------------
--~ other condition
--~ -----------------------------------------------
function bestPlayerChanged()
	return world:IsBestPlayerChanged()
end

function canShootOnBallPos(role)
	return world:canShootOnBallPos(vision:Cycle(),gRoleNum[role])
end

function canPassOnBallPos(role,passPos,guisePos)
	return world:canPassOnBallPos(vision:Cycle(),passPos,guisePos,gRoleNum[role])
end

function canKickAtEnemy(role,kickDir)
	return world:canKickAtEnemy(vision:Cycle(),kickDir,gRoleNum[role])
end


function currentBayes(str)
	return world:CurrentBayes() == str
end

function validNum()
	return vision:getValidNum()
end

function canDefenceExit()
	return world:CanDefenceExit()
end

function timeRemain()
	return vision:TimeRemain()
end

function ourGoal()
	return vision:OurGoal()
end

function theirGoal()
	return vision:TheirGoal()
end

function needExitAttackDef(p1,p2,str)
	--此区域是否还需要保持进攻性防守
	local mp1
	local mp2
	if type(p1) == "function" then
		mp1 = p1()
	else 
		mp1 = p1
	end
	if type(p2) == "fucntion" then
		mp2 = p2()
	else
		mp2 = p2
	end
	local mode
	if str=="horizal" then
		mode = 2
	end
	if str=="vertical" then
		mode = 1
	end
    return world:NeedExitAttackDef(mp1,mp2,mode)
end

function dist4ball2MarkTouch(p1,p2)
	local mp1
	local mp2
	if type(p1) == "function" then
		mp1 = p1()
	else 
		mp1 = p1
	end
	if type(p2) == "fucntion" then
		mp2 = p2()
	else
		mp2 = p2
	end
	return world:ball2MarkingTouch(mp1,mp2)
end

function canExitMiddleDef()
	local result = false
	if ball.velMod()<100 then
		result = true
	end
	return result
end

function  canExitMRLFrontDef()
	local result = false
	if ball.velMod()<150 and vision:ballVelValid() then
		result = true
	--else
		-- if ball.refAntiY() == -1 then
		-- 	--modified for Brazil by thj
		-- 	local leftUp = CGeoPoint:new_local(190,-param.pitchWidth/2)
		-- 	local k = CGeoPoint:new_local(0,-30)
		-- 	local bestEnemy = enemy.bestPos()
	
		-- 	if bestEnemy:x() < leftUp:x() and bestEnemy:x() >rightDown:x()
		-- 		and bestEnemy:y() > leftUp:y() and bestEnemy:y() < rightDown:y() then
		-- 		if ball.velMod()>50 and Utils.AngleBetween(ball.velDir(),Utils.Normalize((bestEnemy - ball.pos()):dir()-math.pi/3),Utils.Normalize((bestEnemy - ball.pos()):dir()+math.pi/3),0) then
		-- 			result = true
		-- 		end
		-- 	end
		-- elseif ball.refAntiY() == 1 then
		-- 	local leftUp = CGeoPoint:new_local(190,param.pitchWidth/2)
		-- 	local rightDown = CGeoPoint:new_local(0,30)
		-- 	local bestEnemy = enemy.bestPos()
		-- 	if bestEnemy:x() < leftUp:x() and bestEnemy:x() >rightDown:x()
		-- 		and bestEnemy:y() < leftUp:y() and bestEnemy:y() > rightDown:y() then
		-- 		if ball.velMod()> 50 and Utils.AngleBetween(ball.velDir(),Utils.Normalize((bestEnemy - ball.pos()):dir()-math.pi/3),Utils.Normalize((bestEnemy - ball.pos()):dir()+math.pi/3),0) then
		-- 			result = true
		-- 		end
		-- 	end
		-- end
	end
	return result
end

function canExitRoboDragonMarkingFront()
	local result = false
	if ball.velMod()<50 then
		result = true
	-- elseif ball.velMod()>50 and Utils.AngleBetween(ball.velDir(),-1.57,1.57,0) then
	-- 	result = true
	end
	return result
end

function canBeImmortalShoot()
	local result = false
	if ball.toPointDist(enemy.bestPos()) < 50 and vision:ballVelValid() then
		result = true
	end
	return result
end

function  checkBallPassed(p1,p2)
	local result = false
	local lastCycle = 0
	local mp1
	local mp2
	local flag = false
	local check = 0
	local checkPosX = 0
	local checkPosY = 0
	if type(p1) == "function" then
		mp1 = p1()
	else 
		mp1 = p1
	end
	if type(p2) == "fucntion" then
		mp2 = p2()
	else
		mp2 = p2
	end
	--reset
	if vision:Cycle() - lastCycle > 6 then
		flag = false
	end
	lastCycle =vision:Cycle()

	if ball.velMod()> 50 and ball.toBestEnemyDist()> 15 and flag == false then
		check = enemy.bestDir()
		local checkPos = enemy.bestPos()
		checkPosX = checkPos:x()
		checkPosY = checkPos:y()
		flag = true
		--print("ball kicked")
	end

	if flag == true then
		local checkField = CGeoRectangle:new_local(mp1,mp2)
		local checkLine = CGeoLine:new_local(CGeoPoint:new_local(checkPosX,checkPosY),CGeoPoint:new_local(checkPosX,checkPosY)+Utils.Polar2Vector(100,check))
		local intersect = CGeoLineRectangleIntersection:new_local(checkLine,checkField)
		if intersect:intersectant() then
			result = true
		else
			result = false
		end
	end
	return result
end

function getValidMarkingTouchArea(p1,p2,p3,p4)
	return world:getMarkingTouchArea(p1,p2,p3,p4)
end

function markingFrontValid(p1,p2)
	return world:isMarkingFrontValid(p1,p2)
end

function kickOffEnemyNum()
	return world:getEnemyKickOffNum()
end

function  kickOffEnemyNumChanged()
	return world:checkEnemyKickOffNumChanged()
end

function judgeFieldArea()
	if gNormalPlay == "NormalPlayNew" then
		local MiddleFrontLine = 100 * param.lengthRatio
		local BackMiddleLine = -80 * param.lengthRatio
		local bufferX = 30
		if gCurrentFieldArea=="MiddleField" then
			if (ball.posX()>MiddleFrontLine+bufferX) then
				return "FrontField"
			end
			if (ball.posX()<BackMiddleLine-bufferX) then
				return "BackField"
			end
		end

		if gCurrentFieldArea=="BackField" then
			if (ball.posX()>BackMiddleLine+bufferX) then
				return "MiddleField"
			end
		end

		if gCurrentFieldArea=="FrontField" then
			if (ball.posX()<MiddleFrontLine-bufferX) then
				return "MiddleField"
			end
		end
	elseif gNormalPlay == "NormalPlayDefend" or gNormalPlay == "NormalPlayMark" or gNormalPlay == "NormalPlayPass" then
		local BackFrontLine = 0
		local bufferX = 30
		if gCurrentFieldArea=="BackField" then
			if (ball.posX()>BackFrontLine+bufferX) then
				return "FrontField"
			end
		end

		if gCurrentFieldArea=="FrontField" then
			if (ball.posX()<BackFrontLine-bufferX) then
				return "BackField"
			end
		end
	elseif gNormalPlay == "NormalPlayOneState" then
		local BackFrontLine = -120 * param.lengthRatio
		local bufferX = 30
		if gCurrentFieldArea=="BackField" then
			if (ball.posX()>BackFrontLine+bufferX) then
				return "FrontField"
			end
		end

		if gCurrentFieldArea=="FrontField" then
			if (ball.posX()<BackFrontLine-bufferX) then
				return "BackField"
			end
		end
	end
	return gCurrentFieldArea
end


--------------------------------------For Freekick--------------------------------------------------
function canPassAndShoot(role)
		if gRoleNum[role] ~= 0 and not world:isPassLineBlocked(gRoleNum[role]) and not world:isShootLineBlocked(gRoleNum[role]) then
			return true
		else
			return false
		end
end

-- 优先级从role1至role5降低, 可少角色
function findChance(role1, role2, role3, role4, role5)
	if role1 ~= nil and type(role1) == "string" then
		if gRoleNum[role1] ~= 0 and not world:isBeingMarked(gRoleNum[role1]) and canPassAndShoot(role1) then
			return role1
		end
	end
	if role2 ~= nil and type(role2) == "string" then
		if gRoleNum[role2] ~= 0 and not world:isBeingMarked(gRoleNum[role2]) and canPassAndShoot(role2) then
			return role2
		end
	end
	if role3 ~= nil and type(role3) == "string" then
		if gRoleNum[role3] ~= 0 and not world:isBeingMarked(gRoleNum[role3]) and canPassAndShoot(role3) then
			return role3
		end
	end
	if role4 ~= nil and type(role4) == "string" then
		if gRoleNum[role4] ~= 0 and not world:isBeingMarked(gRoleNum[role4]) and canPassAndShoot(role4) then
			return role4
		end
	end
	if role5 ~= nil and type(role5) == "string" then
		if gRoleNum[role5] ~= 0 and not world:isBeingMarked(gRoleNum[role5]) and canPassAndShoot(role5) then
			return role5
		end
	end
	return "None"
end

-- str 为所在的区域
-- script 为所使用的脚本
-- MaxRandom 为最大的随机数
local randNumLast = 0 --防止单一脚连续使用
function getOpponentScript(str, script, MaxRandom)
	if type(script) == "table" then
		local totalNum = table.getn(script)
		local randNum = 1
		
		for i=1,2 do
			randNum=math.random(1,totalNum)
			--print("\n## RandNum:"..randNum.." ##")
			if (randNum~=randNumLast) then
				break
			end
		end
		randNumLast=randNum

	if (type(script[randNum]) == "string") then
			return script[randNum]
		else
			return str..script[randNum]
		end
	elseif type(script) == "string" then
		if script == "random" then
			local randNum = math.random(1,MaxRandom)
			print("randNum",str..randNum)
			return str..randNum
		else
			-- 使用固定的模式打定位球
			return script
		end
	else
		print("Error in getOpponentScript "..str)
	end	
end

function canNorPass2Def(kickDir)
	if kickDir == nil then
		--refBall = CGeoPoint:new_local(ball.refPosX(),ball.refPosY())
		return ball.velMod() < gNorPass2NorDefBallVel
			and vision:ballVelValid() 
			--and not (ball.velMod()>10 and (ball.velDir() + math.pi) < math.pi*3/2 and (ball.velDir() + math.pi) > math.pi/2 and ball.toPointDist(refBall) < 100)
	else
		--print("dir: ",math.abs(Utils.Normalize(ball.velDir()-kickDir))/math.pi*180)
		return (ball.velMod() < gNorPass2NorDefBallVel or math.abs(Utils.Normalize(ball.velDir()-kickDir)) > math.pi/2)  
			and vision:ballVelValid()
	end
end