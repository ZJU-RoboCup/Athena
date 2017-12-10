local WAITPOS = CGeoPoint:new_local(80,180)
local WAITDIR = function()
        return dir.playerToBall("Kicker")
end

local FINAL_SHOOT_POS = CGeoPoint:new_local(140, 80)
local ANTIPOS_3 = ball.refAntiYPos(FINAL_SHOOT_POS)


local ballPointx = 0
local ballPointy = 0
gPlayTable.CreatePlay{

firstState = "testState1",

["testState1"] = { 
	switch   = function()
		if bufcnt(ball.valid(),15) then
			ballPointx = ball.posX() + 0.001
			ballPointx = ball.posY() + 0.001
			return "testState1"
		end
	end,
	Kicker=task.InterV6(),
	--Kicker = task.chase(),--task.slowGetBall("Kicker",false),
	-- Kicker = task.InterTouch(),
	match = ""
},
["testState2"] = {
	switch   = function()
		local dist = ball.velMod()*ball.velMod()/58
		debugEngine:gui_debug_x(ball.pos() + Utils.Polar2Vector(dist, ball.velDir()))
		--print(ball.pos() + Utils.Polar2Vector(dist, ball.velDir())
		if bufcnt(not ball.valid(),10) then
			return "testState1"
		end
		-- if bufcnt(player.kickBall("Goalie"), 1,400) then
		-- 	return "testState3"
		-- end
	end,
	-- Goalie = task.goAndTurnKickQuick("Kicker",3000,"chip"),
	-- Kicker = task.goCmuRush(WAITPOS),
	match  = ""
},

["testState3"] = {
        switch   = function()
        end,
        Kicker = task.chase(),
        match  = ""
},


name = "TestSkillPlay",
applicable ={
        exp = "a",
        a = true
},
attribute = "attack",
timeout   = 99999
}
