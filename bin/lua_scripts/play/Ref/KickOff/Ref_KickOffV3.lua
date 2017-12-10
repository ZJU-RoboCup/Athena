--by Fantasy 
-- modify by gty
local FINAL_PASS_POS = CGeoPoint:new_local(350,135)
local PASS_POS   = FINAL_PASS_POS
local FAKE_POS = CGeoPoint:new_local(9, 240)
local ANTI_POS_1 = CGeoPoint:new_local(370, -150)
local ANTI_POS_2 = CGeoPoint:new_local(350, -110)
local ANTI_POS_3 = CGeoPoint:new_local(330, 0)

local SYNT_POS_1 = CGeoPoint:new_local(160, -160)
local SYNT_POS_2 = CGeoPoint:new_local(190, -185)
local BACK_WAIT_POS = CGeoPoint:new_local(-80, 30)
local far_gate = CGeoPoint:new_local(param.pitchLength/2, -300)

local TMP_SHOOT_POS = {
  CGeoPoint:new_local(200,  60),
  CGeoPoint:new_local(380, 150),
  FINAL_PASS_POS
}

local TMP_FAKER_POS= {
  CGeoPoint:new_local(200, 140),
  CGeoPoint:new_local(330, 20),
  CGeoPoint:new_local(350, 10)
}

local dangerous = true
local FLAG_VAL  = -10

local HALF = gOppoConfig.IfHalfField


gPlayTable.CreatePlay{
  firstState = "waitNormalStart",
  ["waitNormalStart"] = {
  switch = function ()
    if cond.isNormalStart() then
      return "start"
    end
  end,
  Assister = task.defendKick(),
  Leader   = task.goSpeciPos(CGeoPoint:new_local(-15, -240)),
  Special  = task.goSpeciPos(CGeoPoint:new_local(-15, 240)),
  Middle   = task.goSpeciPos(CGeoPoint:new_local(-170, 240)),
  Defender = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{A}{LSMD}"
},


  ["start"] = {
    switch = function ()
      --if ball.posX() < FLAG_VAL then return "exit" end
      if bufcnt(player.toTargetDist("Leader") < 20 and
                player.toTargetDist("Defender") < 20, 30, 180) then
        return "runTmpPos"
      end
    end,
    Assister = task.staticGetBall(PASS_POS,false),
    Leader   = task.goCmuRush(TMP_FAKER_POS[1], _, 600, flag.allow_dss),
    Special  = task.goCmuRush(ANTI_POS_1, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(ANTI_POS_2, _, 600, flag.allow_dss),
    Defender = task.goCmuRush(TMP_SHOOT_POS[1]),
    Goalie   = dangerous and task.goalie() or task.goCmuRush(ANTI_POS_1, _, 350),
    match    = "{A}{LDMS}"
  },

  ["runTmpPos"] = {
    switch = function ()
      if ball.posX() < FLAG_VAL then return "exit" end
      if bufcnt(player.toPointDist("Defender", TMP_SHOOT_POS[2]) < 30,10,150)then
        return "passBall"
      end
    end,
    Assister = task.slowGetBall(PASS_POS,_,false),
    Leader   = task.goCmuRush(TMP_FAKER_POS[2], _, 600, flag.allow_dss),
    Special  = task.goCmuRush(ANTI_POS_1, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(ANTI_POS_2, _, 600, flag.allow_dss),
    Defender = task.runMultiPos(TMP_SHOOT_POS, false, 40),
    Goalie   = dangerous and task.goalie() or task.goCmuRush(ANTI_POS_1, _, 350),
    match    = "{ALDMS}"
  },

  ["passBall"] = {
    switch = function ()
      if ball.posX() < FLAG_VAL then return "exit" end
      if player.kickBall("Assister") or player.toBallDist("Assister") > 30 then
        return "waitBall"
      end
    end,
    Assister = task.goAndTurnKick("Defender",450),--task.chipPass(PASS_POS,100,_,false),--task.goAndTurnKick(PASS_POS,300),
    Leader   = task.goCmuRush(TMP_FAKER_POS[3], _, 600, flag.allow_dss),
    Special  = task.goCmuRush(ANTI_POS_3, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(ANTI_POS_2, _, 600, flag.allow_dss),
    Defender = task.continue(),
    Goalie   = task.goalie(),
    match    = "{ALDMS}"
  },


  ["waitBall"] = {
    switch = function ()
      if ball.posX() < FLAG_VAL then return "exit" end
      if bufcnt(player.toPointDist("Defender", TMP_SHOOT_POS[3]) < 20, 10, 20) then
          return "intercept"
      end
    end,
    Assister = task.rightBack(),
    Leader   = task.stop(),
    Special  = task.goCmuRush(ANTI_POS_1, _, 600, flag.allow_dss),
    Middle   = task.leftBack(),
    Defender = task.goCmuRush(TMP_SHOOT_POS[3]),
    Goalie   = task.goalie(),
    match    = "{ALD}[MS]"
  },

  ["intercept"] = {
    switch = function ()
      if ball.posX() < FLAG_VAL then return "exit" end
      if bufcnt(ball.velMod() < 30 and ball.toPlayerDist("Defender") < 30 , 40, 150)  then
        return "shootBall"
      end
    end,
    Assister = task.rightBack(),
    Leader   = task.goSupportPos("Defender"), 
    Special  = task.stop(),
    Middle   = task.leftBack(),
    Defender = task.receiveShoot(),
    Goalie   = task.goalie(),
    match    = "{ALD}[MS]"
  },

  ["shootBall"] = {
    switch = function ()
      if ball.posX() < FLAG_VAL then return "exit" end
      if bufcnt(player.kickBall("Defender"), "fast", 200)  then
        return "exit"
      end
    end,
    Assister = task.rightBack(),
    Leader   = task.goSupportPos("Defender"), 
    Special  = task.stop(),
    Middle   = task.leftBack(),
    Defender = task.dribbleTurnShoot(), ---task.InterceptNew(),
    Goalie   = task.goalie(),
    match    = "{ALD}[MS]"
  },

  name = "Ref_KickOffV3",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}