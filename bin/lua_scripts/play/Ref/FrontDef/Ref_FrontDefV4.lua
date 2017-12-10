--三个进攻车时，有一个车位置上前，普通
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
		elseif bufcnt(cond.isGameOn(), 5) then
			return "norDef"
		end
	end
end,

["beginning"] = {
	Leader   = task.defendKick(),
	Special  = task.goPassPos("Leader"),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][DASM]"
},

["attacker1"] = {
	Leader   = task.defendKick(),
	Special  = task.goPassPos("Leader"),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][DASM]"
},

["attacker2"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][M][DA]"
},

["attacker3"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.defendFront(-70),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][M][DA]"
},

["attacker4"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][A][MD]"
},

["attacker5"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][A][MD]"
},

["attacker6"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][A][MD]"
},

["norDef"] = {
	Leader   = task.advance(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][AD][M]"
},

name = "Ref_FrontDefV4",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}