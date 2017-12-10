-- 针对MRL
-- by yys 2014-04-07
-- 2014-07-20 yys 改

local ANTI_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(210, 130))
local ANTI_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(250, 140))
local ANTI_POS_3 = ball.refAntiYPos(CGeoPoint:new_local(280, 180))
local ANTI_POS_4 = ball.refAntiYPos(CGeoPoint:new_local(290, 140))
local ANTI_POS_5 = ball.refAntiYPos(CGeoPoint:new_local(360, 170))
local SYNT_POS_1 = ball.refSyntYPos(CGeoPoint:new_local(90, 140))
local SYNT_POS_2 = ball.refSyntYPos(CGeoPoint:new_local(250, 190))
local BACK_WAIT_POS = ball.refAntiYPos(CGeoPoint:new_local(-100, 10))
local BALL_POS = CGeoPoint:new_local(350, 100)
local TMP_SHOOT_POS = ball.refAntiYPos(BALL_POS)
local SHOOT_POS = pos.passForTouch(TMP_SHOOT_POS)

local dangerous = true
local USE_CHIP  = true

gPlayTable.CreatePlay{

  firstState = "start",

  ["start"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Leader") < 20 and
                player.toTargetDist("Defender") < 20 and
                player.toTargetDist("Special") < 20, 30, 60) then
        return "runTmpPos"
      end
    end,
    Assister = task.staticGetBall(SHOOT_POS()),
    Leader   = task.goCmuRush(SYNT_POS_2,_,_,flag.allow_dss),
    Special  = task.goCmuRush(ANTI_POS_1,_,_,flag.allow_dss),
    Middle   = task.goCmuRush(ANTI_POS_2,_,_,flag.allow_dss),
    Defender = task.goCmuRush(BACK_WAIT_POS,_,_,flag.allow_dss),
    Goalie   = dangerous and task.goalie() or task.goCmuRush(SYNT_POS_1, _, 350),
    match    = "{A}{LDSM}"
  },

  ["runTmpPos"] = {
    switch = function ()
      if  bufcnt(player.toTargetDist("Special") < 20, "normal", 40) then -- time 120
        return "runTmpPos_2"
      end
    end,
    Assister = USE_CHIP and task.slowGetBall(SHOOT_POS()) or task.staticGetBall(SHOOT_POS()),
    Leader   = task.goCmuRush(SYNT_POS_2,_,_,flag.allow_dss),
    Special  = task.goCmuRush(ANTI_POS_3,_,_,flag.allow_dss),
    Middle   = task.goCmuRush(ANTI_POS_4, _, 100),
    Defender = task.goCmuRush(TMP_SHOOT_POS, _, 100),
    Goalie   = dangerous and task.goalie() or task.goCmuRush(SYNT_POS_1, _, 350),
    match    = "{ALDSM}"
  },

  ["runTmpPos_2"] = {
    switch = function ()
      if  bufcnt(player.toTargetDist("Defender") < 120, "fast", 40) then
        return "passBall"
      end
    end,
    Assister = USE_CHIP and task.slowGetBall(SHOOT_POS()) or task.staticGetBall(SHOOT_POS()),
    Leader   = task.goCmuRush(SYNT_POS_2,_,_,flag.allow_dss),
    Special  = task.goCmuRush(ANTI_POS_5,_,_,flag.allow_dss),
    Middle   = task.goCmuRush(ANTI_POS_4, _, 100),
    Defender = task.goCmuRush(TMP_SHOOT_POS),
    Goalie   = task.goalie(),
    match    = "{ALDSM}"
  },

  ["passBall"] = {
    switch = function ()
       if player.kickBall("Assister") or player.toBallDist("Assister") > 30 then
        if USE_CHIP then
          return "gofix"
        else
          return "shoot"
        end
      end
    end,
    Assister = USE_CHIP and task.chipPass(SHOOT_POS(), 145) or task.goAndTurnKick(SHOOT_POS(), 500),
    Leader   = task.goCmuRush(SYNT_POS_2),
    Special  = task.goCmuRush(ANTI_POS_5),
    Middle   = task.goCmuRush(ANTI_POS_4, _, 100),
    Defender = task.goCmuRush(TMP_SHOOT_POS),
    Goalie   = task.goalie(),
    match    = "{ALDSM}"
  },

  ["gofix"] = {
    switch = function ()
       if bufcnt(true, 10) then
        return "shoot"
      end
    end,
    Assister = task.stop(),
    Leader   = task.goCmuRush(SYNT_POS_2),
    Special  = task.goCmuRush(ANTI_POS_5),
    Middle   = task.goCmuRush(ANTI_POS_4, _, 100),
    Defender = task.goCmuRush(TMP_SHOOT_POS),
    Goalie   = task.goalie(),
    match    = "{ALDSM}"
  },

  ["shoot"] = {
    switch = function ()
      if bufcnt(player.kickBall("Defender"), "fast", 220) then
        return "exit"
      end
    end,
    Assister = task.stop(),
    Leader   = task.goSupportPos("Defender"),
    Special  = task.leftBack(),
    Middle   = task.rightBack(),
    Defender = task.InterNew(),
    Goalie   = task.goalie(),
    match    = "{ALD}[SM]"
  },

  name = "Ref_CornerKickV8",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}