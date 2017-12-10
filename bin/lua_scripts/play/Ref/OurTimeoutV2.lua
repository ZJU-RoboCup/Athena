local centerX = -250;
local centerY = 150;

local dis1 = 100
local dis2 = 75
local dis3 = 20

local time = 9999
local face_out = false
function setPos(dis,num)
	local pos = CGeoPoint:new_local(dis*math.sin(math.rad(60*num))+centerX,dis*math.cos(math.rad(60*num))+centerY)
	local dir 
	if face_out then
		dir = (pos - CGeoPoint:new_local(centerX,centerY)):dir()
	else
		dir = (CGeoPoint:new_local(centerX,centerY) - pos):dir()
	end
	return pos
end

function setDir(dis,num)
	local pos = CGeoPoint:new_local(dis*math.sin(math.rad(60*num))+centerX,dis*math.cos(math.rad(60*num))+centerY)
	local dir 
	if face_out then
		dir = (pos - CGeoPoint:new_local(centerX,centerY)):dir()
	else
		dir = (CGeoPoint:new_local(centerX,centerY) - pos):dir()
	end
	return dir
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
			,120,300) then
			return "goto2"
		end
	end,
	Assister = task.goCmuRush(setPos(dis2,1),setDir(dis2,1),150,flag.allow_dss),
	Leader   = task.goCmuRush(setPos(dis2,2),setDir(dis2,2),150,flag.allow_dss),
	Special  = task.goCmuRush(setPos(dis2,3),setDir(dis2,3),150,flag.allow_dss),
	Defender = task.goCmuRush(setPos(dis2,4),setDir(dis2,4),150,flag.allow_dss),
	Middle   = task.goCmuRush(setPos(dis2,5),setDir(dis2,5),150,flag.allow_dss),
	Goalie   = task.goCmuRush(setPos(dis2,6),setDir(dis2,6),150,flag.allow_dss),
	match = "{ALSMD}"
},
["goto2"] = {
	switch = function ()
		if bufcnt(true,time,10000) then
			return "goto1"
		end
	end,
	Assister = task.goCmuRush(setPos(dis3,1),setDir(dis3,1),150,flag.allow_dss),
	Leader   = task.goCmuRush(setPos(dis3,2),setDir(dis3,2),150,flag.allow_dss),
	Special  = task.goCmuRush(setPos(dis3,3),setDir(dis3,3),150,flag.allow_dss),
	Defender = task.goCmuRush(setPos(dis3,4),setDir(dis3,4),150,flag.allow_dss),
	Middle   = task.goCmuRush(setPos(dis3,5),setDir(dis3,5),150,flag.allow_dss),
	Goalie   = task.goCmuRush(setPos(dis3,6),setDir(dis3,6),150,flag.allow_dss),
	match = "{ALSMD}"
},
name = "OurTimeoutV2",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}



