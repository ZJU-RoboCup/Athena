--by Fantasy


local FINAL_SHOOT_POS = CGeoPoint:new_local(370, 130) -- 360,110
local PASS_POS   = pos.passForTouch(FINAL_SHOOT_POS)
local ANTI_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(140, -160))
local UANTI_POS_1= ball.refAntiYPos(CGeoPoint:new_local(240, -160))
local DANTI_POS_1= ball.refAntiYPos(CGeoPoint:new_local(40, -160))
local ANTI_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(140, -80))
local UANTI_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(240, -80))
local DANTI_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(40, -80))
local ANTI_POS_3 = ball.refAntiYPos(CGeoPoint:new_local(140, 80))
local UANTI_POS_3 = ball.refAntiYPos(CGeoPoint:new_local(240, 80))
local DANTI_POS_3 = ball.refAntiYPos(CGeoPoint:new_local(40, 80))
local ANTI_POS_4 = ball.refAntiYPos(CGeoPoint:new_local(140, 160))
local UANTI_POS_4 = ball.refAntiYPos(CGeoPoint:new_local(240, 160))
local DANTI_POS_4 = ball.refAntiYPos(CGeoPoint:new_local(40, 160))

local SPECIAL_POS = ball.refAntiYPos(CGeoPoint:new_local(100, -200))
local TMP_SHOOT_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(240, 80)),
  ball.refAntiYPos(CGeoPoint:new_local(320, 90)),
  ball.refAntiYPos(CGeoPoint:new_local(300,0)),
  ball.refAntiYPos(FINAL_SHOOT_POS),--pos.passForTouch(FINAL_SHOOT_POS),--

}


local GOALIE_MULTI_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(-80, 140)),
  ball.refAntiYPos(CGeoPoint:new_local(60, 140))
}

local dangerous = true
local USE_CHIP  = false

gPlayTable.CreatePlay{
  firstState = "start",
  ["start"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Leader") < 20 and
                player.toTargetDist("Defender") < 20, 30, 20) then
        return "runUpPos1"
      end
    end,
    Assister = task.staticGetBall(PASS_POS),
    Special  = task.goCmuRush(ANTI_POS_1, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(ANTI_POS_2, _, 600, flag.allow_dss),
    Defender = task.goCmuRush(ANTI_POS_3),
    Leader   = task.goCmuRush(ANTI_POS_4, _, 600, flag.allow_dss),
    Goalie   = dangerous and task.goalie() or task.goCmuRush(ANTI_POS_1, _, 350),
    match    = "{A}{LDMS}"
  },

  ["runUpPos1"] = {
    switch = function ()
      if  bufcnt(player.toTargetDist("Defender") < 20 ,20,40) then
        return "runUpPos2"
      end
    end,
    Assister = USE_CHIP and task.slowGetBall(PASS_POS) or task.staticGetBall(PASS_POS),
    Special  = task.goCmuRush(UANTI_POS_1, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(DANTI_POS_2, _, 600, flag.allow_dss),
    Defender = task.goCmuRush(UANTI_POS_3, _, 600, flag.allow_dss),
    Leader   = task.goCmuRush(DANTI_POS_4, _, 600, flag.allow_dss),
    Goalie   = dangerous and task.goalie() or task.goCmuRush(ANTI_POS_1, _, 350),
    match    = "{ALDMS}"
  },

  ["runUpPos2"] = {
    switch = function ()
        if  bufcnt(player.toTargetDist("Defender") < 20 ,20,40) then
        return "runUpPos3"
      end
    end,
    Assister = USE_CHIP and task.slowGetBall(PASS_POS) or task.staticGetBall(PASS_POS),
    Special  = task.goCmuRush(DANTI_POS_1, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(UANTI_POS_2, _, 600, flag.allow_dss),
    Defender = task.goCmuRush(DANTI_POS_3, _, 600, flag.allow_dss),
    Leader   = task.goCmuRush(UANTI_POS_4, _, 600, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{ALDMS}"
  },

  ["runUpPos3"] = {
    switch = function ()
     if player.toPointDist("Defender", TMP_SHOOT_POS[4]()) < 30 then
        return "passBall"
      end
    end,
    Assister = USE_CHIP and task.slowGetBall(PASS_POS) or task.staticGetBall(PASS_POS),
    Special  = task.goCmuRush(UANTI_POS_1, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(TMP_SHOOT_POS[3], _, 600, flag.allow_dss),--task.goCmuRush(DANTI_POS_2, _, 600, flag.allow_dss),
    Defender = task.goCmuRush(TMP_SHOOT_POS[4], _, 600, flag.allow_dss),--task.runMultiPos(TMP_SHOOT_POS, false, 40),
    Leader   = task.goCmuRush(DANTI_POS_4, _, 600, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{ALDMS}"
  },

  ["passBall"] = {
    switch = function ()
       if player.kickBall("Assister") or player.toBallDist("Assister") > 30 then
        return "waitBall"
      end
    end,
    Assister = USE_CHIP and task.chipPass(PASS_POS, 400) or task.goAndTurnKick(PASS_POS, 650),
    Leader   = task.leftBack(),
    Special  = task.goCmuRush(SPECIAL_POS, _, 600, flag.allow_dss),
    Middle   = task.rightBack(),
    Defender = task.continue(),
    Goalie   = task.goalie(),
    match    = "{ALDMS}"
  },

  ["waitBall"] = {
    switch = function ()
         if bufcnt(player.toPointDist("Defender", TMP_SHOOT_POS[4]()) < 30, "fast", 100) then
        return "shootBall"
      end
    end,
    Assister = task.stop(),
    Leader   = task.leftBack(),
    Special  = task.goCmuRush(SPECIAL_POS, _, 600, flag.allow_dss),
    Middle   = task.rightBack(),
    Defender = task.waitTouch(TMP_SHOOT_POS[4], 0),
    Goalie   = task.goalie(),
    match    = "{ALDMS}"
  },

  ["shootBall"] = {
    switch = function ()
       if bufcnt(player.kickBall("Defender"), "fast", 100)  then
        return "exit"
      end
    end,
    Assister = task.goSupportPos("Defender"),
    Leader   = task.leftBack(),
    Special  = task.goCmuRush(SPECIAL_POS, _, 600, flag.allow_dss),
    Middle   = task.rightBack(),
    Defender = task.waitTouch(TMP_SHOOT_POS[4], 0),
    Goalie   = task.goalie(),
    match    = "{ALDMS}"
  },

  name = "Ref_CornerKickV20",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}
