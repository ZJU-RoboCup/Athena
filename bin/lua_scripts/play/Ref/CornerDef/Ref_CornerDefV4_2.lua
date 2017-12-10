--角球防守，增加norPass的状态，保证norPass的时候盯防first的优先级最高
--单后卫
local COR_DEF_POS1 = CGeoPoint:new_local(-70,-50)
local COR_DEF_POS2 = CGeoPoint:new_local(-70,50)
local COR_DEF_POS3 = CGeoPoint:new_local(-70,0)

gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	if gCurrentState == "beginning" and 
		enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
		return "attacker"..enemy.attackNum()
	elseif gCurrentState == "marking"  or gCurrentState == "markingHead" then
		if bufcnt(not DefendUtils.isPosInOurPenaltyArea(ball.pos()) 
			and ball.velMod() < 150 
			and math.abs(ball.posX()) < param.pitchLength/2,5,100) then
			return "norPass"
		end
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
			if math.abs(ball.velDir()) > math.pi/4 then
				return "markingHead"
			else
				return "marking"
			end
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
	Assister = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "[L][S][A][MD]"
},

["attacker5"] = {
	Leader   = task.marking("First"),
	Special  = task.marking("Second"),
	Middle   = task.marking("Third"),
	Defender = task.marking("Fourth"),
	Assister = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "[L][SMDA]"
},

["attacker6"] = {
	Leader   = task.marking("First"),
	Special  = task.marking("Second"),
	Middle   = task.marking("Third"),
	Defender = task.marking("Fourth"),
	Assister = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "[L][SMDA]"
},

["marking"] = {
	Leader   = task.marking("First"),
	Special  = task.marking("Second"),
	Middle   = task.marking("Third"),
	Defender = task.marking("Fourth"),
	Assister = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "[L][S][A][MD]"
},

["markingHead"] = {
	Leader   = task.marking("First"),
	Special  = task.marking("Second"),
	Middle   = task.marking("Third"),
	Defender = task.marking("Fourth"),
	Assister = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "{A}[L][S][MD]"
},

["norPass"] = {
	Leader   = task.advance(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[S][L][A][MD]"
},

["norDef"] = {
	Leader   = task.advance(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][A][MD]"
},

name = "Ref_CornerDefV4_2",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}