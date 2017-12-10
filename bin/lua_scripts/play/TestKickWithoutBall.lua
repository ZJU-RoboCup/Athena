

local startPos1 = CGeoPoint:new_local(-200,-180)
local finalPos1 = CGeoPoint:new_local(-200,180)

local startPos2 = CGeoPoint:new_local(-120,-180)
local finalPos2 = CGeoPoint:new_local(-120,180)

local startPos3 = CGeoPoint:new_local(-40,-180)
local finalPos3 = CGeoPoint:new_local(-40,180)

local startPos4 = CGeoPoint:new_local(40,-180)
local finalPos4 = CGeoPoint:new_local(40,180)

local startPos5 = CGeoPoint:new_local(120,-180)
local finalPos5 = CGeoPoint:new_local(120,180)

local startPos6 = CGeoPoint:new_local(200,-180)
local finalPos6 = CGeoPoint:new_local(200,180)

gPlayTable.CreatePlay{

firstState = "testState1",
["testState1"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Leader")<20,60) then
			return "testState2"
		end
	end,
	Leader   = task.goTestEmptyKick(startPos1,dir.specified(90),550),
	Middle   = task.goTestEmptyKick(startPos2,dir.specified(90),550),
	Special  = task.goTestEmptyKick(startPos3,dir.specified(90),550),
	Defender = task.goTestEmptyKick(startPos4,dir.specified(90),550),
	Assister = task.goTestEmptyKick(startPos5,dir.specified(90),550),
	Fronter  = task.goTestEmptyKick(startPos6,dir.specified(90),550),
	match    = "(LMSDAF)"
},

["testState2"]={
	switch = function()
		if bufcnt(player.toTargetDist("Leader")<20,60) then
			return "testState1"
		end
	end,
	Leader   = task.goTestEmptyKick(finalPos1,dir.specified(90),550),
	Middle   = task.goTestEmptyKick(finalPos2,dir.specified(90),550),
	Special  = task.goTestEmptyKick(finalPos3,dir.specified(90),550),
	Defender = task.goTestEmptyKick(finalPos4,dir.specified(90),550),
	Assister = task.goTestEmptyKick(finalPos5,dir.specified(90),550),
	Fronter  = task.goTestEmptyKick(finalPos6,dir.specified(90),550),
	match    = "{LMSDAF}"
},

name = "TestKickWithoutBall",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}