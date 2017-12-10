-- by hzy 6/29/2016
-- 模拟Immortal打MRL的定位球，挑禁区横向chase
local function PASS_POS(x,y)
  return ball.refAntiYPos(CGeoPoint:new_local(ball.posX()+x,ball.refAntiY()*y))
end
local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)
local SHOOT_POS_OFFSET_X = -100
local SHOOT_POS_Y = 270
local FINAL_SHOOT_POS = CGeoPoint:new_local(300, 230)
local SHOOT_POS = ball.refAntiYPos(FINAL_SHOOT_POS)--PASS_POS(SHOOT_POS_OFFSET_X,SHOOT_POS_Y)
local SLOW_GET_BALL_FACE_POS = function()
  return CGeoPoint:new_local(ball.posX()+SHOOT_POS_OFFSET_X,SHOOT_POS_Y)
end
local FrontPosX = 310
local Dist = 30
local POS = {
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX-100,-Dist*2.5)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX, -Dist*3)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX, -Dist*2)),
  ball.refAntiYPos(CGeoPoint:new_local(FrontPosX, -Dist*1)),
}
local STATIC_SHOOT_POS = nil
local function RETURN_SHOOT_POS()
  return STATIC_SHOOT_POS
end
gPlayTable.CreatePlay{
  firstState = "start",
  ["start"] = {
    switch = function ()
      if bufcnt(player.toTargetDist("Leader") < 20,10,180) then
        return "rush"
      end
    end,
    Assister = task.staticGetBall(SHOOT_POS,false),
    Leader   = task.goCmuRush(POS[1],_, 600, DSS_FLAG),
    Special  = task.goCmuRush(POS[2],_, 600, DSS_FLAG),
    Middle   = task.goCmuRush(POS[3],_, 600, DSS_FLAG),
    Defender = task.goCmuRush(POS[4],_, 600, DSS_FLAG),
    Goalie   = task.goalie(),
    match    = "{A}{L}{MSD}"
  },
  ["rush"] = {
    switch = function()
      if bufcnt(true,65) then
        STATIC_SHOOT_POS = SHOOT_POS()
        return "chip"
      end
    end,
    Assister = task.staticGetBall(SHOOT_POS,false),--Assister = task.slowGetBall(SLOW_GET_BALL_FACE_POS,false,true),
    Leader   = task.goCmuRush(SHOOT_POS,_, 600, DSS_FLAG),
    Special  = task.goCmuRush(POS[2],_, 600,    DSS_FLAG),
    Middle   = task.goCmuRush(POS[3],_, 600,    DSS_FLAG),
    Defender = task.goCmuRush(POS[4],_, 600,    DSS_FLAG),
    Goalie   = task.goalie(),
    match    = "{ALMSD}"
  },
  ["chip"] = {
    switch = function()
      if player.kickBall("Assister") then
        return "fix"
      elseif bufcnt(true,180) then
        return "exit"
      end
    end,
    Assister = task.chipPass(FINAL_SHOOT_POS,300,false,true),
    Leader   = task.goCmuRush(RETURN_SHOOT_POS,_, 600, DSS_FLAG),
    Special  = task.goCmuRush(POS[2],_, 600,           DSS_FLAG),
    Middle   = task.goCmuRush(POS[3],_, 600,           DSS_FLAG),
    Defender = task.goCmuRush(POS[4],_, 600,           DSS_FLAG),
    Goalie   = task.goalie(),
    match    = "{ALMSD}"
  },
  ["fix"] = {
    switch = function()
      if bufcnt(true,35) then
        return "shoot"
      end
    end,
    Assister = task.stop(),
    Leader   = task.goCmuRush(RETURN_SHOOT_POS,_, 600, DSS_FLAG),
    Special  = task.defendMiddle(),
    Middle   = task.leftBack(),
    Defender = task.rightBack(),
    Goalie   = task.goalie(),
    match    = "{ALMSD}"
  },
  ["shoot"] = {
    switch = function()
      if player.kickBall("Leader") then
        return "finish"
      elseif bufcnt(true,100) then
        return "exit"
      end
    end,
    Assister = task.stop(),
    Leader   = task.InterTouch(),--task.waitTouchNew(RETURN_SHOOT_POS),
    Special  = task.defendMiddle(),
    Middle   = task.leftBack(),
    Defender = task.rightBack(),
    Goalie   = task.goalie(),
    match    = "{ALMSD}"
  },
  name = "Ref_FrontKickV1617",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}