-- by yys 2014-06-10

gPlayTable.CreatePlay{

firstState = "start",

["start"] = {
  switch = function ()
    if bufcnt(cond.isGameOn(), 15) then
      return "advance"
    end
  end,
  Leader   = task.goSpeciPos(CGeoPoint:new_local(150, 0)),
  Special  = task.goSpeciPos(CGeoPoint:new_local(-320, 80)),
  Assister = task.goSpeciPos(CGeoPoint:new_local(-320, -80)),
  Middle   = task.goSpeciPos(CGeoPoint:new_local(-320, 105)),
  Defender = task.goSpeciPos(CGeoPoint:new_local(-320, -105)),
  Goalie   = task.penaltyGoalieV2(),
  match    = "{LASMD}"
},

["advance"] = {
  switch = function ()
    if bufcnt(true, 60) then
      return "exit"
    end
  end,
  Leader   = task.goSpeciPos(CGeoPoint:new_local(150, 0)),
  Special  = task.goSpeciPos(CGeoPoint:new_local(-320, 80)),
  Assister = task.goSpeciPos(CGeoPoint:new_local(-320, -80)),
  Middle   = task.goSpeciPos(CGeoPoint:new_local(-320, 105)),
  Defender = task.goSpeciPos(CGeoPoint:new_local(-320, -105)),
  Goalie   = task.goalie(),
  match    = "{LASMD}"
},

name = "Ref_PenaltyDefV3",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}