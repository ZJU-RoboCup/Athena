--防robodragon的markingFront脚本
local leftUp = CGeoPoint:new_local(0,-param.pitchWidth/2)
local rightDown = CGeoPoint:new_local(-param.pitchLength/2,param.pitchWidth/2)
-- local left = CGeoPoint:new_local(0,-150)
-- local right =CGeoPoint:new_local(-300,110)
local KICKDEFTASK = task.defendKick(100,leftUp,rightDown,1)
-- local protectPos = function ()
-- 	return ball.pos()+Utils.Polar2Vector(50,Utils.Normalize(ball.velDir()-math.pi/2))
-- end
-- local left = CGeoPoint:new_local(-param.pitchLength/2,-param.goalWidth/2-10)
-- local right = CGeoPoint:new_local(-param.pitchLength/2,param.goalWidth/2+10)

-- local KICKDEFTASK = task.defendKick(150,left,right,2)

local FRONT_POS = function()
	return CGeoPoint:new_local(200, -170*ball.refAntiY())
end
local checkPosX = 0
local checkPosY = 0
local checkDir = 0
local bestEnemy = 0
local kickPos = CGeoPoint:new_local(0,0)


local markFrontOut = function ()
	return vision:ballVelValid() and ball.velMod() < 50
end

gPlayTable.CreatePlay{

firstState = "beginning",

switch = function()
	if gCurrentState == "beginning" and 
		enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
		return "attacker"..enemy.attackNum()
	elseif gCurrentState == "attackDef" then
		if bufcnt(not cond.markingFrontValid(CGeoPoint:new_local(checkPosX,checkPosY),checkDir) or cond.canNorPass2Def(checkDir),gNorPass2NorDefCouter,gMarkFront2Def) then
			defenceInfo:clearNoChangeFlag()
			return "norDef"
		elseif bufcnt(cond.canDefenceExit(), 2,gMarkFront2Finish) then
			return "finish"
		end
	elseif gCurrentState == "norPass" then
		if bufcnt((ball.velMod() < gNorPass2NorDefBallVel and ball.toPointDist(kickPos) > 120
			or not enemy.hasReceiver())
			and vision:ballVelValid(), gNorPass2NorDefCouter) then
			return "norDef"
		end
	elseif gCurrentState == "norDef" then
		if bufcnt(cond.canDefenceExit(), 2,100) then
			return "finish"
		end
	else
		if bufcnt(cond.canDefenceExit(), 3) then
			return "finish"
		elseif enemy.situChanged() and 
			enemy.attackNum() <= 6 and enemy.attackNum() > 0 then
			return "attacker"..enemy.attackNum()
		elseif bufcnt(cond.isGameOn(), 1) then
			kickPos = CGeoPoint:new_local(ball.posX(),ball.posY())
			--checkPos, checkDir = enemy.nearest()
			local checkPos
			checkPos, checkDir = enemy.nearest()
			checkPosX = checkPos:x()
			checkPosY = checkPos:y()
			defenceInfo:setNoChangeFlag()
			return "attackDef"
		end
	end
end,

["beginning"] = {
	Leader   = KICKDEFTASK,
	Special  = task.goPassPos("Leader"),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][DASM]"
},

["attacker1"] = {
	Leader   = KICKDEFTASK,
	Special  = task.goPassPos("Leader"),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][DASM]"
},

["attacker2"] = {
	Leader   = KICKDEFTASK,
	Special  = task.markingFront("First"),
	Middle   = task.defendMiddle(),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][M][DA]"
},

["attacker3"] = {
	Leader   = KICKDEFTASK,
	Special  = task.markingFront("First"),
	Middle   = task.markingFront("Second"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][M][DA]"
},

["attacker4"] = {
	Leader   = KICKDEFTASK,
	Special  = task.markingFront("First"),
	Middle   = task.markingFront("Second"),
	Defender = task.markingFront("Third"),
	Assister = task.singleBack(),
	Goalie   = task.goalie(),
	match    = "[L][S][A][MD]"
},

["attacker5"] = {
	Leader   = KICKDEFTASK,
	Special  = task.markingFront("First"),
	Middle   = task.markingFront("Second"),
	Defender = task.markingFront("Third"),
	Assister = task.markingFront("Fourth"),
	Goalie   = task.goalie(),
	match    = "[L][S][AMD]"
},

["attacker6"] = {
	Leader   = KICKDEFTASK,
	Special  = task.markingFront("First"),
	Middle   = task.markingFront("Second"),
	Defender = task.markingFront("Third"),
	Assister = task.markingFront("Fourth"),
	Goalie   = task.goalie(),
	match    = "[L][S][AMD]"
},

["attackDef"] = {
	Leader   = task.goSpeciPos(FRONT_POS),
	Special  = task.continue(),
	Middle   = task.continue(),
	Defender = task.continue(),
	Assister = task.continue(),
	Goalie   = task.goalie(),
	match    = "{LDAMS}"
},

["norPass"] = {
	Leader   = task.shoot(),
	Special  = task.markingDir("First",player.toBallDir),
	Middle   = task.markingDir("Second",player.toBallDir),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[S][L][AD][M]"
},

["norDef"] = {
	Leader   = task.shoot(),
	Special  = task.markingFront("First"),
	Middle   = task.markingFront("Second"),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "[L][AD][SM]"
},

name = "Ref_MiddleDefV10_1",
applicable ={
	exp = "a",
	a   = true
},
attribute = "defense",
timeout   = 99999
}