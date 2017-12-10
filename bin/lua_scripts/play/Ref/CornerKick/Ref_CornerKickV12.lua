
local Left_POS     = ball.refAntiYPos(CGeoPoint:new_local(300, 40))
local Middle_POS   = ball.refAntiYPos(CGeoPoint:new_local(295, 15))
local Right_POS    = ball.refAntiYPos(CGeoPoint:new_local(295, -15))
local Other_POS    = ball.refAntiYPos(CGeoPoint:new_local(300, -40))

local Block_POS_1  = ball.refAntiYPos(CGeoPoint:new_local(300, 15))
local Block_POS_2  = ball.refAntiYPos(CGeoPoint:new_local(300, -15))

local RUN_POS_1    = {
  ball.refAntiYPos(CGeoPoint:new_local(210, 15)),
  ball.refAntiYPos(CGeoPoint:new_local(80, 120)),
  --ball.refAntiYPos(CGeoPoint:new_local(365, 100)),
  ball.refAntiYPos(CGeoPoint:new_local(440, 130))
}
local SHOOT_POS =  ball.refAntiYPos(CGeoPoint:new_local(400, 130))

local chipPower = 280--210
local RUN_POS_2    = {
  ball.refAntiYPos(CGeoPoint:new_local(210, -15)),
  ball.refAntiYPos(CGeoPoint:new_local(30, -120)),
  ball.refAntiYPos(CGeoPoint:new_local(220, -200)),
}

local PASS_POS = CGeoPoint:new_local(400,130)--CGeoPoint:new_local(365, 100)--pos.passForTouch(CGeoPoint:new_local(380, 120))

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
  Special  = task.goCmuRush(Left_POS, _, 400, flag.allow_dss),
  Leader   = task.goCmuRush(Middle_POS, _, 400, flag.allow_dss),
  Middle   = task.goCmuRush(Right_POS, _, 400, flag.allow_dss),
  Defender = task.goCmuRush(Other_POS, _, 400, flag.allow_dss),
  Goalie   = task.goalie(),
  match    = "{A}{LMSD}"
},
["wait"] = {
  switch = function()
    if bufcnt(true, 100) then
      return "goalone"
    end
  end,
  Assister = task.staticGetBall(PASS_POS),
  Special  = task.goSimplePos(Block_POS_1),
  Leader   = task.runMultiPos(RUN_POS_1, false, 25),
  Middle   = task.runMultiPos(RUN_POS_2, false, 40),
  Defender = task.goSimplePos(Block_POS_2),
  Goalie   = task.goalie(),
  match    = "{ALMSD}"
},
["goalone"] = {
  switch = function()
    if bufcnt(true, 60) then
      return "pass"
    end
  end,
  Assister = task.staticGetBall(PASS_POS),--Assister = task.slowGetBall(PASS_POS),
  Special  = task.goSimplePos(Block_POS_1),
  Leader   = task.continue(),--runMultiPos(RUN_POS_1, false, 30),
  Middle   = task.continue(),--runMultiPos(RUN_POS_2, false, 40),
  Defender = task.goSimplePos(Block_POS_2),
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
  Assister = task.chipPass(PASS_POS, chipPower),
  Special  = task.leftBack(),
  Leader   = task.continue(),
  Middle   = task.continue(),
  Defender = task.rightBack(),
  Goalie   = task.goalie(),
  match    = "{ALM}[SD]"
},
["fix"] = {
  switch = function()
    if bufcnt(true,30) then
      return "kick"
    end
  end,
  Assister = task.goSupportPos("Special"),
  Special  = task.leftBack(),
  Leader   = task.goCmuRush(SHOOT_POS),
  Middle   = task.defendMiddle(),
  Defender = task.rightBack(),
  Goalie   = task.goalie(),
  match    = "{ALM}[SD]"
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
  Leader   = task.InterTouch(),--task.InterTouch(RUN_POS_1[3]),--
  Middle   = task.defendMiddle(),
  Defender = task.rightBack(),
  Goalie   = task.goalie(),
  match    = "{ALM}[SD]"
},

name = "Ref_CornerKickV12",
applicable = {
  exp = "a",
  a   = true
},
score = 0,
attribute = "attack",
timeout   = 99999
}