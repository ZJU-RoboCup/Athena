--by Fantasy
local FINAL_SHOOT_POS = CGeoPoint:new_local(363,130)
local FINAL_PASS_POS = CGeoPoint:new_local(360,130)
local PASS_POS   = pos.passForTouch(FINAL_PASS_POS)
local TEST_SHOOT_ANTIPOS = ball.refAntiYPos(FINAL_SHOOT_POS)

local ANTI_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(370, -150))
local ANTI_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(350, -110))
local ANTI_POS_3 = ball.refAntiYPos(CGeoPoint:new_local(330, 0))

local SYNT_POS_1 = ball.refSyntYPos(CGeoPoint:new_local(160, 160))
local SYNT_POS_2 = ball.refSyntYPos(CGeoPoint:new_local(190, 185))
local BACK_WAIT_POS = ball.refAntiYPos(CGeoPoint:new_local(-80, -30))

local TMP_SHOOT_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(220,  60)),
  ball.refAntiYPos(CGeoPoint:new_local(400, 200)),--{410,200}
  --ball.refAntiYPos(CGeoPoint:new_local(360, 150)),
  ball.refAntiYPos(CGeoPoint:new_local(330, 100)),
  ball.refAntiYPos(CGeoPoint:new_local(380, 135))
}

local TMP_FAKER_POS= {
  ball.refAntiYPos(CGeoPoint:new_local(200, 140)),
  ball.refAntiYPos(CGeoPoint:new_local(330, 20)),
  ball.refAntiYPos(CGeoPoint:new_local(350, -10))
}


local GOALIE_MULTI_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(-80, 140)),
  ball.refAntiYPos(CGeoPoint:new_local(60, 140))
}

-- pos for halffield mode
local OTHER_POS = {
  CGeoPoint:new_local(80,-100),
  CGeoPoint:new_local(80,100),
  CGeoPoint:new_local(80,-130),
}
local BACK_POS = CGeoPoint:new_local(80,0)
--
local dangerous = true
local USE_CHIP  = true
local FLAG_VAL  = 200

local HALF = gOppoConfig.IfHalfField


gPlayTable.CreatePlay{
  firstState = "start",
  ["start"] = {
    switch = function ()
      --if HALF then print("HALF : true") else print("HALF : false") end
      if ball.posX() < FLAG_VAL then return "exit" end
      if bufcnt(player.toTargetDist("Leader") < 20 and
                player.toTargetDist("Defender") < 20, 30, 180) then
        return "wait"
      end
    end,
    Assister = task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(TMP_FAKER_POS[1], _, 600, flag.allow_dss),
    Special  = task.goCmuRush(ANTI_POS_1, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(ANTI_POS_2, _, 600, flag.allow_dss),
    Defender = task.goCmuRush(TMP_SHOOT_POS[1]),
    Goalie   = dangerous and (HALF and task.goCmuRush(BACK_POS, _, 600, flag.allow_dss) or task.goalie()) or task.goCmuRush(ANTI_POS_1, _, 350),
    match    = "{AD}{LMS}"
  },
  ["wait"] = {
    switch = function()
      if bufcnt(true,40) then
        return "runTmpPos"
      end
    end,
    Assister = task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(TMP_FAKER_POS[1], _, 600, flag.allow_dss),
    Special  = task.goCmuRush(ANTI_POS_1, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(ANTI_POS_2, _, 600, flag.allow_dss),
    Defender = task.runMultiPos(TMP_SHOOT_POS, false, 30),
    Goalie   = dangerous and (HALF and task.goCmuRush(BACK_POS, _, 600, flag.allow_dss) or task.goalie()) or task.goCmuRush(ANTI_POS_1, _, 350),
    match    = "{ALDMS}"
  },
  ["runTmpPos"] = {
    switch = function ()
      if ball.posX() < FLAG_VAL then return "exit" end
      if bufcnt(true,80) then
        return "passBall"
      end
      -- if player.toPointDist("Defender", TMP_SHOOT_POS[3]()) < 30 then
      --   return "passBall"
      -- end
    end,
    Assister = task.staticGetBall(PASS_POS),--USE_CHIP and task.slowGetBall(PASS_POS) or task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(TMP_FAKER_POS[2], _, 600, flag.allow_dss),
    Special  = task.goCmuRush(ANTI_POS_1, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(ANTI_POS_2, _, 600, flag.allow_dss),
    Defender = task.continue(),--task.runMultiPos(TMP_SHOOT_POS, false, 40),
    Goalie   = dangerous and (HALF and task.goCmuRush(BACK_POS, _, 600, flag.allow_dss) or task.goalie()) or task.goCmuRush(ANTI_POS_1, _, 350),
    match    = "{ALDMS}"
  },

  ["passBall"] = {
    switch = function ()
      if ball.posX() < FLAG_VAL then return "exit" end
      if player.kickBall("Assister") or player.toBallDist("Assister") > 30 then
        if USE_CHIP then 
          return "fix"
        else
          return "shootBall"
        end
      end
    end,
    Assister = USE_CHIP and task.chipPass(PASS_POS, 180) or task.goAndTurnKick(PASS_POS, 300),
    Leader   = task.goCmuRush(TMP_FAKER_POS[3], _, 600, flag.allow_dss),
    Special  = task.goCmuRush(ANTI_POS_3, _, 600, flag.allow_dss),
    Middle   = task.goCmuRush(ANTI_POS_2, _, 600, flag.allow_dss),
    Defender = task.continue(),
    Goalie   = HALF and task.goCmuRush(BACK_POS, _, 600, flag.allow_dss) or task.goalie(),
    match    = "{ALDMS}"
  },


  -- ["waitBall"] = {
  --   switch = function ()
  --     if ball.posX() < FLAG_VAL then return "exit" end
  --     if bufcnt(player.toPointDist("Defender", TMP_SHOOT_POS[4]()) < 20, "fast", 20) then
  --       if USE_CHIP then 
  --         return "fix"
  --       else
  --         return "shootBall"
  --       end
  --     end
  --   end,
  --   Assister = task.stop(),
  --   Leader   = HALF and task.stop() or task.rightBack(),
  --   Special  = task.goCmuRush(ANTI_POS_1, _, 600, flag.allow_dss),
  --   Middle   = HALF and task.stop() or task.leftBack(),
  --   Defender = task.goCmuRush(TMP_SHOOT_POS[4]),
  --   Goalie   = task.goalie(),
  --   match    = "{ALD}[MS]"
  -- },
  ["fix"] = {
    switch = function ()
      if ball.posX() < FLAG_VAL then return "exit" end
      if bufcnt(true,20) then
        return "shootBall"
      end  
    end,
    Assister = task.stop(),
    Leader   = HALF and task.goCmuRush(OTHER_POS[2], _, 600, flag.allow_dss) or task.rightBack(),
    Special  = task.goCmuRush(ANTI_POS_1, _, 600, flag.allow_dss),
    Middle   = HALF and task.goCmuRush(OTHER_POS[1], _, 600, flag.allow_dss) or task.leftBack(),
    Defender = task.goCmuRush(TMP_SHOOT_POS[4]),
    Goalie   = HALF and task.goCmuRush(BACK_POS, _, 600, flag.allow_dss) or task.goalie(),
    match    = "{ALD}[MS]"
  },
  ["shootBall"] = {
    switch = function ()
      if bufcnt(player.kickBall("Defender"), 1, 150)  then
        return HALF and "stop" or "exit"
      end
    end,
    Assister = HALF and task.goCmuRush(OTHER_POS[3], _, 600, flag.allow_dss) or task.goSupportPos("Defender"),
    Leader   = task.goCmuRush(SYNT_POS_2, _, 600, flag.allow_dss),
    Special  = HALF and task.goCmuRush(OTHER_POS[2], _, 600, flag.allow_dss) or task.leftBack(),
    Middle   = HALF and task.stop() or task.rightBack(),
    Defender = task.InterTouch(_, _, 999127),--InterTouch(),--task.InterTouch(TMP_SHOOT_POS[4]),--task.InterTouch(TMP_SHOOT_POS[4]),--task.waitTouchNew(TMP_SHOOT_POS[4]),
    Goalie   = HALF and task.goCmuRush(BACK_POS, _, 600, flag.allow_dss) or task.goalie(),
    match    = "{ALD}[MS]"
  },
  ["stop"] = {
    switch = function ()
        return "stop"
    end,
    Assister = task.stop(),
    Leader   = task.stop(),
    Special  = task.stop(),
    Middle   = task.stop(),
    Defender = task.stop(),
    Goalie   = task.stop(),
    match    = "{ALD}[MS]"
  },

  name = "Ref_CornerKickV1716",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}