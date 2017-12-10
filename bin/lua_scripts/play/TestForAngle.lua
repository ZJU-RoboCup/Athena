-- 测试GoAndTurnKick 转动多少角度时能避开防开球车
-- by yys 2014-03-28

local GET_BALL  = 0          -- 拿球朝向(单位为弧度),可传函数,角色等

local PASS = -math.pi / 2  --- math.pi / 6  -- 传球朝向(单位为弧度),可传函数,角色等

gPlayTable.CreatePlay{

firstState = "readyState",

["readyState"] = {
  switch = function()
    if bufcnt(true, 120) then
      return "goAndTurnKick"
    end
  end,
  Kicker   = task.staticGetBall(GET_BALL),
  Leader   = task.stop(),
  Special  = task.stop(),
  Middle   = task.stop(),
  Defender = task.stop(),
  Goalie   = task.goalie(),
  match    = "{LSDM}"
},

["goAndTurnKick"] = {
  switch = function()
    if bufcnt(player.kickBall("Assister"), "fast", 120) then
      return "stop"
    end
  end,
  Kicker   = task.goAndTurnKick(PASS, 350),
  Leader   = task.stop(),
  Special  = task.stop(),
  Middle   = task.stop(),
  Defender = task.stop(),
  Goalie   = task.goalie(),
  match    = "{SLDM}"
},

["stop"] = {
  switch = function ()
    if bufcnt(true, 120) then
      return "finish"
    end
  end,
  Kicker   = task.stop(),
  Leader   = task.stop(),
  Special  = task.stop(),
  Middle   = task.stop(),
  Defender = task.stop(),
  Goalie   = task.goalie(),
  match    = "{SLDM}"
},

name = "TestForAngle",
applicable = {
  exp = "a",
  a = true
},
score = 0,
attribute = "attack",
timeout   = 99999
}