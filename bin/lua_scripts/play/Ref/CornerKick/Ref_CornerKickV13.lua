-- by yys 2015.06.07

local SHOOT_POS = ball.refAntiYPos(CGeoPoint:new_local(150, -265))
local PASS_POS  = pos.passForTouch(SHOOT_POS)

local USE_CHIP = true

gPlayTable.CreatePlay{

firstState = "getball",

["getball"] = {
  switch = function ()
    if bufcnt(player.toBallDist("Assister") < 20, 180, 240) then
      return "runpos"
    end
  end,
  Assister = task.staticGetBall(CGeoPoint:new_local(450, -280)),
  Special  = task.goCmuRush(CGeoPoint:new_local(-200, 0), _, 300),
  Leader   = task.goCmuRush(ball.refAntiYPos(CGeoPoint:new_local(-250, 100)), _, 300),
  Middle   = task.goCmuRush(ball.refAntiYPos(CGeoPoint:new_local(-250, -100)), _, 300),
  Defender = task.goCmuRush(CGeoPoint:new_local(0, 0), _, 300),
  Goalie   = task.goalie(),
  match    = "{A}{SLMD}"
},

["runpos"] = {
  switch = function ()
    if bufcnt(true, 60) then
      return "prepare"
    end
  end,
  Assister = task.staticGetBall(CGeoPoint:new_local(450, -280)),
  Special  = task.goCmuRush(CGeoPoint:new_local(-200, 0)),
  Leader   = task.goCmuRush(ball.refAntiYPos(CGeoPoint:new_local(305, 260))),
  Middle   = task.goCmuRush(ball.refAntiYPos(CGeoPoint:new_local(420, -160))),
  Defender = task.goCmuRush(ball.refAntiYPos(CGeoPoint:new_local(100, 50))),
  Goalie   = task.goalie(),
  match    = "{A}{SLMD}"
},

["prepare"] = {
  switch = function ()
    if bufcnt(player.toTargetDist("Special") < 300, 1) then
      return "turnpass"
    end
  end,
  Assister = task.staticGetBall(CGeoPoint:new_local(450, -280)),
  Special  = task.goCmuRush(SHOOT_POS),
  Leader   = task.continue(),
  Middle   = task.continue(),
  Defender = task.goCmuRush(ball.refAntiYPos(CGeoPoint:new_local(340, 200))),
  Goalie   = task.goalie(),
  match    = "{A}{SLMD}"
},

["turnpass"] = {
  switch = function ()
    if bufcnt(player.kickBall("Assister"), "fast") then
      return "shoot"
    end
  end,
  Assister = USE_CHIP and task.goAndTurnChip(PASS_POS, 140) or task.goAndTurnKickByGetBall(PASS_POS, 500),
  Special  = task.continue(),
  Leader   = task.continue(),
  Middle   = task.continue(),
  Defender = task.continue(),
  Goalie   = task.goalie(),
  match    = "{A}{SLMD}" 
},

["shoot"] = {
  switch = function ()
    if bufcnt(player.kickBall("Special"), 1, (USE_CHIP and 90 or 60)) then
      return "finish"
    end
  end,
  Assister = task.stop(),
  Special  = USE_CHIP and task.waitTouch(SHOOT_POS, 1.57) or task.touch(),
  Leader   = task.leftBack(),
  Middle   = task.continue(),
  Defender = task.rightBack(),
  Goalie   = task.goalie(),
  match    = "{A}{SLMD}" 
},

name = "Ref_CornerKickV13",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}