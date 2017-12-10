--暂停脚本，我方暂停与敌方暂停都会调用，左右需要side调整
--gty 16.6.15
local side = -1
local TargetPosG  = CGeoPoint:new_local(-400,-250*side)
local TargetPos1  = CGeoPoint:new_local(-370,-250*side)
local TargetPos2  = CGeoPoint:new_local(-340,-250*side)
local TargetPos3  = CGeoPoint:new_local(-310,-250*side)
local TargetPos4  = CGeoPoint:new_local(-280,-250*side)
local TargetPos5  = CGeoPoint:new_local(-250,-250*side)
gPlayTable.CreatePlay{

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
		if bufcnt(true,150) then
			return "halt"
		end
	end,
	Leader   = task.goCmuRush(TargetPos1,side*math.pi/2,150,flag.allow_dss),
	Special  = task.goCmuRush(TargetPos2,side*math.pi/2,150,flag.allow_dss),
	Middle   = task.goCmuRush(TargetPos3,side*math.pi/2,150,flag.allow_dss),
	Defender = task.goCmuRush(TargetPos4,side*math.pi/2,150,flag.allow_dss),
	Assister = task.goCmuRush(TargetPos5,side*math.pi/2,150,flag.allow_dss),
	Goalie   = task.goCmuRush(TargetPosG,side*math.pi/2,150,flag.allow_dss),
	match = "{LSMDA}"
},


["halt"] = {
	switch = function()
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

name = "OurTimeout",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
