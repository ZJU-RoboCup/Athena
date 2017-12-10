--针对对方少车的情况写的角球脚本 by Fantasy

local dangerous = true
local USE_CHIP  = true
local USE_FAKE_DIR = true

local REAL_SHOOT_POS = CGeoPoint:new_local(375, 115) 
local FAKE_SHOOT_POS = CGeoPoint:new_local(340, 70)





--[1]
local DEFENDER_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(100, 80))
local LEADER_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(165, -100))   --FAKER
local MIDDLE_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(100, -100))  
local SPEFCIAL_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(235, -100)) 
local ASSISTER_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(-100, -250)) 



--[2]
local TMP_MIDDLE_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(240, -100)),
  ball.refAntiYPos(CGeoPoint:new_local(260, -100)),
  ball.refAntiYPos(CGeoPoint:new_local(230, -200)) 
}

local TMP_DEFENDER_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(190, 200)),
  ball.refAntiYPos(CGeoPoint:new_local(290, 200)),
  ball.refAntiYPos(CGeoPoint:new_local(315, 130)),
  ball.refAntiYPos(CGeoPoint:new_local(360, 130)),
  ball.refAntiYPos(REAL_SHOOT_POS)
}

local TMP_LEADER_POS= {
  ball.refAntiYPos(CGeoPoint:new_local(165, -5)),
  ball.refAntiYPos(CGeoPoint:new_local(300, 50)),
  ball.refAntiYPos(FAKE_SHOOT_POS)
}

local TMP_SPECIAL_POS= {
  ball.refAntiYPos(CGeoPoint:new_local(300, -180)),
  ball.refAntiYPos(CGeoPoint:new_local(365, -155))
}

local PASS_POS   = pos.passForTouch(TMP_MIDDLE_POS[3])
local FACE_POS = USE_FAKE_DIR and CGeoPoint:new_local(450, -300) or PASS_POS

local GOALIE_MULTI_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(-80, 140)),
  ball.refAntiYPos(CGeoPoint:new_local(60, 140))
}


gPlayTable.CreatePlay{
  firstState = "start",
  ["start"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Leader") < 20 and
                player.toTargetDist("Defender") < 10 and
                player.toTargetDist("Special") < 20, 30, 180) then
        return "runTmpPos"
      end
    end,
    Assister = task.staticGetBall(FACE_POS),
    Leader   = task.goCmuRush(LEADER_POS_1, _, 600, flag.allow_dss),
    Special  = task.goCmuRush(SPEFCIAL_POS_1, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(MIDDLE_POS_1, _, 600, flag.allow_dss),
    Defender = task.goCmuRush(DEFENDER_POS_1, _, 600, flag.allow_dss),
    Goalie   = dangerous and task.goalie() or task.goCmuRush(ANTI_POS_1, _, 350),
    match    = "{A}{DMLS}"
  },

  ["runTmpPos"] = {
    switch = function ()
      if player.toPointDist("Middle", TMP_MIDDLE_POS[2]()) < 40 then
        return "passBall"
      end
    end,
    Assister = task.staticGetBall(FACE_POS),
    Leader   = task.runMultiPos(TMP_LEADER_POS, false, 40),
    Special  = task.runMultiPos(TMP_SPECIAL_POS, false, 40),
    Middle   = task.runMultiPos(TMP_MIDDLE_POS, false, 40),
    Defender = task.runMultiPos(TMP_DEFENDER_POS, false, 40),
    Goalie   = dangerous and task.goalie() or task.goCmuRush(ANTI_POS_1, _, 350),
    match    = "{ALDMS}"
  },

  ["passBall"] = {
    switch = function ()
       if player.kickBall("Assister") or player.toBallDist("Assister") > 30 then
        return "waitBall"
      end
    end,
    Assister = USE_CHIP and task.goAndTurnKick(PASS_POS, 400) or task.chipPass("Middle",100),
    Leader   = task.continue(),
    Special  = task.continue(),
    Middle   = task.continue(),
    Defender = task.continue(),
    Goalie   = task.goalie(),
    match    = "{ALDMS}"
  },

  ["waitBall"] = {
    switch = function ()
      if bufcnt(player.toPointDist("Middle",  TMP_MIDDLE_POS[3]()) < 20, "fast", 100) then
        return "shootBall"
      end
    end,
    Assister = task.stop(),
    Leader   = task.continue(),
    Special  = task.stop(),
    Middle   = task.goCmuRush(TMP_MIDDLE_POS[3], _, 600, flag.allow_dss), 
    Defender = task.continue(),
    Goalie   = task.goalie(),
    match    = "{ALDMS}"
  },

  ["shootBall"] = {
    switch = function ()
      if bufcnt(player.kickBall("Defender"), "fast", 60)  then
        return "exit"
      end
    end,
    Assister = task.goCmuRush(ASSISTER_POS_1, _, 600, flag.allow_dss),
    Leader   = task.singleBack(),
    Special  = task.stop(),
    Middle   = task.InterTouch(),
    Defender = task.goSupportPos("Middle"),
    Goalie   = task.goalie(),
    match    = "{ALDMS}"
  },

  name = "Ref_CornerKickV31",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}