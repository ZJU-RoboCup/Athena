local STOP_FLAG = bit:_or(flag.slowly, flag.dodge_ball)
local STOP_DSS = bit:_or(STOP_FLAG, flag.allow_dss)

gPlayTable.CreatePlay{

firstState = "state1",

["state1"] = {
  switch = function()
    if player.posX("Kicker") >= 245 and player.posY("Kicker") >= 145  then
      return "state2"
    else
      return "state1"
    end
  end,
  Kicker  = task.goSpeciPos(CGeoPoint:new(250, 150), 0, STOP_DSS),
  Special = task.goSimplePos(CGeoPoint:new(240, 0)),
  Leader = task.goSimplePos(CGeoPoint:new(150, 130)),
  Goalie = task.goSimplePos(CGeoPoint:new(-190, 0)),
  Middle = task.goSimplePos(CGeoPoint:new(220, 160)),
  Defender = task.goSimplePos(CGeoPoint:new(190, -100)),
  match = "[SLMD]"
},

["state2"] = {
  switch = function()
    if player.posX("Kicker") >= 245 and player.posY("Kicker") <= -145  then
      return "state3"
    else
      return "state2"
    end
  end,
  Kicker  = task.goSpeciPos(CGeoPoint:new(250, -150), 0, STOP_DSS),
  Special = task.goSimplePos(CGeoPoint:new(240, 0)),
  Leader = task.goSimplePos(CGeoPoint:new(150, 130)),
  Goalie = task.goSimplePos(CGeoPoint:new(-190, 0)),
  Middle = task.goSimplePos(CGeoPoint:new(220, 160)),
  Defender = task.goSimplePos(CGeoPoint:new(190, -100)),
  match = "[SLMD]"
},

["state3"] = {
  switch = function()
    if player.posX("Kicker") <= -245 and player.posY("Kicker") <= -145  then
      return "state4"
    else
      return "state3"
    end
  end,
  Kicker  = task.goSpeciPos(CGeoPoint:new(-250, -150), 0, STOP_DSS),
  Special = task.goSimplePos(CGeoPoint:new(240, 0)),
  Leader = task.goSimplePos(CGeoPoint:new(150, 130)),
  Goalie = task.goSimplePos(CGeoPoint:new(-190, 0)),
  Middle = task.goSimplePos(CGeoPoint:new(220, 160)),
  Defender = task.goSimplePos(CGeoPoint:new(190, -100)),
  match = "[SLMD]"
},

["state4"] = {
  switch = function()
    if player.posX("Kicker") <= -245 and player.posY("Kicker") >= 145  then
      return "state1"
    else
      return "state4"
    end
  end,
  Kicker  = task.goSpeciPos(CGeoPoint:new(-250, 150), 0, STOP_DSS),
  Special = task.goSimplePos(CGeoPoint:new(240, 0)),
  Leader = task.goSimplePos(CGeoPoint:new(150, 130)),
  Goalie = task.goSimplePos(CGeoPoint:new(-190, 0)),
  Middle = task.goSimplePos(CGeoPoint:new(220, 160)),
  Defender = task.goSimplePos(CGeoPoint:new(190, -100)),
  match = "[SLMD]"
},

name = "TestChallenges",
applicable ={
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}
