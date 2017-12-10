-- 0 - 300 : 0.7
-- 300 - 500 : 0.7 - 1.7
-- 500 - XX : 1.7


local cycle = 0
local lastVel = -1
local nowVel = 0
local fileNum = 63
local recordFile = nil
local ballPos = {
  x = 9999,
  y = 9999
}
local distance = 0

local function outputData()
  recordFile:write(cycle,"\t",string.format("%.2f", nowVel),"\t",string.format("%.2f", distance),"\n")
  recordFile:flush()
end

local function init()
  recordFile = io.open("BallSpeedRecord\\BallModelData"..fileNum,"a")
  fileNum = fileNum + 1
  cycle = 0
  distance = 0
  ballPos.x = ball.posX()
  ballPos.y = ball.posY()
  nowVel = ball.velMod()
  lastVel = nowVel
  outputData()
end

local function doPerCycle()
  cycle = cycle + 1
  lastVel = nowVel
  nowVel = ball.velMod()
  if lastVel - nowVel > 100 or nowVel - lastVel > 50 then
    ballPos.x = 9999
    ballPos.y = 9999
    return "!!!"
  end
  distance = ball.pos():dist(CGeoPoint:new_local(ballPos.x,ballPos.y))
  outputData()
end

local function ending(str)
  if str then
    recordFile:write(str)
    recordFile:flush()
  end
  recordFile:close()
end

local function randomKp()
  return function()
    local kp = 738 - fileNum*3
    print("kp : ",kp)
    if kp > 10 then
      return kp
    else
      return 1
    end
  end
end

gPlayTable.CreatePlay{
firstState = "prepare",

["prepare"] = {
  switch = function()
    if bufcnt(ball.valid() and player.toBallDist("Kicker") < 30,60,99999) then
      return "kick"
    end
  end,

  Kicker = function()
    if ball.valid() then
      return task.staticGetBall(CGeoPoint:new_local(0,0))
    else
      return task.stop()
    end
  end,
  match = ""
},

["kick"] = {
  switch = function()
    if player.kickBall("Kicker") then 
      return "ready"
    end
  end,
  Kicker = task.touchPass(CGeoPoint:new_local(0,0),randomKp()),
  match = ""
},
["ready"] = {
  switch = function()
    if bufcnt(true,40) then
      init()
      return "record"
    end
  end,
  Kicker = task.stop(),
  match = ""
},
["record"] = {
  switch = function()
    if doPerCycle() == "!!!" then
      print("!!!")
      ending()
      return "prepare"
    end
    if bufcnt(not ball.valid() or nowVel < 0.1 ,5,99999) then
      ending()
      return "prepare"
    end
  end,
  Kicker = task.stop(),
  match = ""
},

name = "TestForBallModel",

applicable ={
  exp = "a",
  a = true
},

attribute = "attack",
timeout = 99999
}