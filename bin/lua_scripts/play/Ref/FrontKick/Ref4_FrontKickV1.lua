-- 用于4车的前场定位球

local POS_1 = ball.refAntiYPos(CGeoPoint:new_local(50, 0))
local POS_2 = ball.refAntiYPos(CGeoPoint:new_local(180, 220))

local POS_3 = ball.refAntiYPos(CGeoPoint:new_local(180, -220))
local POS_4 = ball.refAntiYPos(CGeoPoint:new_local(50, 150))

gPlayTable.CreatePlay{

firstState = "startball",

["startball"] = {
  switch = function()
    if bufcnt(player.toTargetDist("Leader") < 20, "slow", 120) then
      return "dribble"
    end
  end,
  Assister = task.staticGetBall(CGeoPoint:new_local(360, -50)),
  Leader   = task.goCmuRush(POS_2, _, 600, flag.allow_dss),
  Defender = task.goCmuRush(POS_1, _, 600, flag.allow_dss),
  Goalie   = task.goalie(),
  match    = "{ALD}"
},

["dribble"] = {
  switch = function()
    if bufcnt(player.toTargetDist("Leader") < 20, 1, 120) then
      return "chipShoot"
    end
  end,
  Assister = task.slowGetBall(CGeoPoint:new_local(360, -50)),
  Leader   = task.goCmuRush(POS_3, _, 400, flag.allow_dss),
  Defender = task.goCmuRush(POS_4, _, 600, flag.allow_dss),
  Goalie   = task.goalie(),
  match    = "{ALD}"
},

["chipShoot"] = {
  switch = function()
    if player.kickBall("Assister") then
      return "finish"
    elseif bufcnt(true, 30) then
      return "exit"
    end
  end,
  Assister = task.chipPass(CGeoPoint:new_local(360, -50)),
  Leader   = task.leftBack(),
  Defender = task.rightBack(),
  Goalie   = task.goalie(),
  match    = "{A}[LD]"
},

name = "Ref4_FrontKickV1",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout   = 99999
}