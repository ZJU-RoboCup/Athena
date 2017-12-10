--16.12.28 by Alan
local PassPos = CGeoPoint:new_local(50, -270)
gPlayTable.CreatePlay{

firstState = "judge",
["judge"] = {
  switch = function()
    if bufcnt(ball.valid() and ball.velMod()<10 and ball.posX()>90 , 60) then
      return "goto"
    end
  end,
  Kicker = task.stop(),
  match = ""
},

["goto"] = {
  switch = function()
    if bufcnt( player.toBallDist("Kicker") < 20 ,50,480) then
      return "control"
    end
  end,
  Kicker = task.staticGetBall(PassPos,false),
  match = ""
},

["control"] = {
  switch = function()
    if bufcnt( true,120) then
      return "chippass"
    end
  end,
  Kicker = task.slowGetBall(PassPos),
  match = ""
},

["chippass"] = {
  switch = function()
    print(ball.posX(),ball.posY())
    if bufcnt(player.kickBall("Kicker") or player.toBallDist("Kicker") > 50 , 1 ,480) then
      return "stop"
    end
  end,
  Kicker = task.chipPass(PassPos,260),
  match = ""
},

["stop"] = {
  switch = function()
  print(ball.posX(),ball.posY())
    return "stop"
  end,
  Kicker = task.stop(),
  match = ""
},


name = "TestChip",

applicable ={
  exp = "a",
  a = true
},

attribute = "attack",
timeout = 99999
}

