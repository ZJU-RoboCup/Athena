--有防开球车的一车markingTouch
local LEFT_UP= function()
	return CGeoPoint:new_local(-20,-200)
end

local RIGHT_DOWN = function()
	return CGeoPoint:new_local(-150,200)
end

local FRONT_POS = function()
	return CGeoPoint:new_local(180, 150*ball.antiY())
end

gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	if gCurrentState == "beginning" and 
		enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
		defenceInfo:setNoMarkingField(LEFT_UP(),RIGHT_DOWN())
		return "attacker"..enemy.attackNum()
	elseif gCurrentState == "attackDef" then
		if bufcnt(cond.needExitAttackDef(LEFT_UP(),RIGHT_DOWN(),"horizal"), 2) then
			return "norDef"
		elseif  bufcnt(cond.canDefenceExit(), 2, 100) then
			return "finish"
		end
	elseif gCurrentState == "norDef" then
		if  bufcnt(cond.canDefenceExit(), 2, 100) then
			return "finish"
		end
	else
		if bufcnt(cond.canDefenceExit(), 2) then
			return "finish"
		elseif enemy.situChanged() and
			enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
			return "attacker"..enemy.attackNum()
		elseif bufcnt(cond.isGameOn(), 2) then
			return "attackDef"
		end
	end
end,

["beginning"] = {
	Leader   = task.defendKick(),
	Special  = task.goPassPos("Leader"),
	Middle   = task.markingTouch(0,LEFT_UP,RIGHT_DOWN,"horizal"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][M][DAS]"
},

["attacker1"] = {
	Leader   = task.defendKick(),
	Special  = task.goPassPos("Leader"),
	Middle   = task.markingTouch(0,LEFT_UP,RIGHT_DOWN,"horizal"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L]{M}[DAS]"
},

["attacker2"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.markingTouch(0,LEFT_UP,RIGHT_DOWN,"horizal"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L]{M}[S][DA]"
},

["attacker3"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Assister   = task.marking("Second"),
	Middle = task.markingTouch(0,LEFT_UP,RIGHT_DOWN,"horizal"),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L]{M}[S][DA]"
},

["attacker4"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Assister   = task.marking("Second"),
	Middle = task.markingTouch(0,LEFT_UP,RIGHT_DOWN,"horizal"),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L]{M}[S][DA]"
},

["attacker5"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Assister   = task.marking("Second"),
	Middle = task.markingTouch(0,LEFT_UP,RIGHT_DOWN,"horizal"),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L]{M}[S][DA]"
},

["attacker6"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Assister   = task.marking("Second"),
	Middle = task.markingTouch(0,LEFT_UP,RIGHT_DOWN,"horizal"),
	Defender = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L]{M}[S][DA]"
},

["attackDef"] = {
	Leader   = task.goSpeciPos(FRONT_POS,player.toBallDir),
	Special  = task.continue(),
	Middle   = task.continue(),
	Defender = task.continue(),
	Assister = task.continue(),
	Goalie   = task.goalie(),
	match    = "{LDAMS}"
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

name = "Ref_FrontDefV5",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",




timeout   = 99999

}