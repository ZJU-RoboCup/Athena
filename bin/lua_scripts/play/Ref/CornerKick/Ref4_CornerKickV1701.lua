local FRONT = 150
local BACK = 120
local TMP_ROUND_POS1 = {
  ball.refAntiYPos(CGeoPoint:new_local(FRONT,  0)),
  ball.refAntiYPos(CGeoPoint:new_local(FRONT, -25)),
  ball.refAntiYPos(CGeoPoint:new_local(BACK, -25)),
  ball.refAntiYPos(CGeoPoint:new_local(BACK, 0)),
  ball.refAntiYPos(CGeoPoint:new_local(BACK, 25)),
  ball.refAntiYPos(CGeoPoint:new_local(FRONT, 25)),
  ball.refAntiYPos(CGeoPoint:new_local(FRONT, 0))
}

local TMP_ROUND_POS2 = {
  ball.refAntiYPos(CGeoPoint:new_local(BACK,  0)),
  ball.refAntiYPos(CGeoPoint:new_local(BACK, 25)),
  ball.refAntiYPos(CGeoPoint:new_local(FRONT, 25)),
  ball.refAntiYPos(CGeoPoint:new_local(FRONT, 0)),
  ball.refAntiYPos(CGeoPoint:new_local(FRONT, -25)),
  ball.refAntiYPos(CGeoPoint:new_local(BACK, -25)),
  ball.refAntiYPos(CGeoPoint:new_local(BACK, 0))
}

local PASS_POS1 = CGeoPoint:new_local(235, 140)
local PASS_POS2 = CGeoPoint:new_local(150, -230)
local SHOOT_POS1 = ball.refAntiYPos(PASS_POS1)
local SHOOT_POS2 = ball.refAntiYPos(PASS_POS2)
local FAKE_POS = ball.refAntiYPos(CGeoPoint:new_local(200, -100))
local MIDDLE_POS = ball.refAntiYPos(CGeoPoint:new_local(70, 0))

local HALF = gOppoConfig.IfHalfField

gPlayTable.CreatePlay{
  firstState = "start",
  ["start"] = {
   	switch = function()
  		if bufcnt(player.toTargetDist("Leader") < 20, 20, 180) then
  			return "round"
  		end
    end,
    Assister = task.staticGetBall(PASS_POS1),
  	Leader   = task.goCmuRush(TMP_ROUND_POS1[1], _, 600, flag.allow_dss),
  	Middle   = task.goCmuRush(TMP_ROUND_POS2[1], _, 600, flag.allow_dss),
  	match    = "{AML}" 
  }
,
  ["round"] = {
  	switch = function()
  		if bufcnt(player.toTargetDist("Leader") < 10, 20, 180) then
        	return "goalone"
     	end
    end,
    Assister = task.staticGetBall(PASS_POS2),
  	Leader   = task.runMultiPos(TMP_ROUND_POS1, false, 10),
  	Middle   = task.runMultiPos(TMP_ROUND_POS2, false, 10),
  	match    = "{AML}" 
  },

  ["goalone"] = {
  	switch = function()
  		if bufcnt(true,40) then
  			--[[if player.isMarked("Leader") then
    				return "passball1"
    			else
  				return "passball2"
  			end--]]
  			return "passball"..math.random(2)
  		end
    end,
    Assister = task.staticGetBall(PASS_POS1),
  	Leader   = task.goCmuRush(SHOOT_POS2, _, 600, flag.allow_dss),
  	Middle   = task.goCmuRush(SHOOT_POS1, _, 600, flag.allow_dss),
  	match    = "{AML}" 
  },

  ["passball1"] = {
  	switch = function()
  		if player.kickBall("Assister") or player.toBallDist("Assister") > 30 then
         	return "fix1"
      	end 
    end,
    Assister = task.chipPass(PASS_POS1,250),
  	Leader   = task.goCmuRush(SHOOT_POS2, _, 600, flag.allow_dss),
  	Middle   = task.goCmuRush(SHOOT_POS1, _, 600),
  	match    = "{AML}" 
  },

  ["passball2"] = {
  	switch = function()
      	if player.kickBall("Assister") or player.toBallDist("Assister") > 30 then
         	return "shootball2"
      	end 
    end,
    Assister = task.goAndTurnKick(PASS_POS2,550),
  	Leader   = task.goCmuRush(SHOOT_POS2, _, 600),
  	Middle   = task.goCmuRush(SHOOT_POS1, _, 600, flag.allow_dss),
  	match    = "{AML}" 
  },

  ["fix1"] = {
  	switch = function()
		 if bufcnt(true,30) then
        	return "shootball1"
      	 end  
    end,
    Assister = task.stop(),
  	Leader   = task.goCmuRush(MIDDLE_POS, _, 600, flag.allow_dss),
  	Middle   = task.goCmuRush(SHOOT_POS1, _, 600),
  	match    = "{AML}" 
  },

  ["fix2"] = {
  	switch = function()
		 if bufcnt(true,40) then
        	return "shootball2"
      	 end  
    end,
    Assister = task.stop(),
  	Leader   = task.goCmuRush(SHOOT_POS2, _, 600),
  	Middle   = task.goCmuRush(MIDDLE_POS, player.toBallDir, 600, flag.allow_dss),
  	match    = "{AML}" 
  },

  ["shootball1"] = {
  	switch = function()
	  if bufcnt(player.kickBall("Middle"), 1, 150)  then
         return HALF and "stop" or "finish"
      end  
    end,
    Assister = task.stop(),
  	Leader   = task.goCmuRush(MIDDLE_POS, _, 600, flag.allow_dss),
  	Middle   = task.InterTouch(),
  	match    = "{AML}" 
  },

  ["shootball2"] = {
  	switch = function()
	  if player.kickBall("Leader") and bufcnt(true,150) then
         return HALF and "stop" or "finish"
      end  
    end,
    Assister = task.stop(),
  	Leader   = task.InterTouch(),
  	Middle   = task.goCmuRush(MIDDLE_POS, _, 600, flag.allow_dss),
  	match    = "{AML}" 
  },

  ["stop"] = {
  	switch = function()
	       return "stop"
    end,
    Assister = task.stop(),
    Leader   = task.stop(),
    Special  = task.stop(),
    Middle   = task.stop(),
    Defender = task.stop(),
    Goalie   = task.stop(),
    match    = "[ALDSM]"
  },

  name = "Ref4_CornerKickV1701",
  applicable = {
    exp = "a",
    a   = true
  },
  attribute = "attack",
  timeout = 99999
 }

