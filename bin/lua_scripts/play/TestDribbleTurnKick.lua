
local guisePos=CGeoPoint:new_local(0,100)

local initDir
function getInitDir()
	return initDir
end

function  preparePos()
	local x = ball.posX()-20
	local y = ball.posY()
	return CGeoPoint:new_local(x,y)
end

gPlayTable.CreatePlay{

firstState = "prepare",

["prepare"]={
	switch = function()
		if  bufcnt(player.toTargetDist("Leader")<10,30) then
			return "testState1"
		end
	end,
	Leader =task.staticGetBall(guisePos),
	match = "[L]"
},

["testState1"] = {
	switch = function()
		initDir=player.dir("Leader")
		print("hello",initDir-ball.toTheirGoalDir())
		if  bufcnt(true,150) then
			return "testState2"
		end
	end,
	Leader =task.slowGetBall(3.14),
	match = "[L]"
},

["testState2"]={
	switch=function ()
		if  bufcnt(true,90) then
			return "testState1"
		end
	end,
	Leader =task.dribbleTurnShoot(),
	match ="[L]"
},

name = "TestDribbleTurnKick",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}