-- 转身吸球进行射门

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
	Leader   = task.goSpeciPos(CGeoPoint:new_local(320,0)),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,100)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-100)),
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
	Leader   = {SlowGetBall{pos = ball.pos, dir = player.toPointDir(CGeoPoint:new_local(param.pitchLength/2.0,40))}},
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,100)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-100)),
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
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,100)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-100)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LASDM}"
},

["turn"] = {
	switch = function ()
		if  bufcnt(true, 5) then
			return "kick"
		end
	end,
	Leader   = task.penaltyTurn(dir.specified(-25), false),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,100)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-100)),
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
	Leader   = task.penaltyTurn(dir.specified(-25), true),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(0,100)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(0,-100)),
	Defender = task.leftBack(),
	Assister = task.rightBack(),
	Goalie   = task.goalie(),
	match    = "{LASDM}"
},

name = "Ref_PenaltyKickV2",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
