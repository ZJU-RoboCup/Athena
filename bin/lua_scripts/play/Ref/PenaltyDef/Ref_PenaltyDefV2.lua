local LEFT_POS  = CGeoPoint:new_local(-438, -35)
local SUBLEFT_POS = CGeoPoint:new_local(-438, -15)
local RIGHT_POS = CGeoPoint:new_local(-438, 35)
local SUBRIGHT_POS = CGeoPoint:new_local(-438, 15)

math.randomseed(os.time())

-- print(enemy.dir(bestPlayer:getTheirBestPlayer()))
local localCnt = 0

local getEnemyDir = function ()
	local realDir = enemy.dir(bestPlayer:getTheirBestPlayer())
	if realDir > 0 then
		return math.pi - realDir
	else
		return -(math.pi + realDir)
	end
end

gPlayTable.CreatePlay{
firstState = "goLeft",

["goLeft"] = {
	switch = function ()
		if bufcnt(cond.isGameOn(), 20)then
			return "exit"
		elseif  bufcnt(player.toTargetDist("Goalie") < 5, "normal") then
			if math.random() > 0.6 and getEnemyDir() < -0.03 then
				localCnt = localCnt + 1
				if localCnt > 1 then
					localCnt = 0
					return "goRight"
				else
					return "goSubleft"
				end
			else
				localCnt = 0
				return "goRight"
			end
		end
	end,
	Leader   = task.goSpeciPos(CGeoPoint:new_local(-300,300)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(-320,80)),
	Assister = task.goSpeciPos(CGeoPoint:new_local(-320,-80)),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(-320,105)),
	Defender = task.goSpeciPos(CGeoPoint:new_local(-320,-105)),
	Goalie   = task.goSpeciPos(LEFT_POS),
	match    = "{LASDM}"
},

["goSubleft"] = {
	switch = function()
		if bufcnt(cond.isGameOn(), 20) then
			return "exit"
		elseif bufcnt(player.toTargetDist("Goalie") < 5, "normal") then
			return "goLeft"
		end
	end,
	Leader   = task.goSpeciPos(CGeoPoint:new_local(-300,300)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(-320,80)),
	Assister = task.goSpeciPos(CGeoPoint:new_local(-320,-80)),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(-320,105)),
	Defender = task.goSpeciPos(CGeoPoint:new_local(-320,-105)),
	Goalie   = task.goSpeciPos(SUBLEFT_POS),
	match    = "{LASDM}"
},

["goRight"] = {
	switch = function ()
		if bufcnt(cond.isGameOn(), 20)then
			return "exit"
		elseif  bufcnt(player.toTargetDist("Goalie") < 5, "normal") then
			if math.random() > 0.6 and getEnemyDir() > 0.03 then
				localCnt = localCnt + 1
				if localCnt > 1 then
					localCnt = 0
					return "goLeft"
				else
					return "goSubright"
				end
			else
				return "goLeft"
			end
		end
	end,
	Leader   = task.goSpeciPos(CGeoPoint:new_local(-300,300)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(-320,80)),
	Assister = task.goSpeciPos(CGeoPoint:new_local(-320,-80)),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(-320,105)),
	Defender = task.goSpeciPos(CGeoPoint:new_local(-320,-105)),
	Goalie   = task.goSpeciPos(RIGHT_POS),
	match    = "{LASDM}"
},

["goSubright"] = {
	switch = function ()
		if bufcnt(cond.isGameOn(), 20) then
			return "exit"
		elseif bufcnt(player.toTargetDist("Goalie") < 5, "normal") then
			return "goRight"
		end
	end,
	Leader   = task.goSpeciPos(CGeoPoint:new_local(-300,300)),
	Special  = task.goSpeciPos(CGeoPoint:new_local(-320,80)),
	Assister = task.goSpeciPos(CGeoPoint:new_local(-320,-80)),
	Middle   = task.goSpeciPos(CGeoPoint:new_local(-320,105)),
	Defender = task.goSpeciPos(CGeoPoint:new_local(-320,-105)),
	Goalie   = task.goSpeciPos(SUBRIGHT_POS),
	match    = "{LASDM}"
},

name = "Ref_PenaltyDefV2",
applicable ={
	exp = "a",
	a = true
},
attribute = "attack",
timeout = 99999
}
