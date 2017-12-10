local waitPos = CGeoPoint:new_local(100,-120);
local passPos = pos.passForTouch(waitPos)
local goalPoint = CGeoPoint:new_local(param.pitchLength/2,0)
local inFlag = true 	-- 用于记录入角的标记位，在球靠近到距离40的时候只记录一次数据
local outFlag = true 	-- 用于记录出角的标记位
local player2Goal
local testFlag = true 	-- true是开启补偿。检查补偿是否正确
local count = 0


-- 以下数据为需要记录的值
local ballSpeed = 0
local inAngel = 0
local outAngel = 0

gPlayTable.CreatePlay{
switch = function()
	if bufcnt(not ball.valid(),10,9999) then
		return "stop"
	end
	if true then
		local ballDir = ball.velDir()
		local ballVel = ball.velMod()
		local shootDir = player.toPointDir(goalPoint,"Kicker")
		local ball2Player = ball.toPlayerDir("Kicker")
		if bufcnt(ballVel > 100 
			and math.abs(Utils.Normalize(Utils.Normalize(ballDir) - Utils.Normalize(shootDir)))>math.pi/9
			and math.abs(Utils.Normalize(Utils.Normalize(ballDir) - Utils.Normalize(ball2Player)))>math.pi/9,30,9999) then
			return "stop"
		end
	end
	if gCurrentState ~= "goto" and gCurrentState ~= "pass" and bufcnt(ball.valid() and ball.velMod() < 25 and ball.toPlayerDist("Kicker") > 100,20,9999) then					

		return "goto"
	end
	if gCurrentState == "goto" then
		if bufcnt(player.toTargetDist("Kicker")< 20 and player.toBallDist("Tier")<20, 40,9999) then
			return "pass"
		end
	elseif gCurrentState == "pass" then
		if bufcnt(player.kickBall("Tier"), 1, 360)then      
			return "kick"
		end
	elseif gCurrentState == "kick" then
		if bufcnt(player.kickBall("Kicker"), "fast") then	
			player2Goal = player.toPointDir(goalPoint,"Kicker")
			return "record"
		end
		if inFlag and player.toBallDist("Kicker") < 40 then
			inFlag = false
			ballSpeed = ball.velMod()
			inAngel = Utils.Normalize(player.dir("Kicker") - (ball.velDir() + math.pi))
		end
	elseif gCurrentState == "record" then
		if bufcnt(true,20) then
			inFlag = true
			outFlag = true
			return "stop"
		end
		
	end
end,
firstState = "goto",
["goto"] = {		
	Kicker = task.goCmuRush(waitPos, _, _, flag.allow_dss),
	Tier   = task.staticGetBall("Kicker"),
	match  = ""
},	

["pass"] = {  
	Kicker = task.goCmuRush(waitPos, _, _, flag.allow_dss),
	--Tier   = task.goAndTurnKick(pos.passForTouch(waitPos),430), --传球力度
	Tier   = task.flatPass(pos.passForTouch(waitPos),430),
	match = ""
},

["kick"] = {
	Kicker = task.InterTouch(waitPos,testFlag,500),--false为测试补偿数据专用
	Tier   = task.stop(),
	match  = ""
},

["record"] = {
	Kicker = task.stop(),
	Tier   = task.stop(),
	match  = ""
},
["stop"] = { 	
	Kicker = task.goCmuRush(waitPos, _, _, flag.allow_dss),
	Tier   = task.stop(),
	match  = ""
},	

name = "TestFTQ",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

