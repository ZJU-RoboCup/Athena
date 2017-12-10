module(..., package.seeall)

--~		Play中统一处理的参数（主要是开射门）
--~		1 ---> task, 2 ---> matchpos, 3---->kick, 4 ---->dir,
--~		5 ---->pre,  6 ---->kp,       7---->cp,   8 ---->flag
------------------------------------- 射门相关的skill ---------------------------------------

-------------6.27补的四个task，---------------
-- 强制打近角，只在定位球中使用
function touchNear()
	local idir = dir.compensate(ball.refAntiYPos(CGeoPoint:new_local(param.pitchLength/2.0,15)))
	local mexe, mpos = TouchKick{dir = idir}
	return {mexe, mpos, kick.flat, idir, pre.middle, kp.touch, cp.full, flag.nothing}
end

-- 强制打远角，只在定位球中使用
function touchFar()
	local idir = dir.compensate(ball.refSyntYPos(CGeoPoint:new_local(param.pitchLength/2.0,15)))
	local mexe, mpos = TouchKick{dir = idir}
	return {mexe, mpos, kick.flat, idir, pre.middle, kp.touch, cp.full, flag.nothing}
end

-- 强制打近角，只在定位球中使用
function chaseNear()
	local idir = ball.toFuncDir(ball.refAntiYPos(CGeoPoint:new_local(param.pitchLength/2.0,10)))
	local mexe, mpos = ChaseKick{dir = idir, pre = pre.middle}
	return {mexe, mpos, _, idir, pre.middle, kp.full, cp.full, flag.nothing}
end

-- 强制打远角，只在定位球中使用
function chaseFar()
	local idir = ball.toFuncDir(ball.refSyntYPos(CGeoPoint:new_local(param.pitchLength/2.0,10))) -- 大场的远角和近角不对
	local mexe, mpos = ChaseKick{dir = idir, pre = pre.middle}
	return {mexe, mpos, _, idir, pre.middle, kp.full, cp.full, flag.nothing}
end
--------------------------------

function touch(p, c)
	local idir

	-- 传入是否需要补偿的标志位，默认为需要补偿
	if c == nil or c == true then
		if p ~= nil then
			idir = dir.compensate(p)
		else
			idir = dir.shoot()
		end
	elseif c == false then
		if p ~= nil then
			idir = dir.nocompensation(p)
		else
			idir = dir.nocompensation(CGeoPoint:new_local(pitchLength / 2, 0))
		end
	end

	-- 根据射门位置做射门方向的固定，保证射门的坚定性
	-- by zhyaic 2013.6.27
	-- 可以在此处做，也可以在

	local mexe, mpos = TouchKick{dir = idir}
	-- local mexe, mpos = ShootBallV2{dir = idir}
	-- to test forcekick
	return {mexe, mpos, kick.flat, idir, pre.middle, kp.touch, cp.full, flag.force_kick}
end

function shootV2(p)
	local idir
	if p ~= nil then
		idir = dir.compensate(p)
	else
		idir = dir.shoot()
	end
	local mexe, mpos = ShootBall{dir = idir, pre = pre.low}
	--local mexe, mpos = ShootBallV2{dir = idir}
	return {mexe, mpos, kick.flat, idir, pre.middle, kp.full, cp.full, flag.nothing}
end

function kickAtEnemy(d)
	local mexe, mpos = ShootBall{dir = d}
	return {mexe, mpos, kick.flat, d, pre.middle, kp.full, cp.full, flag.nothing}
end

--d为点
function touchChip(d,power)
	local icp
	local idir
	if d == nil then
		idir = dir.shoot()
	else
		idir = player.toPointDir(d)
	end
	if type(power) == "function" then 
		icp = power
	elseif type(power) == "number" then
		icp = cp.specified(power)
	elseif power == "full" then
		icp = cp.full
	else
		icp = cp.toTarget(d)
	end
	local mexe, mpos = TouchKick{dir = idir}
	return {mexe, mpos, kick.chip, idir, pre.middle, kp.touch, icp , flag.force_kick}
end
-- function forceTouch()
-- 	local idir
-- 	if p ~= nil then
-- 		idir = dir.compensate(p)
-- 	else
-- 		idir = dir.shoot()
-- 	end
-- 	local mexe, mpos = TouchKick{dir = idir}
-- 	return {mexe, mpos, kick.flat, idir, pre.specified(100), kp.full, cp.full, flag.nothing}
-- end

function waitTouch(ipos,p2,style)
	local idir
	local iangel
	
	if p2 == nil then
		iangel = 0
	else
		iangel = p2
	end

	if style == "far" then
		idir = dir.compensate(ball.refAntiYPos(CGeoPoint:new_local(param.pitchLength/2.0,25)))
	elseif style == "near" then
		idir = dir.compensate(ball.refSyntYPos(CGeoPoint:new_local(param.pitchLength/2.0,25)))
	else
		idir = dir.shoot()
	end

	local mexe,mpos = WaitKick{ pos = ipos, dir = idir, angle = iangel}
	return {mexe, mpos, _, idir, pre.middle, kp.touch, cp.full, flag.force_kick}
end

--站在pt点，ps可为点或角色string或点的函数
--pt若为nil，则为当前点
function waitTouchNew(pt,ps)
	local idir

	if ps == nil then
		idir = dir.shoot()
	elseif type(ps) == "string" then
		idir = dir.compensate(player.passPos(ps))
	else
		idir = dir.compensate(ps)
	end
	
	local mexe,mpos = WaitKick2016{ pos = pt, dir = idir}
	return {mexe, mpos}
end

function waitChipNew(pt,ps)
	local idir
	if ps == nil then
		idir = dir.shoot()
	elseif type(ps) == "function" then
		idir = player.toPointDir(ps())
	elseif type(ps) == "string" then
		idir = player.toPlayerHeadDir(ps)
	else
		idir = player.toPointDir(ps)
	end
	local mexe,mpos = WaitChip2016{ pos = pt, dir = idir}
	return {mexe, mpos}
end

function touchBetweenPos(p1,p2)
	local mexe,mpos = TouchBetweenPos{ posA = p1, posB = p2, dir = dir.shoot()}
	return {mexe, mpos, kick.flat, dir.shoot(), pre.middle, kp.touch, cp.full, flag.force_kick}
end

function markingTouch(p,p1,p2,string)
	local iflags=0
	if string=="horizal" then
		iflags=flag.accurately
	end
	if string=="vertical" then
		iflags=0
	end
	local mexe,mpos = MarkingTouch{ areaNum = p, dir = dir.shoot(), leftUpPos = p1, rightDownPos = p2,flag=iflags}
	return {mexe, mpos, _, dir.shoot(), pre.middle, kp.touch, cp.full, flag.force_kick}
end

function forceStartRush()
	local mexe, mpos = ForceStartRush{dir = dir.chase}
	return {mexe, mpos, kick.flat, dir.chase, pre.low, kp.full, cp.full, flag.nothing}
end

function chase()
	local mexe, mpos = ChaseKick{dir = dir.chase}
	return {mexe, mpos, _, dir.chase, pre.low, kp.full, cp.full, flag.nothing}
end

function chaseNew()
	local mexe, mpos = ChaseKickV3{dir = dir.chase}
	return {mexe, mpos, _, dir.chase, pre.low, kp.specified(400), cp.full, flag.nothing}
end

function driftKick()
	local mexe, mpos = DriftKick{dir = dir.chase}
	return {mexe, mpos, kick.flat, dir.chase, pre.low, kp.full, cp.full, flag.nothing}
end

-- role: 截球后朝向的车的角色
function Intercept(role)
	if role == nil then
		local mexe, mpos = InterKick{dir = dir.chase,kickway=false}
		return {mexe, mpos, kick.flat, dir.chase, pre.low, kp.full, cp.full, flag.nothing}
	else
		local mexe, mpos = InterKick{dir = dir.ourPlayerToPlayer(role),kickway=false}
		return {mexe, mpos, kick.none, dir.ourPlayerToPlayer(role)}
	end
end

function InterNew()
	local mexe, mpos = InterKickV2{dir = dir.chase,kickway = false}
	return {mexe, mpos, _, dir.chase, pre.low, kp.full, cp.full, flag.nothing}
end

function InterV6()
	local mexe, mpos = InterKickV6{dir = dir.chase,kickway = false}
	return {mexe, mpos, _, dir.chase, pre.low, kp.full, cp.full, flag.nothing}
end

function InterceptNew(d,s,p)   --d为射门角度，s为射门方式，0表示不射，1表示平射，2表示挑射，p为射门力度
	local idir=dir.shoot()
	local ikick=kick.flat
	local ipower=kp.full
	local mexe,mpos
	if d~=nil then
		idir=dir.specified(d)
	end
	if s~=nil then
		if s==0 then
		ikick=kick.none
		elseif s==1 then
		ikick=kick.flat
		elseif s==2 then
		ikick=kick.chip
		end
	end
	if p~=nil then
		ipower=kp.specified(p)
	end
	if ikick==kick.chip then
	mexe,mpos=InterKick{ dir=idir, kickway = true}
	else
	mexe,mpos=InterKick{ dir=idir}
	end
	return{mexe, mpos, ikick, idir, pre.low, ipower, ipower, flag.nothing}
end

-- p1是截球目标点 默认为当前位置 p2是射门点或传球角色 p是力度 b是截球的缓冲距离 f是是否挑射 
function InterTouch(p1, p2, p, b, f)
	local ipos
	local idir
	local ipower
	local ibuffer
	local ikick = f and kick.chip() or kick.flat()
	local iTestMode = false
	if type(p1) == "function" then
		ipos = p1()
	elseif p1 ~= nil then
		ipos = p1
	else
		ipos = nil
	end

	if p2 ~= nil then
		if type(p2) == "string" then
			idir = dir.compensate(player.passPos(p2))
		elseif type(p2) == "userdata" then
			idir = dir.compensate(p2)
		elseif p2 == false then -- false时为了测试补偿数据
			iTestMode = true
			p2 = CGeoPoint:new_local(param.pitchLength/2,0)
			idir = dir.nocompensation(CGeoPoint:new_local(param.pitchLength/2,0))
		elseif p2 == true then
			idir = dir.shoot()
		end
	else
		idir = dir.shoot()
	end

	if p ~= nil then
		ipower = kp.specified(p)
	else
		if p2 ~= nil then
			ipower = kp.toTarget(p2)
		else
			ipower = kp.full()
		end
	end

	if b ~= nil then
		ibuffer = b
	else
		ibuffer = 9.33  -- 接球时默认后退距离
	end

	local mexe, mpos = InterceptTouch{pos = ipos, dir = idir, power = ipower, buffer = ibuffer, kick = ikick,testMode = iTestMode}
	return {mexe, mpos}
end

function shoot(f)
	local idir
	local iflags=flag.nothing
	idir = dir.shoot()
	if f == "safe" then
		iflags=flag.accurately
	end
	local mexe, mpos = ShootBall{dir = idir, pre = pre.high, flag=iflags}
	return {mexe, mpos}
end

function shootLow(d)
	local idir
	if d == nil then
		idir = dir.shoot()
	else
		idir = d
	end
	local mexe, mpos = ShootBall{dir = idir, pre = pre.low}
	return {mexe, mpos}
end

function slowShoot(d)
	local mexe, mpos = ChaseKick{dir = dir.chase}
	return {mexe, mpos, kick.flat, dir.chase, pre.low, kp.full, cp.full, flag.nothing}
end

function fakeShoot(p)
	local mexe, mpos = GetBall{dir = ball.toPointDir(p)}
	return {mexe, mpos, kick.flat, ball.toPointDir(p), pre.specified(2), kp.specified(400), cp.full, flag.nothing}
end

function penaltyV1()
	local mexe, mpos = GoAndTurnKick{pos = ball.pos,  dir = dir.chase, pre = pre.specified(5)}
	return {mexe, mpos, kick.flat, dir.chase, pre.specified(5), kp.full, cp.full, flag.nothing}
end

function penaltyKick()
	local mexe,mpos = PenaltyKick2013{}
	return {mexe, mpos, kick.flat, dir.chase, pre.specified(5), kp.full, cp.full, flag.nothing}
end

function penaltyKick2017V1()
	local mexe,mpos = PenaltyKick2017V1{}
	return {mexe, mpos, kick.flat, dir.chase, pre.specified(5), kp.full, cp.full, flag.nothing}
end

function penaltyKick2017V2()
	local mexe,mpos = PenaltyKick2017V2{}
	return {mexe, mpos, kick.flat, dir.chase, pre.specified(5), kp.full, cp.full, flag.nothing}
end

function penaltyTurn(d, k)
	if k == true then
		k = kick.flat
	else
		k = kick.none
	end
	local mexe, mpos = SpeedInRobot{speedW = -4.8, speedX = 70} -- 点球甩飞改 speedW
	return {mexe, mpos, k, d, pre.specified(100), kp.full, cp.full, flag.nothing}
end

function protectTurn()
	local mexe, mpos = ProtectTurn{fDir = dir.shoot(),rotV=7,kPower=1000}
	return {mexe, mpos}
end
--GTYGTYGTY

function dribbleTurnShoot(ps)
	local idir
	if ps == nil then
		idir = dir.shoot()
	elseif type(ps) == "function" then
		idir = player.toPointDir(ps)
	elseif type(ps) == "string" then
		idir = player.toPlayerHeadDir(ps)
	else
		idir = player.toPointDir(ps)
	end
	local mexe, mpos = DribbleTurnKick{fDir = idir, rotV=4,kPower=1200}
	return {mexe, mpos}
end

function penaltyTurnShoot(d)
	local mexe, mpos = DribbleTurnKick{fDir = d,rotV=4,kPower=1200}
	return {mexe, mpos}
end

function dribbleTurnPass(p,c)
	local  fdir
	if type(p)=="string" then
		fdir=player.toPlayerHeadDir(p)
	else
		fdir = player.toPointDir(p)
	end

	local ipower
	if c == nil then
		ipower = kp.toTarget(p)
	else
		ipower = kp.specified(c)
	end

	local mexe, mpos = DribbleTurnKick{fDir = fdir,rotV=3,kPower=ipower}
	return {mexe, mpos}
end

function directShoot()
	local mexe, mpos = ChaseKick{dir = dir.chase}
	return {mexe, mpos, kick.flat, dir.chase, pre.low, kp.full, cp.full, flag.force_kick}
end

function penaltyChase(d)
	local mexe, mpos = ChaseKick{dir = d, flag = flag.dribbling}
	return {mexe, mpos, kick.flat, d, pre.middle, kp.specified(700), cp.full, flag.force_kick } --penalty如果超速改这里 by.NaN
end


----------------------------------------------------------------------------------------------

------------------------------------ 传球相关的skill ---------------------------------------
function receivePass(p,c)
	local idir
	if type(p) == "string" then
		idir = player.toPlayerHeadDir(p)
	elseif type(p) == "function" then
		idir = player.toPointDir(p())
	else
		idir = player.toPointDir(p)
	end

	local ipower
	if c == nil then
		ipower = kp.toTarget(p)
	else
		ipower = kp.specified(c)
	end
	local mexe, mpos = ReceivePass{dir = idir}
	return {mexe, mpos, kick.flat, idir, pre.high, ipower, cp.full, flag.nothing}
end

function receiveBall(p)
	local idir
	if type(p) == "string" then
		idir = player.toPlayerHeadDir(p)
	elseif type(p) == "function" then
		idir = p
	else
		idir = player.toPointDir(p)
	end
	local mexe, mpos = ReceivePass{dir = idir}
	return {mexe, mpos, kick.flat, idir, pre.high, ipower, cp.full, flag.nothing}
end

function receiveChip(p, c, pordir)
	local idir,ipower

	if type(p) == "string" then
		idir = player.toPlayerHeadDir(p)
	elseif type(p) == "userdata" then
		idir = player.toPointDir(p)
	elseif type(p) == "function" then
		if pordir == nil or pordir == false then
			idir = p
		else
			idir = player.toPointDir(p())
		end
	else
		print("ERROR IN receiveChip task.");
	end

	if c == nil then
		ipower = kp.toTarget(p)
	else
		ipower = kp.specified(c)
	end

	local mexe, mpos = ReceivePass{dir = idir}
	return {mexe, mpos, kick.chip, idir, pre.specified(3.5), kp.full, ipower, flag.nothing}
end

function receiveShoot(d)
	local idir
	if d ~=nil then
		idir = d
	else
		idir = dir.shoot()
	end
	local mexe, mpos = ReceivePass{dir = idir}
	return {mexe, mpos, kick.flat, idir, pre.high, kp.full, cp.full, flag.nothing}
end

-- f为false代表p为函数,并且返回值为userdata,p不能有参数
-- f为nil是为了兼容以前的情况
function chipPass(p, c, f, anti)
	local idir
	local ipower
	if type(p) == "string" then
		idir = player.toPlayerDir(p)
	elseif type(p) == "function" then
		if f == nil or f == true then
			idir = p
		elseif anti == false then 
			idir = function (role)
				return (p() - player.pos(role)):dir()
			end
		else
			idir = ball.antiYDir(p)
		end
	else
		if anti == false then
			idir = function (role)
				if type(p) == "userdata" then
					return (p - player.pos(role)):dir()
				end
			end
		else
			idir = player.antiYDir(p)
		end
	end

	if c == nil then
		if type(p) == "string" then
			ipower = cp.toPlayer(p)
		elseif type(p) == "userdata" then
			ipower = cp.toTarget(ball.antiYPos(p))
		elseif type(p) == "function" then
			if f == false or f == nil then
				ipower = cp.toTarget(ball.antiYPos(p))
			end
		end
	elseif type(c) == "number" then
		ipower = cp.specified(c)
	else
		ipower = c
	end

	local mexe, mpos = ChaseKick{dir = idir, flag = flag.dribbling}
	return {mexe, mpos, kick.chip, idir, pre.low, kp.specified(0), ipower, flag.nothing}
end

--添加接口 hzy
function flatPass(roleorp, power)
	local idir
	local ipower

	if type(roleorp) == "string" then
		idir = ball.toPlayerHeadDir(roleorp)
	elseif type(roleorp) == "function" then
		idir = player.toPointDir(roleorp())
	else
		idir = player.toPointDir(roleorp)
	end

	if power == nil then
		ipower = kp.toTarget(roleorp)
	elseif type(power) == "string" then
		-- ipower = getluadata:getKickParamPower()
		-- print("is function")
		return function ()
			local ppower = getluadata:getKickParamPower()
			ipower = kp.specified( ppower )
			local mexe, mpos = ChaseKick{ pos = ball.pos, dir = idir}
			return {mexe, mpos, kick.flat, idir, pre.low, ipower, cp.full, flag.nothing}
		end	
	else
		ipower = kp.specified(power)
	end
	
	local mexe, mpos = ChaseKick{ pos = ball.pos, dir = idir}
	return {mexe, mpos, kick.flat, idir, pre.low, ipower, cp.full, flag.nothing}
end

-- role为接球车,也可以为某一个点
function pass(role,power,isPos)
	local idir
	local ipower

	if type(role) == "string" then
		idir = ball.toPlayerHeadDir(role)
	elseif type(role) == "function" then
		if isPos then
			idir = ball.toPointDir(role)
		else
			idir = ball.toFuncDir(role)
		end
	elseif type(role) == "userdata" then
		idir = player.antiYDir(role)
	end
	if power == nil then
		ipower = kp.toTarget(role)
	else
		ipower = kp.specified(power)
	end

	local mexe, mpos = ChaseKick{ pos = ball.pos, dir = idir}
	return {mexe, mpos, kick.flat, idir, pre.low, ipower, cp.toPlayer(role), flag.nothing}
end

function passToDir(p, c)
	--local mexe, mpos = ChaseKick{ pos = ball.pos, dir = p}
	local mexe, mpos = ChaseKick{ pos = ball.pos, dir = p}
	return {mexe, mpos, kick.flat, p, pre.middle, kp.specified(c), cp.full, flag.nothing}
end

function passToPos(p,c)
	local idir=player.toPointDir(p)
	local ipower
	if type(c)== "number" and c ~= nil then
		ipower=kp.specified(c)
	else
		ipower=kp.toTargetNormalPlay(p)
	end
	local mexe, mpos = ChaseKickV2{ pos = ball.pos, dir = idir}
	return {mexe, mpos, kick.flat, idir, pre.middle, ipower, cp.full, flag.nothing}
end

function compensatePass(role,power)
	local ipower
	if power == nil then
		ipower = kp.toTarget(role)
	else
		ipower = kp.specified(power)
	end
	local mexe, mpos = ChaseKick{ pos = ball.pos, dir = player.toPlayerHeadDir(role)}
	return {mexe, mpos, kick.flat, player.toPlayerHeadDir(role), pre.middle, ipower, cp.full, flag.nothing}
end

-- role 为接球车/接球点
function goAndTurnKick(role, power, icircle, a, isPos) -- 2014-03-28 added by yys, 增加转动圈数参数,并且第一个参数可以直接传入弧度
	local idir
	local ipower
	if type(role) == "number" then  -- 这里idir必须为函数，因为play.lua里面函数调用
		idir = function()
		  return role
		end
	elseif type(role) == "string" then
		idir = ball.toPlayerHeadDir(role)
	elseif type(role) == "function" then
		if isPos == true then
			idir = ball.toPointDir(role)
		else
			idir = ball.toFuncDir(role)
		end
	elseif type(role) == "userdata" then
		idir = player.antiYDir(role)
	end

	if power == nil then
		ipower = kp.toTarget(role)
	else
		ipower = kp.specified(power)
	end

	if icircle == nil then
		icircle = 0
	end

	local mexe, mpos = GoAndTurnKickV3{ pos = ball.pos, dir = idir, pre = pre.high, circle = icircle, acc = a}
	return {mexe, mpos, kick.flat, idir, pre.specified(4), ipower, cp.full, flag.nothing}
end

function goAndTurnKickByGetBall(role, power, icircle, a)
	local idir
	local ipower
	if type(role) == "number" then  -- 这里idir必须为函数，因为play.lua里面函数调用
		idir = function()
		  return role
		end
	elseif type(role) == "string" then
		idir = ball.toPlayerHeadDir(role)
	elseif type(role) == "function" then
		idir = ball.toFuncDir(role)
	elseif type(role) == "userdata" then
		idir = player.antiYDir(role)
	elseif role == nil then
		idir = dir.shoot()
	end

	if power == nil then
		if role == nil then
			ipower = kp.specified(800)
		else
			ipower = kp.toTarget(role)
		end
	else
		ipower = kp.specified(power)
	end

	if icircle == nil then
		icircle = 0
	end

	local mexe, mpos = GoAndTurnKickV4{ pos = ball.pos, dir = idir, acc = a}
	return {mexe, mpos, kick.flat, idir, pre.middle, ipower, cp.full, flag.nothing}
end

-- role 为接球车/接球点
function goAndTurnKickWithoutAnti(role, power, icircle, a)
	local idir
	local ipower
	if type(role) == "userdata" then
		idir = player.toPointDir(role)
	end

	if power == nil then
		ipower = kp.toTarget(role)
	else
		ipower = kp.specified(power)
	end

	if icircle == nil then
		icircle = 0
	end

	local mexe, mpos = GoAndTurnKickV3{ pos = ball.pos, dir = idir, pre = pre.high, circle = icircle, acc = a}
	return {mexe, mpos, kick.flat, idir, pre.high, ipower, cp.full, flag.nothing}
end

-- role 为接球车/接球点
function goAndTurnChip(role, power, icircle, a) -- 2014-03-28 added by yys, 增加转动圈数参数,并且第一个参数可以直接传入弧度
	local idir
	local ipower
	if type(role) == "number" then  -- 这里idir必须为函数，因为play.lua里面函数调用
		idir = function()
		  return role
		end
	elseif type(role) == "string" then
		idir = ball.toPlayerHeadDir(role)
	elseif type(role) == "function" then
		idir = ball.toFuncDir(role)
	elseif type(role) == "userdata" then
		idir = player.antiYDir(role)
	end

	if power == nil then
		ipower = cp.toTarget(role)
	else
		ipower = cp.specified(power)
	end

	if icircle == nil then
		icircle = 0
	end

	local mexe, mpos = GoAndTurnKickV3{ pos = ball.pos, dir = idir, pre = pre.high, circle = icircle, acc = a}
	return {mexe, mpos, kick.chip, idir, pre.middle, kp.full, ipower, flag.nothing}
end

function goAndTurnKickOld(role, power)
	local idir
	local ipower
	if type(role) == "string" then
		idir = ball.toPlayerHeadDir(role)
	elseif type(role) == "function" then
		idir = ball.toFuncDir(role)
	elseif type(role) == "userdata" then
		idir = player.antiYDir(role)
	end

	if power == nil then
		ipower = kp.toTarget(role)
	else
		ipower = kp.specified(power)
	end

	-- local mexe, mpos = GoAndTurnKickV3{ pos = ball.pos, dir = idir, pre = pre.high, circle = 0}
	local mexe, mpos = GoAndTurnKick{ pos = ball.pos, dir = idir, pre = pre.high}
	return {mexe, mpos, kick.flat, idir, pre.high, ipower, cp.full, flag.nothing}
end


function goAndTurnKickQuick(role, power, ktype) -- 2015/04/23 at JP_Open by.YuN
	local idir
	local ipower

	if type(role) == "number" then  -- 这里idir必须为函数，因为play.lua里面函数调用
		idir = function()  return role end
	elseif type(role) == "string" then
		idir = ball.toPlayerHeadDir(role)
	elseif type(role) == "function" then
		idir = ball.toFuncDir(role)
	elseif type(role) == "userdata" then
		idir = player.toPointDir(role)
	end

	if power == nil then
		print("role:"..role)
		ipower = kp.toTarget(role)
	else
		ipower = kp.specified(power)
	end

	itype=0
	if type(ktype)=="string" then
		if ktype=="flat" then
			itype=0
		elseif ktype=="chip" then
			itype=1
		end
	end

	local mexe, mpos = GoAndTurnKick{ pos = ball.pos, dir = idir, type=itype, power=ipower}
	return {mexe, mpos}
end


function dribbleTurn(finalDir, adjustPre)
	local idir
	if type(finalDir) == "number" then
		idir = finalDir
	elseif type(finalDir) == "function" then
		idir = finalDir()
	end

	local ipre
	if adjustPre == nil then
		ipre = math.pi / 90
	else
		ipre = adjustPre
	end

	local mexe, mpos = DribbleTurn{ fDir = idir, pre = ipre }
	return {mexe, mpos}
end

function circlePass(role, power, c)
	local idir
	local ipower
	if type(role) == "string" then
		idir = player.toPlayerDir(role)
	elseif type(role) == "function" then
		idir = ball.toFuncDir(role)
	elseif type(role) == "userdata" then
		idir = player.antiYDir(role)
	end

	if power == nil then
		ipower = kp.toTarget(role)
	else
		ipower = kp.specified(power)
	end

	if c == nil then
		c = 2
	end
	-- local mexe, mpos = GoAndTurnKickV3{ pos = ball.pos, dir = idir, pre = pre.high}
	local mexe, mpos = GoAndTurnKickV3{ pos = ball.pos, dir = idir, pre = pre.high, circle = c}
	return {mexe, mpos, kick.flat, idir, pre.high, ipower, cp.full, flag.nothing}
end

function goAndTurn()
	local mexe, mpos = GoAndTurn{}
	return {mexe, mpos}
end

-- 利用车的轮子进行传球
function assPass(x, y)
	local ispeedX = function()
		return x*(-1)
	end

	local ispeedY = function()
		return y*ball.antiY()
	end

	local mexe, mpos = OpenSpeed{ speedX = ispeedX, speedY = ispeedY }
	return {mexe, mpos}
end

function touchPass(p,power)
	local idir
	local ikp
	if p == nil then
		idir = dir.shoot()
	elseif type(p) == "string" then
		idir = player.toPlayerHeadDir(p)
	else
		idir = dir.compensate(p)
	end

	if power == nil then
		if type(p) == "string" then
			ikp = kp.toTarget(player.pos(p))
		else
			ikp = kp.toTarget(p)
		end
	elseif type(power) == "function" then
		ikp = power
	else
		ikp = kp.specified(power)
	end

	local mexe, mpos = TouchKick{dir = idir}
	--local mexe, mpos = ShootBallV2{dir = idir}
	return {mexe, mpos, kick.flat, idir, pre.middle, ikp , cp.full, flag.nothing}
end
----------------------------------------------------------------------------------------------

------------------------------------ 跑位相关的skill ---------------------------------------
-- p为最终要踢向的点
function fakeStand(p)
	local mexe, mpos = SmartGoto{pos = pos.fakeDown(p), dir = dir.fakeDown(p), flag = flag.slowly}
	return {mexe, mpos}
end

--~ p为要走的点,d默认为射门朝向
function goSpeciPos(p, d, f, a) -- 2014-03-26 增加a(加速度参数)
	local idir
	local iflag
	if d ~= nil then
		idir = d
	else
		idir = dir.shoot()
	end

	if f ~= nil then
		iflag = f
	else
		iflag = 0
	end

	local mexe, mpos = SmartGoto{pos = p, dir = idir, flag = iflag, acc = a}
	return {mexe, mpos}
end

function goSimplePos(p, d, f)
	local idir
	if d ~= nil then
		idir = d
	else
		idir = dir.shoot()
	end

	if f ~= nil then
		iflag = f
	else
		iflag = 0
	end

	local mexe, mpos = SimpleGoto{pos = p, dir = idir, flag = iflag}
	return {mexe, mpos}
end

function goSimplePosFaceToBall(p, d, f)
	local idir
	if d ~= nil then
		idir = d
	else
	    idir = dir.playerToBall
	end
	if f ~= nil then
		iflag = f
	else
		iflag = 0
	end

	local mexe, mpos = SimpleGotoFaceBall{pos = p, dir = idir, flag = iflag}
	return {mexe, mpos}
end

-- role为传球车
function goBestShootPos(role)
	local mexe, mpos = SmartGoto{ pos = pos.bestShoot, dir = player.toShootOrRobot(role)}
	return {mexe, mpos}
end

-- role为传球车
function goPassPos(role, f)
	local mexe, mpos = SmartGoto{ pos = player.passPos(role), dir = player.toShootOrRobot(role), flag = f}
	return {mexe, mpos}
end

function goFirstPassPos(role)
	local mexe, mpos = GoAvoidShootLine{ pos = ball.firstPassPos(), dir = player.toShootOrRobot(role),sender=role}
	return {mexe, mpos}
end

function goSecondPassPos(role)
	local mexe, mpos = GoAvoidShootLine{ pos = ball.secondPassPos(), dir = player.toShootOrRobot(role),sender=role}
	return {mexe, mpos}
end

function goWaitAdvancePos(role)
	local mexe, mpos = SmartGoto{ pos = player.waitAdvancePos(role), dir = dir.shoot()}
	return {mexe, mpos}
end

function runMultiPos(p, c, d, idir, a)
	if c == nil then
		c = false
	end

	if d == nil then
		d = 20
	end

	if idir == nil then
		idir = dir.shoot()
	end

	local mexe, mpos = RunMultiPos{ pos = p, close = c, dir = idir, flag = flag.not_avoid_our_vehicle, dist = d, acc = a}
	return {mexe, mpos}
end

function gotoMatchPos(p, d)
	local idir
	if d ~= nil then
		idir = d
	else
		idir = dir.shoot()
	end

	local mexe, mpos = GotoMatchPos{pos = p, dir = idir}
	return {mexe, mpos}
end

--~ p为要走的点,d默认为射门朝向
function goCmuRush(p, d, a, f, r)
	local idir
	if d ~= nil then
		idir = d
	else
		idir = dir.shoot()
	end
	local mexe, mpos = GoCmuRush{pos = p, dir = idir, acc = a, flag = f,rec = r}
	return {mexe, mpos}
end

function noneZeroRush(p, d, a, f, r, v)
	local idir
	if d ~= nil then
		idir = d
	else
		idir = dir.shoot()
	end
	local mexe, mpos = NoneZeroRush{pos = p, dir = idir, acc = a, flag = f, rec = r, vel = v}
	return {mexe, mpos}
end

function goTestEmptyKick(p, d, a)
	local idir
	if d ~= nil then
		idir = d
	else
		idir = dir.shoot()
	end

	local mexe, mpos = GoCmuRush{pos = p, dir = idir, acc = a}
	return {mexe, mpos, kick.flat, idir, pre.middle, kp.specified(9999),cp.specified(9999), flag.force_kick}
end



function goTouchPos(p, t)
	local mexe, mpos = GoCmuRush{pos = p, dir = dir.evaluateTouch(t)}
	return {mexe, mpos}
end

----------------------------------------------------------------------------------------------

------------------------------------ 防守相关的skill ---------------------------------------

-- 传入两个点，p1是左上点，p2是右下点，
function markingField(p1, p2, s)
	local mexe, mpos = MarkingField{ posA = p1, posB = p2, style = s, dir = dir.shoot()}
	return {mexe, mpos, kick.flat, dir.shoot(), pre.middle, kp.full, cp.full, flag.force_kick}
end

function defendDefault(p)
	local mexe, mpos = GotoMatchPos{ pos = pos.defaultDefPos(p), dir = player.toBallDir, acc = 1200,flag = flag.dodge_referee_area}
	return {mexe, mpos}
end

function defendFront(p)
	local mexe, mpos = GotoMatchPos{ pos = pos.defendFrontPos(p), dir = player.toBallDir, acc = 1200}
	return {mexe, mpos}
end

function defendHead()
	local mexe, mpos = GotoMatchPos{ pos = pos.defendHeadPos, dir = player.toBallDir, acc = 800,flag = flag.not_avoid_their_vehicle}
	return {mexe, mpos}
end

function defendKick(p1,p2,p3,p4)
	local mexe, mpos = SmartGoto{ acc = 800, pos = pos.defendKickPos(p1,p2,p3,p4), dir = player.toBallDir}
	return {mexe, mpos}
end

function defendKickDir(p1,p2,p3,p4)
	local mexe, mpos = SmartGoto{ acc = 800, pos = pos.defendKickPos(p1,p2,p3,p4), dir = player.toTheirGoal}
	return {mexe, mpos}
end

function defendOneKick(p1,p2,p3,p4,p5,p6)
	local mexe,mpos =  SmartGoto{ acc = 800, pos = pos.oneKickDefPos(p1,p2,p3,p4,p5,p6), dir = player.toBallDir}
	return {mexe, mpos}
end

function defendTwoKick(p1,p2,p3,p4,p5,p6)
	local mexe,mpos =  SmartGoto{ acc = 800, pos = pos.anotherKickDefPos(p1,p2,p3,p4,p5,p6), dir = player.toBallDir}
	return {mexe, mpos}
end

function leftBack()
	local mexe, mpos = GotoMatchPos{ method = 1, acc = 1000, pos = pos.leftBackPos, dir = dir.backSmartGotoDir, srole = "leftBack", flag = bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle)}
	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(9999), bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle)}
end

function rightBack()
	local mexe, mpos = GotoMatchPos{ method = 1, acc = 1000, pos = pos.rightBackPos, dir = dir.backSmartGotoDir, srole = "rightBack", flag = bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle)}
	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(9999), bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle)}
end

function leftBack4Stop()
	local STOP_FLAG = bit:_or(flag.slowly, flag.dodge_ball)
	local STOP_NO_DODGE_SELF = bit:_or(STOP_FLAG, flag.not_avoid_our_vehicle)
	local STOP_DSS = bit:_or(STOP_NO_DODGE_SELF, flag.allow_dss)
	local mexe, mpos = GotoMatchPos{ pos = pos.leftBackPos, dir = dir.backSmartGotoDir, flag = STOP_DSS}
	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(9999), STOP_DSS}
end

function rightBack4Stop()
	local STOP_FLAG = bit:_or(flag.slowly, flag.dodge_ball)
	local STOP_NO_DODGE_SELF = bit:_or(STOP_FLAG, flag.not_avoid_our_vehicle)
	local STOP_DSS = bit:_or(STOP_NO_DODGE_SELF, flag.allow_dss)
	local mexe, mpos = GotoMatchPos{ pos = pos.rightBackPos, dir = dir.backSmartGotoDir, flag = STOP_DSS}
	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(9999), STOP_DSS}
end

function singleBack4Stop()
	local STOP_FLAG = bit:_or(flag.slowly, flag.dodge_ball)
	local STOP_DSS = bit:_or(STOP_FLAG, flag.allow_dss)
	local mexe, mpos = GotoMatchPos{ pos = pos.singleBackPos, dir = dir.backSmartGotoDir, flag = STOP_DSS}
	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(9999), STOP_DSS}
end

-- 此处的车为射门的车，如果不传则不避射门线
function defendMiddle(role)
	local mflag
	if role == nil then
		mflag = 0--flag.not_avoid_our_vehicle
	else
		mflag = flag.avoid_shoot_line--bit:_or(flag.avoid_shoot_line, flag.not_avoid_our_vehicle)
	end
	mflag=bit:_or(flag.allow_dss,mflag)
	local mexe, mpos = GotoMatchPos{ pos = pos.defendMiddlePos,dir = dir.backSmartGotoDir , srole = "defendMiddle", flag = mflag, sender = role}
	return {mexe, mpos, kick.flat, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(9999), mflag}
end

function singleBack()
	local mexe, mpos = GotoMatchPos{ method = 1, acc = 1000, pos = pos.singleBackPos, dir = dir.backSmartGotoDir, srole = "singleBack",bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle)}
	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(9999), bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle)}
end

function sideBack()
	local mexe, mpos = GotoMatchPos{ pos = pos.sideBackPos, dir = dir.sideBackDir, srole = "sideBack",bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle)}
	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(9999), bit:_or(flag.not_avoid_our_vehicle,flag.not_avoid_their_vehicle)}
end

function goalie2013()
	local mexe, mpos = Goalie2013{ flag = flag.not_avoid_our_vehicle}
	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(9999), flag.not_avoid_our_vehicle}
end

function goalie()
	local mexe, mpos = Goalie2015{ flag = flag.not_avoid_our_vehicle}
	return {mexe, mpos, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(9999), flag.not_avoid_our_vehicle}
end

function goalie2015V2()
	local mexe, mpos = Goalie2015V2{ flag = flag.not_avoid_our_vehicle}
	return {mexe, mpos}--, kick.chip, dir.defendBackClear(), pre.fieldDefender(), kp.specified(9999),cp.specified(9999), flag.not_avoid_our_vehicle}
end

function penaltyGoalie()
	local mexe, mpos = PenaltyGoalie{}
	return {mexe, mpos}
end

function penaltyGoalieV2()
	local mexe, mpos = PenaltyGoalieV2{}
	return {mexe, mpos}
end

function penaltyGoalie2017V1()
	local mexe, mpos = PenaltyGoalie2017V1{}
	return {mexe, mpos}
end

-- 用来盯人的skill,其中p为优先级
function marking(p)
	local mexe, mpos = Marking{pri = p}
	return {mexe, mpos}
end

-- 正常比赛强行绕前盯人
function markingNormal(p)
	local mexe, mpos = Marking {pri = p, flag = flag.turn_around_front}
	return {mexe, mpos}
end

function  markingDir(p,p0)
	local mexe, mpos = Marking{pri = p,dir = p0}
	return {mexe, mpos}
end

-- 强行绕前盯人
-- by zhyaic 2014.7.4
function markingFront(p)
	local mexe, mpos = Marking{pri = p, front = true, dir = dir.shoot()}
	return {mexe, mpos}
end

function markingXFirst()
	local mexe, mpos = MarkingXFirst{}
	return {mexe, mpos}
end

function blocking(p)
	local mexe, mpos = Blocking{pri = p}
	return {mexe, mpos}
end

----------------------------------------------------------------------------------------------

----------------------------------------- 其他动作 --------------------------------------------
--jyl 2017.11.8 add for NormalPlayPass
function universal()
	return function()
		local decision = world:getAttackDecision(vision:Cycle(),gRoleNum["Leader"])
		--print("universal : ",decision)
		local PassPos = pos.getShootPos()
		if decision == "LightKick" then
			debugEngine:gui_debug_msg(CGeoPoint:new_local(-470, 200), "flatPass")
			return flatPass(CGeoPoint:new_local(450, 0),80)
		elseif decision == "LightChip" then
			debugEngine:gui_debug_msg(CGeoPoint:new_local(-470, 200), "chipPass")
			return chipPass(CGeoPoint:new_local(450, 0),30)
		elseif decision == "GetBall" then
			debugEngine:gui_debug_msg(CGeoPoint:new_local(-470, 200), "advance")
			return advance()
		elseif decision == "Pass" then
			debugEngine:gui_debug_msg(CGeoPoint:new_local(-470, 200), "flatPass")
			return flatPass(PassPos())--return flatPass(PassPos)
		elseif decision == "Receive" or decision == "Compute" then
			debugEngine:gui_debug_msg(CGeoPoint:new_local(-470, 200), "receivePass")
			return receivePass(ball.pos(),0)
		elseif decision == "ReceivePass" then
			debugEngine:gui_debug_msg(CGeoPoint:new_local(-470, 200), "receivePass")
			return receivePass(PassPos)
		elseif decision == "ReceiveShoot" then
			debugEngine:gui_debug_msg(CGeoPoint:new_local(-470, 200), "receiveShoot")
			return  receiveShoot()
		elseif decision == "WaitTouch" then
			debugEngine:gui_debug_msg(CGeoPoint:new_local(-470, 200), "WaitTouch")
			return waitTouchNew()
		else
			debugEngine:gui_debug_msg(CGeoPoint:new_local(-470, 200), "advance")
			return advance()
		end
	end
end

function advance(role)
	local itandem = nil
	if type(role) == "string" then
		itandem = role
	elseif type(role) == "function" then
		itandem = role()
	end
	local mexe, mpos = AdvanceBall{pos = pos.advance, srole = "advancer", tandem = itandem}
	return {mexe, mpos}
end

function protectBall()
	local mexe, mpos = ProtectBall{pos = pos.advance,flag = flag.dodge_ball}
	return {mexe, mpos}
end

function getBall(p)
	-- local mexe, mpos = GetBall{ pos = pos.backBall(p), dir = dir.backBall(p)}
	local mexe, mpos = GetBall{ pos = ball.backPos(p), dir = ball.backDir(p)}
	return {mexe, mpos}
end

-- 注意，此处的点是用来做匹配的
-- 最终的StaticGetBall中调用的是拿球，离球4cm
-- anti若为false,则不将点进行反向
-- f为传入的flag
function staticGetBall(p, anti, f)
	-- local mexe, mpos = StaticGetBall{ pos = pos.backBall(p), dir = dir.backBall(p)}
	local ipos
	local idir
	if p == nil then
		ipos = pos.specified(CGeoPoint:new_local(450,0))
	elseif type(p) == "string" then
		ipos = player.pos(p)
	elseif type(p) == "userdata" then
		ipos = ball.backPos(p, _, _, anti)
	end
	
	idir = ball.backDir(p, anti)

	local mexe, mpos = StaticGetBall{ pos = ipos, dir = idir, flag = f}
	return {mexe, mpos}
end

-- p为朝向，如果p传的是pos的话，不需要根据ball.antiY()进行反算
function goBackBall(p, d)
	local mexe, mpos = GoCmuRush{ pos = ball.backPos(p, d, 0), dir = ball.backDir(p), flag = flag.dodge_ball}
	return {mexe, mpos}
end

function goSupportPos(role)
	local mexe, mpos = GoAvoidShootLine{pos = pos.getSupportPos(role), dir = dir.shoot(),sender=role}
	return {mexe, mpos}
end

function goSupportPosNew(role1,role2)
	local mexe, mpos = GoCmuRush{pos = pos.getSupportPosNew(role1,role2), dir = dir.shoot()}
	return {mexe, mpos}
end

function goAssistPos(role1,role2)
	local mexe, mpos = GoCmuRush{pos = pos.getAssistPoint(role1,role2), dir = dir.shoot()}
	return {mexe, mpos}
end

function stop()
	local mexe, mpos = Stop()
	return {mexe, mpos}
end

-- f为false代表p为函数,并且返回值为userdata,p不能有参数
-- f为nil是为了兼容以前的情况
function slowGetBall(p, f, anti)
	local idir
	if type(p) == "string" then
		idir = player.toPlayerDir(p)
	elseif type(p) == "function" then
		if f == nil or f == true then
			idir = p
		elseif anti == false then 
			idir = function (role)
				return (p() - player.pos(role)):dir()
			end
		else
			idir = ball.antiYDir(p)
		end
	else
		if anti == false then
			idir = function (role)
				if type(p) == "userdata" then
					return (p - player.pos(role)):dir()
				end
			end
		else
			idir = ball.antiYDir(p)
		end
	end

	local mexe, mpos = SlowGetBall{ dir = idir}
	return {mexe, mpos}
end


function slowGetBallForKick(d)
	local mexe, mpos = SlowGetBall{ dir = d}
	return {mexe, mpos}
end

function tandem(role)
	local mrole = "Leader"
	if role==nil then
		mrole ="Leader"
	elseif type(role) == "string" then
		mrole =role
	end
	local mexe, mpos = GoAvoidShootLine{ pos = pos.getTandemPos(mrole), dir = dir.getTandemDir(mrole),sender=mrole}
	return {mexe, mpos}
end

function goAroundRobot(role, r)
	local tmpPos = function ()
		return player.pos(role)
	end
	local mexe, mpos = GoAroundRobot{ pos = tmpPos, dir = dir.shoot(), radius = r}
	return {mexe, mpos}
end

function continue()
	return {["name"] = "continue"}
end
----------------------------------------------------------------------------------------------

------------------------------------ 测试相关的skill ---------------------------------------
function testChip()
	local mexe, mpos = ChaseKick{dir = dir.shoot(),flag = flag.dribbling}
	return {mexe, mpos, kick.chip, dir.shoot(), pre.specified(9), kp.full, cp.full, flag.dribbling}
end

function TestGoSpeciPos(p,d)
	local mexe, mpos = SmartGoto{pos = p, dir = d}
	return {mexe, mpos}
end

-- 传入的目标点和目标朝向
function testLearn(p, dir)
	local mexe, mpos = RoundToPenalty{ pos = p, dir = dir}
	return {mexe, mpos}
end

function testRound(role)
	local mexe, mpos = RoundBallTest{ pos = ball.backPos(role,13), dir = ball.backDir(role)}
	return {mexe, mpos, kick.auto(role), player.toPlayerDir(role), pre.middle, kp.toTarget(role), cp.toPlayer(role), flag.nothing}
end

function testCircleBall(role)
	local mexe, mpos
	if IS_SIMULATION then
		mexe, mpos = GoAndTurnKick{ pos = ball.pos, dir = player.toPlayerDir(role), pre = pre.low}
	else
		mexe, mpos = TestCircleBall{ pos = ball.pos, dir = player.toPlayerDir(role), pre = pre.low}
	end
	--local mexe, mpos = TestCircleBall{ pos = ball.pos, dir = player.toPlayerDir(role), pre = pre.low}
	return {mexe, mpos, kick.flat, player.toPlayerDir(role), pre.low, kp.specified(800), cp.toPlayer(role), flag.nothing}
end

function goAndTurnKickV3(role)
	local mexe, mpos = GoAndTurnKickV3{ pos = ball.pos, dir = player.toPlayerDir(role), pre = pre.middle}
	return {mexe, mpos, kick.flat, player.toPlayerDir(role), pre.middle, kp.specified(650), cp.toPlayer(role), flag.nothing}
end

function testGoPassPos(role)
	local mexe, mpos = SmartGoto{ pos = player.testPassPos(role), dir = player.toPlayerDir(role)}
	return {mexe, mpos}
end

function testSlowGetBall(p)
	local mexe, mpos = SlowGetBall{ dir = ball.antiYDir(p)}
	return {mexe, mpos}
end

function testKickOff1()
	local mexe,mpos = SmartGoto{pos = pos.testTwoKickOffPos1(),dir=dir.playerToBall}
	return {mexe, mpos}
end

function testKickOff2()
	local mexe,mpos = SmartGoto{pos = pos.testTwoKickOffPos2(),dir=dir.playerToBall}
	return {mexe, mpos}
end

function ttestChip(p,c)
	local idir
	if type(p) == "string" then
		idir = player.toPlayerDir(p)
	elseif type(p) == "function" then
		idir = p
	else
		idir = ball.toPointDir(p)
	end
	local mexe, mpos = ChaseKick{dir = idir}
	return {mexe, mpos, kick.chip, idir, pre.high, kp.full, cp.specified(c), flag.nothing}
end

function openChip(p, c)
	local idir
	if type(p) == "string" then
		idir = player.toPlayerDir(p)
	elseif type(p) == "function" then
		idir = p
	else
		idir = player.antiYDir(p)
	end
	local mexe, mpos = OpenSpeed{dir = idir, mod = 5}
	return {mexe, mpos, kick.chip, idir, pre.low, kp.full, cp.specified(c), flag.nothing}
end

function crazyPush(d)
	local mexe, mpos = CrazyPush{dir = d}
	return {mexe, mpos}
end
-----------------------------------------------------------------------------------------

function openSpeed(vx, vy, vdir)
	local spdX = function()
		return vx
	end

	local spdY = function()
		return vy
	end

	local spdW = function()
		return vdir
	end

	local mexe, mpos = OpenSpeed{speedX = spdX, speedY = spdY, speedW = spdW}
	return {mexe, mpos}
end

function openSpeedLocalVersion(vx, vy, vdir)
	if type(vx) ~= "function" then
		local spdX = function()
			return vx
		end
	else
		spdX = vx
	end

	if type(vy) ~= "function" then
		local spdY = function()
			return vy
		end
	else
		spdY = vy
	end

	if type(vdir) ~= "function" then
		local spdW = function()
			return vdir
		end
	else
		spdW = vdir
	end

	local mexe, mpos = OpenSpeedLocalVersion{x = spdX, y = spdY, w = spdW}
	return {mexe, mpos}
end

--挑战赛相关,强开射门
function goSupportPosForSpecial(role)
	local mexe, mpos = GoAvoidShootLine{pos = pos.getSupportPos(role), dir = dir.shoot(),sender=role}
	return {mexe, mpos, kick.flat, dir.chase, pre.low, kp.full, cp.full, flag.nothing}
end

function goCmuRushForSpecial(p, d, a, f)
	local idir
	if d ~= nil then
		idir = d
	else
		idir = dir.shoot()
	end
	local mexe, mpos = GoCmuRush{pos = p, dir = idir, acc = a, flag = f}
	return {mexe, mpos, kick.flat, dir.chase, pre.low, kp.full, cp.full, flag.nothing}
end

function goSecondPassPosForSpecial(role)
	local mexe, mpos = GoAvoidShootLine{ pos = ball.secondPassPos(), dir = player.toShootOrRobot(role),sender=role}
	return {mexe, mpos, kick.flat, dir.chase, pre.low, kp.full, cp.full, flag.nothing}
end