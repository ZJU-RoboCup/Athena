--球大于125，三车盯人，一车singleBack (暂未使用)
local SIDEPOS1 = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(180, 190*ball.refAntiY())
	else
		return CGeoPoint:new_local(180, 100*ball.refAntiY())
	end
end

local SIDEPOS2 = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(0, 100*ball.refAntiY())
	else
		return CGeoPoint:new_local(0, 190*ball.refAntiY())
	end
end

local LEFTDOWNPOS1 = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(-40,-190)
	else
		return CGeoPoint:new_local(-80,-190) 
	end
end

local LEFTDOWNPOS2 = function()
	return CGeoPoint:new_local(-200,0)
end

local RIGHTDOWNPOS1 = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(-80,0)
	else 
		return CGeoPoint:new_local(-40,-0)
	end
end

local RIGHTDOWNPOS2 = function()
	return CGeoPoint:new_local(-200,190)
end

local FRONT_POS = function()
	return CGeoPoint:new_local(180, 150*ball.refAntiY())
end


gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	if gCurrentState == "beginning" and 
		enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
		defenceInfo:setNoMarkingField(SIDEPOS1(),SIDEPOS2())
		defenceInfo:setNoMarkingField(LEFTDOWNPOS1(),LEFTDOWNPOS2())
		defenceInfo:setNoMarkingField(RIGHTDOWNPOS1(),RIGHTDOWNPOS2())
		return "attacker"
	elseif gCurrentState == "attackDef" then
		if bufcnt(cond.needExitAttackDef(SIDEPOS1(),SIDEPOS2() ,"vertical") and 
			not cond.needExitAttackDef(LEFTDOWNPOS1(),LEFTDOWNPOS2() ,"horizal") and
			not cond.needExitAttackDef(RIGHTDOWNPOS1(),RIGHTDOWNPOS2(),"horizal"),2) then
			defenceInfo:clearNoMarkingField(SIDEPOS1(),SIDEPOS2())
			return "horizalDef"
		elseif  bufcnt(cond.canExitMiddleDef() or (cond.needExitAttackDef(LEFTDOWNPOS1(),LEFTDOWNPOS2(),"horizal") and
			cond.needExitAttackDef(RIGHTDOWNPOS1(),RIGHTDOWNPOS2() ,"horizal")), 2, 100) then
			return "finish"
		end
	elseif gCurrentState == "horizalDef" then
		if bufcnt( cond.needExitAttackDef(LEFTDOWNPOS1(),LEFTDOWNPOS2() ,"horizal") and
			not cond.needExitAttackDef(RIGHTDOWNPOS1(),RIGHTDOWNPOS2(),"horizal"),2) then
			defenceInfo:clearNoMarkingField(LEFTDOWNPOS1(),LEFTDOWNPOS2())
			return "rightAttackDef"
		elseif  bufcnt( cond.needExitAttackDef(RIGHTDOWNPOS1(),RIGHTDOWNPOS2() ,"horizal") and
			not cond.needExitAttackDef(LEFTDOWNPOS1(),LEFTDOWNPOS2(),"horizal"),2) then
			defenceInfo:clearNoMarkingField(RIGHTDOWNPOS1(),RIGHTDOWNPOS2())
			return "leftAttackDef"
		elseif  bufcnt(cond.canExitMRLFrontDef() or (cond.needExitAttackDef(RIGHTDOWNPOS1(),RIGHTDOWNPOS2(),"horizal") and
			cond.needExitAttackDef(LEFTDOWNPOS1(),LEFTDOWNPOS2() ,"horizal")), 2, 100) then
			return "finish"
		end
	elseif gCurrentState == "rightAttackDef" then
		if bufcnt(cond.canExitMRLFrontDef() or cond.needExitAttackDef(RIGHTDOWNPOS1(),RIGHTDOWNPOS2(),"horizal"), 2, 100) then
			return "finish"
		end
	elseif gCurrentState == "leftAttackDef" then
		if bufcnt(cond.canExitMRLFrontDef() or cond.needExitAttackDef(LEFTDOWNPOS1(),LEFTDOWNPOS2(),"horizal"), 2, 100) then
			return "finish"
		end
	else
		if bufcnt(cond.canDefenceExit(), 2) then
			return "finish"
		elseif bufcnt(cond.isGameOn(), 2) then
			return "attackDef"
		end
	end
end,

["beginning"] = {
	Leader   = task.defendKick(),
	Special  = task.markingTouch(1,LEFTDOWNPOS1,LEFTDOWNPOS2,"horizal"),
	Middle   = task.markingTouch(0,SIDEPOS1,SIDEPOS2,"vertical"),
	Defender = task.markingTouch(2,RIGHTDOWNPOS1,RIGHTDOWNPOS2,"horizal"),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L](MSDA)"
},

["attacker"] = {
	Leader   = task.defendKick(),
	Special  = task.markingTouch(1,LEFTDOWNPOS1,LEFTDOWNPOS2,"horizal"),
	Middle   = task.markingTouch(0,SIDEPOS1,SIDEPOS2,"vertical"),
	Defender = task.markingTouch(2,RIGHTDOWNPOS1,RIGHTDOWNPOS2,"horizal"),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L](MSDA)"
},

["attackDef"] = {
	Leader   = task.goSpeciPos(FRONT_POS,player.toBallDir),
	Special  = task.markingTouch(1,LEFTDOWNPOS1,LEFTDOWNPOS2,"horizal"),
	Middle   = task.markingTouch(0,SIDEPOS1,SIDEPOS2,"vertical"),
	Defender = task.markingTouch(2,RIGHTDOWNPOS1,RIGHTDOWNPOS2,"horizal"),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "{MSD}[A][L]"
},

["horizalDef"] = {
	Leader = task.goSpeciPos(FRONT_POS,player.toBallDir),
	Special = task.markingTouch(1,RIGHTDOWNPOS1,RIGHTDOWNPOS2,"horizal"),
	Middle = task.stop(),
	Defender = task.markingTouch(2,LEFTDOWNPOS1,LEFTDOWNPOS2,"horizal"),
	Assister = task.singleBack(),
	Goalie = task.goalie(),
	match = "{MSD}[A][L]"
	
},

["leftAttackDef"] = {
	Leader = task.goSpeciPos(FRONT_POS,player.toBallDir),
	Special = task.leftBack(),
	Middle = task.stop(),
	Defender = task.markingTouch(2,LEFTDOWNPOS1,LEFTDOWNPOS2,"horizal"),
	Assister = task.rightBack(),
	Goalie = task.goalie(),
	match    = "{DM}[SA][L]"
},

["rightAttackDef"] = {
	Leader = task.goSpeciPos(FRONT_POS,player.toBallDir),
	Special = task.markingTouch(1,RIGHTDOWNPOS1,RIGHTDOWNPOS2,"horizal"),
	Middle = task.stop(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{SM}[DA][L]"
},

name = "Ref_MiddleDefV6",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}