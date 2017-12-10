function bestPos()
	local p = nil
	if p == nil then
		p = pos.bestShoot()
	end
	return p
end

gPlayTable.CreatePlay{

firstState = "getball",
["getball"] = {
	switch = function ()
		print("getBall...");
		if bufcnt(player.toTargetDist("Tier")<20,20,10000) then
			return "pass"
		end
	end,
	Kicker = task.staticGetBall("Tier"),
	Tier   = task.goCmuRush(bestPos),
	match  = ""
},
["pass"] = {
	switch = function ()
		print("pass...");
		if bufcnt( player.kickBall("Kicker") ,2,10000) then
			return "shoot"
		end
	end,
	Kicker = task.receivePass("Tier"),
	Tier   = task.goCmuRush(bestPos),
	match  = ""
},
["shoot"] = {
		switch = function ()
		print("shoot...");
		if bufcnt( player.kickBall("Tier") ,2,10000) then
			return "exit"
		end
	end,
	Kicker = task.stop(),
	Tier   = task.waitTouchNew(),--shoot(),
	match  = ""
},
name = "TestBestShoot",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
