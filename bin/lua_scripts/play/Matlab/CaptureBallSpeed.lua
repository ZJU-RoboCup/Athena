--用于在小场循环测量球速，在c++内采集球速，球的位置（滤波前）
--gty 16.11.27
local target = CGeoPoint:new_local(450,300)
local origin = CGeoPoint:new_local(0,-300)--CGeoPoint:new_local(75,-225)
local x0 = 0    --x方向上界
local x1 = 150  --x方向下界
local y0 = -300
local y1 = -150

local function smallKp()
  return function()
    local kp = math.sqrt(ball.toPointDist(origin))*15
    print (kp)
    return kp
  end
end


gPlayTable.CreatePlay{
firstState = "judge",

["judge"] = {
  switch = function()
    if ball.valid() and ball.velMod() < 10 then --球有，而且处于静态
      if ball.posX() > x0 and ball.posX() < x1 and ball.posY() > y0 and ball.posY() < y1 then --球在指定范围内
        return "getball2target"
      else
        return "getball2origin"
      end

    else
      return "judge"
    end
  end,

  Kicker = task.stop(),
  match = ""
},

["getball2target"] = {
  switch = function()
    if bufcnt(player.toBallDist("Kicker") < 30 ,40,480) then
      return "kick"
    end
  end,
  Kicker = task.staticGetBall(target,false),
  match = ""
},

["kick"] = {
  switch = function()
    if bufcnt(player.kickBall("Kicker"),1,480) then
      return "wait"
    end
  end,

  Kicker = task.touchPass(target,600),
  match = ""
},

["wait"] = {
  switch = function()
    if bufcnt(ball.valid() and ball.velMod() < 10 , 60 , 300) then 
      return "judge"
    end
  end,

  Kicker = task.stop(),
  match = ""
},

["getball2origin"] = {
  switch = function()
    if bufcnt(player.toBallDist("Kicker") < 30 ,40,480) then
      return "smallkick"
    end
  end,
  Kicker = task.staticGetBall(origin,false),
  match = ""
},

["smallkick"] = {
  switch = function()
    if bufcnt(player.kickBall("Kicker"),1,480) then
      return "wait"
    end
  end,

  Kicker = task.touchPass(origin,600),
  match = ""
},

name = "CaptureBallSpeed",

applicable ={
  exp = "a",
  a = true
},

attribute = "attack",
timeout = 99999
}