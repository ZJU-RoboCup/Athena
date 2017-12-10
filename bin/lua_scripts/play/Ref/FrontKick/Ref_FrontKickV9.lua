

local guisePos=CGeoPoint:new_local(0,100)

function  preparePos()
	local x = ball.posX()-20
	local y = ball.posY()
	return CGeoPoint:new_local(x,y)
end

local preparePos1 = ball.refAntiYPos(CGeoPoint:new_local(120,-45))
local preparePos2 = ball.refAntiYPos(CGeoPoint:new_local(50,-15))
local preparePos3 = ball.refAntiYPos(CGeoPoint:new_local(50,15))
local preparePos4 = ball.refAntiYPos(CGeoPoint:new_local(120,45))

local MUTI_POS_1 = {
	ball.refAntiYPos(CGeoPoint:new_local(120,-45)),
	ball.refAntiYPos(CGeoPoint:new_local(110,-60)),
	ball.refAntiYPos(CGeoPoint:new_local(130,-30)),
	ball.refAntiYPos(CGeoPoint:new_local(115,-50)),
	ball.refAntiYPos(CGeoPoint:new_local(120,-45)),
	ball.refAntiYPos(CGeoPoint:new_local(110,-60)),
	ball.refAntiYPos(CGeoPoint:new_local(130,-30)),
	ball.refAntiYPos(CGeoPoint:new_local(115,-50)),
	ball.refAntiYPos(CGeoPoint:new_local(120,-45)),
	ball.refAntiYPos(CGeoPoint:new_local(110,-60)),
	ball.refAntiYPos(CGeoPoint:new_local(130,-30)),
	ball.refAntiYPos(CGeoPoint:new_local(115,-50)),
	ball.refAntiYPos(CGeoPoint:new_local(120,-45)),
	ball.refAntiYPos(CGeoPoint:new_local(110,-60)),
	ball.refAntiYPos(CGeoPoint:new_local(130,-30)),
	ball.refAntiYPos(CGeoPoint:new_local(115,-50)),
	ball.refAntiYPos(CGeoPoint:new_local(120,-45)),
	ball.refAntiYPos(CGeoPoint:new_local(110,-60)),
	ball.refAntiYPos(CGeoPoint:new_local(130,-30)),
	ball.refAntiYPos(CGeoPoint:new_local(115,-50))
}

local MUTI_POS_2 = {
	ball.refAntiYPos(CGeoPoint:new_local(50,-15)),
	ball.refAntiYPos(CGeoPoint:new_local(60,-30)),
	ball.refAntiYPos(CGeoPoint:new_local(70,-20)),
	ball.refAntiYPos(CGeoPoint:new_local(40,-10)),
	ball.refAntiYPos(CGeoPoint:new_local(50,-15)),
	ball.refAntiYPos(CGeoPoint:new_local(60,-30)),
	ball.refAntiYPos(CGeoPoint:new_local(70,-20)),
	ball.refAntiYPos(CGeoPoint:new_local(40,-10)),
	ball.refAntiYPos(CGeoPoint:new_local(50,-15)),
	ball.refAntiYPos(CGeoPoint:new_local(60,-30)),
	ball.refAntiYPos(CGeoPoint:new_local(70,-20)),
	ball.refAntiYPos(CGeoPoint:new_local(40,-10)),
	ball.refAntiYPos(CGeoPoint:new_local(50,-15)),
	ball.refAntiYPos(CGeoPoint:new_local(60,-30)),
	ball.refAntiYPos(CGeoPoint:new_local(70,-20)),
	ball.refAntiYPos(CGeoPoint:new_local(40,-10)),
	ball.refAntiYPos(CGeoPoint:new_local(50,-15)),
	ball.refAntiYPos(CGeoPoint:new_local(60,-30)),
	ball.refAntiYPos(CGeoPoint:new_local(70,-20)),
	ball.refAntiYPos(CGeoPoint:new_local(40,-10))
}

local MUTI_POS_3 = {
	ball.refAntiYPos(CGeoPoint:new_local(50,15)),
	ball.refAntiYPos(CGeoPoint:new_local(60,30)),
	ball.refAntiYPos(CGeoPoint:new_local(70,20)),
	ball.refAntiYPos(CGeoPoint:new_local(40,10)),
	ball.refAntiYPos(CGeoPoint:new_local(50,15)),
	ball.refAntiYPos(CGeoPoint:new_local(60,30)),
	ball.refAntiYPos(CGeoPoint:new_local(70,20)),
	ball.refAntiYPos(CGeoPoint:new_local(40,10)),
	ball.refAntiYPos(CGeoPoint:new_local(50,15)),
	ball.refAntiYPos(CGeoPoint:new_local(60,30)),
	ball.refAntiYPos(CGeoPoint:new_local(70,20)),
	ball.refAntiYPos(CGeoPoint:new_local(40,10)),
	ball.refAntiYPos(CGeoPoint:new_local(50,15)),
	ball.refAntiYPos(CGeoPoint:new_local(60,30)),
	ball.refAntiYPos(CGeoPoint:new_local(70,20)),
	ball.refAntiYPos(CGeoPoint:new_local(40,10)),
	ball.refAntiYPos(CGeoPoint:new_local(50,15)),
	ball.refAntiYPos(CGeoPoint:new_local(60,30)),
	ball.refAntiYPos(CGeoPoint:new_local(70,20)),
	ball.refAntiYPos(CGeoPoint:new_local(40,10))
}

local MUTI_POS_4 = {
	ball.refAntiYPos(CGeoPoint:new_local(120,45)),
	ball.refAntiYPos(CGeoPoint:new_local(110,60)),
	ball.refAntiYPos(CGeoPoint:new_local(130,30)),
	ball.refAntiYPos(CGeoPoint:new_local(115,50)),
	ball.refAntiYPos(CGeoPoint:new_local(120,45)),
	ball.refAntiYPos(CGeoPoint:new_local(110,60)),
	ball.refAntiYPos(CGeoPoint:new_local(130,30)),
	ball.refAntiYPos(CGeoPoint:new_local(115,50)),
	ball.refAntiYPos(CGeoPoint:new_local(120,45)),
	ball.refAntiYPos(CGeoPoint:new_local(110,60)),
	ball.refAntiYPos(CGeoPoint:new_local(130,30)),
	ball.refAntiYPos(CGeoPoint:new_local(115,50)),
	ball.refAntiYPos(CGeoPoint:new_local(120,45)),
	ball.refAntiYPos(CGeoPoint:new_local(110,60)),
	ball.refAntiYPos(CGeoPoint:new_local(130,30)),
	ball.refAntiYPos(CGeoPoint:new_local(115,50)),
	ball.refAntiYPos(CGeoPoint:new_local(120,45)),
	ball.refAntiYPos(CGeoPoint:new_local(110,60)),
	ball.refAntiYPos(CGeoPoint:new_local(130,30)),
	ball.refAntiYPos(CGeoPoint:new_local(115,50))
}


local runPos1 = ball.refAntiYPos(CGeoPoint:new_local(80,0))
local runPos2 = ball.refAntiYPos(CGeoPoint:new_local(260,100))
local runPos3 = ball.refAntiYPos(CGeoPoint:new_local(-100,80))
local runPos4 = ball.refAntiYPos(CGeoPoint:new_local(-20,180))

local delayTime = (OPPONENT_NAME == "delay") and 400 or 100

gPlayTable.CreatePlay{

firstState = "prepare",

["prepare"]={
	switch = function()
		if  bufcnt(player.toTargetDist("Kicker") < 15, delayTime) then
			return "run"
		end
	end,
	Kicker   = task.staticGetBall(guisePos),
	Special  = task.runMultiPos(MUTI_POS_1, _, 5),
	Middle   = task.runMultiPos(MUTI_POS_2, _, 5),
	Defender = task.runMultiPos(MUTI_POS_3, _, 5),
	Assister = task.runMultiPos(MUTI_POS_4, _, 5),
	Goalie   = task.goalie(),
	match = "[DASM]"
},

["run"] = {
	switch = function()
		if  bufcnt(true,80) then
			return "TurnKick"
		end
	end,
	Kicker   =task.slowGetBall(dir.dirForDribbleTurnKick),
	Special  = task.goCmuRush(runPos1),
	Middle   = task.goCmuRush(runPos2),
	Defender = task.goCmuRush(runPos3),
	Assister = task.goCmuRush(runPos4),
	Goalie   = task.goalie(),
	match = "[DASM]"
},

["TurnKick"]={
	switch=function ()
		if  bufcnt(player.kickBall("Kicker"), "fast", 20) then
			return "finish"
		end
	end,
	Kicker   = task.dribbleTurnShoot(),
	Special  = task.goCmuRush(runPos1),
	Middle   = task.goCmuRush(runPos2),
    Assister = task.rightBack(),
    Defender = task.leftBack(),
    Goalie   = task.goalie(),
	match ="[DASM]"
},

name = "Ref_FrontKickV9",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}