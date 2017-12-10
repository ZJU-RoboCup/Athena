gPlayTable.CreatePlay{

firstState = "start",

["start"] = {
  switch = function ()
    if bufcnt(cond.isNormalStart(), 1) then --cond.isNormalStart(), 1
      return "advance"
    end
  end,
  Leader   = task.goSpeciPos(CGeoPoint:new_local(150, 0)),
  Special  = task.goSpeciPos(CGeoPoint:new_local(-320, 80)),
  Assister = task.goSpeciPos(CGeoPoint:new_local(-320, -80)),
  Middle   = task.goSpeciPos(CGeoPoint:new_local(-320, 105)),
  Defender = task.goSpeciPos(CGeoPoint:new_local(-320, -105)),
  Goalie   = task.goSpeciPos(CGeoPoint:new_local(-450, 0)), --goalie(), --= task.penaltyGoalie2017V1(),
  match    = "{LASMD}"
},

["advance"] = {
  switch = function ()
    if bufcnt(true, 600) then
      return "exit"
    end
  end,
  Leader   = task.goSpeciPos(CGeoPoint:new_local(150, 0)),
  Special  = task.goSpeciPos(CGeoPoint:new_local(-320, 80)),
  Assister = task.goSpeciPos(CGeoPoint:new_local(-320, -80)),
  Middle   = task.goSpeciPos(CGeoPoint:new_local(-320, 105)),
  Defender = task.goSpeciPos(CGeoPoint:new_local(-320, -105)),
  Goalie   = task.penaltyGoalie2017V1(),
  match    = "{LASMD}"
},

name = "Ref_PenaltyDefV4",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}