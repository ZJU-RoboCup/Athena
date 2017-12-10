--针对死防远角设计的定位球 by Fantasy  7.14


local Final_Shoot_POS= ball.refAntiYPos(CGeoPoint:new_local(315, 15))
local PASS_POS = pos.passForTouch(CGeoPoint:new_local(315, 15))
--[1]
local Left_POS     = ball.refAntiYPos(CGeoPoint:new_local(287, 175))
local Middle_POS   = ball.refAntiYPos(CGeoPoint:new_local(229, 114))
local Right_POS    = ball.refAntiYPos(CGeoPoint:new_local(164, -110))
local Other_POS    = ball.refAntiYPos(CGeoPoint:new_local(260, -170))


--[2]
local Left_POS_2     = ball.refAntiYPos(CGeoPoint:new_local(340, 120))
local Middle_POS_2   = ball.refAntiYPos(CGeoPoint:new_local(300, 90))
local Other_POS_2    = ball.refAntiYPos(CGeoPoint:new_local(320, -50))
local Right_POS_2    = ball.refAntiYPos(CGeoPoint:new_local(180, 50))




--[3]
local Left_POS_3   = ball.refAntiYPos(CGeoPoint:new_local(390, 160))
local Middle_POS_3   ={ 
  ball.refAntiYPos(CGeoPoint:new_local(190, 100)),
  ball.refAntiYPos(CGeoPoint:new_local(200, -150))
}

local Other_POS_3    = ball.refAntiYPos(CGeoPoint:new_local(150, -200))


--[4]
local back_Pos = ball.refAntiYPos(CGeoPoint:new_local(-80, 0))


--[Defender]
local RUN_POS_1    = {
  ball.refAntiYPos(CGeoPoint:new_local(200, 50)),
  ball.refAntiYPos(CGeoPoint:new_local(280, 30)),
  ball.refAntiYPos(CGeoPoint:new_local(315, 15))
}



gPlayTable.CreatePlay{

firstState = "start",

["start"] = {
  switch = function ()
    if bufcnt(player.toTargetDist("Middle") < 10 and
              player.toTargetDist("Special") < 10 and
              player.toTargetDist("Leader") < 10 and
              player.toTargetDist("Defender") < 10, 120, 240) then
      return "goTmpPos1"
    end
  end,
  Assister = task.staticGetBall(PASS_POS),
  Special  = task.goCmuRush(Left_POS, _, 600, flag.allow_dss),
  Leader   = task.goCmuRush(Middle_POS, _, 600, flag.allow_dss),
  Middle   = task.goCmuRush(Other_POS, _, 600, flag.allow_dss),
  Defender = task.goCmuRush(Right_POS, _, 600, flag.allow_dss),
  Goalie   = task.goalie(),
  match    = "{A}{LSDM}"
},

["goTmpPos1"] = {
  switch = function()
     if player.toTargetDist("Defender") < 10 then
      return "goTmpPos2"
    end
  end,
  Assister = task.staticGetBall(PASS_POS),
  Special  = task.goCmuRush(Left_POS_2, _, 600, flag.allow_dss),
  Leader   = task.goCmuRush(Middle_POS_2, _, 600, flag.allow_dss),
  Middle   = task.goCmuRush(Other_POS_2, _, 600, flag.allow_dss),
  Defender = task.goCmuRush(Right_POS_2, _, 600, flag.allow_dss),
  Goalie   = task.goalie(),
  match    = "{ALSMD}"
},

["goTmpPos2"] = {
  switch = function()
    if player.toPointDist("Defender", RUN_POS_1[2]()) < 10 then
      return "pass"
    end
  end,
  Assister = task.slowGetBall(PASS_POS),
  Special  = task.goCmuRush(Left_POS_3, _, 600, flag.allow_dss),
  Leader   = task.runMultiPos(Middle_POS_3, false, 40),
  Middle   = task.goCmuRush(Other_POS_3, _, 600, flag.allow_dss),
  Defender = task.runMultiPos(RUN_POS_1, false, 40),
  Goalie   = task.goalie(),
  match    = "{ALSMD}"
},

["pass"] = {
    switch = function()
    if player.kickBall("Assister") or player.toBallDist("Assister") > 30 then
      return "waitBall"
    end
  end,
  Assister = task.chipPass(PASS_POS, 130),
  Special  = task.goCmuRush(Left_POS_3, _, 600, flag.allow_dss),
  Leader   = task.continue(),
  Middle   = task.goCmuRush(Other_POS_3, _, 600, flag.allow_dss),
  Defender = task.continue(),
  Goalie   = task.goalie(),
  match    = "{ALSMD}"
},

["waitBall"] = {
    switch = function ()
      if bufcnt(player.toPointDist("Defender", RUN_POS_1[3]()) < 10, "fast", 60) then
        return "shootBall"
      end
    end,
    Assister = task.stop(),
    Special  = task.goCmuRush(Left_POS_3, _, 600, flag.allow_dss),
    Leader   = task.continue(),
    Middle   = task.goCmuRush(back_Pos, _, 600, flag.allow_dss),
    Defender = task.goCmuRush(RUN_POS_1[3], 0, 600, flag.allow_dss),
    Goalie   = task.goalie(),
    match    = "{ALDMS}"
  },

["shootBall"] = {
    switch = function ()
      if bufcnt(player.kickBall("Defender"), "fast", 100)  then
        return "exit"
      end
    end,
    Assister = task.rightBack(),
    Special  = task.goCmuRush(Left_POS_3, _, 600, flag.allow_dss),
    Leader   = task.goCmuRush(Middle_POS_3[2], _, 600, flag.allow_dss),
    Middle   = task.leftBack(),
    Defender = task.waitTouch(RUN_POS_1[3], 1.57),
    Goalie   = task.goalie(),
    match    = "{ALD}[MS]"
  },

name = "Ref_CornerKickV21",
applicable = {
  exp = "a",
  a   = true
},
score = 0,
attribute = "attack",
timeout   = 99999
}