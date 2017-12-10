-- 2012年决赛对Skuba进球的战术,大场地版 yys 2014/03/12

local  FrontKickV4_Go_Pos = function ()
  local absPoint = CGeoPoint:new_local(450,0)
  local factor
  local Pos_FrontKickV4_Go = function ()
    if ball.posY() > 0 then
      factor = -1
    else
      factor = 1
    end
    local targetPos = CGeoPoint:new_local(absPoint:x(),(absPoint:y())* factor)
    local standDir = (targetPos - ball.pos()):dir()
    local finalPos = ball.pos() + Utils.Polar2Vector(55,standDir)
    return finalPos
  end
  return Pos_FrontKickV4_Go
end

local JAM_BACK_POS = ball.refAntiYPos(CGeoPoint:new_local(270,-150))

local RUSH_POS = ball.refAntiYPos(CGeoPoint:new_local(340, 140))

local win_Attack_Pos = function (role)
  local _lastPos = CGeoPoint:new_local(0,0)
  local _lastCycle = 0
  local Pos_Attack_Win = function ()
    if (vision:Cycle() - _lastCycle) > 6 then
      local shooterPos = player.pos(role)
      local enemyNum = bestPlayer:getTheirBestPlayer()
      local enemyPos = enemy.pos(enemyNum)
      local enemy2shooterVec = CVector:new_local(shooterPos - enemyPos)
      local winPos = enemyPos + Utils.Polar2Vector(16,enemy2shooterVec:dir())
      _lastPos = winPos
    end
    _lastCycle = vision:Cycle()
    return _lastPos
  end
  return Pos_Attack_Win
end

local FRONTKICKV4_GO_POS  = FrontKickV4_Go_Pos()
local WINATTACK_POS

local TMP_POS = ball.refAntiYPos(CGeoPoint:new_local(60,60))
gPlayTable.CreatePlay{

firstState = "startball",

["startball"] = {
  switch = function ()
    if bufcnt(player.toTargetDist("Special") < 50 and
              player.toTargetDist("Middle") < 50,  "normal", 180)then
      return "go"
    end
  end,
  Assister = task.staticGetBall(CGeoPoint:new_local(450, 0)),
  Middle   = task.goCmuRush(ball.antiYPos(CGeoPoint:new_local(60,0)), _, 500, flag.allow_dss),
  Special  = task.goCmuRush(ball.antiYPos(CGeoPoint:new_local(200,0)), _, 500, flag.allow_dss),
  Leader   = task.goCmuRush(ball.antiYPos(CGeoPoint:new_local(-200,150)), _, 500, flag.allow_dss),
  Defender = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{A}{SMDL}"
},

["go"] = {
  switch = function ()
    if bufcnt(player.toTargetDist("Middle") < 20 and
              player.toTargetDist("Special") < 30, "normal", 180)then
      return "sidegoto"
    end
  end,
  Assister = task.staticGetBall(CGeoPoint:new_local(450, 0)),
  Middle   = task.goCmuRush(FRONTKICKV4_GO_POS, _, 500, flag.allow_dss),
  Special  = task.goCmuRush(JAM_BACK_POS, _, 500, flag.allow_dss),
  Leader   = task.goCmuRush(ball.antiYPos(CGeoPoint:new_local(-200,150)), _, 500, flag.allow_dss),
  Defender = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{AMSDL}"
},

["sidegoto"] = {
  switch = function ()
    if  bufcnt(true, 60) then
      return "continuegoto"
    end
  end,
  Assister = task.staticGetBall(CGeoPoint:new_local(450, 0)),
  Leader   = task.goCmuRush(TMP_POS, _, 500, flag.allow_dss),
  Special  = task.goCmuRush(JAM_BACK_POS, _, 500, flag.allow_dss),
  Middle   = task.goCmuRush(FRONTKICKV4_GO_POS, _, 500, flag.allow_dss),
  Defender = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{AMSDL}"
},

["continuegoto"] = {
  switch = function ()
    if bufcnt(player.toTargetDist("Defender") < 150, 10, 180) then
      return "continuepass"
    end
  end,
  Assister = task.staticGetBall(CGeoPoint:new_local(450, 0)),
  Leader   = task.goCmuRush(ball.jamPos(CGeoPoint:new_local(400, 0), 70, 40), _, 500, flag.allow_dss),
  Special  = task.goCmuRush(JAM_BACK_POS, _, 500, flag.allow_dss),
  Middle   = task.goCmuRush(FRONTKICKV4_GO_POS, _, 500, flag.allow_dss),
  Defender = task.goCmuRush(ball.goRush(), _, _, flag.allow_dss),
  Goalie   = task.goalie(),
  match    = "{AMDSL}"
},

["continuepass"] = {
    switch = function ()
    if bufcnt(player.kickBall("Assister") or
              player.isBallPassed("Assister", "Defender"), 1, 90) then
      return "continuekick"
    elseif bufcnt(true, 90) then
      return "exit"
    end
  end,
  Leader   = task.goCmuRush(ball.jamPos(CGeoPoint:new_local(400, 0), 70, 40)),
  Assister = task.goAndTurnKick(pos.passForTouch(ball.goRush()), 400),
  Middle   = task.goCmuRush(FRONTKICKV4_GO_POS, _, 500, flag.allow_dss),
  Special  = task.goCmuRush(RUSH_POS, _, 500, flag.allow_dss),
  Defender = task.goCmuRush(ball.goRush()),
  Goalie   = task.goalie(),
  match    = "{AMDSL}"
},

["continuekick"] = {
    switch = function ()
    if bufcnt(player.kickBall("Defender"), 1, 60) then
      return "exit"
    end
  end,
  Defender = task.touch(),
  Middle   = task.rightBack(),
  Special  = task.goCmuRush(RUSH_POS, _, 500, flag.allow_dss),
  Leader   = task.goCmuRush(JAM_BACK_POS, _, 500, flag.allow_dss),
  Assister = task.leftBack(),
  Goalie   = task.goalie(),
  match    = "{D}{LS}(MA)"
},

name = "Ref_FrontKickV1",
applicable ={
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}