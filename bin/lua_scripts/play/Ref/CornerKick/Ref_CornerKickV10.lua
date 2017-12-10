-- 类似于篮球挡拆的定位球
-- by zhyaic 2014-07-09
-- 2014-07-20 yys 改

local TMP_POS1 = ball.refAntiYPos(CGeoPoint:new_local(145,95))
local TMP_POS2 = ball.refAntiYPos(CGeoPoint:new_local(160,60))
local JAM_POS1 = ball.refSyntYPos(CGeoPoint:new_local(100,170))
local JAM_POS2 = ball.refSyntYPos(CGeoPoint:new_local(125,125))
local JAM_POS3 = ball.refSyntYPos(CGeoPoint:new_local(145,95))
local JAM_POS4 = ball.refSyntYPos(CGeoPoint:new_local(160,60))
local FINAL_SHOOT_POS = CGeoPoint:new_local(350,100)
local CHIP_POS  = pos.passForTouch(FINAL_SHOOT_POS)
local SHOOT_POS = ball.refAntiYPos(FINAL_SHOOT_POS)
local RUN_POS = ball.refAntiYPos(CGeoPoint:new_local(130,110))

-- 让挑球的速度更快一点
gPlayTable.CreatePlay{

  firstState = "tmp",

  ["tmp"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Middle") < 30, 1, 180) then
        return "start"
      end
    end,
    Assister = task.staticGetBall(CHIP_POS),
    Leader   = task.goCmuRush(JAM_POS1, ball.refSyntYDir(1.57),_,flag.allow_dss),
    Special  = task.goCmuRush(JAM_POS2,_,_,flag.allow_dss),
    Middle   = task.goCmuRush(TMP_POS1,_,_,flag.allow_dss),
    Defender = task.goCmuRush(TMP_POS2,_,_,flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{A}[LSMD]"
  },

  ["start"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Leader") < 30 and player.toTargetDist("Middle") < 30, 40, 180) then
        return "gomiddle"
      end
    end,
    Assister = task.staticGetBall(CHIP_POS),
    Leader   = task.goCmuRush(JAM_POS1, ball.refSyntYDir(1.57)),
    Special  = task.goCmuRush(JAM_POS2),
    Middle   = task.goCmuRush(JAM_POS3),
    Defender = task.goCmuRush(JAM_POS4),
    Goalie   = task.goalie(),
    match    = "{A}[LSMD]"
  },

  ["gomiddle"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Leader") < 20, "fast", 120) then
        return "goshoot"
      end
    end,
    Assister = task.slowGetBall(CHIP_POS),
    Leader   = task.goCmuRush(RUN_POS, ball.refSyntYDir(1.57)),
    Special  = task.goCmuRush(JAM_POS2),
    Middle   = task.goCmuRush(JAM_POS3),
    Defender = task.goCmuRush(JAM_POS4),
    Goalie   = task.goalie(),
    match    = "{ALSMD}"
  },

  ["goshoot"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Leader") < 250, "fast", 120) then
        return "pass"
      end
    end,
    Assister = task.slowGetBall(CHIP_POS),
    Leader   = task.goCmuRush(SHOOT_POS),
    Special  = task.goCmuRush(JAM_POS2),
    Middle   = task.goCmuRush(JAM_POS3),
    Defender = task.goCmuRush(JAM_POS4),
    Goalie   = task.goalie(),
    match    = "{ALSMD}"
  },

  ["pass"] = {
    switch = function ()
      if player.kickBall("Assister") or player.toBallDist("Assister") > 30 then
        return "fix"
      end
    end,
    Assister = task.chipPass(CHIP_POS, 180),
    Leader   = task.continue(),
    Middle   = task.leftBack(),
    Special  = task.rightBack(),
    Defender = task.defendMiddle(),
    Goalie   = task.goalie(),
    match    = "{AL}[SMD]"
  },

  ["fix"] = {
    switch = function ()
      if  bufcnt(true, 40) then
        return "touch"
      end
    end,
    Assister = task.goSupportPos("Leader"),
    Leader   = task.continue(),
    Middle   = task.leftBack(),
    Special  = task.rightBack(),
    Defender = task.defendMiddle(),
    Goalie   = task.goalie(),
    match    = "{AL}[SMD]"
  },

  ["touch"] = {
    switch = function ()
      if bufcnt(player.kickBall("Leader"), "fast", 90) then
        return "exit"
      end
    end,
    Assister = task.goSupportPos("Leader"),
    Leader   = task.waitTouch(SHOOT_POS, 0),
    Middle   = task.leftBack(),
    Special  = task.rightBack(),
    Defender = task.defendMiddle(),
    Goalie   = task.goalie(),
    match    = "{AL}[SMD]"
  },

  name = "Ref_CornerKickV10",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}