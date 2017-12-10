gPlayTable.CreatePlay{

firstState = "getball",
["getball"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Kicker") < 30 , "slow") then
			
			return "pause"
		end
	end	,
	Kicker = task.testSlowGetBall(CGeoPoint:new_local(200,100)),
	match  = ""
	
},

["pause"] = {
	switch = function ()
		if bufcnt(false,1,300) then
			return "chip"
		end
	
	end,
	Kicker = task.testSlowGetBall(CGeoPoint:new_local(200,100)),
	match  = ""	
},

["chip"] = {
	switch = function ()
		if bufcnt(player.kickBall("Kicker"),"fast") then
			return "record"
		end
	
	end,
	Kicker = task.ttestChip(CGeoPoint:new_local(200,-100),100),
	match = ""	
},
	
["record"] = {

	switch = function ()
		local recordfile = io.open("chipBallPos.txt","a")
				recordfile:write(ball.posX().."  "..ball.posY().."	"..ball.velDir().."	"..player.posX("Kicker").."	"..player.posY("Kicker").."	"..player.dir("Kicker").."\n")
				recordfile:close()
		if bufcnt(false,1,200) then
			return "getball"
		end
	end,		
	Kicker = {Stop()},
	match = ""
},


name = "RecordChipData",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}