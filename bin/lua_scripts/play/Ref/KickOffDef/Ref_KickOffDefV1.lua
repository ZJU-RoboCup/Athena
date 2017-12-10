local function KICKOFF_DEF_POS(str)
	return function()
		local x, y
		x, y = CKickOffDefPos(str)
		return CGeoPoint:new_local(x,y)
	end
end

gPlayTable.CreatePlay{
firstState = "start",

["start"] = {
	switch = function ()
		if cond.isGameOn() then
			return "advance"
		end
	end,
	Leader   = task.goSpeciPos(KICKOFF_DEF_POS("left")),
	Special  = task.goSpeciPos(KICKOFF_DEF_POS("right")),
	Assister = task.goSpeciPos(KICKOFF_DEF_POS("middle")),
	Middle   = task.rightBack(),
	Defender = task.leftBack(),
	Goalie   = task.goalie(),
	match    = "[ADMLS]"
},

["advance"] = {
	switch = function ()
		if  bufcnt(true, 90) then
			return "exit"
		end
	end,
	Special  = task.goSpeciPos(KICKOFF_DEF_POS("right")),
	Assister = task.advance(),
	Leader   = task.goSpeciPos(KICKOFF_DEF_POS("left")),
	Middle   = task.rightBack(),
	Defender = task.leftBack(),
	Goalie   = task.goalie(),
	match    = "{A}[DMLS]"	
},

name = "Ref_KickOffDefV1",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
