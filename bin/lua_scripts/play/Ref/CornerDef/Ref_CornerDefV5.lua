--角球防守，增加norPass的状态，保证norPass的时候盯防first的优先级最高
--和一比，匹配优先级不一样
local COR_DEF_POS1 = CGeoPoint:new_local(-70,-50)
local COR_DEF_POS2 = CGeoPoint:new_local(-70,50)
local COR_DEF_POS3 = ball.refAntiYPos(CGeoPoint:new_local(-70,-20))

gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	if gCurrentState == "beginning" and 
		enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
		return "attacker"..enemy.attackNum()
	elseif gCurrentState == "norPass" then
		if bufcnt(ball.velMod() < gNorPass2NorDefBallVel 
			or not enemy.hasReceiver(), 2) then
			return "norDef"
		end
	elseif gCurrentState == "norDef" then
		if bufcnt(cond.canDefenceExit(), 2,60) then
			return "finish"
		end
	else
		if bufcnt(cond.canDefenceExit(), 2) then
			return "finish"
		elseif enemy.situChanged() and 
			enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
			return "attacker"..enemy.attackNum()
		elseif bufcnt(cond.isGameOn(), 5) then
			return "norPass"
		end
	end
end,

["beginning"] = {
	Leader   = task.defendKick(),
	Special  = task.goPassPos("Leader"),
	Middle   = task.goSpeciPos(COR_DEF_POS1, player.toBallDir),
	Defender = task.goSpeciPos(COR_DEF_POS2, player.toBallDir),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L][DASM]"
},

["attacker1"] = {
	Leader   = task.defendKick(),
	Special  = task.goPassPos("Leader"),
	Middle   = task.goSpeciPos(COR_DEF_POS1, player.toBallDir),
	Defender = task.goSpeciPos(COR_DEF_POS2, player.toBallDir),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L][DASM]"
},

["attacker2"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.goSpeciPos(COR_DEF_POS1, player.toBallDir),
	Defender = task.goSpeciPos(COR_DEF_POS2, player.toBallDir),
	Assister = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "[L][S][A][DM]"
},

["attacker3"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.goSpeciPos(COR_DEF_POS3, player.toBallDir),
	Assister = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "[L][S][A][MD]"
},

["attacker4"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Assister = task.goSpeciPos(COR_DEF_POS3, player.toBallDir),
	Goalie   = task.goalie(),
	match    = "[L][S][MD][A]"
},

["attacker5"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Assister = task.marking("Fourth"),
	Goalie   = task.goalie(),
	match    = "[L][S][MDA]"
},

["attacker6"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Assister = task.marking("Fourth"),
	Goalie   = task.goalie(),
	match    = "[L][S][MDA]"
},

["norPass"] = {
	Leader   = task.advance(),
	Special  = task.markingDir("First",player.toBallDir),
	Middle   = task.markingDir("Second",player.toBallDir),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[S][L][DA][M]"
},

["norDef"] = {
	Leader   = task.advance(),
	Special  = task.markingDir("First",player.toBallDir),
	Middle   = task.markingDir("Second",player.toBallDir),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][DA][M]"
},

name = "Ref_CornerDefV5",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}