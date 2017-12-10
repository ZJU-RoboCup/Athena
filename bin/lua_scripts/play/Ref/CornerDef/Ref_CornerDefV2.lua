--角球防守，防守KIKS预案
--有bug
--没用
local COR_DEF_POS1 = CGeoPoint:new_local(-70,-50)
local COR_DEF_POS2 = CGeoPoint:new_local(-70,50)
local COR_DEF_POS3 = CGeoPoint:new_local(-70,0)

gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	if gCurrentState == "beginning" and 
		enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
		return "attacker"..enemy.attackNum()
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
		elseif cond.isGameOn() then
			return "norDef"
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
	Special  = task.blocking("First"),
	Middle   = task.goSpeciPos(COR_DEF_POS1, player.toBallDir),
	Defender = task.goSpeciPos(COR_DEF_POS2, player.toBallDir),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][A][MD]"
},

["attacker3"] = {
	Leader   = task.defendKick(),
	Special  = task.blocking("First"),
	Middle   = task.blocking("Second"),
	Defender = task.goSpeciPos(COR_DEF_POS3, player.toBallDir),
	Assister = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "[L][S][A][MD]"
},

["attacker4"] = {
	Leader   = task.defendKick(),
	Special  = task.blocking("First"),
	Middle   = task.blocking("Second"),
	Defender = task.blocking("Third"),
	Assister = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "[L][A][SMD]"
},

["attacker5"] = {
	Leader   = task.defendKick(),
	Special  = task.blocking("First"),
	Middle   = task.blocking("Second"),
	Defender = task.blocking("Third"),
	Assister = task.blocking("Fourth"),
	Goalie   = task.goalie(),
	match    = "[L][ASMD]"
},

["attacker6"] = {
	Leader   = task.defendKick(),
	Special  = task.blocking("First"),
	Middle   = task.blocking("Second"),
	Defender = task.blocking("Third"),
	Assister = task.blocking("Fourth"),
	Goalie   = task.goalie(),
	match    = "[L][ASMD]"
},

["norDef"] = {
	Leader   = task.blocking("Zero"),
	Special  = task.blocking("First"),
	Middle   = task.blocking("Second"),
	Defender = task.blocking("Third"),
	Assister = task.blocking("Fourth"),
	Goalie   = task.goalie(),
	match    = "[LSMDA]"
},

name = "Ref_CornerDefV2",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}