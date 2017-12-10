gPlayTable.CreatePlay{
  firstState = "start",
  ["start"] = {
    switch = function ()
      if bufcnt(false,10,10) then 
        return "start"
      end
    end,
    Kicker = task.waitTouchNew(CGeoPoint:new_local(160, 160)),
    match = ""
  },
  name = "TestTouch",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
}