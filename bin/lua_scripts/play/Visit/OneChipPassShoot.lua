--16.12.28 by Alan
local PassPos = CGeoPoint:new_local(380, 110)
local KickerPos = ball.antiYPos(CGeoPoint:new_local(380, 110))

gPlayTable.CreatePlay{

firstState = "judge",
["judge"] = {
  switch = function()
    if bufcnt(ball.valid() and ball.velMod()<10 and ball.posX()>90 , 60) then
      return "goto"
    end
  end,
  Tier = task.stop(),
  Kicker = task.stop(),
  match = ""
},

["goto"] = {
  switch = function()
    if bufcnt( player.toBallDist("Kicker") < 20 and player.toTargetDist("Tier") < 30,50,480) then
      return "control"
    end
  end,
  Tier = task.goCmuRush(KickerPos),
  Kicker = task.staticGetBall(PassPos,false),
  match = ""
},

["control"] = {
  switch = function()
    if bufcnt( true,100) then
      return "chippass"
    end
  end,
  Tier = task.goCmuRush(KickerPos),
  Kicker = task.slowGetBall(PassPos),
  match = ""
},

["chippass"] = {
  switch = function()
    if bufcnt(player.kickBall("Kicker") or player.toBallDist("Kicker") > 50 , 1 ,480) then
      return "fix"
    end
  end,
  Tier = task.goCmuRush(KickerPos),
  Kicker = task.chipPass(PassPos,125),
  match = ""
},

["fix"] = {
  switch = function()
    if bufcnt(true,35) then
      return "kick"
    end
  end,
  Tier = task.goCmuRush(KickerPos),
  Kicker = task.stop(),
  match = ""
},

["kick"] = {
  switch = function()
    if bufcnt(player.kickBall("Tier") , 1 ,480) then
      return "judge"
    end
  end,
  Tier = task.InterTouch(),--task.waitTouchNew(),
  Kicker = task.stop(),
  match = ""
},
name = "OneChipPassShoot",

applicable ={
  exp = "a",
  a = true
},

attribute = "attack",
timeout = 99999
}

