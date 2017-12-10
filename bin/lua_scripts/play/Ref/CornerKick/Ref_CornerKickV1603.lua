--模仿2015 CMU vs MRL final 第一个进球 --by hzy 3/28/2016

local USE_FAKE_DIR = true
local TSHOOT_POS = CGeoPoint:new_local(445,-135)
local TFACE_POS1 = CGeoPoint:new_local(450, 300)
local TFACE_POS2 = CGeoPoint:new_local(-450, 0)
local FACE_POS1 = USE_FAKE_DIR and ball.refSyntYPos(TFACE_POS1) or ball.refSyntYPos(CGeoPoint:new_local(0,0))
local FACE_POS2 = USE_FAKE_DIR and ball.refSyntYPos(TFACE_POS2) or ball.refSyntYPos(CGeoPoint:new_local(0,0))
local DIR_POS = ball.refSyntYPos(CGeoPoint:new_local(450, -100))
local DIR_POS2 = ball.refSyntYPos(CGeoPoint:new_local(450, 100))
local SHOOT_POS = ball.refSyntYPos(TSHOOT_POS)

local SOME_POS1 = {
	ball.refSyntYPos(CGeoPoint:new_local(430,130)),
	ball.refSyntYPos(CGeoPoint:new_local(430,170)),
	ball.refSyntYPos(CGeoPoint:new_local(330,70)),
	ball.refSyntYPos(CGeoPoint:new_local(220,0)),
	ball.refSyntYPos(CGeoPoint:new_local(260,-170))
}

local SOME_POS2 = {
	ball.refSyntYPos(CGeoPoint:new_local(300,230)),
	ball.refSyntYPos(CGeoPoint:new_local(420,220)),
	ball.refSyntYPos(CGeoPoint:new_local(420,130)),
	ball.refSyntYPos(CGeoPoint:new_local(370,90))
}
local SOME_POS3 = {
	ball.refSyntYPos(CGeoPoint:new_local(240,-130)),
	ball.refSyntYPos(CGeoPoint:new_local(300,-40)),
	SHOOT_POS
}

local USE_CHIP = false

local debug = false

gPlayTable.CreatePlay{

firstState = "start",

["start"] = {
	switch = function ()
		if debug then print("start") end
		if bufcnt(true, 60) then
			return "continue"
		end
	end,
	Assister = task.staticGetBall(FACE_POS1),
	Leader   = task.runMultiPos(SOME_POS3,false,30),
	Special  = task.runMultiPos(SOME_POS1,false,30),
	Defender = task.runMultiPos(SOME_POS2,false,30),
	match    = "{A}[LSD]"
},
["continue"] = {
	switch = function ()
		if debug then print("continue") end
		if bufcnt(true, 60) then
			return "shoot"
		end
	end,
	Assister = USE_CHIP and task.slowGetBall(FACE_POS2,false) or task.staticGetBall(FACE_POS2),
	Leader   = task.continue(),
	Special  = task.continue(),
	Defender = task.continue(),
	match    = "{ALSD}"
},
["shoot"] = {
	switch = function()
		if debug then print("Assister shoot") end
		if player.kickBall("Assister") then
			return "deadshoot"
		end
	end,
	Assister = USE_CHIP and task.chipPass("Leader",300) or task.waitTouchNew(_,DIR_POS),
	Leader   = task.goCmuRush(SHOOT_POS),
	Special  = task.continue(),
	Defender = task.continue(),
	match    = "{ALSD}"
},
["deadshoot"] = {
	switch = function()
		if debug then print("deadshoot") end
		if bufcnt(true,40) then
			return "exit"
		end
	end,
	Assister = task.goSupportPos("Leader"),
	Leader   = task.waitTouch(SHOOT_POS),
	Special  = task.rightBack(),
	Defender = task.leftBack(),
	match    = "{ALSD}"
},


name = "Ref_CornerKickV1603",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

