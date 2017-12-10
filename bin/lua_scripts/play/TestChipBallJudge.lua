local  ballpos = CGeoPoint:new_local(0,0)
local  ballDir
local  isChip , BallMovigDir,StartBallPosX, StartBallPosY = CChipBallJudge(ballDir,ballpos:x(),ballpos:y())

local  GOTOPOS = function (  )
	local  Fuckpos = function (  )
		local pos = CGeoPoint:new_local(StartBallPosX,StartBallPosY)
		local  BallMovingLine = CGeoLine:new_local(pos,BallMovigDir)
		local  pos = BallMovingLine:projection(CGeoPoint:new_local(0,0))
		return pos
	end
	return Fuckpos
end

local  FINALPOS = GOTOPOS()

gPlayTable.CreatePlay {

	firstState = "goto1",

	["goto1"] = {
		switch = function (  )
			if  bufcnt(false, 1, 100) then
				return "goto1"
			end
		end,
		Leader   = task.goSpeciPos(FINALPOS),
	
		match    = "[L]"
	},

	name 	   	= "TestChipBallJudge",
	applicable 	={
		exp = "a",
		a   = true
	},
	attribute 	= "attack",
	timeout 	= 99999

}
