--后场无盯人车，两个横向markingTouch，两个后卫，一个防开球
--添加checkImmortal状态，让车直接上去抢球
--防开球区域现设置较小，待修改
local LEFTSIDE_POS1= function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(-100,-param.pitchWidth/2+10)
	else
		return CGeoPoint:new_local(-120,-param.pitchWidth/2+10)
	end
end

local LEFTSIDE_POS2 = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(-200,0)
	else 
		return CGeoPoint:new_local(-220,0)
	end
end

local RIGHTSIDE_POS1 = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(-120,0)
	else 
		return CGeoPoint:new_local(-100,0)
	end
end

local RIGHTSIDE_POS2 = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(-220, param.pitchWidth/2)
	else 
		return CGeoPoint:new_local(-200, param.pitchWidth/2)
	end
end

local FRONT_POS = function()
	return CGeoPoint:new_local(200, -170*ball.refAntiY())
end

local left = CGeoPoint:new_local(-param.pitchLength/2,-param.goalWidth/2-10)
local right = CGeoPoint:new_local(-param.pitchLength/2,param.goalWidth/2+10)

local KICKDEFTASK = task.defendKick(150,left,right,2)

gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	if gCurrentState == "beginning" and 
		enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
		defenceInfo:setNoMarkingField(LEFTSIDE_POS1(),LEFTSIDE_POS2())
		defenceInfo:setNoMarkingField(RIGHTSIDE_POS1(),RIGHTSIDE_POS2())
		return "attacker"
	elseif gCurrentState == "checkImmortal" then
		if bufcnt(cond.canBeImmortalShoot(),10) then
			return "finish"
		elseif bufcnt(cond.needExitAttackDef(LEFTSIDE_POS1(),LEFTSIDE_POS2() ,"horizal") and 
			not cond.needExitAttackDef(RIGHTSIDE_POS1(),RIGHTSIDE_POS2() ,"horizal"), 2) 
			or (cond.checkBallPassed(RIGHTSIDE_POS1(),RIGHTSIDE_POS2()) and 
		    not cond.checkBallPassed(LEFTSIDE_POS1(),LEFTSIDE_POS2())) then
			defenceInfo:clearNoMarkingField(LEFTSIDE_POS1(),LEFTSIDE_POS2())
			return "rightAttackDef"
		elseif  bufcnt(cond.needExitAttackDef(RIGHTSIDE_POS1(),RIGHTSIDE_POS2(),"horizal") and
			not cond.needExitAttackDef(LEFTSIDE_POS1(),LEFTSIDE_POS2() ,"horizal"), 2) 
			or (cond.checkBallPassed(LEFTSIDE_POS1(),LEFTSIDE_POS2()) and 
		    not cond.checkBallPassed(RIGHTSIDE_POS1(),RIGHTSIDE_POS2())) then
			defenceInfo:clearNoMarkingField(RIGHTSIDE_POS1().RIGHTSIDE_POS2())
			return "leftAttackDef"
		elseif  bufcnt(cond.canExitMRLFrontDef() or (cond.needExitAttackDef(RIGHTSIDE_POS1(),RIGHTSIDE_POS2(),"horizal") and
			cond.needExitAttackDef(LEFTSIDE_POS1(),LEFTSIDE_POS2() ,"horizal")) or cond.canDefenceExit(), 2, 100) then
			return "finish"
		end
	elseif gCurrentState == "rightAttackDef" then
		print(cond.canExitMRLFrontDef(),cond.needExitAttackDef(RIGHTSIDE_POS1(),RIGHTSIDE_POS2(),"horizal"),cond.canDefenceExit(),cond.canBeImmortalShoot())
		if bufcnt(cond.canExitMRLFrontDef() or cond.needExitAttackDef(RIGHTSIDE_POS1(),RIGHTSIDE_POS2(),"horizal") or cond.canDefenceExit(), 2, 100) then
			return "finish"
		elseif bufcnt(cond.canBeImmortalShoot(),8) then
			return "finish" 
		end
	elseif gCurrentState == "leftAttackDef" then
		if bufcnt(cond.canExitMRLFrontDef() or cond.needExitAttackDef(LEFTSIDE_POS1(),LEFTSIDE_POS2() or cond.canDefenceExit(),"horizal"), 2, 100) then
			return "finish"
		elseif bufcnt(cond.canBeImmortalShoot(),8) then
			return "finish" 
		end
	else
		--print("ball still situation",enemy.situChanged() )
		if bufcnt(cond.canDefenceExit(), 2) then
			return "finish"
		elseif bufcnt(cond.isGameOn(), 2) then
			return "checkImmortal"
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
	Special  = task.markingTouch(1,RIGHTSIDE_POS1,RIGHTSIDE_POS2,"horizal"),
	Middle   = task.markingTouch(0,LEFTSIDE_POS1,LEFTSIDE_POS2,"horizal"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L](MSDA)"
},


["checkImmortal"] = {
	Leader   = task.stop(),
	Special  = task.markingTouch(1,RIGHTSIDE_POS1,RIGHTSIDE_POS2,"horizal"),
	Middle   = task.markingTouch(0,LEFTSIDE_POS1,LEFTSIDE_POS2,"horizal"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{MS}[L][DA]"
},


["leftAttackDef"] = {
	Leader = task.stop(),
	Special = task.defendMiddle("Special"),
	Middle = task.markingTouch(0,LEFTSIDE_POS1,LEFTSIDE_POS2,"horizal"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{M}[DA][L][S]"
},

["rightAttackDef"] = {
	Leader = task.stop(),
	Special = task.markingTouch(1,RIGHTSIDE_POS1,RIGHTSIDE_POS2,"horizal"),
	Middle = task.defendMiddle("Middle"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{S}[DA][L][M]"
},

name = "Ref_FrontDefV8",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",




timeout   = 99999

}