local theirGoal=CGeoPoint:new_local(450,0)
local ourGoal=CGeoPoint:new_local(-450,0)


gPlayTable.CreatePlay{

firstState = "Pause",

["Pause"] = {
  switch = function ()
    if bufcnt(ball.valid() ,60) then  --and player.toBallDist("Kicker")<300 
      return "staticGetBall"
    end
  end,
  Goalie = task.stop(),
  match = ""
},

["staticGetBall"]={
  switch = function ()
    if bufcnt(player.toBallDist("Goalie")<50 ,150) then  --and player.toBallDist("Kicker")<300 
      return "goAndTurnKick"
    end
  end,
  Goalie = task.staticGetBall(CGeoPoint:new_local(0,-200)),
  match = ""

},

["goAndTurnKick"] = {
  switch = function ()
    if bufcnt(player.kickBall("Goalie"),"fast") then
      return "Wait"
    end
  end,

  Goalie = function() 
    if ball.posX()>0 then
      print "x>0"
      return task.goAndTurnKickQuick(theirGoal,1000,"flat")
    else
      print "x<0"
      return task.goAndTurnKickQuick(ourGoal,1000,"flat")
    end
  end,
  --Kicker =  task.goAndTurnKick(theirGoal,1000),
  match = "" 
},

["Wait"] = {
  switch = function ()
    if bufcnt(true,100) then
      return "Psuse"
    end
  end,
  Goalie =  task.stop(),
  match = "" 
},

name = "TestGoAndTurnKick",

applicable ={
  exp = "a",
  a = true
},

attribute = "attack",

timeout = 99999
}