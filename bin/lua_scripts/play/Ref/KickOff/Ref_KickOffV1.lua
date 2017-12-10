-- 直接挑门
-- by yys 2015.03.22

local CHIP_POS = CGeoPoint:new_local(500, 20)

local ASSISTER_POS_1 = ball.refAntiYPos(CGeoPoint:new_local(-15, 230))
local ASSISTER_POS_2 = ball.refAntiYPos(CGeoPoint:new_local(-60, 60))

local SPECIAL_POS_1  = ball.refAntiYPos(CGeoPoint:new_local(-15, -230))
local SPECIAL_POS_2  = ball.refAntiYPos(CGeoPoint:new_local(-60, -60))

local MIDDLE_POS_1   = CGeoPoint:new_local(-220, 250)
local MIDDLE_POS_2   = CGeoPoint:new_local(-5, 250)


gPlayTable.CreatePlay{
firstState = "start",

["start"] = {
  switch = function ()
    if cond.isNormalStart() then
      return "temp"
    end
  end,
  Kicker   = task.staticGetBall(CHIP_POS),
  Assister = task.goCmuRush(ASSISTER_POS_1),
  Special  = task.goCmuRush(SPECIAL_POS_1),
  Middle   = task.goCmuRush(MIDDLE_POS_1),
  Defender = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{ASMD}"
},

["temp"] = {
  switch = function ()
    if bufcnt(true, 60) then
      return "chip"
    end
  end,
  Kicker   = task.slowGetBall(CHIP_POS),
  Assister = task.goCmuRush(ASSISTER_POS_2),
  Special  = task.goCmuRush(SPECIAL_POS_2),
  Middle   = task.goSpeciPos(MIDDLE_POS_2),
  Defender = task.singleBack(),
  Goalie   = task.goalie(),
  match    = "{ASMD}"
},


["chip"] = {
    switch = function ()
    if bufcnt(player.kickBall("Kicker"), 1, 40) then
      return "exit"
    end
  end,
  Kicker   = task.chipPass(CHIP_POS,300), --力度在这里
  Special  = task.goCmuRush(SPECIAL_POS_2),
  Middle   = task.defendMiddle(),
  Defender = task.rightBack(),
  Assister = task.leftBack(),
  Goalie   = task.goalie(),
  match    = "{ASMD}"
},

name = "Ref_KickOffV1",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}
