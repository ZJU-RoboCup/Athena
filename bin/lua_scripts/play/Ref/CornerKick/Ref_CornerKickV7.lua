-- 针对MRL两盯人两防朝向设计的角球定位球
-- by zhyaic 2014-04-07
-- 2014-07-20 yys 改

local FINAL_SHOOT_POS = CGeoPoint:new_local(360, 90)
local PASS_POS   = pos.passForTouch(FINAL_SHOOT_POS)
local ANTI_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(60, 140))
local ANTI_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(380, 240))
local SYNT_POS_1 = ball.refSyntYPos(CGeoPoint:new_local(160, 160))
local SYNT_POS_2 = ball.refSyntYPos(CGeoPoint:new_local(190, 185))
local BACK_WAIT_POS = ball.refAntiYPos(CGeoPoint:new_local(-80, -30))

local TMP_SHOOT_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(150, 50)),
  ball.refAntiYPos(CGeoPoint:new_local(200, 110)),
  ball.refAntiYPos(CGeoPoint:new_local(300, 150)),
  ball.refAntiYPos(PASS_POS)
}

local GOALIE_MULTI_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(-80, 140)),
  ball.refAntiYPos(CGeoPoint:new_local(60, 140))
}

local dangerous = true
local USE_CHIP  = true

local TEST = true

gPlayTable.CreatePlay{
  firstState = "start",
  ["start"] = {
    switch = function ()
      if TEST then print("start") end
      if bufcnt(player.toTargetDist("Leader") < 20 and
                player.toTargetDist("Defender") < 20, 30, 180) then
        return "runTmpPos"
      end
    end,
    Assister = task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(SYNT_POS_2, _, 600, flag.allow_dss),
    Special  = task.goCmuRush(SYNT_POS_1, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(ANTI_POS_2, _, 600, flag.allow_dss),
    Defender = task.goCmuRush(BACK_WAIT_POS),
    Goalie   = dangerous and task.goalie() or task.goCmuRush(ANTI_POS_1, _, 350),
    match    = "{A}{LDMS}"
  },

  ["runTmpPos"] = {
    switch = function ()
      if TEST then print("runTmpPos") end
      if player.toPointDist("Defender", TMP_SHOOT_POS[2]()) < 70 then
        return "passBall"
      end
    end,
    Assister = USE_CHIP and task.slowGetBall(PASS_POS) or task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(SYNT_POS_2, _, 600, flag.allow_dss),
    Special  = task.goCmuRush(SYNT_POS_1, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(ANTI_POS_2, _, 600, flag.allow_dss),
    Defender = task.runMultiPos(TMP_SHOOT_POS, false, 40),
    Goalie   = dangerous and task.goalie() or task.goCmuRush(ANTI_POS_1, _, 350),
    match    = "{ALDMS}"
  },

  ["passBall"] = {
    switch = function ()
      if TEST then print("passBall") end
      if player.kickBall("Assister") or player.toBallDist("Assister") > 30 then
        return "waitBall"
      end
    end,
    Assister = USE_CHIP and task.chipPass(PASS_POS, 120) or task.goAndTurnKick(PASS_POS, 450),
    Leader   = task.goCmuRush(SYNT_POS_2, _, 600, flag.allow_dss),
    Special  = task.goCmuRush(SYNT_POS_1, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(ANTI_POS_2, _, 600, flag.allow_dss),
    Defender = task.continue(),
    Goalie   = task.goalie(),
    match    = "{ALDMS}"
  },

  ["waitBall"] = {
    switch = function ()
      if TEST then print("waitBall") end
      if bufcnt(true, 30) then
        return "shootBall"
      end
    end,
    Assister = task.stop(),
    Leader   = task.goCmuRush(SYNT_POS_2, _, 600, flag.allow_dss),
    Special  = task.leftBack(),
    Middle   = task.rightBack(),
    Defender = task.continue(),
    Goalie   = task.goalie(),
    match    = "{ALD}[MS]"
  },

  ["shootBall"] = {
    switch = function ()
      if TEST then print("shootBall") end
      if bufcnt(player.kickBall("Defender"), "fast", 100)  then
        return "exit"
      end
    end,
    Assister = task.goSupportPos("Defender"),
    Leader   = task.goCmuRush(SYNT_POS_2, _, 600, flag.allow_dss),
    Special  = task.leftBack(),
    Middle   = task.rightBack(),
    Defender = task.waitTouch(TMP_SHOOT_POS[4], 0),
    Goalie   = task.goalie(),
    match    = "{ALD}[MS]"
  },

  name = "Ref_CornerKickV7",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}