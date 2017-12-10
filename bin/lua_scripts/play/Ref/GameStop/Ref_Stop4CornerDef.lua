local STOP_POS1 = function()
	if ball.toOurGoalDist() < 160*param.lengthRatio then
		return CGeoPoint:new_local(-150*param.lengthRatio,0)
	else
		return ball.pos() + Utils.Polar2Vector(59, ball.toOurGoalDir())
	end
end

local COR_DEF_POS1 = CGeoPoint:new_local(-50*param.lengthRatio,-50*param.widthRatio)

gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()	
	if gCurrentState == "beginning" and 
		enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
		return "attacker"..enemy.attackNum()
	else
		if cond.isGameOn() then
			return "finish"
		elseif enemy.situChanged() and
			enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
			return "attacker"..enemy.attackNum()
		end
	end
end,

["beginning"] = {
	Kicker   = task.defendKick(),
	Special  = task.goPassPos("Kicker"),
	Middle   = task.goCmuRush(COR_DEF_POS1, player.toBallDir, _, flag.slowly),
	Defender = task.leftBack(),
	Leader   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[DLSM]"
},

["attacker1"] = {
	Kicker   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.slowly),
	Special  = task.goPassPos("Kicker"),
	Middle   = task.goCmuRush(COR_DEF_POS1, player.toBallDir, _, flag.slowly),
	Defender = task.leftBack(),
	Leader   = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[DLSM]"
},

["attacker2"] = {
	Kicker   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.slowly),
	Special  = task.marking("First"),
	Middle   = task.goCmuRush(COR_DEF_POS1, player.toBallDir, _, flag.slowly),
	Defender = task.defendHead(),
	Leader   = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[DLSM]"
},

["attacker3"] = {
	Kicker   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.slowly),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.goCmuRush(COR_DEF_POS1, player.toBallDir, _, flag.slowly),
	Leader   = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "[LSMD]"
},

["attacker4"] = {
	Kicker   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.slowly),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Leader   = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "[LSMD]"
},

["attacker5"] = {
	Kicker   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.slowly),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Leader   = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "[LSMD]"
},

["attacker6"] = {
	Kicker   = task.goCmuRush(STOP_POS1, player.toBallDir, _, flag.slowly),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Leader   = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "[LSMD]"
},

name = "Ref_Stop4CornerDef",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}