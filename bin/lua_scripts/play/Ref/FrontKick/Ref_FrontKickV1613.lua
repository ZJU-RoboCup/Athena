-- 前场挑对角 制造混乱
-- by soap 16/06/18

-- 主要调挑球力度
-- 球接不住可以调整传球和接球位置

local FIRST_CHIP_POWER = 200
local SECOND_CHIP_POWER = 300
local SECOND_PASS_POWER = 500
local DIRECT_SHOOT_POS = ball.refAntiYPos(CGeoPoint:new_local(param.pitchLength/2.0, 0))

-- 一传相关
local FIRST_PASS_POS    = CGeoPoint:new_local(400, 150) -- chipPass里面会自动做对称变换
local FIRST_RECEIVE_POS = ball.refAntiYPos(CGeoPoint:new_local(400, 150))

local FIRST_PASS_FORWARD_POS = function()
  local screen_dist = 40
  local screen_dir = (CVector:new_local(FIRST_RECEIVE_POS() - ball.pos())):dir()
  return ball.pos() + Utils.Polar2Vector(screen_dist, screen_dir)
end

-- 一传假传位置
local FAKE_FIRST_PASS_POS = ball.refAntiYPos(CGeoPoint:new_local(350, -200))

-- 射门相关
local SHOOT_POS = {
  ["Middle"]   = ball.refAntiYPos(CGeoPoint:new_local(270, -30)),
  ["Defender"] = ball.refAntiYPos(CGeoPoint:new_local(230, -60)),
  ["Assister"] = ball.refAntiYPos(CGeoPoint:new_local(380, -60)) -- 头球射门点
}

-- 阵型相关
local FORMATION_A_POS = {
  ["Special"]  = ball.refAntiYPos(CGeoPoint:new_local(ball.refPosX(), 260)),
  ["Defender"]   = ball.refAntiYPos(CGeoPoint:new_local(-250, -100)),
  ["Middle"] = ball.refAntiYPos(CGeoPoint:new_local(-250, 100))
}

local FORMATION_B_POS = {
  ["Special"]  = ball.refAntiYPos(CGeoPoint:new_local(100, 280)),
  ["Middle"]   = ball.refAntiYPos(CGeoPoint:new_local(-250 + 520, 100)), -- 520很重要 防撞
  ["Defender"] = ball.refAntiYPos(CGeoPoint:new_local(-250 + 480, -100)) -- 480很重要 防撞
}

local FORMATION_C_POS = {
  ["Special"]  = ball.refAntiYPos(CGeoPoint:new_local(250, 270)),
  ["Middle"]   = ball.refAntiYPos(CGeoPoint:new_local(270, -60)),
  ["Defender"] = ball.refAntiYPos(CGeoPoint:new_local(230, -120))
}

gPlayTable.CreatePlay{

firstState = "formation_A",

["formation_A"] = {
  switch = function()
    if bufcnt(player.toTargetDist("Special")  < 5 and
              player.toTargetDist("Defender") < 5 and
              player.toTargetDist("Middle")   < 5, 2, 180) then
      return "formation_B"
    end
  end,
  Leader   = task.staticGetBall(FIRST_PASS_POS),
  Special  = task.goSpeciPos(FORMATION_A_POS["Special"]),
  Defender = task.goSpeciPos(FORMATION_A_POS["Defender"]),
  Middle   = task.goSpeciPos(FORMATION_A_POS["Middle"]),
  Assister = task.goSpeciPos(FIRST_PASS_FORWARD_POS),
  Goalie   = task.goalie(),
  match    = "{L}[SD][MA]"
},

["formation_B"] = {
  switch = function()
    if bufcnt(player.toTargetDist("Special")  < 5 and
              player.toTargetDist("Defender") < 5 and
              player.toTargetDist("Middle")   < 5, 2, 180) then
      return "formation_C"
    end
  end,
  Leader   = task.staticGetBall(FIRST_PASS_POS),
  Special  = task.goSpeciPos(FORMATION_B_POS["Special"], _, flag.allow_dss),
  Defender = task.goSpeciPos(FORMATION_B_POS["Defender"], _, flag.allow_dss),
  Middle   = task.goSpeciPos(FORMATION_B_POS["Middle"], _, flag.allow_dss),
  Assister = task.goSpeciPos(FIRST_PASS_FORWARD_POS),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["formation_C"] = {
  switch = function()
    if bufcnt(player.toTargetDist("Special") < 5, 2, 180) then
      return "prepare_pass"
    end
  end,
  Leader   = task.slowGetBall(FIRST_PASS_POS),
  Special  = task.goSpeciPos(FORMATION_C_POS["Special"]),
  Defender = task.goSpeciPos(FORMATION_C_POS["Defender"]),
  Middle   = task.goSpeciPos(FORMATION_C_POS["Middle"]),
  Assister = task.goSpeciPos(FAKE_FIRST_PASS_POS, dir.playerToBall),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["prepare_pass"] = {
  switch = function()
    if bufcnt(player.toTargetDist("Special") < 50, 5) then
      return "pass"
    elseif bufcnt(true, 120) then
      return "exit"
    end
  end,
  Leader   = task.slowGetBall(FIRST_PASS_POS),
  Special  = task.goSpeciPos(FIRST_RECEIVE_POS),
  Defender = task.goTouchPos(SHOOT_POS["Defender"]),
  Middle   = task.goTouchPos(SHOOT_POS["Middle"]),
  Assister = task.goSpeciPos(FAKE_FIRST_PASS_POS, dir.playerToBall),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["pass"] = {
  switch = function()
    if bufcnt(player.kickBall("Leader") or player.toBallDist("Leader") > 30, 20) then
      return "wait_receive"
    elseif bufcnt(true, 150) then
      return "exit"
    end
  end,
  Leader   = task.chipPass(FIRST_PASS_POS, FIRST_CHIP_POWER),
  Special  = task.InterTouch(FIRST_RECEIVE_POS),
  Defender = task.goTouchPos(SHOOT_POS["Defender"]),
  Middle   = task.goTouchPos(SHOOT_POS["Middle"]),
  Assister = task.goSpeciPos(FAKE_FIRST_PASS_POS, dir.playerToBall),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["wait_receive"] = {
  switch = function()
    if bufcnt(true, 10) then
      return "receive"
    end
  end,
  Leader   = task.singleBack(),
  Special  = task.InterTouch(FIRST_RECEIVE_POS),
  Defender = task.leftBack(),
  Middle   = task.rightBack(),
  Assister = task.goSpeciPos(FAKE_FIRST_PASS_POS, dir.playerToBall),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["receive"] = {
  switch = function()
    if bufcnt(player.toBallDist("Special") < 20 and ball.velMod() < 20, 5) then
      return "direct_shoot"
    elseif bufcnt(true, 180) then
      return "exit"
    end
  end,
  Leader   = task.singleBack(),
  Special  = task.InterTouch(FIRST_RECEIVE_POS),
  Defender = task.leftBack(),
  Middle   = task.rightBack(),
  Assister = task.goTouchPos(SHOOT_POS["Assister"]),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["direct_shoot"] = {
  switch = function()
    if bufcnt(true, 30) then
      return "exit"
    end
  end,
  Leader   = task.singleBack(),
  Special  = task.InterTouch(FIRST_RECEIVE_POS),
  Defender = task.leftBack(),
  Middle   = task.rightBack(),
  Assister = task.defendMiddle(),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

name = "Ref_FrontKickV1613",
applicable ={
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}