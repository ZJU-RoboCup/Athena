-- 修改为强行传球 by zhyaic 2014.07.21
local TMP_POS = ball.refSyntYPos(CGeoPoint:new_local(170, 100))
local SYNT_CORNER_POS = ball.refSyntYPos(CGeoPoint:new_local(370, 240))
local ANTI_CORNER_POS = ball.refAntiYPos(CGeoPoint:new_local(370, 240))
local SIDE_POS = ball.refSyntYPos(CGeoPoint:new_local(-300, 290))
local ASS_SIDE_POS = ball.refSyntYPos(CGeoPoint:new_local(-350, 290))
local LEA_SIDE_POS = ball.refSyntYPos(CGeoPoint:new_local(-100, 290))
local BACK_DIR = function(d)
	return function()
		return ball.antiY()*d
	end
end

local DSS_FLAG = bit:_or(flag.allow_dss, flag.dodge_ball)

local function magicPos()
	return function ()
		-- changed to Brazil by zhyaic
		return CGeoPoint:new_local(ball.refPosX() - 150, ball.refPosY() + 120 * ball.refAntiY())
	end
end
gPlayTable.CreatePlay{

firstState = "tmpState",

["tmpState"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Special") < 20, 20, 180) then
			return "oneReady"
		end
	end,
	Assister = task.goBackBall(CGeoPoint:new_local(0, 300), 30),
	Leader   = task.goBackBall(0, 50),
	Special  = task.goCmuRush(magicPos(), _, 600, DSS_FLAG),
	Middle   = task.goCmuRush(TMP_POS, _, 600, DSS_FLAG),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{A}{LSDM}"
},

["oneReady"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Assister") < 5, "normal", 180) then
			return "twoReady"
		end
	end,
	Assister = task.goBackBall(CGeoPoint:new_local(0, 300), 30),
	Leader   = task.goBackBall(0, 50),
	Special  = task.goCmuRush(magicPos(), _, 600, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, 600, DSS_FLAG),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{A}{LSDM}"
},

["twoReady"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Leader") < 5, "normal", 180) then
			return "chooseKicker"
		end
	end,
	Assister = task.goBackBall(CGeoPoint:new_local(0, 300), 30),
	Leader   = task.goBackBall(CGeoPoint:new_local(450, -100), 20),
	Special  = task.goCmuRush(magicPos(), _, 600, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, 600, DSS_FLAG),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALSDM}"
},

["chooseKicker"] = {
	switch = function()
		-- 由于CMU三车盯人，因此坚定地跑掉一辆车
		if  bufcnt(true, 60) then
			return "continueGo"
		end
	end,
	Assister = task.goBackBall(CGeoPoint:new_local(0, 300), 30),
	Leader   = task.goBackBall(CGeoPoint:new_local(450, -100), 20),
	Special  = task.goCmuRush(ball.goRush(), _, 600, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, 600, DSS_FLAG),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ALSDM}"
},

["continueGo"] = {
	switch = function()
		if  bufcnt(player.toTargetDist("Defender") < 50, 1, 180) then			
			if pos.isChip("Defender",true) == true then
				return "continuePass"--"chip"
			else
				return "continuePass"
			end
		end
	end,
	Assister = task.stop(),
	Leader   = task.goBackBall(CGeoPoint:new_local(450, -100), 20),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, 600, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, 600, DSS_FLAG),
	Defender = task.goCmuRush(magicPos(), _, 600, DSS_FLAG),
	Goalie   = task.goalie(),
	match    = "{ALSDM}"
},

["chip"] = {
	switch = function()
		if  bufcnt(true, 60) then
			return "continueChipPass"
		end
	end,
	Assister = task.slowGetBall(magicPos(),false,false),
	Leader   = task.goBackBall(CGeoPoint:new_local(450, -100), 20),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, 600, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, 600, DSS_FLAG),
	Defender = task.goCmuRush(magicPos(), _, 600, DSS_FLAG),
	Goalie   = task.goalie(),
	match    = "{ALSDM}"
},

["continuePass"] = {
	switch = function()
		if  player.kickBall("Assister") or 
			player.isBallPassed("Assister","Defender") then
			return "continueShoot"
		elseif  bufcnt(true, 90) then
			return "exit"
		end
	end,
	Assister = task.passToPos(magicPos(),270),--task.passToPos(pos.passForTouch(ball.goRush())),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, 650, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, 650, DSS_FLAG),
	Defender = task.goCmuRush(magicPos()),
	Leader   = task.goCmuRush(SIDE_POS),
	Goalie   = task.goalie(),
	match    = "{ASDLM}"
},

["continueChipPass"] = {
	switch = function()
		if  player.kickBall("Assister") or 
			player.isBallPassed("Assister","Defender") then
			return "fix"
		elseif  bufcnt(true, 90) then
			return "exit"
		end
	end,
	Assister = task.chipPass(pos.passForTouch(magicPos()),100,false),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, 650, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, 650, DSS_FLAG),
	Defender = task.goCmuRush(magicPos()),
	Leader   = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ASDLM}"
},

["fix"] = {
	switch = function()
		if bufcnt(true,30) then
			return "waitForShoot"
		end
	end,
	Assister = task.goCmuRush(ASS_SIDE_POS, _, 650, DSS_FLAG),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, 650, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, 650, DSS_FLAG),
	Defender = task.goCmuRush(magicPos()),
	Leader   = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{ASDLM}"
},

["waitForShoot"] = {
	switch = function()
		if  bufcnt(true,40) then
			return "continueShoot"
		end
	end,
	Assister = task.goCmuRush(ASS_SIDE_POS, _, _, DSS_FLAG),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, _, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, _, DSS_FLAG),
	Defender = task.InterTouch(magicPos()),
	Leader   = task.goCmuRush(LEA_SIDE_POS, _, _, DSS_FLAG),
	Goalie   = task.goalie(),
	match    = "{ASDLM}"
},

["continueShoot"] = {
	switch = function()
		if  bufcnt(true, 80) then
			print (magicPos)
			return "shoot"
		end
	end,
	Defender = task.InterTouch(),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, _, DSS_FLAG),
	Middle   = task.goCmuRush(SYNT_CORNER_POS, _, _, DSS_FLAG),
	Assister = task.goCmuRush(ASS_SIDE_POS, _, _, DSS_FLAG),
	Leader   = task.goCmuRush(LEA_SIDE_POS, _, _, DSS_FLAG),
	Goalie   = task.goalie(),
	match    = "{SD}[LAM]"
},
["shoot"] = {
	switch = function()
		if  bufcnt(player.kickBall("Defender"), 1, 120) then
			return "exit"
		end
	end,
	Defender = task.InterTouch(),
	Special  = task.goCmuRush(ANTI_CORNER_POS, _, 600, DSS_FLAG),
	Middle   = task.defendMiddle(),
	Assister = task.leftBack(),
	Leader   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{SD}[LAM]"
},
name = "Ref_FrontKickV1704",
applicable = {
	exp = "a",
	a = true
},
attribute = "attack",
timeout   = 99999
}
