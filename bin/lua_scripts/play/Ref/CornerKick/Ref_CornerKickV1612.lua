local WAIT_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(300, 40)),
  ball.refAntiYPos(CGeoPoint:new_local(295, 15)),
  ball.refAntiYPos(CGeoPoint:new_local(295, -15)),
  ball.refAntiYPos(CGeoPoint:new_local(300, -40))
}
local MIDDLE_POSX = 260
local MIDDLE_POSY = 40
local DEFENDER_POSX = 210
local DEFENDER_POSY = 5
local RUSH_POS = {
  ball.refAntiYPos(CGeoPoint:new_local(320, 60)),
  ball.refAntiYPos(CGeoPoint:new_local(290, 40)),
  ball.refAntiYPos(CGeoPoint:new_local(MIDDLE_POSX, MIDDLE_POSY)),
  ball.refAntiYPos(CGeoPoint:new_local(DEFENDER_POSX, DEFENDER_POSY))
}
local X = {-50, 130, 130}
local Y = {120, 90, 90}

local CHIP1OR2 = true
local USE_FAR_POINT = gOppoConfig.CornerKickPosSwitch
local PASS_POS1 = USE_FAR_POINT and (CGeoPoint:new_local(370, 200)) or CGeoPoint:new_local(395, 125)--pos.passForTouch(CGeoPoint:new_local(380, 120))
local PASS_POS2 = USE_FAR_POINT and (CGeoPoint:new_local(230, 150)) or CGeoPoint:new_local(345, 90)
local PASS_POS = CHIP1OR2 and PASS_POS2 or PASS_POS1
local CHIP_POWER = USE_FAR_POINT and 230 or 190

local RUN_POS1    = {
  ball.refAntiYPos(CGeoPoint:new_local(MIDDLE_POSX+X[1], MIDDLE_POSY+Y[1])),
  ball.refAntiYPos(CGeoPoint:new_local(MIDDLE_POSX+X[2], MIDDLE_POSY+Y[2])),
  ball.refAntiYPos(PASS_POS1),
}
local RUN_POS2    = {
  ball.refAntiYPos(CGeoPoint:new_local(DEFENDER_POSX+X[1], DEFENDER_POSY+Y[1])),
  ball.refAntiYPos(CGeoPoint:new_local(DEFENDER_POSX+X[2], DEFENDER_POSY+Y[2])),
  ball.refAntiYPos(PASS_POS2),
}

local RETURN_POS =  ball.refSyntYPos(CGeoPoint:new_local(300,-270))

--(395,125)
gPlayTable.CreatePlay{

firstState = "start",

["start"] = {
  switch = function ()
    if bufcnt(player.toTargetDist("Middle") < 20 and
              player.toTargetDist("Special") < 20, 120, 180) then
      return "wait"
    end
  end,
  Assister = task.staticGetBall(PASS_POS),
  Special  = task.goCmuRush(WAIT_POS[1], _, 400, flag.allow_dss),
  Leader   = task.goCmuRush(WAIT_POS[2], _, 400, flag.allow_dss),
  Middle   = task.goCmuRush(WAIT_POS[3], _, 400, flag.allow_dss),
  Defender = task.goCmuRush(WAIT_POS[4], _, 400, flag.allow_dss),
  Goalie   = task.goalie(),
  match    = "{A}{LMSD}"
},
["wait"] = {
  switch = function()
    if bufcnt(true, 90) then
      return "goalone"
    end
  end,
  Assister = task.staticGetBall(PASS_POS),
  Special  = task.goCmuRush(RUSH_POS[1], _, 400, flag.allow_dss),
  Leader   = task.goCmuRush(RUSH_POS[2], _, 400, flag.allow_dss),
  Middle   = task.goCmuRush(RUSH_POS[3], _, 400, flag.allow_dss),
  Defender = task.goCmuRush(RUSH_POS[4], _, 400, flag.allow_dss),
  Goalie   = task.goalie(),
  match    = "{ALMSD}"
},
["goalone"] = {
  switch = function()
    if bufcnt(true, 90) then
      return "pass"
    end
  end,
  Assister = task.staticGetBall(PASS_POS),--task.slowGetBall(PASS_POS),
  Special  = task.goCmuRush(RUSH_POS[1], _, 400, flag.allow_dss),
  Leader   = task.goCmuRush(RUSH_POS[2], _, 400, flag.allow_dss),
  Middle   = task.runMultiPos(RUN_POS1),
  Defender = task.runMultiPos(RUN_POS2),
  Goalie   = task.goalie(),
  match    = "{ALMSD}"
},

["pass"] = {
  switch = function()
    if player.kickBall("Assister") or player.toBallDist("Assister") > 20 then
      return "fix"
    elseif bufcnt(true, 120) then
      return "exit"
    end
  end,
  Assister = task.chipPass(PASS_POS, CHIP_POWER),
  Special  = task.leftBack(),
  Leader   = task.rightBack(),
  Middle   = task.continue(),
  Defender = task.continue(),
  Goalie   = task.goalie(),
  match    = "{ALMSD}"
},
["fix"] = {
  switch = function()
    if bufcnt(true,30) then
      return "kick"
    end
  end,
  Assister = task.goSupportPos("Special"),
  Special  = task.leftBack(),
  Leader   = task.rightBack(),
  Middle   = task.goCmuRush(RUN_POS1[3]),
  Defender = task.goCmuRush(RUN_POS2[3]),
  Goalie   = task.goalie(),
  match    = "{ALMSD}"
},
["kick"] = {
    switch = function()
    if player.kickBall("Leader") then
      return "finish"
    elseif bufcnt(true, 110) then
      return "exit"
    end
  end,
  Assister = task.goSupportPos("Special"),
  Special  = task.leftBack(),
  Leader   = task.rightBack(),
  Middle   = CHIP1OR2 and task.goCmuRush(RETURN_POS) or task.waitTouchNew(RUN_POS1[3]),
  Defender = CHIP1OR2 and task.InterTouch(_, _, 999127) or task.goCmuRush(RETURN_POS),
  Goalie   = task.goalie(),
  match    = "{ALMSD}"
},

name = "Ref_CornerKickV1612",
applicable = {
  exp = "a",
  a   = true
},
score = 0,
attribute = "attack",
timeout   = 99999
}