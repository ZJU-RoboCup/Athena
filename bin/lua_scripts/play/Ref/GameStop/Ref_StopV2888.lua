local side = 1
local TargetPosG  = CGeoPoint:new_local(300,0)
local TargetPos1  = CGeoPoint:new_local(270,-60)
local TargetPos2  = CGeoPoint:new_local(270,-30)
local TargetPos3  = CGeoPoint:new_local(270,0)
local TargetPos4  = CGeoPoint:new_local(270,30)
local TargetPos5  = CGeoPoint:new_local(270,60)

gPlayTable.CreatePlay {

firstState = "move",
["move"] = {
  switch = function ()
    if bufcnt(
      player.toTargetDist("Assister") <20 and 
      player.toTargetDist("Leader")   <20 and 
      player.toTargetDist("Special")  <20 and 
      player.toTargetDist("Defender") <20 and 
      player.toTargetDist("Middle")   <20 and 
      player.toTargetDist("Goalie")   <20
      ,20,400) then
      return "turn"
    end
    if cond.isGameOn() then
      return "exit"
    end   
  end,
  Leader   = task.goCmuRush(TargetPos1,0,150,flag.allow_dss),
  Special  = task.goCmuRush(TargetPos2,0,150,flag.allow_dss),
  Middle   = task.goCmuRush(TargetPos3,0,150,flag.allow_dss),
  Defender = task.goCmuRush(TargetPos4,0,150,flag.allow_dss),
  Assister = task.goCmuRush(TargetPos5,0,150,flag.allow_dss),
  Goalie   = task.goCmuRush(TargetPosG,0,150,flag.allow_dss),
  match = "{LSMDA}"
},

["turn"] = {
  switch = function ()
    if bufcnt(true,10) then
      return "turn2"
    end
    if cond.isGameOn() then
      return "exit"
    end   
  end,
  Leader   = task.goCmuRush(TargetPos1,math.pi/2,150,flag.allow_dss),
  Special  = task.goCmuRush(TargetPos2,math.pi/2,150,flag.allow_dss),
  Middle   = task.goCmuRush(TargetPos3,math.pi/2,150,flag.allow_dss),
  Defender = task.goCmuRush(TargetPos4,math.pi/2,150,flag.allow_dss),
  Assister = task.goCmuRush(TargetPos5,math.pi/2,150,flag.allow_dss),
  Goalie   = task.goCmuRush(TargetPosG,math.pi/2,150,flag.allow_dss),
  match = "{LSMDA}"
},
["turn2"] = {
  switch = function ()
    if bufcnt(true,150) then
      return "halt"
    end
    if cond.isGameOn() then
      return "exit"
    end   
  end,
  Leader   = task.goCmuRush(TargetPos1,math.pi,150,flag.allow_dss),
  Special  = task.goCmuRush(TargetPos2,math.pi,150,flag.allow_dss),
  Middle   = task.goCmuRush(TargetPos3,math.pi,150,flag.allow_dss),
  Defender = task.goCmuRush(TargetPos4,math.pi,150,flag.allow_dss),
  Assister = task.goCmuRush(TargetPos5,math.pi,150,flag.allow_dss),
  Goalie   = task.goCmuRush(TargetPosG,math.pi,150,flag.allow_dss),
  match = "{LSMDA}"
},


["halt"] = {
  switch = function()
    if cond.isGameOn() then
      return "exit"
    end   
    return "halt"
  end,
  ["Leader"]   = task.stop(),
  ["Special"]  = task.stop(),
  ["Assister"] = task.stop(),
  ["Defender"] = task.stop(),
  ["Middle"]   = task.stop(),
  ["Goalie"]   = task.stop(),
  match = "[LSADM]"
},

name = "Ref_StopV2888",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999
}