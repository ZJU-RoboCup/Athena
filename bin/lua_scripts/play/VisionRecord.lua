
gPlayTable.CreatePlay{


firstState = "pause",

["pause"] = {
	switch = function ()
		if bufcnt(false,1,500) then
			return "move"			
		end
	
	end,
	Kicker = task.testSlowGetBall(CGeoPoint:new_local(-50,40)),
	match = ""	
},

["move"] = {
	switch = function ()
		local recordfile = io.open("VisionRecord.txt","a")
				recordfile:write(player.posX("Kicker").."	"..player.posY("Kicker").."	"..player.dir("Kicker").."\n")
				recordfile:close()
		if bufcnt(false,1,5000) then
			return "exit"
		end
	
	end,
	Kicker = task.testChip(),
	match = ""	
},
	

name = "VisionRecord",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}