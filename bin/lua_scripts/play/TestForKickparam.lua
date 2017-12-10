-- 对某辆车
-- 发给底层power, 30 - 127吧，5一档，每档测3次，数据传输到C++里面
local power = 999100
-- 注： 一个小trick，判断输入的power>999001，若大于，则取后三位。不做任何处理下发即可。
local shootPos = CGeoPoint:new_local(450, -0)
local outFlag = true 	-- 用于记录踢出球球速的标记位
local count = 0
local thisPowerVel = 0
local lastPowerVel = 0
local testPowerFlag = false
local testPower = 999100
ballPowSpeed = {}

-- 以下数据为需要记录的值
local ballSpeed = 0

function checkData(speed, lastPowerVel)
	local reMeasureCount = 0
	reMeasureIndex = {}
	thisPowerVel = (speed[0] + speed[1] + speed[2]) / 3
	for i = 0, 2 do
		if speed[i] < lastPowerVel * 0.8
			or math.abs(speed[i] - (thisPowerVel * 3 - speed[i]) / 2)/speed[i] > 0.2 then
			print(speed[0]," ", speed[1], " ", speed[2], " ", thisPowerVel, " ", lastPowerVel, " ", i)
   			reMeasureIndex[reMeasureCount] = i
   			reMeasureCount = reMeasureCount + 1
		end
	end
	return reMeasureCount, reMeasureIndex
end

function swap(a, b)
	temp = a
	a = b
	b = temp
	return a, b
end



gPlayTable.CreatePlay{
switch = function()
	if bufcnt(not ball.valid(),10,9999) then
		return "stop"
	end

	if gCurrentState ~= "goto" and gCurrentState ~= "pass" and bufcnt(ball.valid() and ball.velMod() < 25 and ball.toPlayerDist("Tier") > 100,20,9999) then					
		return "goto"
	end
	if gCurrentState == "goto" then
		-- print ("goto")
		if bufcnt(player.toBallDist("Tier")<20, 40,9999) then
			return "pass"
		end
	elseif gCurrentState == "pass" then
		-- print(player.kickBall("Tier"))
		if bufcnt(player.kickBall("Tier"), 1, 360)then      
			return "record"
		end

	elseif gCurrentState == "record" then

		if bufcnt(true,20) then
			outFlag = true
			return "stop"
		end
		if not testPowerFlag then
			if player.toBallDist("Tier") > (power - 999000)/2 + 40 and ball.velMod() > 150 and outFlag then
				outFlag = false
				count = count + 1
				ballSpeed = ball.velMod()

				if power <= 999127 then
					local recordfile = io.open("kickParam_flat.txt","a")
					-- TODO:
					-- 数据的简单滤波？
					-- 这里加一个判断，比如小于前面的速度的话就不记录
					-- 每一个速度踢三脚
					print (power - 999000, "---test---", ballSpeed, " ", count)
					if count % 3 == 1 or count % 3 == 2 then 
						-- count 在1和2的时候先只记录，不写入txt
						-- count 在3的时候判断用不用记录
						ballPowSpeed[(count - 1) % 3] = ballSpeed
		
					
					else	
						ballPowSpeed[(count - 1) % 3] = ballSpeed
						thisPowerVel = (ballPowSpeed[0] + ballPowSpeed[1] + ballPowSpeed[2]) / 3
						reMeasureCount, reMeasureIndex = checkData(ballPowSpeed, lastPowerVel)
						-- remove all the constraints
						if reMeasureCount >= 0 then
							recordfile:write(power - 999000, " ", ballPowSpeed[0], " ",ballPowSpeed[1], " " ,ballPowSpeed[2], "\n")
							print(power - 999000, " ",  ballPowSpeed[0], " ",ballPowSpeed[1], " " ,ballPowSpeed[2], "\n")
							lastPowerVel = thisPowerVel
							power = power + 5
							ballPowSpeed = {}
						elseif reMeasureCount < 2 then -- 重新测量一波
							count = count - 1
							ballPowSpeed[2], ballPowSpeed[reMeasureCount - 1] = swap(ballPowSpeed[2], ballPowSpeed[reMeasureCount - 1])
						else -- 完全重新测量
							count = count - 3
						end

					end
				else
					print("Param measuring stop!!")
				end
			end
		else
			power = testPower
		end
	end
end,
firstState = "goto",
["goto"] = {		
	Tier   = task.staticGetBall(shootPos),
	match  = ""
},	

["pass"] = {  
	Tier   = task.flatPass(pos.passForTouch(shootPos), power),
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

name = "TestForKickparam",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

