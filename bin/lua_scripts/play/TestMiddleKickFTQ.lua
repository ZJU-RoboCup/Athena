-- 

local MIDDLE_POS1 = ball.antiYPos(CGeoPoint:new_local(110, 200))
local S_POS = function()
  local tmpY
  local inFlag = true
  local POS
  local updatepos = function()
    tmpY = ball.posY()
  end 
  local pos_BallChange = function ()
    updatepos()
    if inFlag then
      inFlag = false
      if tmpY > 0 then 
        POS = CGeoPoint:new_local(170, -220)
      else
        POS = CGeoPoint:new_local(170, 220)
      end
    end
    return POS
  end
  return pos_BallChange
end
local SHOOT_POS = S_POS()


local FAKE_POS1	  = ball.antiYPos(CGeoPoint:new_local(380, -200))
local FAKE_POS2   = ball.antiYPos(CGeoPoint:new_local(190, -220))

gPlayTable.CreatePlay{

firstState = "startBall",

["startBall"] = {
	switch = function ()
  	if bufcnt(player.toTargetDist("Defender") < 30 and
        	player.toTargetDist("Middle") 		  < 30, "normal") then
    		return "firstPass"
  	end
	end,
	Assister = task.staticGetBall(CGeoPoint:new_local(450, 0)),
	Middle	 = task.goCmuRush(MIDDLE_POS1),
  Defender = task.goCmuRush(FAKE_POS1),
  Leader   = task.leftBack(),
  Special  = task.rightBack(),
  Goalie   = task.goalie(),
  match    = "{A}(MD)(LS)"
},


["firstPass"] = {
	switch = function ()
    	if bufcnt(player.kickBall("Assister"), 1) then
      		return "secondPass"
    	end
  	end,
  	Assister = task.goAndTurnKick(MIDDLE_POS1, 450),
  	Middle   = task.goCmuRush(MIDDLE_POS1, _, _, flag.allow_dss),
  	Defender = task.goCmuRush(FAKE_POS2, _, _, flag.allow_dss),
    Leader   = task.leftBack(),
    Special  = task.rightBack(),
    Goalie   = task.goalie(),
  	match 	 = "{AMD}(LS)"
},


["secondPass"] = {
	switch = function ()
    	if bufcnt(player.kickBall("Middle"), 1) then
      		return "fix"
    	end
  	end,	
	Assister = task.rightBack(),
	Middle   = task.receivePass(SHOOT_POS, _),
	Defender = task.goCmuRush(SHOOT_POS, _, _, flag.allow_dss),
  Leader   = task.leftBack(),
  Special  = task.defendMiddle(),
  Goalie   = task.goalie(),
	match = "{AMD}(LS)"
},

["fix"] = {
  switch = function ()
      if bufcnt(true, 10) then
          return "shoot"
      end
    end,  
  Assister = task.rightBack(),
  Middle   = task.goSupportPos("Defender"),
  Defender = task.task.goCmuRush(SHOOT_POS, _, _),
  Leader   = task.leftBack(),
  Special  = task.defendMiddle(),
  Goalie   = task.goalie(),
  match = "{AMD}(LS)"
},


["shoot"] = {
	switch = function ()
    	if bufcnt(player.kickBall("Middle"), 1) then
      		return "shoot"
    	end
  	end,	
	Assister = task.rightBack(),
	Middle   = task.leftBack(),
	Defender = task.waitTouchNew(),
  Leader   = task.leftBack(),
  Special  = task.rightBack(),
  Goalie   = task.goalie(),
	match = "{AMD}(LS)"
},

name = "TestMiddleKickFTQ",
applicable = {
  exp = "a",
  a = true
},
attribute = "attack",
timeout = 99999

}