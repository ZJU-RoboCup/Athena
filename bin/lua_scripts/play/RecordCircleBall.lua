gPlayTable.CreatePlay{

firstState = "GoToBall",

["GoToBall"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Kicker") < 30 , "slow") then
			return "Pause"
		end
	
	end,
	Assister = {Stop()},
	Kicker = task.staticGetBall(CGeoPoint:new_local(300,-200)),
	--Assister = task.TestGoSpeciPos(nPos,0),
	match = "(A)"	
},

["Pause"] = {
	switch = function ()
		if bufcnt(false,1,100) then
			return "goAndTurnKick"
		end
	
	end,
	Assister = {Stop()},
	Kicker = task.staticGetBall(CGeoPoint:new_local(300,-200)),
	match = "{A}"
},

["goAndTurnKick"] = {
	switch = function ()
		local recordfile = io.open("RecordCircle".."PI"..".txt","a")
				recordfile:write(player.posX("Kicker").."	"..player.posY("Kicker").."	"..Utils.Normalize(player.dir("Kicker"))*180 / math.pi.."\n")
				recordfile:close()
		if bufcnt(player.kickBall("Kicker"),"fast") then
			return "stop"
		end
	
	end,
	Kicker = task.testCircleBall("Assister"),
	Assister = {Stop()},
	match = "{A}"	
},

["stop"] = {
	switch = function ()
		if bufcnt(false,1,200) then
			return "GoToBall"
		end
	end,
	Assister = {Stop()},
	Kicker = {Stop()},
	match = "{A}"
},
	
name = "RecordCircleBall",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
