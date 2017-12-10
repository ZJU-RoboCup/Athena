-- by Soap 2015-04-14
-- YYS 2015-06-10
local PENALTY_THRESHOLD_DIST = 86 --18+18+50
local MIDDLE_THRESHOLD_Y = 0.125 * param.pitchWidth --75

local SIDE_POS, MIDDLE_POS, INTER_POS = pos.refStopAroundBall()
local ONE_POS, TWO_POS, THREE_POS, FOUR_POS, FIVE_POS = pos.stopPoses()

local STOP_FLAG = bit:_or(flag.slowly, flag.dodge_ball)
local STOP_DSS = bit:_or(STOP_FLAG, flag.allow_dss)

local gBallPosXInStop = 0
local gBallPosYInStop = 0

local OTHER_SIDE_POS = function()
  local factor = 1
  if ball.posY() > 0 then
    factor = -1
  end
  return CGeoPoint:new_local(INTER_POS():x(), factor * param.pitchWidth * 0.4)
  --return CGeoPoint:new_local(ball.posX() - 50*param.lengthRatio, 160*factor*param.widthRatio)
end

local getBufTime = function()
  if IS_SIMULATION then
    return 9999
  else
    return 240
  end
end

local ACC = 300

gPlayTable.CreatePlay {

firstState = "start",

["start"] = {
  switch = function()
    gBallPosXInStop = ball.posX()
    gBallPosYInStop = ball.posY()
    if cond.isGameOn() then
      return "exit"
    elseif ball.toOurPenaltyDist() < PENALTY_THRESHOLD_DIST then
    --elseif ball.toOurGoalDist() < PENALTY_THRESHOLD_DIST then
      return "standByPenalty"
    elseif ball.posY() < MIDDLE_THRESHOLD_Y and ball.posY() > -MIDDLE_THRESHOLD_Y then
      return "standInMiddle"
    elseif ball.posY() > MIDDLE_THRESHOLD_Y or ball.posY() < -MIDDLE_THRESHOLD_Y then
      return "standByLine"
    else
      return "exit"
    end
  end,

  Leader   = task.stop(),
  Special  = task.stop(),
  Kicker   = task.stop(),
  Defender = task.stop(),
  Middle   = task.stop(),
  Goalie   = task.goalie(),
  match    = "[MDLS]"
},

["standInMiddle"] = {
  switch = function()
    if cond.isGameOn() then
      return "exit"
    --elseif ball.toOurGoalDist() < PENALTY_THRESHOLD_DIST then
    elseif ball.toOurPenaltyDist() < PENALTY_THRESHOLD_DIST then
      gBallPosXInStop = ball.posX()
      gBallPosYInStop = ball.posY()
      return "standByPenalty"
    elseif ball.posY() < MIDDLE_THRESHOLD_Y and ball.posY() > -MIDDLE_THRESHOLD_Y then
      if (math.abs(gBallPosXInStop - ball.posX()) >= 8 or math.abs(gBallPosYInStop - ball.posY()) >= 8) then
        gBallPosXInStop = ball.posX()
        gBallPosYInStop = ball.posY()
        return nil
      end
      if bufcnt( ((gRoleNum["Leader"] ~= 0) and (player.toTargetDist("Leader") > 8)) or
                 ((gRoleNum["Special"] ~= 0) and (player.toTargetDist("Special") > 8)) or
                 ((gRoleNum["Kicker"] ~= 0) and (player.toTargetDist("Kicker") > 8)) or
                 ((gRoleNum["Defender"] ~= 0) and (player.toTargetDist("Defender") > 8)) or
                 ((gRoleNum["Middle"] ~= 0) and (player.toTargetDist("Middle") > 8)), getBufTime()) then
        return "reDoStop"
      end
      if bufcnt( ((gRoleNum["Leader"] ~= 0) and (player.toTargetDist("Leader") < 10)) and
                 ((gRoleNum["Special"] ~= 0) and (player.toTargetDist("Special") < 10)) and
                 ((gRoleNum["Kicker"] ~= 0) and (player.toTargetDist("Kicker") < 10)) and
                 ((gRoleNum["Defender"] ~= 0) and (player.toTargetDist("Defender") < 10)) and
                 ((gRoleNum["Middle"] ~= 0) and (player.toTargetDist("Middle") < 10)), 15) then
        gBallPosXInStop = ball.posX()
        gBallPosYInStop = ball.posY()
        return nil
      end
    elseif ball.posY() > MIDDLE_THRESHOLD_Y + param.playerRadius or ball.posY() < -MIDDLE_THRESHOLD_Y - param.playerRadius then
      gBallPosXInStop = ball.posX()
      gBallPosYInStop = ball.posY()
      return "standByLine"
    else
      return "exit"
    end
  end,

  Leader   = task.goCmuRush(SIDE_POS, dir.playerToBall, ACC, STOP_DSS),
  Special  = task.goCmuRush(INTER_POS, dir.playerToBall, ACC, STOP_DSS),
  Kicker   = task.goCmuRush(MIDDLE_POS, dir.playerToBall, ACC, STOP_DSS),
  Defender = task.leftBack4Stop(),
  Middle   = task.rightBack4Stop(),
  Goalie   = task.goalie(),
  match    = "[MDLS]"
},

["standByPenalty"] = {
  switch = function()
    if cond.isGameOn() then
      return "exit"
    --elseif ball.toOurGoalDist() < PENALTY_THRESHOLD_DIST then
    elseif ball.toOurPenaltyDist() < PENALTY_THRESHOLD_DIST then
      if (math.abs(gBallPosXInStop - ball.posX()) >= 8 or math.abs(gBallPosYInStop - ball.posY()) >= 8) then
        gBallPosXInStop = ball.posX()
        gBallPosYInStop = ball.posY()
        return nil
      end
      if bufcnt( ((gRoleNum["Leader"] ~= 0) and (player.toTargetDist("Leader") > 8)) or
                 ((gRoleNum["Special"] ~= 0) and (player.toTargetDist("Special") > 8)) or
                 ((gRoleNum["Kicker"] ~= 0) and (player.toTargetDist("Kicker") > 8)) or
                 ((gRoleNum["Defender"] ~= 0) and (player.toTargetDist("Defender") > 8)) or
                 ((gRoleNum["Middle"] ~= 0) and (player.toTargetDist("Middle") > 8)), getBufTime()) then
        return "reDoStop"
      end
      if bufcnt( ((gRoleNum["Leader"] ~= 0) and (player.toTargetDist("Leader") < 10)) and
                 ((gRoleNum["Special"] ~= 0) and (player.toTargetDist("Special") < 10)) and
                 ((gRoleNum["Kicker"] ~= 0) and (player.toTargetDist("Kicker") < 10)) and
                 ((gRoleNum["Defender"] ~= 0) and (player.toTargetDist("Defender") < 10)) and
                 ((gRoleNum["Middle"] ~= 0) and (player.toTargetDist("Middle") < 10)), 15) then
        gBallPosXInStop = ball.posX()
        gBallPosYInStop = ball.posY()
        return nil
      end
    elseif ball.posY() < MIDDLE_THRESHOLD_Y - param.playerRadius and ball.posY() > -MIDDLE_THRESHOLD_Y + param.playerRadius then
      gBallPosXInStop = ball.posX()
      gBallPosYInStop = ball.posY()
      return "standInMiddle"
    elseif ball.posY() > MIDDLE_THRESHOLD_Y or ball.posY() < -MIDDLE_THRESHOLD_Y then
      gBallPosXInStop = ball.posX()
      gBallPosYInStop = ball.posY()
      return "standByLine"
    else
      return "exit"
    end
  end,

  Leader   = task.goCmuRush(FIVE_POS, dir.playerToBall, ACC, STOP_DSS),
  Special  = task.goCmuRush(TWO_POS, dir.playerToBall, ACC, STOP_DSS),
  Kicker   = task.goCmuRush(THREE_POS, dir.playerToBall, ACC, STOP_DSS),
  Defender = task.goCmuRush(ONE_POS, dir.playerToBall, ACC, STOP_DSS),
  Middle   = task.goCmuRush(FOUR_POS, dir.playerToBall, ACC, STOP_DSS),
  Goalie   = task.goalie(),
  match    = "(MDLS)"
},

["standByLine"] = {
  switch = function()
    if cond.isGameOn() then
      return "exit"
    --elseif ball.toOurGoalDist() < PENALTY_THRESHOLD_DIST then
    elseif ball.toOurPenaltyDist() < PENALTY_THRESHOLD_DIST then
      gBallPosXInStop = ball.posX()
      gBallPosYInStop = ball.posY()
      return "standByPenalty"
    elseif ball.posY() < MIDDLE_THRESHOLD_Y and ball.posY() > -MIDDLE_THRESHOLD_Y then
      gBallPosXInStop = ball.posX()
      gBallPosYInStop = ball.posY()
      return "standInMiddle"
    elseif ball.posY() > MIDDLE_THRESHOLD_Y or ball.posY() < -MIDDLE_THRESHOLD_Y then
      if (math.abs(gBallPosXInStop - ball.posX()) >= 8 or math.abs(gBallPosYInStop - ball.posY()) >= 8) then
        gBallPosXInStop = ball.posX()
        gBallPosYInStop = ball.posY()
        return nil
      end
      if bufcnt( ((gRoleNum["Leader"] ~= 0) and  (player.toTargetDist("Leader") > 8)) or
                 ((gRoleNum["Special"] ~= 0) and (player.toTargetDist("Special") > 8)) or
                 ((gRoleNum["Kicker"] ~= 0) and (player.toTargetDist("Kicker") > 8)) or
                 ((gRoleNum["Defender"] ~= 0) and (player.toTargetDist("Defender") > 8)) or
                 ((gRoleNum["Middle"] ~= 0) and (player.toTargetDist("Middle") > 8)), getBufTime()) then
        return "reDoStop"
      end
      if bufcnt( ((gRoleNum["Leader"] ~= 0) and  (player.toTargetDist("Leader") < 10)) and
                 ((gRoleNum["Special"] ~= 0) and (player.toTargetDist("Special") < 10)) and
                 ((gRoleNum["Kicker"] ~= 0) and (player.toTargetDist("Kicker") < 10)) and
                 ((gRoleNum["Defender"] ~= 0) and (player.toTargetDist("Defender") > 10)) and
                 ((gRoleNum["Middle"] ~= 0) and (player.toTargetDist("Middle") > 10)), 15) then
        gBallPosXInStop = ball.posX()
        gBallPosYInStop = ball.posY()
        return nil
      end
    else
      return "exit"
    end
  end,

  Leader   = task.goCmuRush(MIDDLE_POS, dir.playerToBall, ACC, STOP_DSS),
  Special  = task.goCmuRush(OTHER_SIDE_POS, dir.playerToBall, ACC, STOP_DSS),
  Kicker   = task.goCmuRush(SIDE_POS, dir.playerToBall, ACC, STOP_DSS),
  Defender = task.leftBack4Stop(),
  Middle   = task.rightBack4Stop(),
  Goalie   = task.goalie(),
  match    = "[MDLS]"
},

["reDoStop"] = {
  switch = function()
    if bufcnt(true, 30) then
      return "start"
    end
  end,

  Leader   = task.goCmuRush(ball.refAntiYPos(CGeoPoint:new_local(-140,-120)), dir.playerToBall, ACC, STOP_DSS),
  Special  = task.goCmuRush(ball.refAntiYPos(CGeoPoint:new_local(-140, 120)), dir.playerToBall, ACC, STOP_DSS),
  Kicker   = task.goCmuRush(CGeoPoint:new_local(-120, 0), dir.playerToBall, ACC, STOP_DSS),
  Defender = task.goCmuRush(CGeoPoint:new_local(-290, -130), dir.playerToBall, ACC, STOP_DSS),
  Middle   = task.goCmuRush(CGeoPoint:new_local(-290, 130), dir.playerToBall, ACC, STOP_DSS),
  Goalie   = task.goalie(),
  match    = "[MDLS]"
},

name = "Ref_StopV2",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}