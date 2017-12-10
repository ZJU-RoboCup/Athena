--根据卡位车判断是否两车防开球
local LEFTPOS1= function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(0,-180)
	else
		return CGeoPoint:new_local(-param.pitchLength/2,-param.goalWidth/2-10)
	end
end

local RIGHTPOS1 = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(-param.pitchLength/2,param.goalWidth/2+10)
	else 
		return CGeoPoint:new_local(0,180)
	end
end

local LEFTPOS2 = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(param.pitchLength/2,param.goalWidth/2)
	else 
		return CGeoPoint:new_local(0,180)
	end
end

local RIGHTPOS2 = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(0,-180)
	else 
		return CGeoPoint:new_local(param.pitchLength/2,-param.goalWidth/2)
	end
end

local KICKDEFTASK = task.defendOneKick(60,LEFTPOS2,RIGHTPOS2,60,LEFTPOS1,RIGHTPOS1)
local KICKDEFTASK2 = task.defendTwoKick(60,LEFTPOS2,RIGHTPOS2,60,LEFTPOS1,RIGHTPOS1)

-- local KICKDEFTASK = task.defendKick(60,LEFTPOS1,RIGHTPOS1,2)
-- local KICKDEFTASK2 = task.defendKick(80,LEFTPOS2,RIGHTPOS2,2)


gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	local Num = cond.kickOffEnemyNum()
	--print("kickOffNum is ",Num)
	if gCurrentState == "beginning" and
		enemy.attackNum() <= 6 and enemy.attackNum() > 0  then
		return "attacker"..enemy.attackNum()
	elseif gCurrentState == "norPass" then
		if bufcnt(ball.velMod() < gNorPass2NorDefBallVel
			or not enemy.hasReceiver(), 2) then
			return "norDef"
		end
	elseif gCurrentState == "norDef" then
		if bufcnt(cond.canDefenceExit(), 2,100) then
			return "finish"
		end
	else
		if bufcnt(cond.canDefenceExit(), 2) then
			return "finish"
		elseif enemy.situChanged() and
			enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
			return "attacker"..enemy.attackNum()
		elseif bufcnt(cond.isGameOn(), 5) then
			defenceInfo:resetMarkingInfo()
			return "norPass"
		end
	end
end,

["beginning"] = {
	Leader   = KICKDEFTASK,
	Special  = KICKDEFTASK2,
	Middle   = task.defendDefault(2),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[LS][DAM]"
},

["attacker1"] = {
	Leader   = KICKDEFTASK,
	Special  = KICKDEFTASK2,
	Middle   = task.defendDefault(2),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[LS][DAM]"
},

["attacker2"] = {
	Leader   = KICKDEFTASK,
	Special  = KICKDEFTASK2,
	Middle   = task.marking("First"),
	Defender = task.defendDefault(2),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[LS][M][DA]"
},

["attacker3"] = {
	Leader   = KICKDEFTASK,
	Special  = KICKDEFTASK2,
	Middle   = task.marking("First"),
	Defender = task.marking("Second"),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[LS][M][A][D]"
},

["attacker4"] = {
	Leader   = KICKDEFTASK,
	Special  = KICKDEFTASK2,
	Middle   = task.marking("First"),
	Defender = task.marking("Second"),
	Assister = task.marking("Third"),
	Goalie   = task.goalie(),
	match    = "[LS][M][AD]"
},

["attacker5"] = {
	Leader   = KICKDEFTASK,
	Special  = KICKDEFTASK2,
	Middle   = task.marking("First"),
	Defender = task.marking("Second"),
	Assister = task.marking("Third"),
	Goalie   = task.goalie(),
	match    = "[LS][M][AD]"
},

["attacker6"] = {
	Leader   = KICKDEFTASK,
	Special  = KICKDEFTASK2,
	Middle   = task.marking("First"),
	Defender = task.marking("Second"),
	Assister = task.marking("Third"),
	Goalie   = task.goalie(),
	match    = "[LS][M][AD]"
},

["norPass"] = {
	Leader   = task.advance(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[S][L][A][DM]"
},

["norDef"] = {
	Leader   = task.advance(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][A][DM]"
},

name = "Ref_BackDefV7",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}