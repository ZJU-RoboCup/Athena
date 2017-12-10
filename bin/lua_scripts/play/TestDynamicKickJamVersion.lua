local PassPos = pos.getShootPos()
local FakePos = pos.getAssistPos()
local JamPos  = pos.getJamPos()
local RunPos1,RunPos2,RunPos3,RunPos4 = pos.getRunPos()
local GoaliePos = pos.getGoaliePos()
local theirGoal = CGeoPoint:new_local(450, 0)
local GoalieRushPos = CGeoPoint:new_local(0, 0)
local JamMode = false
local DEBUG = true
local WaitPos = {
	{
		ball.refAntiYPos(CGeoPoint:new_local(275, -200)),
		ball.refAntiYPos(CGeoPoint:new_local(210, -25)),
		ball.refAntiYPos(CGeoPoint:new_local(225, 145)),
		ball.refAntiYPos(CGeoPoint:new_local(330, 240))
	},
	{
		ball.refAntiYPos(CGeoPoint:new_local(300, 0)),
		ball.refAntiYPos(CGeoPoint:new_local(250, 0)),
		ball.refAntiYPos(CGeoPoint:new_local(200, 0)),
		ball.refAntiYPos(CGeoPoint:new_local(150, 0))
	},
	{
		ball.refAntiYPos(CGeoPoint:new_local(225, -60)),
		ball.refAntiYPos(CGeoPoint:new_local(225, -30)),
		ball.refAntiYPos(CGeoPoint:new_local(225, 30)),
		ball.refAntiYPos(CGeoPoint:new_local(225, 60))
	}
}
local randomNum
local StayPos = WaitPos[1]

local CheckPos = function()
	local dir = (CGeoPoint:new_local(0, -300 * ball.antiY()) - ball.pos()):dir()
	return ball.pos() + Utils.Polar2Vector(200, dir)
end

local function randomPos(i)
	return function()
		return StayPos[i]()
	end
end
local acc = 300

gPlayTable.CreatePlay{
firstState = "randomStayPos",
["randomStayPos"] = {
	switch = function()
		randomNum = 1--math.random(3)
		StayPos = WaitPos[randomNum]
		return "move"
	end,
	Assister = task.staticGetBall(),
	Leader   = task.stop(),
	Special  = task.stop(),
	Middle   = task.stop(),
	Defender = task.stop(),
	Goalie   = task.goalie(),
	match    = "{AL}{SMD}"
},

["move"] = {
	switch = function()
		--if bufcnt(player.toTargetDist("Leader") < 20 and player.toTargetDist("Special") < 20,30 ,180) then
			return "detectDefendkick"
		--end
	end,
	Assister = task.staticGetBall(theirGoal,false),
	Leader   = task.goCmuRush(randomPos(1), _, _, flag.allow_dss),
	Special  = task.goCmuRush(randomPos(2), _, _, flag.allow_dss),
	Middle   = task.goCmuRush(randomPos(3), _, _, flag.allow_dss),
	Defender = task.goCmuRush(randomPos(4), _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}[LSMD]"
},

["detectDefendkick"] = {
	switch = function()
		pos.sendVehicleNumber(gRoleNum["Leader"], gRoleNum["Special"], gRoleNum["Middle"], gRoleNum["Defender"], gRoleNum["Assister"], gRoleNum["Goalie"])
		pos.detectDefendKicked(3)
		if cond.isGameOn() then
		if DEBUG then print("Illegal EXIT of game on") end
			return "exit"
		elseif bufcnt(pos.detectDefendKickedOver()==true,1) then
			if pos.isChip() then
				JamMode = true
				return "blockTheirDefendKicker"
			else
				return "prepareToDetect"
			end
		end
	end,
	Assister = task.staticGetBall(FakePos,false),
	Leader   = task.goCmuRush(randomPos(1), _, _, flag.allow_dss),
	Special  = task.goCmuRush(randomPos(2), _, _, flag.allow_dss),
	Middle   = task.goCmuRush(randomPos(3), _, _, flag.allow_dss),
	Defender = task.goCmuRush(randomPos(4), _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{LSMD}"
},	

["blockTheirDefendKicker"] = {
	switch = function()
		if cond.isGameOn() then
			if DEBUG then print("Illegal EXIT of game on") end
			return "exit"
		elseif bufcnt(player.toTargetDist("Special") < 20 and player.toTargetDist("Goalie")<20,20 ,180) then
			return "prepareToDetect"
		end
	end,
	Assister = task.staticGetBall(theirGoal,false),
	Leader   = task.goCmuRush(randomPos(1), _, _, flag.allow_dss),
	Special  = task.goCmuRush(JamPos),
	Middle   = task.goCmuRush(randomPos(3), _, _, flag.allow_dss),
	Defender = task.goCmuRush(randomPos(4), _, _, flag.allow_dss),
	Goalie   = task.goCmuRush(GoalieRushPos, _, _, flag.allow_dss),
	match    = "{A}[LSMD]"
},	

["moveGoalie"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Goalie")<20,20 ,180) then
			return "prepareToDetect"
		end
	end,
	Assister = task.staticGetBall(theirGoal,false),
	Leader   = task.goCmuRush(randomPos(1), _, _, flag.allow_dss),
	Special  = task.goCmuRush(randomPos(2), _, _, flag.allow_dss),
	Middle   = task.goCmuRush(randomPos(3), _, _, flag.allow_dss),
	Defender = task.goCmuRush(randomPos(4), _, _, flag.allow_dss),
	Goalie   = task.goCmuRush(GoalieRushPos, _, _, flag.allow_dss),
	match    = "{A}[LSMD]"
},	

["prepareToDetect"] = {
	switch = function()
		pos.sendVehicleNumber(gRoleNum["Leader"], gRoleNum["Special"], gRoleNum["Middle"], gRoleNum["Defender"], gRoleNum["Goalie"], gRoleNum["Assister"])
		pos.detect(_,_,50,true)
		return "detect"
	end,
	Assister = task.staticGetBall(theirGoal,false),
	Leader   = task.goCmuRush(randomPos(1), _, _, flag.allow_dss),
	Special  = task.continue(),
	Middle   = task.goCmuRush(randomPos(3), _, _, flag.allow_dss),
	Defender = task.goCmuRush(randomPos(4), _, _, flag.allow_dss),
	Goalie   = task.goCmuRush(GoaliePos, _, _, flag.allow_dss),
	match    = "{A}{S}[LMD]"
},

["detect"] = {
	switch = function()
		pos.sendVehicleNumber(gRoleNum["Leader"], gRoleNum["Special"], gRoleNum["Middle"], gRoleNum["Defender"], gRoleNum["Goalie"], gRoleNum["Assister"])
		pos.detect()
		if cond.isGameOn() then
			if DEBUG then print("Illegal EXIT of game on") end
			return "exit"
		elseif pos.detectMarkedOver()==true and pos.detectDefendKickedOver()==true then
			return "scan"
		elseif pos.detectAgain()==true then
			if JamMode then
				return "detectAgainWithJam"
			else
				return "detectAgain"
			end
		end
	end,
	Assister = task.staticGetBall(theirGoal,false),
	Leader   = task.continue(),
	Special  = task.continue(),
	Middle   = task.continue(),
	Defender = task.continue(),
	Goalie   = task.continue(),
	match    = "{A}{S}[LMD]"
},	

["detectAgainWithJam"] = {
	switch = function()
		print('detectagain')
		pos.detect(true,_,80,true)
		return "detect"
	end,
	Assister = task.staticGetBall(theirGoal,false),
	Leader   = task.goCmuRush(RunPos1,player.toBallDir,acc),
	Special  = task.goCmuRush(JamPos),
	Middle   = task.goCmuRush(RunPos2,player.toBallDir,acc),
	Defender = task.goCmuRush(RunPos3,player.toBallDir,acc),
	Goalie   = task.goCmuRush(GoaliePos,player.toBallDir,acc),
	match    = "{A}{S}[LMD]"
},

["detectAgain"] = {
	switch = function()
		print('detectagain')
		pos.detect(true,_,80,true)
		return "detect"
	end,
	Assister = task.staticGetBall(theirGoal,false),
	Leader   = task.goCmuRush(RunPos1,player.toBallDir,acc),
	Special  = task.goCmuRush(RunPos2,player.toBallDir,acc),
	Middle   = task.goCmuRush(RunPos3,player.toBallDir,acc),
	Defender = task.goCmuRush(RunPos4,player.toBallDir,acc),
	Goalie   = task.goCmuRush(GoaliePos,player.toBallDir,acc),
	match    = "{A}[LSMD]"
},

["scan"] = {
	switch = function()
		pos.scan()
		if cond.isGameOn() then
			return "exit"
		elseif pos.scanOver()==true then
			return "wait"
		end
	end,
	Assister = task.staticGetBall(theirGoal,false),
	Leader   = task.goCmuRush(RunPos1,player.toBallDir,acc),
	Special  = task.continue(),
	Middle   = task.goCmuRush(RunPos3,player.toBallDir,acc),
	Defender = task.goCmuRush(RunPos4,player.toBallDir,acc),
	Goalie   = task.goCmuRush(GoaliePos,player.toBallDir,acc),
	match    = "{A}{S}{LMD}"
},

["wait"] = {
	switch = function()
		if bufcnt(true,2) then
			return "confirm"
		end
	end,
	Assister = task.staticGetBall(theirGoal,false),
	Leader   = task.goCmuRush(RunPos1,player.toBallDir,acc),
	Special  = task.continue(),
	Middle   = task.goCmuRush(RunPos3,player.toBallDir,acc),
	Defender = task.goCmuRush(RunPos4,player.toBallDir,acc),
	Goalie   = task.goCmuRush(GoaliePos,player.toBallDir,acc),
	match    = "{A}{S}{LMD}"
},

["confirm"] = {
	switch = function()
		if bufcnt(true,2) then
			if pos.isChip()==true then
				return"dribble"
			else
				return"fake"
			end
		end
	end,
	Assister = task.staticGetBall(theirGoal,false),
	Leader   = task.goCmuRush(RunPos1,player.toBallDir,acc),
	Special  = task.continue(),
	Middle   = task.goCmuRush(RunPos3,player.toBallDir,acc),
	Defender = task.goCmuRush(RunPos4,player.toBallDir,acc),
	Goalie   = task.goCmuRush(GoaliePos,player.toBallDir,acc),
	match    = "{A}[L][SMD]"
},

["dribble"] = {
	switch = function()
		if bufcnt(true,80) then
			return"fake2"
		end
	end,
	Assister = task.slowGetBall(PassPos,false,false),
	Leader   = task.goCmuRush(RunPos1,player.toBallDir,acc),
	Special  = task.goCmuRush(RunPos2,player.toBallDir,acc),
	Middle   = task.goCmuRush(RunPos3,player.toBallDir,acc),
	Defender = task.goCmuRush(RunPos4,player.toBallDir,acc),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

["fake"] = {
	switch = function()
	    pos.sendVehicleNumber(gRoleNum["Leader"], gRoleNum["Special"], gRoleNum["Middle"], gRoleNum["Defender"], gRoleNum["Assister"], gRoleNum["Goalie"])
		pos.fakePosCompute()
		if pos.canPass()==true then
			return"pass"
		end
	end,
	Assister = task.staticGetBall(theirGoal,false),
	Leader   = task.goCmuRush(PassPos),
	Special  = task.continue(),
	Middle   = task.goCmuRush(RunPos3, _, _, flag.allow_dss),
	Defender = task.goCmuRush(RunPos4, _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

["fake2"] = {
	switch = function()
		pos.sendVehicleNumber(gRoleNum["Leader"], gRoleNum["Special"], gRoleNum["Middle"], gRoleNum["Defender"], gRoleNum["Assister"], gRoleNum["Goalie"])
		pos.fakePosCompute()
		if pos.canPass()==true then
			return"chippass"
		end
	end,
	Assister = task.slowGetBall(PassPos,false,false),
	Leader   = task.goCmuRush(PassPos),
	Special  = task.goCmuRush(RunPos2, _, _, flag.allow_dss),
	Middle   = task.goCmuRush(RunPos3, _, _, flag.allow_dss),
	Defender = task.goCmuRush(RunPos4, _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},


["chippass"] = {
	switch = function()
		if bufcnt(player.kickBall("Assister") ,1,100) then
			return"fix"
		end
	end,
	Assister = task.chipPass(PassPos, pos.getChipPower(),false,false),
	Leader   = task.goCmuRush(PassPos),
	Special  = task.goCmuRush(RunPos2, _, _, flag.allow_dss),
	Middle   = task.goCmuRush(RunPos3, _, _, flag.allow_dss),
	Defender = task.goCmuRush(RunPos4, _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

["fix"] = {
	switch = function()
		local buf = pos.getFixBuf() 
		if bufcnt(true,buf) then
			return"shoot"
		end
	end,
	Assister = task.stop(),
	Leader   = task.goCmuRush(PassPos),
	Special  = task.goCmuRush(RunPos2, _, _, flag.allow_dss),
	Middle   = task.goCmuRush(RunPos3, _, _, flag.allow_dss),
	Defender = task.goCmuRush(RunPos4, _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{L}[SMD]"
},

["pass"] = {
	switch = function()
		if bufcnt(player.kickBall("Assister") ,1,100) then
			return"shoot"
		end
	end,
	Assister = task.goAndTurnKick(pos.passForTouch(PassPos)),
	Leader   = task.goCmuRush(PassPos),
	Special  = task.continue(),
	Middle   = task.goCmuRush(RunPos3, _, _, flag.allow_dss),
	Defender = task.goCmuRush(RunPos4, _, _, flag.allow_dss),
	Goalie   = task.goalie(),
	match    = "{A}{S}{L}[MD]"
},

["shoot"] = {
	switch = function()
		if bufcnt(player.kickBall("Leader") ,1,120) then
			return"exit"
		end
	end,
	Assister = task.goSupportPos("Leader"),
	Leader   = task.waitTouchNew(),
	Special  = task.leftBack(),
	Middle   = task.defendMiddle(),
	Defender = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{L}[ASMD]"
},

name = "TestDynamicKickJamVersion",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}