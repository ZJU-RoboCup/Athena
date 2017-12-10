-- 两传一射 针对没有绕前盯人的队
-- 一传挑同侧禁区 二传回传射门
-- by soap 16/06/14

-- 主要调挑球力度
-- 球接不住可以调整传球和接球位置

-- TODO 加入根据是否被盯住判断射门车

local RANDOM_SHOOT = false
local FISRT_CHIP_POWER = 130
local SECOND_PASS_POWER = 500

-- 一传相关
local FIRST_PASS_POS    = CGeoPoint:new_local(310, -100) -- chipPass里面会自动做对称变换
local FIRST_RECEIVE_POS = ball.refAntiYPos(CGeoPoint:new_local(310, -100))

local FIRST_PASS_FORWARD_POS = function()
  local screen_dist = 40
  local screen_dir = (CVector:new_local(FIRST_RECEIVE_POS() - ball.pos())):dir()
  return ball.pos() + Utils.Polar2Vector(screen_dist, screen_dir)
end

-- 一传掩护位置
local FIRST_PASS_SCREEN_POS = ball.refAntiYPos(CGeoPoint:new_local(100, -120))

-- 二传相关
local SECOND_PASS_POS = {
  ["Leader"]  = ball.refAntiYPos(CGeoPoint:new_local(ball.refPosX(), 100)),
  ["Middle"]  = ball.refAntiYPos(CGeoPoint:new_local(ball.refPosX() + 100, 190)),
  ["Special"] = ball.refAntiYPos(CGeoPoint:new_local(ball.refPosX() + 100, -100))
}

-- 射门相关
local SHOOT_POS = {
  ["Leader"]  = ball.refAntiYPos(CGeoPoint:new_local(ball.refPosX(), 25)),
  ["Middle"]  = ball.refAntiYPos(CGeoPoint:new_local(ball.refPosX() + 75, 175)),
  ["Special"] = ball.refAntiYPos(CGeoPoint:new_local(ball.refPosX() + 100, -100))
}

-- 阵型相关
local FORMATION_A_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(180, 100)),
  ball.refAntiYPos(CGeoPoint:new_local(150, 140)),
  ball.refAntiYPos(CGeoPoint:new_local(120, 180))
}

local FORMATION_B_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(200, -160)),
  ball.refAntiYPos(CGeoPoint:new_local(160, -120)),
  ball.refAntiYPos(CGeoPoint:new_local(120, -80))
}

local REAL_SHOOTER = ""

local function GetRealShooter()
  if RANDOM_SHOOT then
    local role = {"Leader", "Middle", "Special"}
    return role[math.random(3)]
  else
    return "Middle"
  end
end

local function GetRealTask(role)
  return function()
    if role == "Defender" then
      if gCurrentState == "wait_rotate" then
        local dir = player.toPointDir(SHOOT_POS[REAL_SHOOTER](), "Defender")
        return task.dribbleTurn(dir)
      elseif gCurrentState == "first_receive" then
        return task.Intercept(REAL_SHOOTER)
      else
        return task.touchPass(SHOOT_POS[REAL_SHOOTER], SECOND_PASS_POWER)
        --local secondPassPos = CGeoPoint:new_local(player.posX(REAL_SHOOTER),
        --                       ball.refAntiY() * (player.posY(REAL_SHOOTER) + 25))
        --return task.InterTouch(FIRST_RECEIVE_POS, secondPassPos, SECOND_PASS_POWER)
      end
    elseif role == REAL_SHOOTER then
      --return task.Intercept()
      return task.InterTouch(SHOOT_POS[REAL_SHOOTER])
    else
      return task.goSpeciPos(SHOOT_POS[role])
    end
  end
end

local function GetRealShootTask(role)
  return function()
    if role == REAL_SHOOTER then
      --return task.touchNear()
      return task.InterTouch(SHOOT_POS[REAL_SHOOTER], _, 700)
    elseif role == "Special" then
      return task.goSupportPos(REAL_SHOOTER)
    else
      if role == "Leader" then
        return task.leftBack()
      elseif role == "Assister" then
        return task.rightBack()
      else -- "Middle"
        if REAL_SHOOTER == "Leader" then
          return task.leftBack()
        else
          return task.rightBack()
        end
      end
    end
  end
end

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
  match    = "{L}[SD][MA]"
},

["formation_B"] = {
  switch = function()
    if bufcnt(player.toTargetDist("Defender") < 5, 2, 180) then
      return "prepare_first_pass"
    end
  end,
  Leader   = task.staticGetBall(FIRST_PASS_POS),
  Special  = task.goSpeciPos(FIRST_PASS_FORWARD_POS),
  Defender = task.goSpeciPos(FORMATION_B_POS[1], dir.playerToBall),
  Middle   = task.goSpeciPos(FORMATION_B_POS[2], dir.playerToBall),
  Assister = task.goSpeciPos(FORMATION_B_POS[3], dir.playerToBall),
  Goalie   = task.goalie(),
  match    = "{L}[SD][MA]"
},

["prepare_first_pass"] = {
  switch = function()
    if bufcnt(player.toTargetDist("Defender") < 60, 40) then
      return "first_pass"
    elseif bufcnt(true, 120) then
      return "exit"
    end
  end,
  Leader   = task.slowGetBall(FIRST_PASS_POS),
  Special  = task.goSpeciPos(FIRST_PASS_FORWARD_POS),
  Defender = task.goSpeciPos(FIRST_RECEIVE_POS, dir.playerToBall),
  Middle   = task.defendMiddle(),
  Assister = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["first_pass"] = {
  switch = function()
    if bufcnt(player.kickBall("Leader") or player.toBallDist("Leader") > 30, 2) then
      return "wait_receive"
    elseif bufcnt(true, 150) then
      return "exit"
    end
  end,
  Leader   = task.chipPass(FIRST_PASS_POS, FISRT_CHIP_POWER),
  Special  = task.goSpeciPos(FIRST_PASS_FORWARD_POS),
  Defender = task.goSpeciPos(FIRST_RECEIVE_POS, dir.ourPlayerToPlayer("Leader")),
  Middle   = task.goSpeciPos(SHOOT_POS["Middle"]),
  Assister = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["wait_receive"] = {
  switch = function()
    if bufcnt(true, 30) then
      REAL_SHOOTER = GetRealShooter()
      return "first_receive"
    end
  end,
  Leader   = task.goSpeciPos(SHOOT_POS["Leader"]),
  Special  = task.goSpeciPos(FIRST_PASS_FORWARD_POS),
  Defender = task.goSpeciPos(FIRST_RECEIVE_POS, dir.ourPlayerToPlayer("Leader")),
  Middle   = task.goSpeciPos(SHOOT_POS["Middle"]),
  Assister = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["first_receive"] = {
  switch = function()
    if bufcnt(player.toBallDist("Defender") < 20 and ball.velMod() < 20, 30) then
      return "wait_rotate"
      --return "second_pass"
    elseif bufcnt(true, 180) then
      return "exit"
    end
  end,
  Leader   = task.goSpeciPos(SHOOT_POS["Leader"]),
  Special  = task.goSpeciPos(FIRST_PASS_SCREEN_POS),
  Defender = task.Intercept("Leader"),
  --Defender = GetRealTask("Defender"),
  Middle   = task.goSpeciPos(SHOOT_POS["Middle"]),
  Assister = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["wait_rotate"] = {
  switch = function()
    local is_dir_ok = false
    local diff_dir = math.abs(Utils.Normalize(player.dir("Defender") -
                                              player.toPlayerDir("Defender", REAL_SHOOTER)))
    if diff_dir < math.pi / 72 then
      is_dir_ok = true
    end
    if bufcnt(is_dir_ok, 5, 40) then
      return "second_pass"
    end
  end,
  Leader   = task.goSpeciPos(SHOOT_POS["Leader"]),
  Special  = task.goSpeciPos(SHOOT_POS["Special"]),
  Defender = GetRealTask("Defender"),
  Middle   = task.goSpeciPos(SHOOT_POS["Middle"]),
  Assister = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
  
},

["second_pass"] = {
  switch = function()
    if bufcnt(player.kickBall("Defender") or player.toBallDist("Defender") > 20, 2) then
      return "second_receive"
    elseif bufcnt(true, 60) then
      return "exit"
    end
  end,
  Leader   = task.goSpeciPos(SHOOT_POS["Leader"]),
  Special  = task.goSpeciPos(SHOOT_POS["Special"]),
  Defender = GetRealTask("Defender"),
  Middle   = task.goSpeciPos(SHOOT_POS["Middle"]),
  Assister = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["second_receive"] = {
  switch = function()
    if bufcnt(player.toBallDist(REAL_SHOOTER) < 200, 1) then
      return "intercept"
    elseif bufcnt(ball.velMod() < 100, 30) then
      return "exit"
    elseif bufcnt(true, 60) then
      return "exit"
    end
  end,
  Leader   = GetRealTask("Leader"),
  Special  = GetRealTask("Special"),
  Defender = task.defendMiddle(),
  Middle   = GetRealTask("Middle"),
  Assister = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

["intercept"] = {
  switch = function()
    if bufcnt(player.toBallDist(REAL_SHOOTER) < 50, 1) then
      return "shoot"
    elseif bufcnt(true, 90) then
      return "exit"
    end
  end,
  Leader   = GetRealTask("Leader"),
  Special  = GetRealTask("Special"),
  Defender = task.stop(),
  Middle   = GetRealTask("Middle"),
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
  Leader   = GetRealShootTask("Leader"),
  Special  = GetRealShootTask("Special"),
  Defender = task.defendMiddle(),
  Middle   = GetRealShootTask("Middle"),
  Assister = GetRealShootTask("Assister"),
  Goalie   = task.goalie(),
  match    = "{LSDMA}"
},

name = "Ref_MiddleKickV1607",
applicable ={
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}