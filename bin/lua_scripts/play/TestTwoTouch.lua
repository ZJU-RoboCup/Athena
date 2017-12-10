
local TouchPos1=CGeoPoint:new_local(0,180)
local TouchPos2=CGeoPoint:new_local(200,-180)
local TouchPos3=CGeoPoint:new_local(-200,-180)


gPlayTable.CreatePlay{
	firstState = "prepare",
["prepare"]	={
	switch=function (  )
	print("prepare")
		if  bufcnt(player.toTargetDist("Leader")<20 and
			player.toTargetDist("Assister")<20 and
			player.toTargetDist("Special")<20 and
			ball.toPointDist(TouchPos1)<20, 120) then
			return "prepareBall"
		end
	end,
	Leader=task.goCmuRush(TouchPos1,player.toPointDir(TouchPos2)),
	Assister=task.goCmuRush(TouchPos2,player.toPointDir(TouchPos3)),
	Special=task.goCmuRush(TouchPos3,player.toPointDir(TouchPos1)),
	match="[LAS]"
},

["prepareBall"]	={
	switch=function (  )
	print("prepareBall")
		if  bufcnt(player.toTargetDist("Leader")<20 and
			player.toTargetDist("Assister")<20 and
			player.toTargetDist("Special")<20 ,"slow") then
			return "firstKick"
		end
	end,
	Leader=task.staticGetBall(TouchPos2),
	Assister=task.goCmuRush(TouchPos2),
	Special=task.goCmuRush(TouchPos3),
	match="{LAS}"
},

["firstKick"]    ={
	switch=function (  )
	print("firstKick")
		if  bufcnt(player.kickBall("Leader") or player.isBallPassed("Leader","Assister"), "fast") then
			return "secondKick"
		end
	end,
	Leader=task.receivePass(TouchPos2),
	Assister=task.goCmuRush(TouchPos2),
	Special=task.goCmuRush(TouchPos3),
	match="{LAS}"
},

["secondKick"]    ={
	switch=function ()
		if  bufcnt(player.kickBall("Assister") or player.isBallPassed("Assister","Special"),"fast") then
			return "thirdKick"
		end
	end,
	Leader=task.goCmuRush(TouchPos1),
	Assister=task.receivePass(TouchPos3),
	Special=task.goCmuRush(TouchPos3),
	match="{LAS}"
},

["thirdKick"]    ={
	switch=function ()
		if  bufcnt(player.kickBall("Special") or player.isBallPassed("Special","Leader"),"fast") then
			return "firstKick"
		end
	end,
	Leader=task.goCmuRush(TouchPos1),
	Assister=task.goCmuRush(TouchPos2),
	Special=task.receivePass(TouchPos1),
	match="{LAS}"
},


	name 	   	= "TestTwoTouch",
	applicable 	={
		exp = "a",
		a   = true
	},
	attribute 	= "attack",
	timeout 	= 99999
}
