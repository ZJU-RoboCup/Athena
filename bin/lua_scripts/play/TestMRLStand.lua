local centerX = -250;
local centerY = 150;

local dis1 = 100
local dis2 = 20
local dis3 = 20

local time = 80
local face_out = false
function setPos(dis,num)
	local pos = CGeoPoint:new_local(dis*math.sin(math.rad(60*num))+centerX,dis*math.cos(math.rad(60*num))+centerY)
	local dir 
	if face_out then
		dir = (pos - CGeoPoint:new_local(centerX,centerY)):dir()
	else
		dir = (CGeoPoint:new_local(centerX,centerY) - pos):dir()
	end
	return pos,dir
end



gPlayTable.CreatePlay{

firstState = "goto1",
["goto1"] = {
	switch = function ()
		if bufcnt(
			player.toTargetDist("Assister") <20 and 
			player.toTargetDist("Leader")   <20 and 
			player.toTargetDist("Special")  <20 and 
			player.toTargetDist("Defender") <20 and 
			player.toTargetDist("Middle")   <20 and 
			player.toTargetDist("Goalie")   <20
			,20,300) then
			return "goto2"
		end
	end,
	Assister = task.goSpeciPos(setPos(dis2,1)),
	Leader   = task.goSpeciPos(setPos(dis2,2)),
	Special  = task.goSpeciPos(setPos(dis2,3)),
	Defender = task.goSpeciPos(setPos(dis2,4)),
	Middle   = task.goSpeciPos(setPos(dis2,5)),
	Goalie   = task.goSpeciPos(setPos(dis2,6)),
	match = "{ALSMD}"
},
["goto2"] = {
	switch = function ()
		if bufcnt(true,time,10000) then
			return "goto1"
		end
	end,
	Assister = task.goSpeciPos(setPos(dis3,1)),
	Leader   = task.goSpeciPos(setPos(dis3,2)),
	Special  = task.goSpeciPos(setPos(dis3,3)),
	Defender = task.goSpeciPos(setPos(dis3,4)),
	Middle   = task.goSpeciPos(setPos(dis3,5)),
	Goalie   = task.goSpeciPos(setPos(dis3,6)),
	match = "{ALSMD}"
},
name = "TestMRLStand",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}



