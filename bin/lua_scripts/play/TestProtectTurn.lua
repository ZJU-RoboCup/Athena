
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
			return "slowGetBall"
		end
	end,
	Leader =task.staticGetBall(guisePos),
	match = "[L]"
},

["slowGetBall"] = {
	switch = function()
		initDir=player.dir("Leader")
		if  bufcnt(true,150) then
			return "turn"
		end
	end,
	Leader =task.slowGetBall(3.14),
	match = "[L]"
},

["turn"]={
	switch=function ()
		if  bufcnt(player.kickBall("Assister"),2,600) then
			return "prepare"
		end
	end,
	Leader =task.protectTurn(),
	match ="[L]"
},

name = "TestProtectTurn",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}