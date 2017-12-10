-- 守门员上
-- by yys 2015-07-11

local FINAL_SHOOT_POS = CGeoPoint:new_local(360, 90)
local PASS_POS   = pos.passForTouch(FINAL_SHOOT_POS)

local SYNT_POS_1 = ball.refSyntYPos(CGeoPoint:new_local(290, 250))
local SYNT_POS_2 = ball.refSyntYPos(CGeoPoint:new_local(200, 250))
local SYNT_POS_3 = ball.refSyntYPos(CGeoPoint:new_local(100, 250))
local SYNT_POS_4 = ball.refSyntYPos(CGeoPoint:new_local(40, 250))

local BACK_WAIT_POS = ball.refAntiYPos(CGeoPoint:new_local(20, 50))

local TMP_SHOOT_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(70, -180)),
  ball.refAntiYPos(CGeoPoint:new_local(240, 20)),
  ball.refAntiYPos(CGeoPoint:new_local(310, 130)),
  ball.refAntiYPos(FINAL_SHOOT_POS)
}

local USE_CHIP  = true

gPlayTable.CreatePlay{
  firstState = "start",
  ["start"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Leader") < 20 and
                player.toTargetDist("Defender") < 20, 30, 180) then
        return "runTmpPos"
      end
    end,
    Assister = task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(SYNT_POS_1, _, _, flag.allow_dss),
    Special  = task.goCmuRush(SYNT_POS_2, _, _, flag.allow_dss),
    Middle   = task.goCmuRush(SYNT_POS_3, _, _, flag.allow_dss),
    Defender = task.goCmuRush(SYNT_POS_4, _, _, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{A}{LDMS}"
  },

  ["runTmpPos"] = {
    switch = function ()
      if player.toPointDist("Goalie", TMP_SHOOT_POS[2]()) < 50 then
        return "dribble"
      end
    end,
    Assister = task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(SYNT_POS_1, _, _, flag.allow_dss),
    Special  = task.goCmuRush(SYNT_POS_2, _, _, flag.allow_dss),
    Middle   = task.goCmuRush(SYNT_POS_3, _, _, flag.allow_dss),
    Defender = task.goCmuRush(SYNT_POS_4, _, _, flag.allow_dss),
    Goalie   = task.runMultiPos(TMP_SHOOT_POS, false, 40, _, 480),
    match    = "{ALDMS}"
  },

  ["dribble"] = {
    switch = function ()
      if player.toPointDist("Goalie", TMP_SHOOT_POS[3]()) < 30 then
        return "passBall"
      end
    end,
    Assister = USE_CHIP and task.slowGetBall(PASS_POS) or task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(SYNT_POS_1, _, _, flag.allow_dss),
    Special  = task.goCmuRush(SYNT_POS_2, _, _, flag.allow_dss),
    Middle   = task.goCmuRush(SYNT_POS_3, _, _, flag.allow_dss),
    Defender = task.goCmuRush(SYNT_POS_4, _, _, flag.allow_dss),
    Goalie   = task.continue(),
    match    = "{ALDMS}"
  },

  ["passBall"] = {
    switch = function ()
       if player.kickBall("Assister") or player.toBallDist("Assister") > 30 then
        return "waitBall"
      end
    end,
    Assister = USE_CHIP and task.chipPass(PASS_POS, 180) or task.goAndTurnKick(PASS_POS, 450),
    Leader   = task.goCmuRush(SYNT_POS_1, _, _, flag.allow_dss),
    Special  = task.goCmuRush(SYNT_POS_1, _, _, flag.allow_dss),
    Middle   = task.goCmuRush(SYNT_POS_3, _, _, flag.allow_dss),
    Defender = task.goCmuRush(SYNT_POS_4, _, _, flag.allow_dss),
    Goalie   = task.continue(),
    match    = "{ALDMS}"
  },

  ["waitBall"] = {
    switch = function ()
      if bufcnt(player.toPointDist("Goalie", TMP_SHOOT_POS[4]()) < 40, "fast", 60) then
        return "shootBall"
      end
    end,
    Assister = task.stop(),
    Leader   = task.goCmuRush(SYNT_POS_1, _, _, flag.allow_dss),
    Special  = task.defendMiddle(),
    Middle   = task.rightBack(),
    Defender = task.leftBack(),
    Goalie   = task.continue(),
    match    = "{AL}[DMS]"
  },

  ["shootBall"] = {
    switch = function ()
      if bufcnt(player.kickBall("Goalie"), "fast", 100)  then
        return "goalieBack"
      end
    end,
    Assister = task.goSupportPos("Goalie"),
    Leader   = task.goCmuRush(SYNT_POS_1, _, _, flag.allow_dss),
    Special  = task.defendMiddle(),
    Middle   = task.rightBack(),
    Defender = task.leftBack(),
    Goalie   = task.waitTouch(TMP_SHOOT_POS[4], 0),
    match    = "{AL}[DMS]"
  },

  ["goalieBack"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Goalie") < 40, 'fast', 180) then
        return "exit"
      end
    end,
    Assister = task.goSupportPos("Leader"),
    Leader   = task.advance(),
    Special  = task.defendMiddle(),
    Middle   = task.rightBack(),
    Defender = task.leftBack(),
    Goalie   = task.goCmuRush(CGeoPoint:new_local(-400, 0), 3.14, 400, flag.allow_dss),
    match    = "[AL][DMS]"
  },

  name = "Ref_CornerKickV18",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}