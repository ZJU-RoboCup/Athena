-- 挡板角球, 需要在比较靠后的位置使用
-- by yys 2014-07-09
-- 2014-07-20 yys 改

local FRONT_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(285, 175))
local FRONT_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(260, 155))
local FRONT_POS_3 = ball.refAntiYPos(CGeoPoint:new_local(290, 100))
local FRONT_POS_4 = ball.refAntiYPos(CGeoPoint:new_local(410, 140))
local FRONT_POS_5 = function()
  return CGeoPoint:new_local(ball.posX() - 30,ball.syntY() * 140)
end

local GOALIE_POS  = ball.refAntiYPos(CGeoPoint:new_local(0, 0))
local PASS_POS_FOR_GOALIE = pos.passForTouch(GOALIE_POS)

local KICK_POS = CGeoPoint:new_local(0, -270)

local MIDDLE_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(280, 0))
local MIDDLE_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(200, 150))
local MIDDLE_POS_3 = ball.refAntiYPos(CGeoPoint:new_local(150, 100))
local MIDDLE_POS_4 = ball.refAntiYPos(CGeoPoint:new_local(20, 50))

local SIDE_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(320, 150))
local SIDE_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(270, 150))
local SIDE_POS_3 = ball.refAntiYPos(CGeoPoint:new_local(150, -150))

local PRE_CHIP_POS = function ()
  return (ball.refAntiYPos(CGeoPoint:new_local(ball.posX() + 10, -(math.abs(ball.posY()) + 15))))()
end

local reflectDir = function()
  if ball.posY()>100 then 
    return 0.74
  end
  if ball.posY()<-100 then
    return 0.90 --0.82 chuan
  end
end

--0.80--0.95Y<0--0.98可以横传  --0.88可以传

local dangerous = true

gPlayTable.CreatePlay{

  firstState = "start",

  ["start"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Kicker") < 30 and
                player.toTargetDist("Special") < 30, 10, 180) then
        return "temp"
      end
    end,
    Assister = task.staticGetBall(KICK_POS),
    Kicker     = task.goCmuRush(MIDDLE_POS_1),
    Special  = task.goCmuRush(MIDDLE_POS_2),
    Middle   = task.goCmuRush(MIDDLE_POS_3),
    Defender = task.goCmuRush(SIDE_POS_3),
    Goalie   = task.goalie(),
    match    = "{A}{SMD}"
  },

  ["temp"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Kicker") < 20 and
                player.toTargetDist("Special") < 20, "normal", 180) then
        return "prepare"
      end
    end,
    Assister = task.staticGetBall(KICK_POS),
    Kicker     = task.goCmuRush(pos.reflectPos(-30, 10), dir.reflectDir(reflectDir)),
    Special  = task.goCmuRush(SIDE_POS_1),
    Middle   = task.goCmuRush(SIDE_POS_2),
    Defender = task.goCmuRush(FRONT_POS_3),
    Goalie   = dangerous and task.goalie() or task.goCmuRush(GOALIE_POS),
    match    = "{ASMD}"
  },

  ["prepare"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Kicker") < 5, 40, 180) then
        if (not dangerous) and (not world:isPassLineBlocked(gRoleNum["Goalie"])) and (not world:isShootLineBlocked(gRoleNum["Goalie"], 200)) then
          return "reflect"
        elseif world:isShootLineBlocked(gRoleNum["Kicker"], 100, 1.5) then
          return "reflect"
        else
          return "reflect"
        end
      end
    end,
    Assister = task.staticGetBall(KICK_POS),
    Kicker     = task.goCmuRush(pos.reflectPos(-30, 10), dir.reflectDir(reflectDir)),
    Special  = task.goCmuRush(FRONT_POS_1),
    Middle   = task.goCmuRush(FRONT_POS_2),
    Defender = task.goCmuRush(FRONT_POS_3),
    Goalie   = dangerous and task.goalie() or task.goCmuRush(GOALIE_POS),
    match    = "{ASMD}"
  },

  ["reflect"] = {
    switch = function ()
      if bufcnt(player.kickBall("Assister") or player.toBallDist("Assister") > 35, 10) then
        return "Inter"
      end
    end,
    Assister = task.touchPass(pos.reflectPassPos("Kicker"), 800), --task.goAndTurnKick(pos.reflectPassPos("Kicker"), 1000),--
    Kicker     = task.stop(),
    Special  = task.goCmuRush(FRONT_POS_4),
    Middle   = task.rightBack(),
    Defender = task.leftBack(),
    Goalie   = task.goalie(),
    match    = "{AS}[MD]"
  },

  ["Inter"] = {
    switch = function ()
      if bufcnt(player.kickBall("Middle"),1,60) then
        return "exit"
      end
    end,
    Assister = task.stop(),
    Kicker   = task.stop(),
    Special  = task.waitTouchNew(_,ball.refSyntYPos(CGeoPoint:new_local(param.pitchLength/2.0,15))),
    Middle   = task.rightBack(),
    Defender = task.leftBack(),
    Goalie   = task.goalie(),
    match    = "{AS}[MD]"
  },


  name = "Ref_CornerKickV14",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}