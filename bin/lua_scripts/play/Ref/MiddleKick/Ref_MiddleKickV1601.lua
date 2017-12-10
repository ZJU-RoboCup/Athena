-- 后卫上来开球
-- yys 2015/03/17
-- gty 很无脑的直接开球，有可能造前场定位
local LEADER_POS_1 = ball.antiYPos(CGeoPoint:new_local(320, 180))

local SPECIAL_POS_1 = ball.antiYPos(CGeoPoint:new_local(0, 180))

local MIDDLE_POS_1 = ball.syntYPos(CGeoPoint:new_local(-280, 180))

local ASSISTER_POS = ball.antiYPos(CGeoPoint:new_local(350, -290))

gPlayTable.CreatePlay{

firstState = "startBall",

["startBall"] = {
  switch = function ()
    if bufcnt(player.toTargetDist("Leader") < 30 and
          player.toTargetDist("Special") < 30 and
          player.toTargetDist("Middle") < 30, "normal") then
      return "waitRobot"
    end
  end,
  Assister = task.staticGetBall(CGeoPoint:new_local(-100, 200)),
  Leader   = task.goCmuRush(LEADER_POS_1, _, 500, flag.allow_dss),
  Special  = task.goCmuRush(SPECIAL_POS_1, _, 500, flag.allow_dss),
  Middle   = task.goCmuRush(MIDDLE_POS_1, _, 500, flag.allow_dss),
  Defender = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{AMLSD}"
},

["waitRobot"] = {
  switch = function ()
    if bufcnt(player.toTargetDist("Middle") < 40, "fast") then
      return "rushBall"
    end
  end,
  Assister = task.staticGetBall(CGeoPoint:new_local(-100, 200)),
  Leader   = task.goCmuRush(LEADER_POS_1, _, _, flag.allow_dss),
  Middle   = {GoCmuRush{acc = 400, pos = ball.backPos(CGeoPoint:new_local(450, 0), 40, 0), dir = ball.backDir(p)}},
  Special  = task.rightBack(),
  Defender = task.leftBack(),
  Goalie   = task.goalie(),
  match    = "{AML}[SD]"
},

["rushBall"] = {
  switch = function ()
    if bufcnt(player.kickBall("Middle"), "normal", 60) then
      return "exit"
    end
  end,
  Middle   = task.chaseNew(),
  Leader   = task.goCmuRush(LEADER_POS_1, _, _, flag.allow_dss),
  Assister = task.goCmuRush(ASSISTER_POS, player.toBallDir, 300, flag.allow_dss),
  Special  = task.rightBack(),
  Defender = task.leftBack(),
  Goalie   = task.goalie(),
  match    = "{AML}[SD]"
},

name = "Ref_MiddleKickV1601",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}
