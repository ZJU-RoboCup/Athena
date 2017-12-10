-- by zhyaic 2014-06-26
-- 利用反向挑球进行射门，传球力度和射门力度是要考虑的

local FINAL_SHOOT_POS  = CGeoPoint:new_local(CGeoPoint:new_local(233,20))
local TMP_RECEIVER_POS = ball.refAntiYPos(CGeoPoint:new_local(-50,0))
local SHOOT_POS_1 = ball.refAntiYPos(FINAL_SHOOT_POS)
local BOUND_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(233,42))
local BOUND_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(173,20))
local SIDE_POS  = ball.refAntiYPos(CGeoPoint:new_local(40,170))
local JAM_POS = ball.jamPos(CGeoPoint:new_local(0,200),55,-15)

local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)

local BOUND_POS = function (r)
  return function ()
    return player.pos(r) + Utils.Polar2Vector(26,3.14)
  end
end

local USE_CHIP = true

gPlayTable.CreatePlay{

  firstState = "tmp",

  ["tmp"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Receiver") < 20, 20, 180) then
          return "start"
      end
    end,
    Assister = USE_CHIP and task.staticGetBall(FINAL_SHOOT_POS) or task.staticGetBall(CGeoPoint:new_local(0,200)),
    Middle   = task.goCmuRush(BOUND_POS_1, _,_,DSS_FLAG),
    Receiver = task.goCmuRush(TMP_RECEIVER_POS, _,_,DSS_FLAG),
    Special  = task.goCmuRush(SIDE_POS, _,_,DSS_FLAG),
    Defender = task.goCmuRush(JAM_POS, _,_,DSS_FLAG),
    Goalie   = task.goalie(),
    match    = "{A}[MDS]"
  },

  ["start"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Receiver") < 20 and
          player.toTargetDist("Middle") < 20, 20, 180) then
          return "pass"
      end
    end,
    Assister = USE_CHIP and task.staticGetBall(FINAL_SHOOT_POS) or task.staticGetBall(CGeoPoint:new_local(0,200)),
    Middle   = task.goCmuRush(BOUND_POS_1, _, _, DSS_FLAG),
    Receiver = task.goCmuRush(SHOOT_POS_1,dir.playerToBall),
    Special  = task.goCmuRush(SIDE_POS, _, _, DSS_FLAG),
    Defender = task.goCmuRush(JAM_POS, _, _, DSS_FLAG),
    Goalie   = task.goalie(),
    match    = "{A}[MDS]"
  },

  ["pass"] = {
    switch = function ()
      if player.kickBall("Assister") or player.isBallPassed("Assister","Receiver") then
        if USE_CHIP then
          return "fix"
        else
          return "touch"
        end
      elseif bufcnt(true, 120) then
        return "exit"
      end
    end,
    Assister = USE_CHIP and task.chipPass(FINAL_SHOOT_POS, 160) or task.goAndTurnKick(FINAL_SHOOT_POS),
    Middle   = task.goCmuRush(BOUND_POS_1),
    Receiver = task.goCmuRush(SHOOT_POS_1,dir.playerToBall),
    Special  = task.goCmuRush(SIDE_POS),
    Defender = task.goCmuRush(JAM_POS),
    Goalie   = task.goalie(),
    match    = "{AMDS}"
  },

  ["fix"] = {
    switch = function ()
      if bufcnt(true, 20)then
        return "touch"
      end
    end,
    Receiver = task.goCmuRush(SHOOT_POS_1,dir.playerToBall, _, DSS_FLAG),
    Middle   = task.goCmuRush(BOUND_POS("Receiver"),dir.specified(0), _, DSS_FLAG),
    Assister = task.leftBack(),
    Special  = task.rightBack(),
    Defender = task.defendMiddle(),
    Goalie   = task.goalie(),
    match    = "{M}[DAS]"
  },

  ["touch"] = {
    switch = function ()
      if bufcnt(player.kickBall("Receiver"), 1, 150)then
        return "stop"
      end
    end,
    Receiver = task.receiveChip(dir.specified(180), 999127),
    Middle   = task.goCmuRush(BOUND_POS("Receiver"), dir.specified(0), _, DSS_FLAG),
    Assister = task.leftBack(),
    Special  = task.rightBack(),
    Defender = task.defendMiddle(),
    Goalie   = task.goalie(),
    match    = "{M}[DAS]"
  },

  ["stop"] = {
    switch = function ()
      if  bufcnt(true, 10) then
        return "exit"
      end
    end,
    Receiver = task.stop(),
    Middle   = task.goCmuRush(BOUND_POS("Receiver"),dir.specified(0), _, DSS_FLAG),
    Assister = task.leftBack(),
    Special  = task.rightBack(),
    Defender = task.defendMiddle(),
    Goalie   = task.goalie(),
    match    = "{M}[DAS]"
  },
  -- ["stop"] = {
  --   switch = function ()
  --     if bufcnt(true, 120)then
  --       return "exit"
  --     end
  --   end,
  --   Receiver = task.stop(),
  --   Middle   = task.goCmuRush(BOUND_POS("Receiver"),dir.specified(0)),
  --   Assister = task.leftBack(),
  --   Special  = task.rightBack(),
  --   Defender = task.defendMiddle(),
  --   Goalie   = task.goalie(),
  --   match    = "{MDAS}"
  -- },  

  name = "Ref_FrontKickV7",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}