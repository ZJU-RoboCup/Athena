local PassPos  = {
	CGeoPoint:new_local(150,-150),
	CGeoPoint:new_local(150,150),
	CGeoPoint:new_local(-150,150),
	CGeoPoint:new_local(-150,-150)
}

-- local PassPos  = {
-- 	CGeoPoint:new_local(150,-150),
-- 	CGeoPoint:new_local(150,150),
-- 	CGeoPoint:new_local(-150,0)
-- }

local testNum=4

local couter=1
local function getPassPos()
	return PassPos[couter+1]
end

local function getNextPassPos()
	return PassPos[(couter+1)%testNum+1]
end

local function getLastPassPos()
	return PassPos[(couter+2)%testNum+1]
end

gPlayTable.CreatePlay{

firstState = "prepare",

["prepare"] = {
	switch = function ()
		if bufcnt(player.toPointDist("Leader",PassPos[1])<30 and ball.toPlayerDist("Leader")<30,60) then
			return "pass"
		end
	end,
	Leader   = task.goCmuRush(PassPos[1],player.toPointDir(CGeoPoint:new_local(0,0))),
	Assister = task.goCmuRush(PassPos[2],player.toPointDir(CGeoPoint:new_local(0,0))),
	Special  = task.goCmuRush(PassPos[3],player.toPointDir(CGeoPoint:new_local(0,0))),
	Middle   = task.goCmuRush(PassPos[4],player.toPointDir(CGeoPoint:new_local(0,0))),
	match = "[LASM]"
},

["pass"] = {
	switch = function ()
		print(couter)
		if bufcnt(player.isBallPassed("Leader","Assister"), "fast") then
			gRoleNum["Leader"]=gRoleNum["Assister"]
			couter=(couter+1)%testNum
			return "pass"
		end
	end,
	Leader   = task.receivePass("Assister",500),
	Assister = task.goCmuRush(getPassPos,player.toPlayerDir("Leader")),
	Special  = task.goCmuRush(getNextPassPos,player.toPlayerDir("Assister")),
	Middle	 = task.goCmuRush(getLastPassPos,player.toPlayerDir("Middle")),
	match = "{L}[ASM]"
},


name = "TestReceivePass",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

