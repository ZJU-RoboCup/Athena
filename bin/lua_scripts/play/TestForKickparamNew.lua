-- 使用方法：	在下面写好是几号车，测量的时候把MeasureMode改为true，测完之后把MeasureMode改为false，再运行一次脚本，就可以计算出分档参数
-- 				对某辆车,直接发给底层power, 50 - 127，10一档，每档测3次，数据传输到C++里面
-- 				注： 一个小trick，flatShoot时判断输入的power>999001，若大于，则取后三位。不做任何处理下发即可。
--				记得改getKickParamData中的playernumber

local shootPos = CGeoPoint:new_local(450, -0)
local MeasureMode = true
PlayerNum = 8
local filename = "kickParam_flat_"..tostring(PlayerNum)..".txt"
local initFlag = true
local ballSpeed = 0 	-- 测量球速

local power = 999030

local recordFlag = true 	-- 用于记录踢出球球速的标记位
local count = 0

local testPowerFlag = false -- 测量分档的时候改为false。 true的话测量力度为testPower时的球速
local testPower = 999030

local A_est = 0.15
local B_est = 4


gPlayTable.CreatePlay{
switch = function()
	if bufcnt(not ball.valid(),10,9999) then
		return "stop"
	end

	if gCurrentState == "stop" and bufcnt(ball.valid() and ball.velMod() < 25 and ball.toPlayerDist("Tier") > 10,20,9999) then					
		return "goto"
	end
	if gCurrentState == "goto" then
		if MeasureMode == false then
			print("========================================")
			print("     kick Param:: calculating mode!")
			print("========================================")
			recordFlag = false
			getluadata:getKickParamData(true, 8)
			return "nothing"
		end
		if testPowerFlag then 
			power = getluadata:updateKickParamPower(testPower)	
			print("Test Power : ", power - 999000)
		end
		if initFlag then
			power = getluadata:updateKickParamPower(power) -- power 初值为50
			initFlag = false
		end
		if bufcnt(player.toBallDist("Tier")<20, 40,9999) then
			return "pass"
		end
	elseif gCurrentState == "pass" then
		if bufcnt(player.kickBall("Tier"), 1, 360)then      
			return "record"
		end

	elseif gCurrentState == "record" then
		if bufcnt(true,20) then
			recordFlag = true
			return "stop"
		end
		if MeasureMode then -- not testPowerFlag and 
			-- question : 大于多少距离开始计算速度？
			if not testPowerFlag then
				if player.toBallDist("Tier") > 	math.pow(power - 999030,1.5) / 8 + 40  and ball.velMod() > 50 and recordFlag then
					recordFlag = false
					count = count + 1
					ballSpeed = ball.velMod()
					local recordfile = io.open("kickParam_flat_.txt","a")
					if power <= 999100 then
						-- 如果数据波动不是特别大，就记录，否则重测
						if power-999000 > A_est * ballSpeed + B_est - 20 and power - 999000 < A_est * ballSpeed + B_est+ 20 then 
							
							local recordfile = io.open(filename,"a")
							recordfile:write(power - 999000, " ", ballSpeed, "\n")
							recordfile:close()
							print(power - 999000, " ",  ballSpeed)
							power = getluadata:updateKickParamPower(power + 5) -- power 初值为50
						else
							print("(error)estimated power:",A_est * ballSpeed + B_est)
							print("data inaccurate, remeasure!!! power = ", power)	
						end												
					else					
						print("Param measuring stop!!")
						return "nothing"
					end
				end
			else -- 测试模式
				-- power = getluadata:updateKickParamPower(testPower)	
				-- print("Test Power : ", power - 999000)
			end
		else
			if recordFlag then
				print("========================================")
				print("     kick Param:: calculating mode!")
				print("========================================")
				recordFlag = false
				getluadata:getKickParamData(true, 4)
			end
		end
	end
end,
firstState = "goto",
["goto"] = {		
	Tier   = task.staticGetBall(shootPos),
	match  = ""
},	

["pass"] = {  
	Tier   = task.flatPass(pos.passForTouch(shootPos), "Kicker"), -- getluadata:getKickParamPower()
	match = ""
},

["record"] = {
	Tier   = task.stop(),
	match  = ""
},
["stop"] = { 	
	Tier   = task.stop(),
	match  = ""
},	
["nothing"] = { 	
	Tier   = task.stop(),
	match  = ""
},	

name = "TestForKickparamNew",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

