-- 两传一射 针对没有绕前盯人的队
-- 一传挑同侧前场 二传回传射门
-- by soap 16/06/14

-- 主要调挑球力度
-- 球接不住可以调整传球和接球位置

-- TODO 加入根据是否被盯住判断射门车

local FISRT_CHIP_POWER = 100
local SECOND_PASS_POWER = 400

local THEIR_GOAL_POS = CGeoPoint:new_local(param.pitchLength / 2, 0)

-- 一传相关 chipPass和拿球里面都会自动做对称变换
local FIRST_PASS_POS = function()
  return CGeoPoint:new_local(ball.refPosX() + 300, ball.refPosY())
end

local FIRST_RECEIVE_POS = function()
  return CGeoPoint:new_local(ball.refPosX() + 300, ball.refPosY())
end

local PREPARE_RECEIVE_POS = function()
  return CGeoPoint:new_local(ball.refPosX() + 300, -ball.refPosY())
end

local FAKE_FIRST_RECEIVE_POS = function()
  return CGeoPoint:new_local(ball.refPosX() + 50, ball.refPosY())
end

-- 挡防开球车位置
local FIRST_PASS_FORWARD_POS = function()
  local screen_dist = 70
  local screen_dir = (CVector:new_local(FIRST_RECEIVE_POS() - ball.pos())):dir()
  return ball.pos() + Utils.Polar2Vector(screen_dist, screen_dir)
end

-- 二传射门相关
local SECOND_PASS_POS = function()
  return CGeoPoint:new_local(ball.refPosX() + 200, ball.refPosY())
end

local SECOND_SHOOT_POS = function()
  return CGeoPoint:new_local(ball.refPosX() + 150, ball.refPosY())
end

-- 二传时两个前锋跑位点 吸引防守
local WEAK_SIDE_POS     = ball.refAntiYPos(CGeoPoint:new_local(360, 120))  -- 弱侧边路
local STRONG_SIDE_POS   = ball.refAntiYPos(CGeoPoint:new_local(330, -20))  -- 强侧边路
local WEAK_CORNER_POS   = ball.refAntiYPos(CGeoPoint:new_local(400, 200))  -- 弱侧边路
local STRONG_CORNER_POS = ball.refAntiYPos(CGeoPoint:new_local(340, -20))  -- 强侧边路

-- 阵型相关
local FORMATION_A_POS = {
  ["Defender"] = ball.refAntiYPos(CGeoPoint:new_local(-50, 200)),
  ["Middle"]   = ball.refAntiYPos(CGeoPoint:new_local(-50, -100)),
  ["Assister"] = ball.refAntiYPos(CGeoPoint:new_local(-50, 100))
}

gPlayTable.CreatePlay {

firstState = "formation_A",

["formation_A"] = {
  switch = function()
    if bufcnt(player.toTargetDist("Defender") < 5 and
              player.toTargetDist("Middle")   < 5 and
              player.toTargetDist("Assister") < 5, 2, 180) then
      return "formation_B"
    end
  end,
  Leader   = task.staticGetBall(THEIR_GOAL_POS),
  Special  = task.goSpeciPos(FIRST_PASS_FORWARD_POS),
  Defender = task.goSpeciPos(FORMATION_A_POS["Defender"], dir.playerToBall),
  Middle   = task.goSpeciPos(FORMATION_A_POS["Middle"], dir.playerToBall),
  Assister = task.goSpeciPos(FORMATION_A_POS["Assister"], dir.playerToBall),
  Goalie   = task.goalie(),
  match    = "{L}[SD][MA]"
},

["formation_B"] = {
  switch = function()
    if bufcnt(player.toTargetDist("Defender") < 5, 2, 180) then
      return "prepare_first_pass"
    end
  end,
  Leader   = task.staticGetBall(THEIR_GOAL_POS),
  Special  = task.goSpeciPos(FAKE_FIRST_RECEIVE_POS, dir.playerToBall),
  Defender = task.goSpeciPos(PREPARE_RECEIVE_POS, dir.playerToBall),
  Middle   = task.goSpeciPos(STRONG_SIDE_POS, dir.playerToBall),
  Assister = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{L}[SD][MA]"
},

["prepare_first_pass"] = {
  switch = function()
    if bufcnt(player.toTargetDist("Defender") < 10, 10) then
      return "first_pass"
    elseif bufcnt(true, 180) then
      return "exit"
    end
  end,
  Leader   = task.slowGetBall(player.toPointDir(FIRST_PASS_POS)),
  Special  = task.goSpeciPos(FAKE_FIRST_RECEIVE_POS, dir.playerToBall),
  Defender = task.goSpeciPos(FIRST_RECEIVE_POS, dir.playerToBall),
  Middle   = task.goSpeciPos(STRONG_SIDE_POS, dir.playerToBall),
  Assister = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["first_pass"] = {
  switch = function()
    if bufcnt(player.kickBall("Leader") or player.toBallDist("Leader") > 20 , 2) then
      return "wait_receive"
    elseif bufcnt(true, 120) then
      return "exit"
    end
  end,
  Leader   = task.chipPass(player.toPointDir(FIRST_PASS_POS), FISRT_CHIP_POWER),
  Special  = task.goSpeciPos(WEAK_SIDE_POS),
  Defender = task.goSpeciPos(FIRST_RECEIVE_POS, dir.ourPlayerToPlayer("Leader")),
  Middle   = task.goSpeciPos(STRONG_SIDE_POS),
  Assister = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["wait_receive"] = {
  switch = function()
    if bufcnt(true, 30) then
      return "receive_and_pass"
    end
  end,
  Leader   = task.goSpeciPos(SECOND_SHOOT_POS),
  Special  = task.goSpeciPos(WEAK_SIDE_POS),
  Defender = task.goSpeciPos(FIRST_RECEIVE_POS, dir.ourPlayerToPlayer("Leader")),
  Middle   = task.goSpeciPos(STRONG_SIDE_POS),
  Assister = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["receive_and_pass"] = {
  switch = function()
    if bufcnt(player.kickBall("Defender"), 2, 30) then
      return "shoot"
    elseif bufcnt(true, 60) then
      return "exit"
    end
  end,
  Leader   = task.goSpeciPos(SECOND_SHOOT_POS),
  Special  = task.goSpeciPos(WEAK_SIDE_POS),
  Defender = task.InterTouch(SECOND_PASS_POS, SECOND_SHOOT_POS, SECOND_PASS_POWER),
  Middle   = task.goSpeciPos(STRONG_CORNER_POS),
  Assister = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["shoot"] = {
  switch = function()
    if bufcnt(true, 60) then
      return "exit"
    end
  end,
  Leader   = task.InterTouch(SECOND_SHOOT_POS),
  Special  = task.defendMiddle(),
  Defender = task.stop(),
  Middle   = task.goSpeciPos(STRONG_CORNER_POS),
  Assister = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

name = "Ref_BackKickV1606",
applicable ={
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}