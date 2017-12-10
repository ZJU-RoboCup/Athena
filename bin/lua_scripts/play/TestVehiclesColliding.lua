local d = {
  Leader   = function()
    --return dir.ourPlayerToPlayer("Leader", "Assister")
    return 1.57
  end,

  Assister = function()
    --return dir.ourPlayerToPlayer("Assister", "Leader")
    return 1.57
  end
}

local maxLeaderSpeed = 0.0
local maxAssistSpeed = 0.0

gPlayTable.CreatePlay {

  firstState = "goto",
  
  switch = function()
    if gCurrentState == "goto" then
      if player.toTargetDist("Leader") < 5 and player.toTargetDist("Assister") < 5 then
        return "dash"
      end
    elseif gCurrentState == "dash" then
      if player.velMod("Leader") > maxLeaderSpeed then
        maxLeaderSpeed = player.velMod("Leader")
      end
      if player.velMod("Assister") > maxAssistSpeed then
        maxAssistSpeed = player.velMod("Assister")
      end
      if player.toTargetDist("Leader") < 5 and player.toTargetDist("Assister") < 5 then
        print("Max Leader Speed:\t"..maxLeaderSpeed)
        print("Max Assister Speed:\t"..maxAssistSpeed)
        maxLeaderSpeed = 0.0
        maxAssistSpeed = 0.0
        return "goto"
      end
    end
  end,

  ["goto"] = {
    Leader   = task.goSimplePos(CGeoPoint:new_local(-50, -200)),
    Assister = task.goSimplePos(CGeoPoint:new_local(-350, -200)),
    match    = "(LA)"
  },

  ["dash"] = {
    Leader   = task.goSimplePos(CGeoPoint:new_local(-350, -200), d["Leader"]),
    Assister = task.goSimplePos(CGeoPoint:new_local(-50, -200), d["Assister"]),
    match    = "{LA}"
  },
  
  name       = "TestVehiclesColliding",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute  = "attack",
  timeout    = 99999
}