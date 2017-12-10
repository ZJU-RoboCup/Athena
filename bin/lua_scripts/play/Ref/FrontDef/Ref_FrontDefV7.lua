--后场无盯人车，两个后场横向markingTouch，两个后卫，一个防开球
--针对MRL直接挑前场
local BorderUpRight = -150
local BorderDownRight = -300
local BorderUpLeft = -100
local BorderDownLeft = -300
local LEFTSIDE_POS1= function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(BorderUpLeft,-param.pitchWidth/2+10)
	else
		return CGeoPoint:new_local(BorderUpRight,-param.pitchWidth/2+10)
	end
end

local LEFTSIDE_POS2 = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(BorderDownLeft,0)
	else 
		return CGeoPoint:new_local(BorderDownRight,0)
	end
end

local RIGHTSIDE_POS1 = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(BorderUpRight, 0)
	else 
		return CGeoPoint:new_local(BorderUpLeft,0)
	end
end

local RIGHTSIDE_POS2 = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(BorderDownRight, param.pitchWidth/2-10)
	else 
		return CGeoPoint:new_local(BorderDownLeft, param.pitchWidth/2-10)
	end
end

local FRONT_POS = function()
	return CGeoPoint:new_local(200, -170*ball.refAntiY())
end

local left = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(0,-param.pitchWidth/2)
	else
		return CGeoPoint:new_local(-param.pitchLength/2,-param.pitchWidth/2+10)
	end
end

local right = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(-param.pitchLength/2,param.pitchWidth/2-10)
	else
		return CGeoPoint:new_local(0,param.pitchWidth/2)
	end
end

local KICKDEFTASK = task.defendKick(150,left,right,2)

gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	--print(gCurrentState,cond.canExitMRLFrontDef(),cond.needExitAttackDef(LEFTSIDE_POS1(),LEFTSIDE_POS2(),"horizal"))

	if gCurrentState == "beginning" and 
		enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
		defenceInfo:setNoMarkingField(LEFTSIDE_POS1(),LEFTSIDE_POS2())
		defenceInfo:setNoMarkingField(RIGHTSIDE_POS1(),RIGHTSIDE_POS2())
		return "attacker"
	elseif gCurrentState == "attackDef" then
		--print(cond.needExitAttackDef(LEFTSIDE_POS1(),LEFTSIDE_POS2() ,"horizal") ,cond.needExitAttackDef(RIGHTSIDE_POS1(),RIGHTSIDE_POS2() ,"horizal"))
		-- print(cond.checkBallPassed(RIGHTSIDE_POS1(),RIGHTSIDE_POS2()),cond.checkBallPassed(LEFTSIDE_POS1(),LEFTSIDE_POS2()),(cond.checkBallPassed(LEFTSIDE_POS1(),LEFTSIDE_POS2()) and 
		--     not cond.checkBallPassed(RIGHTSIDE_POS1(),RIGHTSIDE_POS2())))
		if bufcnt(cond.needExitAttackDef(LEFTSIDE_POS1(),LEFTSIDE_POS2() ,"horizal") and 
			not cond.needExitAttackDef(RIGHTSIDE_POS1(),RIGHTSIDE_POS2() ,"horizal"), 2)
			or (cond.checkBallPassed(RIGHTSIDE_POS1(),RIGHTSIDE_POS2()) and 
		    not cond.checkBallPassed(LEFTSIDE_POS1(),LEFTSIDE_POS2())) then
			defenceInfo:clearNoMarkingField(LEFTSIDE_POS1(),LEFTSIDE_POS2())
			return "rightAttackDef"
		elseif  bufcnt(cond.needExitAttackDef(RIGHTSIDE_POS1(),RIGHTSIDE_POS2(),"horizal") and
			not cond.needExitAttackDef(LEFTSIDE_POS1(),LEFTSIDE_POS2() ,"horizal"), 2) 
			or (cond.checkBallPassed(LEFTSIDE_POS1(),LEFTSIDE_POS2()) and 
		    not cond.checkBallPassed(RIGHTSIDE_POS1(),RIGHTSIDE_POS2())) then
			defenceInfo:clearNoMarkingField(RIGHTSIDE_POS1(),RIGHTSIDE_POS2())
			return "leftAttackDef"
		elseif  bufcnt( (cond.canExitMRLFrontDef() and ball.valid()) or (cond.needExitAttackDef(RIGHTSIDE_POS1(),RIGHTSIDE_POS2(),"horizal") and
			cond.needExitAttackDef(LEFTSIDE_POS1(),LEFTSIDE_POS2() ,"horizal")) or cond.canDefenceExit(), 8, 100) then
			return "finish"
		elseif  cond.checkBallPassed(LEFTSIDE_POS1(),LEFTSIDE_POS2()) and cond.checkBallPassed(RIGHTSIDE_POS1(),RIGHTSIDE_POS2()) then
			local checkNum = cond.getValidMarkingTouchArea(LEFTSIDE_POS1(),LEFTSIDE_POS2(),RIGHTSIDE_POS1(),RIGHTSIDE_POS2())
			if checkNum == 1 then
				defenceInfo:clearNoMarkingField(RIGHTSIDE_POS1(),RIGHTSIDE_POS2())
				return "leftAttackDef"
			elseif  checkNum == 2 then
				defenceInfo:clearNoMarkingField(LEFTSIDE_POS1(),LEFTSIDE_POS2())
				return "rightAttackDef"
			end
		elseif bufcnt(cond.canBeImmortalShoot(),10) then
			return "finish"
		end
	elseif gCurrentState == "rightAttackDef" then
		if bufcnt((cond.canExitMRLFrontDef() and ball.valid())or cond.needExitAttackDef(RIGHTSIDE_POS1(),RIGHTSIDE_POS2(),"horizal") or cond.canDefenceExit(), 8, 100) then
			return "finish"
		elseif bufcnt(cond.canBeImmortalShoot(),8) then
			return "finish"
		end
	elseif gCurrentState == "leftAttackDef" then
		--print((cond.canExitMRLFrontDef() and ball.valid()),cond.needExitAttackDef(LEFTSIDE_POS1(),LEFTSIDE_POS2(),"horizal"))
		if bufcnt((cond.canExitMRLFrontDef() and ball.valid()) or cond.needExitAttackDef(LEFTSIDE_POS1(),LEFTSIDE_POS2(),"horizal") or cond.canDefenceExit(), 8, 100) then
			return "finish"
		elseif bufcnt(cond.canBeImmortalShoot(),15) then
			return "finish"
		end
	else
		--print("ball still situation",enemy.situChanged() )
		if bufcnt(cond.canDefenceExit(), 2) then
			return "finish"
		elseif bufcnt(cond.isGameOn(), 2) then
			return "attackDef"
		end
	end
end,

["beginning"] = {
	Leader   = KICKDEFTASK,
	Special  = task.markingTouch(1,RIGHTSIDE_POS1,RIGHTSIDE_POS2,"horizal"),
	Middle   = task.markingTouch(0,LEFTSIDE_POS1,LEFTSIDE_POS2,"horizal"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L](MSDA)"
},

["attacker"] = {
	Leader   = KICKDEFTASK,
	Special  = task.continue(),
	Middle   = task.continue(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L](MSDA)"
},

["attackDef"] = {
	Leader   = task.stop(),
	Special  = task.continue(),
	Middle   = task.continue(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{MS}[L][DA]"
},

["leftAttackDef"] = {
	Leader = task.goSpeciPos(FRONT_POS,player.toBallDir),
	Special = task.defendMiddle("Special"),
	Middle = task.continue(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{M}[L][DA][S]"
},

["rightAttackDef"] = {
	Leader = task.goSpeciPos(FRONT_POS,player.toBallDir),
	Special = task.continue(),
	Middle = task.defendMiddle("Middle"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{S}[L][DA][M]"
},

name = "Ref_FrontDefV7",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",




timeout   = 99999

}