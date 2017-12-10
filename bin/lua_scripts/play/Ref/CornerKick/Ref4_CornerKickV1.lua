-- 当只有4个车时的角球定位球策略

local SHOOT_POS = CGeoPoint:new_local(300, 90)
local FINAL_SHOOT_POS = ball.refAntiYPos(SHOOT_POS)
local PASS_POS  = pos.passForTouch(SHOOT_POS)
local POS_1     = ball.refAntiYPos(CGeoPoint:new_local(140, 230))
local POS_2     = ball.refAntiYPos(CGeoPoint:new_local(290, 140))

local POS_4     = ball.refAntiYPos(CGeoPoint:new_local(150, 160))
local POS_5     = ball.refAntiYPos(CGeoPoint:new_local(125, -183))


gPlayTable.CreatePlay{

firstState = "startball",

["startball"] = {
  switch = function()
    if bufcnt(player.toTargetDist("Leader") < 20, "normal", 120) then
      return "changepos"
    end
  end,
  Assister = task.staticGetBall(PASS_POS),
  Leader   = task.goCmuRush(POS_1, _, 600, flag.allow_dss),
  Defender = task.goCmuRush(POS_4, _, 600, flag.allow_dss),
  Goalie   = task.goalie(),
  match    = "{ALD}"
},

["changepos"] = {
  switch = function()
    if bufcnt(player.toTargetDist("Leader") < 20, "normal", 120) then
      return "chipball"
    end
  end,
  Assister = task.slowGetBall(PASS_POS),
  Leader   = task.goCmuRush(POS_2, _, 500, flag.allow_dss),
  Defender = task.goCmuRush(POS_5, _, 500, flag.allow_dss),
  Goalie   = task.goalie(),
  match    = "{ALD}"
},

["chipball"] = {
  switch = function()
    if bufcnt(player.kickBall("Assister") or
              player.toBallDist("Assister") > 20, "normal", 120) then
      return "fix"
    end
  end,
  Assister = task.chipPass(PASS_POS, 200),
  Leader   = task.goCmuRush(FINAL_SHOOT_POS),
  Defender = task.goCmuRush(POS_5, _, 500, flag.allow_dss),
  Goalie   = task.goalie(),
  match    = "{ALD}"
},

["fix"] = {
  switch = function()
    if bufcnt(true, 20) then
      return "shootball"
    end
  end,
  Assister = task.stop(),
  Leader   = task.goCmuRush(FINAL_SHOOT_POS),
  Defender = task.goCmuRush(POS_5, _, 500, flag.allow_dss),
  Goalie   = task.goalie(),
  match    = "{ALD}"
},


["shootball"] = {
  switch = function()
    if player.kickBall("Leader") then
      return "finish"
    elseif  bufcnt(true, 120) then
      return "exit"
    end
  end,
  Leader   = task.InterTouch(FINAL_SHOOT_POS, 0),
  Assister = task.rightBack(),
  Defender = task.leftBack(),
  Goalie   = task.goalie(),
  match    = "{L}(AD)"
},

name = "Ref4_CornerKickV1",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout   = 99999
}