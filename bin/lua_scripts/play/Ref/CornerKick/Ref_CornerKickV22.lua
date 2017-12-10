local FIRST_POS_1 = function()
  return (ball.refAntiYPos(CGeoPoint:new_local(ball.posX() > 350 and 260 or ball.posX() - 80, -195)))()
end
local FIRST_POS_2 = function()
  return (ball.refAntiYPos(CGeoPoint:new_local(ball.posX() > 350 and 210 or ball.posX() - 150, -195)))()
end
local FIRST_POS_3 = function()
  return (ball.refAntiYPos(CGeoPoint:new_local(ball.posX() > 350 and 145 or ball.posX() - 210, -195)))()
end
local FIRST_POS_4 = function()
  return (ball.refAntiYPos(CGeoPoint:new_local(ball.posX() > 350 and 90  or ball.posX() - 270, -195)))()
end

local SECOND_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(200, 225))
local SECOND_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(200, 75))
local SECOND_POS_3 = ball.refAntiYPos(CGeoPoint:new_local(200, -225))
local SECOND_POS_4 = ball.refAntiYPos(CGeoPoint:new_local(200, -75)) 

local THIRD_POS_1  = ball.refAntiYPos(CGeoPoint:new_local(380, 240))
local THIRD_POS_2  = ball.refAntiYPos(CGeoPoint:new_local(290, 195))
local THIRD_POS_3  = function()
  return (ball.refAntiYPos(CGeoPoint:new_local(player.posX("Assister") - 50, -math.abs(player.posY("Assister")))))()
end
local THIRD_POS_4  = ball.refAntiYPos(CGeoPoint:new_local(185, -75))

local PASS_POS = pos.passForTouch(THIRD_POS_4)

local dangerous = true
local USE_CHIP  = true

gPlayTable.CreatePlay{

  firstState = "start",

  ["start"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Assister") < 20 and
                player.toTargetDist("Leader") < 20 and
                player.toTargetDist("Special") < 20, 20, 180) then
        return "ready"
      end
    end,
    Assister = task.staticGetBall(PASS_POS()),
    Leader   = task.goCmuRush(ball.refAntiYPos(CGeoPoint:new_local(270, 180)), _, 500, flag.allow_dss),
    Special  = task.goCmuRush(ball.refAntiYPos(CGeoPoint:new_local(240, 150)), _, 500, flag.allow_dss),
    Middle   = task.goCmuRush(ball.refAntiYPos(CGeoPoint:new_local(100, -180)), _, 500, flag.allow_dss),
    Defender = task.stop(),
    Goalie   = task.goalie(),
    match    = "{A}{LSMD}"
  },

  ["ready"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Assister") < 20 and
                player.toTargetDist("Leader") < 20 and
                player.toTargetDist("Special") < 20 and
                player.toTargetDist("Defender") < 20, 20, 180) then
                
        return "runByLine"
      end
    end,
    Assister = task.staticGetBall(PASS_POS()),
    Leader   = task.goCmuRush(FIRST_POS_1, _, _, flag.allow_dss),
    Special  = task.goCmuRush(FIRST_POS_2, _, _, flag.allow_dss),
    Middle   = task.goCmuRush(FIRST_POS_3, _, _, flag.allow_dss),
    Defender = task.goCmuRush(FIRST_POS_4, _, _, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{A}{LSMD}"
  },

  ["runByLine"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Defender") < 20, 15, 180) then
        return "passBall"
      end
    end,
    Assister = USE_CHIP and task.slowGetBall(PASS_POS()) or task.staticGetBall(PASS_POS()),
    Leader   = task.goCmuRush(SECOND_POS_1, _, _, flag.allow_dss),
    Special  = task.goCmuRush(SECOND_POS_2, _, _, flag.allow_dss),
    Middle   = task.goCmuRush(SECOND_POS_3, _, _, flag.allow_dss),
    Defender = task.goCmuRush(SECOND_POS_4, _, _, flag.allow_dss),
    Goalie   = dangerous and task.goalie() or task.goCmuRush(ball.refAntiYPos(CGeoPoint:new_local(-100, 100))),
    match    = "{ALSMD}"
  },

  ["passBall"] = {
    switch = function ()
      if player.kickBall("Assister") or player.toBallDist("Assister") > 30 then
        --false,10) then
        if USE_CHIP then
          return "fix"
        else
          return "shoot"
        end
      end
    end,
    Assister = USE_CHIP and task.chipPass(PASS_POS(), 160) or task.goAndTurnKick(PASS_POS(), 500),
    Leader   = task.goCmuRush(THIRD_POS_1, _, _, flag.allow_dss),
    Special  = task.goCmuRush(THIRD_POS_2, _, _, flag.allow_dss),
    Middle   = task.goCmuRush(THIRD_POS_3, _, _, flag.allow_dss),
    Defender = task.goCmuRush(THIRD_POS_4),
    Goalie   = task.goalie(),
    match    = "{ALSMD}"
  },

  ["fix"] = {
    switch = function ()
      if bufcnt(true, 10) then
        return "shoot"
      end
    end,
    Assister = task.stop(),
    Leader   = task.goCmuRush(THIRD_POS_1, _, _, flag.allow_dss),
    Special  = task.goCmuRush(THIRD_POS_2, _, _, flag.allow_dss),
    Middle   = task.goCmuRush(THIRD_POS_3, _, _, flag.allow_dss),
    Defender = task.goCmuRush(THIRD_POS_4),
    Goalie   = task.goalie(),
    match    = "{ALSMD}"
  },

  ["shoot"] = {
    switch = function ()
      if player.kickBall("Defender") then
        return "finish"
      elseif bufcnt(true, 80) then
        return "exit"
      end
    end,
    Assister = task.stop(),
    Leader   = task.goCmuRush(THIRD_POS_2, _, 500, flag.allow_dss),
    Special  = task.rightBack(),
    Middle   = task.leftBack(),
    Defender = task.waitTouch(THIRD_POS_4, 1.57),
    Goalie   = task.goalie(),
    match    = "{ALSMD}"
  },

  name = "Ref_CornerKickV22",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}