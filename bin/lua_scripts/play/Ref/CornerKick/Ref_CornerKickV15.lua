local pTarget =ball.refSyntYPos(CGeoPoint:new_local(350,15))

local pFake_1=ball.refAntiYPos(CGeoPoint:new_local(335,66))
local pFake_2=ball.refSyntYPos(CGeoPoint:new_local(335,66))
local pFake_3=ball.refSyntYPos(CGeoPoint:new_local(305,83))
local pFake_4=ball.refSyntYPos(CGeoPoint:new_local(275,101))


local pAttack    = ball.refSyntYPos(CGeoPoint:new_local(200,150))
local pPass      = pos.passForTouch(pAttack)
local pPassFake  = pos.passForTouch(pFake_2)

local pDefendRoute=
{
 ball.refAntiYPos(CGeoPoint:new_local(235,66)),
 ball.refAntiYPos(CGeoPoint:new_local(0,0)),
 ball.refAntiYPos(CGeoPoint:new_local(-317,0))
}

local pAttackRoute = 
{
 ball.refSyntYPos(CGeoPoint:new_local(275,101)),
 ball.refSyntYPos(CGeoPoint:new_local(165,61)),
 ball.refSyntYPos(CGeoPoint:new_local(130,101)),
 pAttack
}

local pBewilderRoute1=
{
 ball.refSyntYPos(CGeoPoint:new_local(335,66)),
 ball.refSyntYPos(CGeoPoint:new_local(323,34)),
 ball.refSyntYPos(CGeoPoint:new_local(318,0)),
 ball.refAntiYPos(CGeoPoint:new_local(323,34)),
 ball.refAntiYPos(CGeoPoint:new_local(335,66))

}
local pBewilderRoute2=
{
 ball.refSyntYPos(CGeoPoint:new_local(305,83)),
 ball.refSyntYPos(CGeoPoint:new_local(289,43)),
 ball.refSyntYPos(CGeoPoint:new_local(283,0)),
 ball.refAntiYPos(CGeoPoint:new_local(289,43)),
 ball.refAntiYPos(CGeoPoint:new_local(305,83))
}

gPlayTable.CreatePlay{

firstState = "start",

["start"] = {
  switch = function ()
    if bufcnt(player.toTargetDist("Middle") < 10 and
              player.toTargetDist("Special") < 10 and
              player.toTargetDist("Leader") < 10 and
              player.toTargetDist("Defender") < 10, 10, 300) then
      return "gradual"
    end
  end,
  Assister = task.staticGetBall(pPassFake),
  Leader   = task.goCmuRush(pFake_4, _, 400),
  Special  = task.goCmuRush(pFake_3, _, 400),
  Middle   = task.goCmuRush(pFake_2, _, 400),
  Defender = task.goCmuRush(pFake_1, _, 400),
  Goalie   = task.goalie(),
  match    = "{A}{LSDM}"
},

["gradual"] = {
  switch = function()
    print("gradual")
    if bufcnt(player.toPointDist("Leader", pAttack) < 20, 3, 120) then
      return "pass"
    end
  end,
  Assister = task.staticGetBall(pPass),
  Leader   = task.runMultiPos(pAttackRoute, false, 30),
  Special  = task.runMultiPos(pBewilderRoute2, false, 30),
  Middle   = task.runMultiPos(pBewilderRoute1, false, 30),
  Defender = task.runMultiPos(pDefendRoute, false, 30),
  Goalie   = task.goalie(),
  match    = "{ALSMD}"
},

["pass"] = {
  switch = function()
    if player.kickBall("Assister") or player.toBallDist("Assister") > 20 then
      return "kick"
    end
  end,
  Assister = task.goAndTurnKick(pPass, 350),
  Leader   = task.goCmuRush(pAttack, _, 450),
  Special  = task.goCmuRush(pFake_1, _, 400),
  Middle   = task.goCmuRush(pFake_2, _, 400),
  Defender = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{ASDM}[L]"
},

["kick"] = {
    switch = function()
    if player.kickBall("Leader") then
      return "finish"
    elseif bufcnt(true, 90) then
      return "exit"
    end
  end,
  Assister = task.goSupportPos("Leader"),
  Leader   = task.waitTouch(pAttack, 0),--important
  Special  = task.goCmuRush(pFake_1, _, 400),
  Middle   = task.goCmuRush(pFake_2, _, 400),
  Defender = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{ADS}[LM]"
},

name = "Ref_CornerKickV15",
applicable = {
  exp = "a",
  a   = true
},
score = 0,
attribute = "attack",
timeout   = 99999
}