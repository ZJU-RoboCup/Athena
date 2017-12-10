-- 通过单边的扯动和开球边的阻挡进行的前场定位球战术,大场地版
-- 此战术面对弱队时（即单边有一固定车不动）效果可能会不好

local READY_POS = CGeoPoint:new_local(-150, 150)
local JAM_FACE_POS1 = CGeoPoint:new_local(450, 0)
local JAM_FACE_POS2 = CGeoPoint:new_local(450, 35)
local JAM_FACE_POS3 = CGeoPoint:new_local(450, 70)

local useLearnRound = false
local GO_BACK_BALL
local TURN_KICK

if useLearnRound then
  GO_BACK_BALL = task.goBackBall(0, 20)
  TURN_KICK    = task.testRound("Breaker")
else
  GO_BACK_BALL = task.staticGetBall(CGeoPoint:new_local(450, 0))
  TURN_KICK    = task.goAndTurnKick(ball.goRush(),_,_,_,true);
end

gPlayTable.CreatePlay{

firstState = "startball",

["startball"] = {
  switch = function ()
    if  bufcnt(true, 60) then
      return "goOneJam"
    end
  end,
  Assister = GO_BACK_BALL,
  Leader   = task.goCmuRush(ball.goRush(), _, 600, flag.allow_dss),
  Special  = task.goCmuRush(ball.antiYPos(READY_POS), _, 600, flag.allow_dss),
  Middle   = task.rightBack(),
  Defender = task.leftBack(),
  Goalie   = task.goalie(),
  match    = "{ALS}[MD]"
},

["goOneJam"] = {
  switch = function ()
    if  bufcnt(true, 60) then
      return "goTwoJam"
    end
  end,
  Assister = GO_BACK_BALL,
  Leader   = task.goCmuRush(ball.jamPos(JAM_FACE_POS1, 60, 10), _, 600, flag.allow_dss),
  Special  = task.goCmuRush(ball.goRush(), _, 600, flag.allow_dss),
  Middle   = task.goCmuRush(ball.antiYPos(READY_POS), _, 600, flag.allow_dss),
  Defender = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{ASL}[MD]"
},

["goTwoJam"] = {
  switch = function ()
    if  bufcnt(true, 60) then
      return "goThreeJam"
    end
  end,
  Assister = GO_BACK_BALL,
  Leader   = task.goCmuRush(ball.jamPos(JAM_FACE_POS1, 60, 10), _, 600, flag.allow_dss),
  Special  = task.goCmuRush(ball.jamPos(JAM_FACE_POS2, 80, 20), _, 600, flag.allow_dss),
  Middle   = task.goCmuRush(ball.goRush(), _, 600, flag.allow_dss),
  Defender = task.goCmuRush(ball.antiYPos(READY_POS), _, 600, flag.allow_dss),
  Goalie   = task.goalie(),
  match    = "{ASLMD}"
},

["goThreeJam"] = {
  switch = function ()
    if bufcnt(player.toTargetDist("Defender") < 20, "fast", 120) then
      gRoleNum["Breaker"] = gRoleNum["Defender"]
      return "goPass"
    end
  end,
  Assister = GO_BACK_BALL,
  Leader   = task.goCmuRush(ball.jamPos(JAM_FACE_POS1, 60, 10), _, _, flag.allow_dss),
  Special  = task.goCmuRush(ball.jamPos(JAM_FACE_POS2, 80, 20), _, _, flag.allow_dss),
  Middle   = task.goCmuRush(ball.jamPos(JAM_FACE_POS3, 105, 30), _, _, flag.allow_dss),
  Defender = task.goCmuRush(ball.goRush(), _, _, flag.allow_dss),
  Goalie   = task.goalie(),
  match    = "{ASLMD}"
},

["goPass"] = {
  switch = function ()
    if player.kickBall("Assister") or ball.velMod() > 100 then
      return "goFix"
    elseif  bufcnt(true, 80) then
      return "exit"
    end
  end,
  Assister = TURN_KICK,
  Leader   = task.continue(),
  Special  = task.continue(),
  Middle   = task.continue(),
  Defender = task.continue(),
  Goalie   = task.goalie(),
  match    = "{ALSMD}"
},

["goFix"] = {
    switch = function ()
    if bufcnt(true, 20) then
      return "goKick"
    end
  end,
  Assister = TURN_KICK,
  Leader   = task.continue(),
  Special  = task.continue(),
  Middle   = task.continue(),
  Defender = task.continue(),
  Goalie   = task.goalie(),
  match    = "{ALSDM}"
},

["goKick"] = {
    switch = function ()
    if player.kickBall("Breaker") then
      return "exit"
    elseif bufcnt(true, 120) then
      return "finish"
    elseif ball.posX() < -50 then 
      return "exit" 
    end
  end,
  Breaker  = task.waitTouchNew(ball.goRush()),
  Assister = task.goPassPos("Breaker"),
  Special  = task.defendMiddle(),
  Middle   = task.rightBack(),
  Defender = task.leftBack(),
  Goalie   = task.goalie(),
  match    = "{B}(ADSM)"
},

name = "Ref_FrontKickV2",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}