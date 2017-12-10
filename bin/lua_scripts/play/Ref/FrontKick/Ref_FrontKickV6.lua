-- 针对MRL两盯人两防朝向设计的定位球
-- by zhyaic 2014-04-07

local FINAL_SHOOT_POS = CGeoPoint:new_local(265,238)
local CHIP_POS  = pos.passForTouch(FINAL_SHOOT_POS)
local ANTI_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(150,150))
local ANTI_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(180,170))
local SYNT_UP_POS = ball.backPos(CGeoPoint:new_local(450,0),45,-60)
local SYNT_BLOCK_POS = ball.jamPos(CHIP_POS,65,60)
local TMP_SHOOT_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(180,230)),
  ball.refAntiYPos(CGeoPoint:new_local(230,215)),
  ball.refAntiYPos(FINAL_SHOOT_POS),
}

local USE_CHIP  = true

gPlayTable.CreatePlay{
  firstState = "start",
  ["start"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Leader") < 20 and
                player.toTargetDist("Special") < 20, 30, 180) then
        return "runTmpPos"
      end
    end,
    Assister = task.staticGetBall(CHIP_POS),
    Leader   = task.goCmuRush(ANTI_POS_1),
    Special  = task.goCmuRush(ANTI_POS_2),
    Middle   = task.goCmuRush(SYNT_UP_POS),
    Defender = task.goCmuRush(SYNT_BLOCK_POS),
    Goalie   = task.goalie(),
    match    = "{A}{LSDM}"
  },

  ["runTmpPos"] = {
    switch = function ()
      if bufcnt(player.toPointDist("Leader", TMP_SHOOT_POS[2]()) < 30, 1, 180) then
        return "passBall"
      end
    end,
    Assister = USE_CHIP and task.slowGetBall(CHIP_POS) or task.staticGetBall(CHIP_POS),
    Leader   = task.runMultiPos(TMP_SHOOT_POS),
    Special  = task.goCmuRush(ANTI_POS_2),
    Middle   = task.goCmuRush(SYNT_UP_POS),
    Defender = task.goCmuRush(SYNT_BLOCK_POS),
    Goalie   = task.goalie(),
    match    = "{ALSDM}"
  },

  ["passBall"] = {
    switch = function ()
       if player.kickBall("Assister") or player.toBallDist("Assister")> 30 then
        return "waitBall"
      end
    end,
    Assister = USE_CHIP and task.chipPass(CHIP_POS, 200) or task.touchPass(CHIP_POS,550),
    Leader   = task.continue(),
    Special  = task.goCmuRush(ANTI_POS_2),
    Middle   = task.goCmuRush(SYNT_UP_POS),
    Defender = task.goCmuRush(SYNT_BLOCK_POS),
    Goalie   = task.goalie(),
    match    = "{ALSDM}"
  },

  ["waitBall"] = {
    switch = function ()
      if bufcnt(true, 40) then
        return "shootBall"
      end
    end,
    Assister = task.leftBack(),
    Leader   = task.continue(),
    Special  = task.goCmuRush(ANTI_POS_2),
    Middle   = task.rightBack(),
    Defender = task.stop(),
    Goalie   = task.goalie(),
    match    = "{SLD}[AM]"
  },

  ["shootBall"] = {
    switch = function ()
      if bufcnt(player.kickBall("Leader"), "fast", 120)  then
        print("exit")
        return "exit"
      end
    end,
    Assister = task.leftBack(),
    Leader   = USE_CHIP and task.waitTouch(ball.refAntiYPos(FINAL_SHOOT_POS), 0) or task.waitTouchNew(ball.refAntiYPos(FINAL_SHOOT_POS)),
    Special  = task.goCmuRush(CHIP_POS),
    Middle   = task.rightBack(),
    Defender = task.defendMiddle(),
    Goalie   = task.goalie(),
    match    = "{SLD}[AM]"
  },

  name = "Ref_FrontKickV6",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}