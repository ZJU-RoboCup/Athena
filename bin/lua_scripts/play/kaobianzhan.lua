local dir = 1
local TargetPos1  = CGeoPoint:new_local(-60,-280*dir)
local TargetPos2  = CGeoPoint:new_local(-30,-280*dir)
local TargetPos3  = CGeoPoint:new_local(0,-280*dir)
local TargetPos4  = CGeoPoint:new_local(30,-280*dir)
local TargetPos5  = CGeoPoint:new_local(60,-280*dir)
local TargetPosG  = CGeoPoint:new_local(0,-250*dir)
gPlayTable.CreatePlay{
firstState = "run",
["run"] = {
	switch = function ()
		return "run"
	end,
	Leader   = task.goCmuRush(TargetPos1,0,100),
	Special  = task.goCmuRush(TargetPos2,0,100),
	Middle   = task.goCmuRush(TargetPos3,0,100),
	Defender = task.goCmuRush(TargetPos4,0,100),
	Assister = task.goCmuRush(TargetPos5,0,100),
	Goalie   = task.goCmuRush(TargetPosG,0,100),
	match = "{LSMDA}"
},




name = "kaobianzhan",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

