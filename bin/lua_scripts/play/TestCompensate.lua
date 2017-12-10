local targetdierction
local realdirection = 0
local ballspeed
local balldirection
local flag = true
local recordflag = true
local compensateflag = 1
--compensateflag记录接球的方向，如果为负代表是嘴巴左边接住球，为0则代表正中央接住球，正代表右边接住球
gPlayTable.CreatePlay{

firstState = "goto",

["goto"] = {
	switch = function ()
		print("in goto state", player.toBallDist("Kicker"), ball.velMod(), player.dir("Kicker")*180/math.pi)
		local ballVelLine = CGeoLine:new_local(ball.pos(),ball.velDir())
		recordflag = true
		if bufcnt( player.toBallDist("Kicker")< 280 and ball.velMod()>20,  1)then
			return "kick"
		end
	end,		
	Kicker  = task.goSpeciPos(CGeoPoint:new_local(100,-120)),		
	match = ""
},	

["kick"] = {
    switch = function ()
		print("in kick state", player.dir("Kicker")*180/math.pi)
		print(vision:Cycle(),player.posX("Kicker"),player.posY("Kicker"), player.toBallDist("Kicker"))
		if ball.pos():dist(player.pos("Kicker"))<40 and flag == true then	
			ballspeed = ball.velMod()
			balldirection = ball.velDir()
			local ballVelLine = CGeoLine:new_local(ball.pos(),ball.velDir())				
			local target2me = pos.theirGoal()- player.pos("Kicker")
			targetdirection = Utils.Normalize(Utils.Normalize(ball.velDir() + math.pi)- player.dir("Kicker"))*180/math.pi
			local palyermouthline = CGeoLine:new_local(player.pos("Kicker")+Utils.Polar2Vector(param.playerFrontToCenter,player.dir("Kicker")),Utils.Normalize(player.dir("Kicker")+math.pi/2))
			local intersect = CGeoLineLineIntersection:new_local(ballVelLine,palyermouthline)
			local intersectantpoint
			if intersect:Intersectant()== true then
				intersectantpoint = intersect:IntersectPoint()
			end
			local playermouthpoint =player.pos("Kicker")+Utils.Polar2Vector(param.playerFrontToCenter,player.dir("Kicker"))
			local intersect2mouth
			if intersectantpoint ~= playermouthpoint then
				intersect2mouth = intersectantpoint - playermouthpoint
				if intersect2mouth:dir() > 0 then
					compensateflag = intersect2mouth:mod() 
				elseif intersect2mouth:dir() < 0 then
					compensateflag = -intersect2mouth:mod()
				end
			elseif intersectantpoint == playermouthpoint then
				compensateflag = 0
			end
			print("firstin balldirection "..targetdirection)
			flag = false				
		end	
		if bufcnt(player.kickBall("Kicker"), "fast") then						
			return "recordfile"																			
		end	

		debugEngine:gui_debug_msg(CGeoPoint:new_local(30,0),string.format("%s = %f","ballspeed",ballspeed),3)
		debugEngine:gui_debug_msg(CGeoPoint:new_local(70,0),string.format("%s = %f","realdirection",realdirection),3)
		debugEngine:gui_debug_msg(CGeoPoint:new_local(110,0),string.format("%s = %f","targetdirection",targetdirection),3)
	end,
	Kicker = task.touch(CGeoPoint:new_local(302,-20)),
	match = ""
},

["recordfile"] = { 
 switch = function ()
	    print("record file !")
		if ball.posX()>200  and recordflag == true then					
				realdirection = Utils.Normalize((Utils.Normalize(balldirection + math.pi)- ball.velDir()))*180/math.pi
				print("hehehesecondin balldirection "..balldirection*180/math.pi)
				print("mamamamsecondin ball "..ball.velDir()*180/math.pi)
				print("ballspeed=  "..ballspeed.."\n")
				print("targetdirection=  "..targetdirection.."\n")
				print("realdirection=  "..realdirection.."\n")
				-- local recordfile = io.open("onepassshoot.txt","a")
				-- recordfile:write(compensateflag.."  "..ballspeed.."  "..realdirection.."  "..targetdirection.."\n")
				-- recordfile:close()
				flag = true
				recordflag = false
				return "goto"																			
		end					
	end,	
	Kicker = task.stop(),	
	match = ""
},	

name = "TestCompensate",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}

