-- 两传一射 针对没有绕前盯人的队
-- 一传挑对侧前场 二传回传射门
-- by soap 16/06/30

-- 主要调挑球力度
-- 球接不住可以调整传球和接球位置

local REAL_SHOOTER = ""

local RANDOM_SHOOT = false
local FISRT_CHIP_POWER = 200
local SECOND_PASS_POWER = 500

-- 一传相关
-- chipPass里面会自动做对称变换
local FIRST_PASS_POS    = CGeoPoint:new_local(310, 250)
local FIRST_RECEIVE_POS = ball.refAntiYPos(CGeoPoint:new_local(310, 250))
local FIRST_RECEIVE_DIR = function()
  return dir.ourPlayerToPlayer("Defender", REAL_SHOOTER)
end

local FIRST_PASS_FORWARD_POS = function()
  local screen_dist = 70
  local screen_dir = (CVector:new_local(FIRST_RECEIVE_POS() - ball.pos())):dir()
  return ball.pos() + Utils.Polar2Vector(screen_dist, screen_dir)
end

-- 二传相关
local SECOND_PASS_POS = {
  ["Leader"]  = ball.refAntiYPos(CGeoPoint:new_local(ball.refPosX(), 100)),
  ["Middle"]  = ball.refAntiYPos(CGeoPoint:new_local(ball.refPosX() + 100, 190))
}

-- 射门相关
local SHOOT_POS = {
  ["Leader"]  = ball.refAntiYPos(CGeoPoint:new_local(ball.refPosX() + 20, 25)),
  ["Middle"]  = ball.refAntiYPos(CGeoPoint:new_local(ball.refPosX() + 75, 175))
}

-- 阵型相关
local FORMATION_A_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(120, -80)),
  ball.refAntiYPos(CGeoPoint:new_local(160, -120)),
  ball.refAntiYPos(CGeoPoint:new_local(200, -160))
}

local FORMATION_B_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(300, 0)),
  ball.refAntiYPos(CGeoPoint:new_local(300, 120)),
  ball.refAntiYPos(CGeoPoint:new_local(400, -150))
}

gPlayTable.CreatePlay{

firstState = "formation_A",

["formation_A"] = {
  switch = function()
    if bufcnt(player.toTargetDist("Defender") < 5 and
              player.toTargetDist("Middle")   < 5 and
              player.toTargetDist("Assister") < 5, 2, 180) then
      return "formation_B"
    end
  end,
  Leader   = task.staticGetBall(FIRST_PASS_POS),
  Special  = task.goSpeciPos(FIRST_PASS_FORWARD_POS),
  Defender = task.goSpeciPos(FORMATION_A_POS[1], dir.playerToBall),
  Middle   = task.goSpeciPos(FORMATION_A_POS[2], dir.playerToBall),
  Assister = task.goSpeciPos(FORMATION_A_POS[3], dir.playerToBall),
  Goalie   = task.goalie(),
  match    = "{L}[SDMA]"
},

["formation_B"] = {
  switch = function()
    if bufcnt(player.toTargetDist("Defender") < 5, 2, 180) then
      if RANDOM_SHOOT then
        local role = {"Leader", "Middle"}
        REAL_SHOOTER = role[math.random(2)]
      else
        REAL_SHOOTER = "Middle"
      end
      return "prepare_first_pass"
    end
  end,
  Leader   = task.staticGetBall(FIRST_PASS_POS),
  Special  = task.goSpeciPos(FIRST_PASS_FORWARD_POS),
  Defender = task.goSpeciPos(FORMATION_B_POS[1], dir.playerToBall),
  Middle   = task.goSpeciPos(FORMATION_B_POS[2]),
  Assister = task.goSpeciPos(FORMATION_B_POS[3]),
  Goalie   = task.goalie(),
  match    = "{L}[SDMA]"
},

["prepare_first_pass"] = {
  switch = function()
    if bufcnt(player.toTargetDist("Defender") < 10, 30) then
      return "first_pass"
    elseif bufcnt(true, 200) then
      return "exit"
    end
  end,
  Leader   = task.slowGetBall(FIRST_PASS_POS),
  Special  = task.goSpeciPos(FIRST_PASS_FORWARD_POS),
  Defender = task.goSpeciPos(FORMATION_B_POS[1], dir.playerToBall),
  Middle   = task.goSpeciPos(FORMATION_B_POS[2]),
  Assister = task.goSpeciPos(FORMATION_B_POS[3]),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["first_pass"] = {
  switch = function()
    if bufcnt(player.kickBall("Leader") or player.toBallDist("Leader") > 30, 2) then
      return "wait_receive"
    elseif bufcnt(true, 120) then
      return "exit"
    end
  end,
  Leader   = task.chipPass(FIRST_PASS_POS, FISRT_CHIP_POWER),
  Special  = task.goSpeciPos(FIRST_PASS_FORWARD_POS),
  Defender = task.goSpeciPos(FORMATION_B_POS[1], dir.playerToBall),
  Middle   = task.goSpeciPos(FORMATION_B_POS[2]),
  Assister = task.goSpeciPos(FORMATION_B_POS[3]),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["wait_receive"] = {
  switch = function()
    if bufcnt(true, 60) then
      return "receive_and_pass"
    end
  end,
  Leader   = task.goSpeciPos(SHOOT_POS["Leader"]),
  Special  = task.defendMiddle(),
  Defender = task.goSpeciPos(FIRST_RECEIVE_POS, FIRST_RECEIVE_DIR),
  Middle   = task.goSpeciPos(SHOOT_POS["Middle"]),
  Assister = task.goSpeciPos(FORMATION_B_POS[3], dir.playerToBall),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["receive_and_pass"] = {
  switch = function()
    if bufcnt(player.kickBall("Defender"), 2) then
      return "receive_and_shoot"
    elseif bufcnt(true, 120) then
      return "exit"
    end
  end,
  Leader   = task.goSpeciPos(SHOOT_POS["Leader"]),
  Special  = task.defendMiddle(),

  Defender =  function()
    return task.InterTouch(FIRST_RECEIVE_POS, SECOND_PASS_POS[REAL_SHOOTER], SECOND_PASS_POWER)
  end,

  Middle   = task.goSpeciPos(SHOOT_POS["Middle"]),
  Assister = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["receive_and_shoot"] = {
  switch = function()
    if bufcnt(true, 120) then
      return "exit"
    end
  end,

  Leader   = function()
    if (REAL_SHOOTER == "Leader") then
      return task.InterTouch(SHOOT_POS["Leader"])
    else
      return task.goSpeciPos(SHOOT_POS["Leader"])
    end
  end,

  Special  = task.defendMiddle(),
  Defender = task.defendMiddle(),

  Middle   = function()
    if (REAL_SHOOTER == "Middle") then
      return task.InterTouch(SHOOT_POS["Middle"])
    else
      return task.goSpeciPos(SHOOT_POS["Middle"])
    end
  end,

  Assister = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

name = "Ref_MiddleKickV1609",
applicable ={
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}