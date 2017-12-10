local TargetPos  = CGeoPoint:new_local(110,-150)

gPlayTable.CreatePlay{

firstState = "prepare",
["prepare"] = {
	switch = function ()
		if bufcnt(player.toPointDist("Kicker",TargetPos)<20 , "slow") then
		print("Jump wait")
			return "wait"
		end
	end,
	Kicker  = task.goCmuRush(TargetPos,_,300),
	match = ""
},

["wait"] = {
	switch = function ()
		--print("wait")
		if bufcnt(player.toPointDist("Kicker",ball.pos())<200 and ball.velMod()>50 and ball.valid(), "fast")then
		    print("Jump inter")
			return "inter"
		end
	end,
	Kicker  = task.goCmuRush(TargetPos,_,300),
	match = ""
},

["inter"] = {
    switch = function ()
    	--print("chase")
		if bufcnt(player.kickBall("Kicker") , 2) then
		--print("GoingThrough") --这个函数似乎有问题
		--print(player.kickBall("Kicker"))
		print("Jump prepare")
			return "prepare"
		end
		if bufcnt(player.posX("Kicker")==0 and player.posY("Kicker")==0, 500) then
			return "prepare"
		end
	end,
	Kicker = task.InterTouch(TargetPos,nil,nil,10),
	match = ""
},

name = "TestInterTouch",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}



