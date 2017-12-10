-- by Alan 2016-11-29
local MIDDLE_THRESHOLD_Y = 0.125 * param.pitchWidth --75

local OTHER_POS = {
  CGeoPoint:new_local(110,-100),
  CGeoPoint:new_local(110,100)
}
local BACK_POS = CGeoPoint:new_local(80,0)

local BLOCK_DIST = param.freeKickAvoidBallDist + param.playerRadius
local AWAY_DIST = 2.5 + param.playerRadius
local BLOCK_ANGLE = math.asin(AWAY_DIST / BLOCK_DIST)*2
local factor = ball.antiY

local SIDE_POS = function()
    local tmpX=150
    local tmpY=-200
    if ball.posX()>150 then
      return ball.pos() + Utils.Polar2Vector(BLOCK_DIST, dir.ballToOurGoal() + factor()*BLOCK_ANGLE)
    else
      return CGeoPoint:new_local(tmpX, tmpY)
    end
end

local INTER_POS = function()
    local tmpX=150
    local tmpY=200
    if ball.posX()>150 then
      return ball.pos() + Utils.Polar2Vector(BLOCK_DIST, dir.ballToOurGoal() - factor()*BLOCK_ANGLE)
    else
      return CGeoPoint:new_local(tmpX, tmpY)
    end
end

local MIDDLE_POS = function()
    local tmpX=150
    local tmpY=0
    if ball.posX()>150 then
      return ball.pos() + Utils.Polar2Vector(BLOCK_DIST, dir.ballToOurGoal())
    else
      return CGeoPoint:new_local(tmpX, tmpY)
    end
end

local OTHER_SIDE_POS = function()
  local factor = 1
  if ball.posY() > 0 then
    factor = -1
  end
  if ball.posX()>150 then
    return CGeoPoint:new_local(INTER_POS():x(), factor * param.pitchWidth * 0.4)
  else
    return CGeoPoint:new_local(120, factor * param.pitchWidth * 0.4)
  end
end

local getBufTime = function()
  if IS_SIMULATION then
    return 9999
  else
    return 240
  end
end

local ACC = 400

gPlayTable.CreatePlay {

firstState = "start",

["start"] = {
  switch = function()
    if cond.isGameOn() then
      return "exit"
    elseif (ball.posY() < MIDDLE_THRESHOLD_Y and ball.posY() > -MIDDLE_THRESHOLD_Y) or ball.posX() <150 then
      return "standInMiddle"
    else
      return "standByLine"
    end
  end,

  Leader   = task.stop(),
  Special  = task.stop(),
  Kicker   = task.stop(),
  Defender = task.stop(),
  Middle   = task.stop(),
  Goalie   = task.stop(),
  match    = "[LSDM]"
},

["standInMiddle"] = {
  switch = function()
    if cond.isGameOn() then
      return "exit"
    elseif (ball.posY() < MIDDLE_THRESHOLD_Y and ball.posY() > -MIDDLE_THRESHOLD_Y) or ball.posX() <150 then
      return "standInMiddle"
    else
      return "standByLine"
    end
  end,
  Leader   = task.goCmuRush(SIDE_POS, dir.playerToBall, ACC, STOP_DSS),
  Special  = task.goCmuRush(INTER_POS, dir.playerToBall,ACC, STOP_DSS),
  Kicker   = task.goCmuRush(MIDDLE_POS, dir.playerToBall,ACC, STOP_DSS),
  Defender = task.goCmuRush(OTHER_POS[1], dir.playerToBall,ACC, STOP_DSS),
  Middle   = task.goCmuRush(OTHER_POS[2], dir.playerToBall, ACC, STOP_DSS),
  Goalie   = task.goCmuRush(BACK_POS, dir.playerToBall, ACC, STOP_DSS),
  match    = "{LS}[DM]"
},

["standByLine"] = {
  switch = function()
    if cond.isGameOn() then
      return "exit"
    elseif (ball.posY() < MIDDLE_THRESHOLD_Y and ball.posY() > -MIDDLE_THRESHOLD_Y) or ball.posX() <150 then
      return "standInMiddle"
    else
      return "standByLine"
    end
  end,

  Leader   = task.goCmuRush(MIDDLE_POS, dir.playerToBall, ACC, STOP_DSS),
  Special  = task.goCmuRush(OTHER_SIDE_POS, dir.playerToBall, ACC, STOP_DSS),
  Kicker   = task.goCmuRush(SIDE_POS, dir.playerToBall, ACC, STOP_DSS),
  Defender = task.goCmuRush(OTHER_POS[1], dir.playerToBall,ACC, STOP_DSS),
  Middle   = task.goCmuRush(OTHER_POS[2], dir.playerToBall, ACC, STOP_DSS),
  Goalie   = task.goCmuRush(BACK_POS, dir.playerToBall, ACC, STOP_DSS),
  match    = "{LS}[DM]"
},

name = "Ref_StopV1701",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}