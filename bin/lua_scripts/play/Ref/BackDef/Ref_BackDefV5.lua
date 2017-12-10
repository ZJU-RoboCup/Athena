--走默认点，四车进攻，单后卫

local kickPos = CGeoPoint:new_local(0,0)

gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	if gCurrentState == "beginning" and
		enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
		return "attacker"..enemy.attackNum()
	elseif gCurrentState == "norPass" then
		if bufcnt(cond.canNorPass2Def() or not enemy.hasReceiver(), gNorPass2NorDefCouter) then
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
			kickPos = CGeoPoint:new_local(ball.posX(),ball.posY())
			return "norPass"
		end
	end
end,

["beginning"] = {
	Leader   = task.defendKick(),
	Special  = task.goPassPos("Leader"),
	Middle   = task.defendDefault(2),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[LDASM]"
},

["attacker1"] = {
	Leader   = task.defendKick(),
	Special  = task.goPassPos("Leader"),
	Middle   = task.defendDefault(2),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][DASM]"
},

["attacker2"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.defendDefault(2),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][DA][M]"
},

["attacker3"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.defendDefault(2),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][A][D][M]"
},

["attacker4"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.rightBack(),
	Assister = task.leftBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][A][MD]"
},

["attacker5"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Assister = task.marking("Fourth"),
	Goalie   = task.goalie(),
	match    = "[L][S][AMD]"
},

["attacker6"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Assister = task.marking("Fourth"),
	Goalie   = task.goalie(),
	match    = "[L][S][AMD]"
},

["norPass"] = {
	Leader   = task.shoot(),
	Special  = task.markingDir("First",player.toBallDir),
	Middle   = task.markingDir("Second",player.toBallDir),
	Defender = task.markingDir("Third",player.toBallDir),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[S][L][A][DM]"
},

["norDef"] = {
	Leader   = task.shoot(),
	Special  = task.markingDir("First",player.toBallDir),
	Middle   = task.markingDir("Second",player.toBallDir),
	Defender = task.markingDir("Third",player.toBallDir),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][A][DM]"
},

name = "Ref_BackDefV5",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}