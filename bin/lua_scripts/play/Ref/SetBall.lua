--放球脚本，测试时替代ourtime out 脚本
--gty 16.6.15 
--暂停脚本，我方暂停与敌方暂停都会调用，左右需要side调整
--gty 16.6.15
local TargetPos  = CGeoPoint:new_local(179,166)

gPlayTable.CreatePlay{

firstState = "move",
["move"] = {
	switch = function ()
		if bufcnt(player.toTargetDist("Kicker") <20 ,20,300) then
			return "get"
		end
	end,
	Kicker = task.staticGetBall(TargetPos),
	["Special"]  = task.stop(),
	["Assister"] = task.stop(),
	["Defender"] = task.stop(),
	["Middle"]   = task.stop(),
	["Goalie"]   = task.stop(),
	match = "{SADM}"
},

["get"] = {
	switch = function ()
		if bufcnt(true,120) then
			return "go"
		end
	end,
	Kicker = task.slowGetBall(TargetPos),
	["Special"]  = task.stop(),
	["Assister"] = task.stop(),
	["Defender"] = task.stop(),
	["Middle"]   = task.stop(),
	["Goalie"]   = task.stop(),
	match = "{SADM}"
},


["go"] = {
	switch = function()
		if bufcnt(player.toTargetDist("Kicker")<10,60,300) then
			return "go"
		end
	end,
	Kicker = task.goCmuRush(TargetPos,_,100,_,1),
	match = ""
},

name = "SetBall",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
