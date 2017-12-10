--角球防守，增加norPass的状态，保证norPass的时候盯防first的优先级最高
--防守头球，defendHead,实时有防开球车
--可以修改的地方有attacker5,attacker6,可以改变防头球车的匹配优先级，或者去掉防开球车，加一盯人车
local COR_DEF_POS1 = CGeoPoint:new_local(-170,-50)
local COR_DEF_POS2 = CGeoPoint:new_local(-170,50)
local COR_DEF_POS3 = CGeoPoint:new_local(-170,0)

local PreBallPos = function()
	return ball.pos() + Utils.Polar2Vector(ball.velMod()/10, ball.velDir())
end

gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	if gCurrentState == "beginning" and 
		enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
		return "attacker"..enemy.attackNum()
	elseif gCurrentState == "marking" or gCurrentState == "markingHead" then
		--print(DefendUtils.isPosInOurPenaltyArea(ball.pos()))
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
	Special  = task.defendHead(),--task.goPassPos("Leader"),
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
	match    = "(A)[L][S][DM]"
},

["attacker3"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.goSpeciPos(COR_DEF_POS3, player.toBallDir),
	Assister = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "(A)[L][S][MD]"
},

["attacker4"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Assister = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "(A)[L][S][MD]"
},

["attacker5"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Assister = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "(A)[L][S][MD]"
},

["attacker6"] = {
	Leader   = task.defendKick(),
	Special  = task.marking("First"),
	Middle   = task.marking("Second"),
	Defender = task.marking("Third"),
	Assister = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "(A)[L][S][MD]"
},

["marking"] = {
	Leader   = task.markingDir("First",player.toBallDir),
	Special  = task.markingDir("Second",player.toBallDir),
	Middle   = task.marking("Third"),
	Defender = task.advance(),--task.marking("Fourth"),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[LD][S][AM]"--"[L][S][A][MD]"
},

["markingHead"] = {
	Leader   = task.markingDir("First",player.toBallDir),
	Special  = task.markingDir("Second",player.toBallDir),
	Middle   = task.marking("Third"),
	Defender = task.advance(),--task.marking("Fourth"),
	Assister = task.defendHead(),
	Goalie   = task.goalie(),
	match    = "{A}[LD][S][M]"--"{A}[L][S][MD]"
},

["norPass"] = {
	Leader   = task.advance(),
	Special  = task.markingDir("First",player.toBallDir),
	Middle   = task.markingDir("Second",player.toBallDir),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[S][L][M][DA]"
},

["norDef"] = {
	Leader   = task.advance(),
	Special  = task.markingDir("First",player.toBallDir),
	Middle   = task.markingDir("Second",player.toBallDir),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][M][DA]"
},

name = "Ref_CornerDefV4_3",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}