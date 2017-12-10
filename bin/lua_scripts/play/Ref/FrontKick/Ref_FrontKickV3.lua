local POS1 = ball.refAntiYPos(CGeoPoint:new_local(0, 0))
local POS2 = ball.refAntiYPos(CGeoPoint:new_local(80, 200))
local POS3 = ball.refAntiYPos(CGeoPoint:new_local(-200, 200))
local POS4 = ball.refAntiYPos(CGeoPoint:new_local(200, 0))
local POS5 = ball.refAntiYPos(CGeoPoint:new_local(0, 200))
-- by zhyaic 2013.6.30
-- 对阵CMU时将一个车拉去顶住防开球车
local POS6 = ball.jamPos(CGeoPoint:new_local(0, 200), 65, -15)
local POS7 = ball.refAntiYPos(CGeoPoint:new_local(310, 180))
local PASS_POS = POS7--pos.passForTouch(POS7)

local MORE_FASTER = true

gPlayTable.CreatePlay{

firstState = "startball",

["startball"] = {
  switch = function ()
    if bufcnt(player.toTargetDist("Special") < 30 and
              player.toTargetDist("Defender") < 30 and
              player.toTargetDist("Middle") < 30, "normal", 180) then
      return "gotmp"
    end
  end,
  Assister = task.staticGetBall(CGeoPoint:new_local(0, 200)),
  Middle   = task.goCmuRush(POS1, _, 600, flag.allow_dss),
  Special  = task.goCmuRush(POS2, _, 600, flag.allow_dss),
  Defender = task.goCmuRush(POS3, _, 600, flag.allow_dss),
  Leader   = task.goCmuRush(POS4, _, 600, flag.allow_dss),
  Goalie   = task.goalie(),
  match    = "{A}{DSML}"
},

["gotmp"] = {
  switch = function ()
    if bufcnt(player.toTargetDist("Leader") < 15, "normal", 40) then
      return "sidegoto"
    end
  end,
  Assister = task.staticGetBall(CGeoPoint:new_local(0, 200)),
  Middle   = task.goCmuRush(POS1, _, 600, flag.allow_dss),
  Special  = task.goCmuRush(POS2, _, 600, flag.allow_dss),
  Defender = task.goCmuRush(POS3, _, 600, flag.allow_dss),
  Leader   = task.goCmuRush(POS6),
  Goalie   = task.goalie(),
  match    = "{A}{DLMS}"
},

["sidegoto"] = {
  switch = function ()
    if bufcnt(player.toTargetDist("Defender") < 100, "normal", 180) then
      return "siderush"
    end
  end,
  Assister = task.staticGetBall(CGeoPoint:new_local(0, 200)),
  Special  = task.goCmuRush(POS1, _, 600, flag.allow_dss),
  Defender = task.goCmuRush(POS5, _, 600, flag.allow_dss),
  Leader   = task.goCmuRush(POS6),
  Middle   = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{ADMSL}"
},

["siderush"] = {
  switch = function ()
    local tmpDist = MORE_FASTER and 300 or 100
    if bufcnt(player.toTargetDist("Defender") < tmpDist, "fast", 180) then
      return "sidepass"
    end
  end,
  Assister = task.staticGetBall(CGeoPoint:new_local(0, 200)),
  Defender = task.goCmuRush(POS7),
  Leader   = task.goCmuRush(POS6),
  Middle   = task.rightBack(),
  Special  = task.leftBack(),
  Goalie   = task.goalie(),
  match    = "{ALD}(MS)"
},

["sidepass"] = {
  switch = function ()
    if bufcnt(player.kickBall("Assister"), "fast", 90)then
      return "sidekick"
    end
  end,
  Assister = task.goAndTurnKick(PASS_POS, 500),
  Defender = task.goCmuRush(POS7),
  Leader   = task.goCmuRush(POS6),
  Middle   = task.rightBack(),
  Special  = task.leftBack(),
  Goalie   = task.goalie(),
  match    = "{ADLMS}"
},

["sidekick"] = {
    switch = function ()
    if bufcnt(player.kickBall("Defender"), "fast", 120) then
      return "exit"
    end
  end,
  Assister = task.goPassPos("Defender", flag.allow_dss),
  Defender = task.waitTouchNew(POS7),
  Leader   = task.stop(),
  Middle   = task.rightBack(),
  Special  = task.leftBack(),
  Goalie   = task.goalie(),
  match    = "{ADLMS}"
},

name = "Ref_FrontKickV3",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}