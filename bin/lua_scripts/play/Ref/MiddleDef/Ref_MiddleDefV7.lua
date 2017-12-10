--immortal 靠近中线附近middledef防守脚本，修改区域
local LEFTUP_POS = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(ball.refPosX()+100, 210*ball.refAntiY())
	else
		return CGeoPoint:new_local(ball.refPosX()+100, 100*ball.refAntiY())
	end
end

local RIGHTDOWN_POS = function()
	if ball.refAntiY() == -1 then
		return CGeoPoint:new_local(ball.refPosX()-100, 100*ball.refAntiY())
	else
		return CGeoPoint:new_local(ball.refPosX()-100, 210*ball.refAntiY())
	end
end

local leftUp
local rightDown

local left = CGeoPoint:new_local(-param.pitchLength/2,-param.goalWidth/2-10)
local right = CGeoPoint:new_local(-param.pitchLength/2,param.goalWidth/2+10)

local KICKDEFTASK = task.defendKick(100,left,right,2)
gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	if gCurrentState == "beginning" and
		enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
		leftUp = LEFTUP_POS()
		rightDown = RIGHTDOWN_POS()
		defenceInfo:setNoMarkingField(LEFTUP_POS(),RIGHTDOWN_POS())
		return "attacker"..enemy.attackNum()
	elseif gCurrentState == "attackDef" then
		print(leftUp:x(),rightDown:x())
		--print(cond.needExitAttackDef(leftUp,rightDown,"vertical"),cond.canExitMiddleDef())
		if bufcnt((cond.needExitAttackDef(leftUp,rightDown,"vertical") and not cond.dist4ball2MarkTouch(leftUp,rightDown)) or cond.canExitMiddleDef(), 2) then
			defenceInfo:clearNoMarkingField(LEFTUP_POS(),RIGHTDOWN_POS())
			return "norDef"
		elseif bufcnt((cond.needExitAttackDef(leftUp,rightDown,"vertical") and cond.dist4ball2MarkTouch(leftUp,rightDown)) or cond.canExitMiddleDef(), 2) then
			defenceInfo:clearNoMarkingField(LEFTUP_POS(),RIGHTDOWN_POS())
			return "norPass"
		elseif  bufcnt(cond.canDefenceExit(), 2, 100) then
			return "finish"
		end
	elseif gCurrentState == "norPass" then
		if  bufcnt(cond.canDefenceExit(), 2, 100) then
			return "finish"
		end
	elseif gCurrentState == "norDef" then
		if  bufcnt(cond.canDefenceExit(), 2, 100) then
			return "finish"
		end
	else
		if bufcnt(cond.canDefenceExit(), 2) then
			return "finish"
		elseif bufcnt(cond.isGameOn(), 2) then
			return "attackDef"
		elseif enemy.situChanged() and
			enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
			return "attacker"..enemy.attackNum()
		end
	end
end,

["beginning"] = {
	Leader   = KICKDEFTASK,
	Special  = task.goPassPos("Leader"),
	Middle   = task.markingTouch(0,LEFTUP_POS,RIGHTDOWN_POS,"vertical"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][M][DAS]"
},

["attacker1"] = {
	Leader   = KICKDEFTASK,
	Special  = task.goPassPos("Leader"),
	Middle   = task.markingTouch(0,LEFTUP_POS,RIGHTDOWN_POS,"vertical"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L]{M}[DAS]"
},

["attacker2"] = {
	Leader   = KICKDEFTASK,
	Special  = task.marking("First"),
	Middle   = task.markingTouch(0,LEFTUP_POS,RIGHTDOWN_POS,"vertical"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L]{M}[S][DA]"
},

["attacker3"] = {
	Leader   = KICKDEFTASK,
	Special  = task.marking("First"),
	Assister   = task.marking("Second"),
	Middle = task.markingTouch(0,LEFTUP_POS,RIGHTDOWN_POS,"vertical"),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L]{M}[S][AD]"
},

["attacker4"] = {
	Leader   = KICKDEFTASK,
	Special  = task.marking("First"),
	Assister   = task.marking("Second"),
	Defender = task.marking("Third"),
	Middle = task.markingTouch(0,LEFTUP_POS,RIGHTDOWN_POS,"vertical"),
	Goalie   = task.goalie(),
	match    = "[L]{M}[S][AD]"
},

["attacker5"] = {
	Leader   = KICKDEFTASK,
	Special  = task.marking("First"),
	Assister   = task.marking("Second"),
	Defender = task.marking("Third"),
	Middle = task.markingTouch(0,LEFTUP_POS,RIGHTDOWN_POS,"vertical"),
	Goalie   = task.goalie(),
	match    = "[L]{M}[S][AD]"
},

["attacker6"] = {
	Leader   = KICKDEFTASK,
	Special  = task.marking("First"),
	Assister   = task.marking("Second"),
	Defender = task.marking("Third"),
	Middle = task.markingTouch(0,LEFTUP_POS,RIGHTDOWN_POS,"vertical"),
	Goalie   = task.goalie(),
	match    = "[L]{M}[S][AD]"
},

["attackDef"] = {
	Leader   = task.goPassPos("Middle"),
	Special  = task.continue(),
	Middle   = task.continue(),
	Defender = task.continue(),
	Assister = task.continue(),
	Goalie   = task.goalie(),
	match    = "{LDAMS}"
},

["norPass"] = {
	Leader   = task.advance(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{L}[S][DA][M]"
},

["norDef"] = {
	Leader   = task.advance(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][DA][M]"
},

name = "Ref_MiddleDefV7",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}