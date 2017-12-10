module(..., package.seeall)

function specified(p)
	return function ()
		return p
	end
end

function ourGoal()
	return CGeoPoint:new_local(-param.pitchLength / 2.0, 0)
end

function theirGoal()
	return CGeoPoint:new_local(param.pitchLength / 2.0, 0)
end

function refStopAroundBall()
	local BLOCK_DIST = param.freeKickAvoidBallDist + param.playerRadius
	local AWAY_DIST = 2.5 + param.playerRadius
	local BLOCK_ANGLE = math.asin(AWAY_DIST / BLOCK_DIST)*2
	local factor = ball.antiY

	local SIDE_POS = function()
		return ball.pos() + Utils.Polar2Vector(BLOCK_DIST, dir.ballToOurGoal() + factor()*BLOCK_ANGLE)
	end

	local INTER_POS = function()
		return ball.pos() + Utils.Polar2Vector(BLOCK_DIST, dir.ballToOurGoal() - factor()*BLOCK_ANGLE)
	end

	local MIDDLE_POS = function()
		return ball.pos() + Utils.Polar2Vector(BLOCK_DIST, dir.ballToOurGoal())
	end

	return SIDE_POS, MIDDLE_POS, INTER_POS
end

function backBall(p)
	return function ()
		local targetP = CGeoPoint:new_local(p:x(), ball.antiY()*p:y())
		return  ball.pos() + Utils.Polar2Vector(18, Utils.Normalize((ball.pos() - targetP):dir()))
	end
end


function bestShoot()
	bestAlg:setSendPoint(ball.pos())
	print(ball.posY());
	if ball.posY() > 0 then
		return bestAlg:getBestPoint(CGeoPoint:new_local(280*param.lengthRatio,-180*param.widthRatio), CGeoPoint:new_local(0,0), 0)
	else
		return bestAlg:getBestPoint(CGeoPoint:new_local(280*param.lengthRatio,0), CGeoPoint:new_local(0,180*param.widthRatio), 0)
	end
end

--ischip为true算挑球点，false算平传点
function bestShootPointForFreeKicks(ischip)
	if ball.refPosX() > 360 and math.abs(ball.refPosY()) >= 200 then
		if ischip == nil then
			ischip = false
		end
		return bestAlg:bestPointForCornerKicks(ischip)
	--elseif ball.refPosX() > 100 and ball.refPosX() < 362 then
		--return bestAlg:bestPointForFrontKicks()
	else
		return bestAlg:bestPointForBackKicks()
	end
end

--role是接球车，getpos为true代表取得接球车要到的点，否则取得接球车的点
function isChip(role,getpos)
	if role == nil then
		return bestAlg:isChip()
	elseif getpos == true then
		local p = gRolePos[role]
		return bestAlg:isChip(p)
	else
		local number = gRoleNum[role]
		return bestAlg:isChip(number)
	end
end

function getChipPower()
	local power = function()
		return bestAlg:getChipPower()
	end
	return power
end

function getFixBuf()
	return bestAlg:getFixBuf()
end

function detect(generatePos,detectDist,detectCycle,useGoalie)
	if generatePos == nil then
		generatePos = false
	end
	if detectDist == nil then 
		detectDist = 50
	end
	if detectCycle == nil then 
		detectCycle = 999
	end
	if useGoalie == nil then 
		useGoalie = false
	end
 	bestAlg:detectModule(generatePos,detectDist,detectCycle,useGoalie)
end

function detectDefendKicked(checkAttepmts,defendKickCycle)
	if checkAttepmts == nil then
		checkAttepmts = 3
	end
    if defendKickCycle == nil then
		defendKickCycle = 30
	end
 	bestAlg:detectDefendKickedModule(checkAttepmts,defendKickCycle)
end

function scan()
 	bestAlg:scanModule()
end

function receiveOwnPos(posVector,role)
 	bestAlg:receiveOwnPos(posVector,role)
 end

function fakePosCompute()
 	bestAlg:fakePosComputeModule()
end

function getRunPos()
	local first = function()
		return bestAlg:getPosForFirstVehicle()
	end
	local second = function()
		return bestAlg:getPosForSecondVehicle()
	end
	local third = function()
		return bestAlg:getPosForThirdVehicle()
	end
	local fourth = function()
		return bestAlg:getPosForFourthVehicle()
	end
	return first,second,third,fourth
end

function confirmPos()
	local first = function()
		return bestAlg:confirmPosForFirstVehicle()
	end
	local second = function()
		return bestAlg:confirmPosForSecondVehicle()
	end
	local third = function()
		return bestAlg:confirmPosForThirdVehicle()
	end
	local fourth = function()
		return bestAlg:confirmPosForFourthVehicle()
	end
	return first,second,third,fourth
end

function getAssistPos()
	local assistPos = function()
		return bestAlg:getPosForAssister()
	end
	return assistPos
end

function getGoaliePos()
	local gPos = function()
		return bestAlg:getPosForGoalie()
	end
	return gPos
end

function getShootPos()
	local shootPos = function()
		return bestAlg:getPosForShoot()
	end
	return shootPos
end

function getJamPos()
	local jamPos = function()
		return bestAlg:generateJamPos()
	end
	return jamPos
end

function detectMarkedOver()
	return bestAlg:detectMarkedOver()
end

function detectDefendKickedOver()
	return bestAlg:detectDefendKickedOver()
end

function detectAgain()
	return bestAlg:detectAgain()
end

function scanOver()
	return bestAlg:scanOver()
end

function canPass()
	return bestAlg:canPass()
end

function isDefendedHead()
	return bestAlg:isDefendedHead()
end

function allMarked()
	return bestAlg:allMarked()
end

function isMarked(role)
	return bestAlg:isMarked(role)
end

function sendVehicleNumber(first, second, third, fourth, assister, goalie)
	bestAlg:numberReceive(first, second, third, fourth, assister, goalie)
end

function changeLeader()
	return bestAlg:changeLeader()
end

function changeLeaderOver(finishflag)
	return bestAlg:changeLeaderOver(finishflag)
end

function playerBest()
	local robotNum = bestPlayer:getOurBestPlayer()
	if robotNum >0 and robotNum <= param.maxPlayer then
		return player.pos(robotNum)
	else
		return ball.pos()
	end
end

function fakeDown(p)
	local factor

	return function ()
		if ball.posY() > 10 then
			factor = -1
		else
			factor = 1
		end
		local targetP = CGeoPoint:new_local(p:x(), factor * p:y())
		local standVec = (targetP - ball.pos()):rotate(factor * math.pi * 100 / 180)
		return ball.pos() + Utils.Polar2Vector(30, standVec:dir())
	end
end

function stopPoses()
	local Rtarget1, Rtarget2, Rtarget3, Rtarget4, Rtarget5
	local Rdir1, Rdir2, Rdir3, Rdir4, Rdir5
	local RADIUS = param.penaltyRadius + param.playerRadius + param.goalWidth/2
	local angleDiff = 3.5 * DefendUtils.calcBlockAngle(CGeoPoint:new_local(0, 0), CGeoPoint:new_local(0, RADIUS))
	local outerLength = RADIUS + 50

	local RReferenceX = param.pitchLength/2 - param.penaltyDepth/2
	local RLeftFixedPoint = CGeoPoint:new_local(param.pitchLength/2 - param.penaltyDepth, param.pitchWidth/2 - param.playerRadius*4)
	local RRightFixedPoint = CGeoPoint:new_local(param.pitchLength/2 - param.penaltyDepth, -param.pitchWidth/2 + param.playerRadius*4)

	local UpdateState = function()
		local RballPos = DefendUtils.reversePoint(ball.pos())
		local RgoalCenter = CGeoPoint:new_local(param.pitchLength/2,0)
		local Rgoal2ball = RballPos - RgoalCenter
		Rdir3 = Rgoal2ball:dir()
		Rtarget3 = RgoalCenter + Utils.Polar2Vector(outerLength,Rdir3)
		local Rpoint3 = DefendUtils.calcDefenderPoint(Rtarget3,Utils.Normalize(Rdir3+math.pi),POS_SIDE_MIDDLE)
		if Rpoint3:x() >= RReferenceX and Rpoint3:y() > 0 then
			Rdir3 = (RLeftFixedPoint - RgoalCenter):dir()
		elseif Rpoint3:x() >= RReferenceX and Rpoint3:y() < 0 then
			Rdir3 = (RRightFixedPoint - RgoalCenter):dir()
		else
			Rdir3 = Rgoal2ball:dir()
		end

		Rdir1 = Utils.Normalize(Rdir3 - angleDiff * 2)
		Rdir2 = Utils.Normalize(Rdir3 - angleDiff)
		Rdir4 = Utils.Normalize(Rdir3 + angleDiff)
		Rdir5 = Utils.Normalize(Rdir3 + angleDiff * 2)
		Rtarget1 = RgoalCenter + Utils.Polar2Vector(outerLength, Rdir1)
		Rtarget2 = RgoalCenter + Utils.Polar2Vector(outerLength, Rdir2)
		Rtarget3 = RgoalCenter + Utils.Polar2Vector(outerLength, Rdir3)
		Rtarget4 = RgoalCenter + Utils.Polar2Vector(outerLength, Rdir4)
		Rtarget5 = RgoalCenter + Utils.Polar2Vector(outerLength, Rdir5)
	end

	local STOP_POINT1 = function()
		UpdateState()
		local Rpoint1 = DefendUtils.calcDefenderPoint(Rtarget1, Utils.Normalize(Rdir1 + math.pi), POS_SIDE_MIDDLE)
		return DefendUtils.reversePoint(Rpoint1)
	end
	local STOP_POINT2 = function()
		UpdateState()
		local Rpoint2 = DefendUtils.calcDefenderPoint(Rtarget2, Utils.Normalize(Rdir2 + math.pi), POS_SIDE_MIDDLE)
		return DefendUtils.reversePoint(Rpoint2)
	end
	local STOP_POINT3 = function()
		UpdateState()
		local Rpoint3 = DefendUtils.calcDefenderPoint(Rtarget3, Utils.Normalize(Rdir3 + math.pi), POS_SIDE_MIDDLE)
		return DefendUtils.reversePoint(Rpoint3)
	end
	local STOP_POINT4 = function()
		UpdateState()
		local Rpoint4 = DefendUtils.calcDefenderPoint(Rtarget4, Utils.Normalize(Rdir4 + math.pi), POS_SIDE_MIDDLE)
		return DefendUtils.reversePoint(Rpoint4)
	end
	local STOP_POINT5 = function()
		UpdateState()
		local Rpoint5 = DefendUtils.calcDefenderPoint(Rtarget5, Utils.Normalize(Rdir5 + math.pi), POS_SIDE_MIDDLE)
		return DefendUtils.reversePoint(Rpoint5)
	end

	return STOP_POINT1,STOP_POINT2,STOP_POINT3,STOP_POINT4,STOP_POINT5
end

function defendFrontPos(p)
	return function ()
		return DefendUtils.getMiddleDefender(p)
	end
end

function defaultDefPos(p)
	return function ()
		return DefendUtils.getDefaultPos(p)
	end
end

function leftBackPos()
	return defPos2015:getDefPos2015(vision):getLeftPos()
end

function rightBackPos()
	return defPos2015:getDefPos2015(vision):getRightPos()
end

function defendMiddlePos()
	return defPos2015:getDefPos2015(vision):getMiddlePos()
end

function singleBackPos()
	return defPos2015:getDefPos2015(vision):getSinglePos()
end

function sideBackPos()
	return defPos2015:getDefPos2015(vision):getSidePos()
end

function goaliePos()
	return defPos2015:getDefPos2015(vision):getGoaliePos()
end

function getSupportPos(role)
	return function()
		local p = supportPos:getSupPos(vision,gRoleNum[role])
		return p
	end
end

function getSupportPosNew(role1,role2)
	return function()
		local p = supportPos:getSupPosNew(vision,gRoleNum[role1],gRoleNum[role2])
		return p
	end
end

function getAssistPoint(role1,role2)
	return function()
		local p = assistPoint:getBestAssistPoint(vision,gRoleNum[role1],gRoleNum[role2]);
		return p
	end 
end

function advance()
	if false then
		return ball.pos()
	else
		local robotNum = bestPlayer:getOurBestPlayer()
		if Utils.PlayerNumValid(robotNum) then
			return player.pos(robotNum)
		else
			return ball.pos()
		end
	end
end

function tandemPos2013()
	return  tandemPos:getTandemPos()
end

function defendHeadPos()
	return DefendUtils.getCornerAreaPos()
end


--p1是防守半径，p2,p3是防对方车的区域，p4为防开球的模式。1为老模式，2为防止直接射门
function defendKickPos(p1,p2,p3,p4)
	return function()
		if p1==nil then
			local radius = 60
			local left = CGeoPoint:new_local(1000,-1000)
			local right = CGeoPoint:new_local(-1000,1000)
			local mode = 1
			return DefendUtils.getIndirectDefender(radius,left,right,mode)
		else
			if type(p1) == "function" then
				mp1 = p1()
			else
				mp1 = p1
			end
			if type(p2) == "function" then
				mp2 = p2()
			else
				mp2 = p2
			end
			if type(p3) == "function" then
				mp3 = p3()
			else
				mp3 = p3
			end
			if type(p4) == "function" then
				mp4 = p4()
			else
				mp4 = p4
			end
			return DefendUtils.getIndirectDefender(mp1,mp2,mp3,mp4)
		end
	end
end

function oneKickDefPos(p1,p2,p3,p4,p5,p6)
	return function()
		if type(p1) == "function" then
			mp1 = p1()
		else
			mp1 = p1
		end
		if type(p2) == "function" then
			mp2 = p2()
		else
			mp2 = p2
		end
		if type(p3) == "function" then
			mp3 = p3()
		else
			mp3 = p3
		end
		if type(p4) == "function" then
			mp4 = p4()
		else
			mp4 = p4
		end
		if type(p5) == "function" then
			mp5 = p5()
		else
			mp5 = p5
		end
		if type(p6) == "function" then
			mp6 = p6()
		else
			mp6 = p6
		end
		return indirectDefender:getTwoDefPos(vision,mp1,mp2,mp3,mp4,mp5,mp6):getOnePos()
	end
end

function anotherKickDefPos(p1,p2,p3,p4,p5,p6)
	return function()
		if type(p1) == "function" then
			mp1 = p1()
		else
			mp1 = p1
		end
		if type(p2) == "function" then
			mp2 = p2()
		else
			mp2 = p2
		end
		if type(p3) == "function" then
			mp3 = p3()
		else
			mp3 = p3
		end
		if type(p4) == "function" then
			mp4 = p4()
		else
			mp4 = p4
		end
		if type(p5) == "function" then
			mp5 = p5()
		else
			mp5 = p5
		end
		if type(p6) == "function" then
			mp6 = p6()
		else
			mp6 = p6
		end
		--print(mp1,mp2:x(),mp3:x(),mp4,mp5:x(),mp6:x())
		return indirectDefender:getTwoDefPos(vision,mp1,mp2,mp3,mp4,mp5,mp6):getAnotherPos()
	end
end

function testTwoKickOffPos1()
	local BLOCK_DIST = 30
	return function ()
		return ball.pos() + Utils.Polar2Vector(BLOCK_DIST, dir.theirGoalToBall())
	end
end

function testTwoKickOffPos2()
	local BLOCK_DIST = 30
	local AWAY_DIST = 7.5 + param.playerRadius
	local BLOCK_ANGLE = math.asin(AWAY_DIST / BLOCK_DIST)*2
	local factor = ball.antiY
	return function()
		return ball.pos() + Utils.Polar2Vector(BLOCK_DIST, dir.theirGoalToBall()+ factor()*BLOCK_ANGLE)
	end
end

function theirBestPlayer()
	local oppNum = bestPlayer:getTheirBestPlayer()
	return enemy.pos(oppNum)
end

function getTandemPos(role)
	return function ()
		return world:getTandemPos(gRoleNum[role])
	end
end

-- x, y分别为相对于球的偏移量, yys 2014-06-11
function reflectPos(x, y)
	return function ()
	    if ball.posY()<0 then 
			return (ball.refAntiYPos(CGeoPoint:new_local(ball.posX() + x, -(math.abs(ball.posY()) + y) )))()
		end
		if ball.posY()>=0 then 
			return (ball.refSyntYPos(CGeoPoint:new_local(ball.posX() + x, math.abs(ball.posY()) + y )))()	
		end
	end
end

function reflectPassPos(role)
	return function ()
		return world:getReflectPos(gRoleNum[role])
	end
end

-- y表示在距离中线多远的地方做touch, yys 2014-06-20
function reflectTouchPos(role, y)
	return function ()
		return world:getReflectTouchPos(gRoleNum[role], y)
	end
end

-- 定位球传球点, yys 2014-06-16
function generateFreeKickPassPos(role)
	return function ()
		return ( player.pos(role) + Utils.Polar2Vector(param.playerRadius + 2, dir.shoot()(role)) )
	end
end

-- 根据touch所跑的跑位点, 计算需要传球的点 zhyaic 2014.6.28
-- p为传入的目Touch跑位点, p可以为返回值为坐标点的函数
function passForTouch(p)
	local length = 9.33
	--local length = 9.33 -- 2017年伊朗赛前测量
	-- local length = 9.33+2 --gty改
	if type(p) == "function" then
		return function ()
			return p() + Utils.Polar2Vector(length, dir.posToTheirGoal(p()))
		end
	else
		return p + Utils.Polar2Vector(length, dir.posToTheirGoal(p))
	end
end

function chipPassForTouch(p)
	local length = 10
	--local length = 9.33 -- 2017年伊朗赛前测量
	-- local length = 9.33+2 --gty改
	if type(p) == "function" then
		return function ()
			return p() + Utils.Polar2Vector(length, dir.posToTheirGoal(p()))
		end
	else
		return p + Utils.Polar2Vector(length, dir.posToTheirGoal(p))
	end
end

function passForHead(p)
	if type(p) == "function" then
		return function ()
			return p() + Utils.Polar2Vector(1, dir.posToTheirGoal(p()))
		end
	else
		return p + Utils.Polar2Vector(1, dir.posToTheirGoal(p))
	end
end

-- 利用图像使车消失的点. 传入第一辆车的位置和角度,返回第二辆(会消失的那辆)的坐标,角度在dir.lua里,
-- i表示用车的左边顶还是右边顶,-1表示左边,1表示右边(球在右边，若球在右边则-1表示右边,1表示左边)
-- 若pos为函数,dir最好也是函数
function disappearPos(pos, dir, i)
	if i == -1 then
		if type(pos) == "function" then
			if type(dir) == "function" then
				return function ()
					return pos() + Utils.Polar2Vector(15.0, Utils.Normalize(dir() + (ball.posY() > 0 and 1 or -1) * math.pi / 4))
				end
			else
				return function ()
					return pos() + Utils.Polar2Vector(15.0, Utils.Normalize(dir + (ball.posY() > 0 and 1 or -1) * math.pi / 4))
				end
			end
		else
			if type(dir) == "function" then
				return pos + Utils.Polar2Vector(15.0, Utils.Normalize(dir() + (ball.posY() > 0 and 1 or -1) * math.pi / 4))
			else
				return pos + Utils.Polar2Vector(15.0, Utils.Normalize(dir + (ball.posY() > 0 and 1 or -1) * math.pi / 4))
			end
		end
	elseif i == 1 then
		if type(pos) == "function" then
			if type(dir) == "function" then
				return function ()
					return pos() + Utils.Polar2Vector(15.0, Utils.Normalize(dir() + (ball.posY() > 0 and -1 or 1) * math.pi / 4))
				end
			else
				return function ()
					return pos() + Utils.Polar2Vector(15.0, Utils.Normalize(dir + (ball.posY() > 0 and -1 or 1) *  math.pi / 4))
				end
			end
		else
			if type(dir) =="function" then
				return pos + Utils.Polar2Vector(15.0, Utils.Normalize(dir() + (ball.posY() > 0 and -1 or 1) *  math.pi / 4))
			else
				return pos + Utils.Polar2Vector(15.0, Utils.Normalize(dir + (ball.posY() > 0 and -1 or 1) *  math.pi / 4))
			end
		end
	end
end