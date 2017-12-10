-- by yys 2015-07-11

local FINAL_SHOOT_POS = CGeoPoint:new_local(360, 110)
local PASS_POS   = FINAL_SHOOT_POS--pos.passForTouch(FINAL_SHOOT_POS)

local SYNT_POS_1 = ball.refSyntYPos(CGeoPoint:new_local(290, 250))
local SYNT_POS_2 = ball.refSyntYPos(CGeoPoint:new_local(200, 250))
local SYNT_POS_3 = ball.refSyntYPos(CGeoPoint:new_local(100, 250))

local BACK_WAIT_POS = ball.refAntiYPos(CGeoPoint:new_local(20, 80))

local TMP_SHOOT_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(70, -180)),
  ball.refAntiYPos(CGeoPoint:new_local(240, 20)),
  ball.refAntiYPos(CGeoPoint:new_local(310, 130)),
  ball.refAntiYPos(FINAL_SHOOT_POS)
}

local dangerous = false
local USE_CHIP  = true

gPlayTable.CreatePlay{
  firstState = "start",
  ["start"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Leader") < 20 and
                player.toTargetDist("Defender") < 20, 30, 40) or bufcnt(true,"fast",120) then
        return "runTmpPos"
      end
    end,
    Assister = task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(SYNT_POS_1),
    Special  = task.goCmuRush(SYNT_POS_2, _, _, flag.allow_dss),
    Middle   = task.goCmuRush(SYNT_POS_3, _, _, flag.allow_dss),
    Defender = task.singleBack(),
    Goalie   = dangerous and task.goalie() or task.goCmuRush(BACK_WAIT_POS,_,500),
    match    = "{A}{LDMS}"
  },

  ["runTmpPos"] = {
    switch = function ()
      if player.toPointDist("Defender", TMP_SHOOT_POS[1]()) < 50 then
        return "dribble"
      end
    end,
    Assister = task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(SYNT_POS_1, _, _, flag.allow_dss),
    Special  = task.goCmuRush(SYNT_POS_2, _, _, flag.allow_dss),
    Middle   = task.goCmuRush(SYNT_POS_3, _, _, flag.allow_dss),
    Defender = task.goCmuRush(TMP_SHOOT_POS[1]),--runMultiPos(TMP_SHOOT_POS, false, 40,_,400),
    Goalie   = dangerous and task.goalie() or task.goCmuRush(BACK_WAIT_POS,_,500),
    match    = "{ALDMS}"
  },

  ["dribble"] = {
    switch = function ()
      if player.toPointDist("Defender", TMP_SHOOT_POS[3]()) < 30 then
        return "passBall"
      end
    end,
    Assister = task.staticGetBall(PASS_POS),
    Leader   = task.goCmuRush(SYNT_POS_1, _, _, flag.allow_dss),
    Special  = task.goCmuRush(SYNT_POS_2, _, _, flag.allow_dss),
    Middle   = task.goCmuRush(SYNT_POS_3, _, _, flag.allow_dss),
    Defender = task.goCmuRush(TMP_SHOOT_POS[3]),--task.continue(),
    Goalie   = dangerous and task.goalie() or task.goCmuRush(CGeoPoint:new_local(-400,0), 3.14, 300, flag.allow_dss),
    match    = "{ALDMS}"
  },

  ["passBall"] = {
    switch = function ()
       if player.kickBall("Assister") or player.toBallDist("Assister") > 30 then
          if USE_CHIP then
            return "gofix"
          else
            return "shootBall"--return "waitBall"
          end
      end
    end,
    Assister = USE_CHIP and task.chipPass(PASS_POS, 130) or task.goAndTurnKick(PASS_POS, 600),
    Leader   = task.goCmuRush(SYNT_POS_1, _, _, flag.allow_dss),
    Special  = task.goCmuRush(SYNT_POS_1, _, _, flag.allow_dss),
    Middle   = task.goCmuRush(SYNT_POS_3, _, _, flag.allow_dss),
    Defender = task.goCmuRush(TMP_SHOOT_POS[4]),
    Goalie   = dangerous and task.goalie() or task.goCmuRush(CGeoPoint:new_local(-400,0), 3.14, 300, flag.allow_dss),
    match    = "{ALDMS}"
  },

  ["gofix"] = {
    switch = function ()
       if bufcnt(true, 5) then
        return "shootBall"
      end
    end,
    Assister = task.stop(),
    Leader   = task.goCmuRush(SYNT_POS_1, _, _, flag.allow_dss),
    Special  = task.goCmuRush(SYNT_POS_1, _, _, flag.allow_dss),
    Middle   = task.goCmuRush(SYNT_POS_3, _, _, flag.allow_dss),
    Defender = task.goCmuRush(TMP_SHOOT_POS[4]),
    Goalie   = task.goalie(),
    match    = "{ALDSM}"
  },
  -- ["waitBall"] = {
  --   switch = function ()
  --     if bufcnt(player.toPointDist("Defender", TMP_SHOOT_POS[4]()) < 40, "fast", 60) then
  --       return "shootBall"
  --     end
  --   end,
  --   Assister = task.stop(),
  --   Leader   = task.goCmuRush(SYNT_POS_1, _, _, flag.allow_dss),
  --   Special  = task.continue(),--task.leftBack(),
  --   Middle   = task.continue(),--task.rightBack(),
  --   Defender = task.waitTouchNew(TMP_SHOOT_POS[4]),
  --   Goalie   = dangerous and task.goalie() or task.goCmuRush(CGeoPoint:new_local(-400,0), 3.14, 200, flag.allow_dss),
  --   match    = "{ALD}[MS]"
  -- },

  ["shootBall"] = {
    switch = function ()
      if bufcnt(player.kickBall("Defender"), "fast", 60)  then
        return "exit"
      end
    end,
    Assister = task.goSupportPos("Defender"),
    Leader   = task.goCmuRush(SYNT_POS_1, _, _, flag.allow_dss),
    Special  = task.leftBack(),
    Middle   = task.rightBack(),
    Defender = task.waitTouch(TMP_SHOOT_POS[4],1.57),--task.waitTouchNew(TMP_SHOOT_POS[4]),--task.waitTouch(TMP_SHOOT_POS[4],1.57),--task.waitTouchNew(TMP_SHOOT_POS[4]),
    Goalie   = task.goalie(),
    match    = "{ALD}[MS]"
  },

  name = "Ref_CornerKickV17",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}