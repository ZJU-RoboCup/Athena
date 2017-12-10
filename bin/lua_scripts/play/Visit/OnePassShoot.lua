--熊老师参观临时编写的脚本 16.11.28 by tyGavin
local KickerPos = ball.antiYPos(CGeoPoint:new_local(100, -200))
gPlayTable.CreatePlay{

firstState = "judge",
["judge"] = {
  switch = function()
    if bufcnt(ball.valid() and ball.velMod()<10 , 60) then
      return "goto"
    end
  end,
  Kicker = task.stop(),
  Tier = task.stop(),
  match = ""
},

["goto"] = {
  switch = function()
    --[[if bufcnt( player.toBallDist("Kicker") < 20 and player.toTargetDist("Tier") < 30,50,480) then
      return "pass"
    end--]]
    return "goto"
  end,
  Tier = task.goCmuRush(KickerPos),--task.goCmuRush(KickerPos),
  Kicker = task.staticGetBall(KickerPos,false),
  match = ""
},

["pass"] = {
  switch = function()
    if bufcnt(player.kickBall("Kicker") or player.toBallDist("Kicker") > 50 , 1 ,480) then
      return "kick"
    end
  end,
  Tier = task.goCmuRush(KickerPos),
  Kicker = task.flatPass(KickerPos,450),
  match = ""
},

["kick"] = {
  switch = function()
    if bufcnt(player.kickBall("Tier") , 1 ,480) then
      return "judge"
    end
  end,
  Tier = task.waitTouchNew(),
 Kicker = task.stop(),
  match = ""
},
name = "OnePassShoot",

applicable ={
  exp = "a",
  a = true
},

attribute = "attack",
timeout = 99999
}

