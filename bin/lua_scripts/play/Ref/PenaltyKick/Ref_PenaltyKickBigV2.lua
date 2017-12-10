-- 转身吸球进行射门
-- by zhyaic 2013.6.21

local function canPenaltyShoot(role1, d)
	if d == nil then
		d = 70
	end
	local p1 = player.pos(role1)
	local p2 = player.pos(role1) + Utils.Polar2Vector(d,player.dir(role1))
	local seg = CGeoSegment:new_local(p1, p2)
	local tmPPos = enemy.getTheirGoaliePos()
	local dist = seg:projection(tmPPos):dist(tmPPos)
	print(tmPPos:y())
	local isprjon = seg:IsPointOnLineOnSegment(seg:projection(tmPPos))
	if dist < 18 and isprjon then
		return false
	else
		return true
	end
end

gPlayTable.CreatePlay{

firstState = "goto",

["goto"] = {
	switch = function ()
		if cond.isNormalStart() then
			return "slowGoto"
		elseif cond.isGameOn() then
			return "exit"
		end
	end,
	Leader   = task.goSpeciPos(CGeoPoint:new_local(270,0)),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,50)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-50)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{L}{ASDM}"
},

["slowGoto"] = {
	switch = function ()
		if  bufcnt(canPenaltyShoot("Leader", 180), 80) then
			return "direct"
		elseif bufcnt(true, 100) then
			return "turn"
		end
	end,
	Leader   = {SlowGetBall{pos = ball.pos, dir = player.toPointDir(CGeoPoint:new_local(param.pitchLength/2.0,30))}},
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,50)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-50)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LASDM}"
},

["direct"] = {
	switch = function ()
		if  bufcnt( player.kickBall("Leader"), "normal", 10) then
			return "exit"
		end
	end,
	Leader   = task.directShoot(),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,50)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-50)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LASDM}"
},

["turn"] = {
	switch = function ()
		if  bufcnt(true, 3) then
			return "kick"
		end
	end,
	Leader   = task.penaltyTurn(dir.specified(-20), false),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,50)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-50)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LASDM}"
},

["kick"] = {
	switch = function ()
		if  bufcnt( player.kickBall("Leader"), "normal", 10) then
			return "exit"
		end
	end,
	Leader   = task.penaltyTurn(dir.specified(-20), true),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,50)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-50)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LASDM}"
},

name = "Ref_PenaltyKickBigV2",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
