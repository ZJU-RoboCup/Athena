local TargetPos  = CGeoPoint:new_local(-180,120)

gPlayTable.CreatePlay{

firstState = "prepare",

["prepare"] = {
	switch = function ()
		print("prepare")
		if bufcnt(player.toPointDist("Kicker",TargetPos)<20, "slow") then --  and ball.posX()<player.posX("Kicker")-30
			return "wait"
		end
	end,
	Kicker  = task.goCmuRush(TargetPos),
	match = ""
},

["wait"] = {
	switch = function ()
		print("wait")
		if bufcnt(ball.posX()>player.posX("Kicker")-15, "fast")then --  and ball.velMod()>20 and ball.valid()
			return "chase"
		end
	end,
	Kicker  = task.goCmuRush(TargetPos),
	match = ""
},

["chase"] = {
    switch = function ()
    	print("chase")
		if bufcnt(player.kickBall("Kicker") or math.abs(player.posX("Kicker"))>param.pitchLength/2-10
			or math.abs(player.posY("Kicker"))>param.pitchWidth/2-5 and ball.valid(), 2) then
			return "prepare"
		end
	end,
	Kicker = task.chaseNew(),
	match = ""
},

name = "TestChase",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

