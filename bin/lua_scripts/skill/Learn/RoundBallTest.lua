local lK    = 900
local lD    = 60
local la    = 0.345
local lTau  = 5
local lNum  = 80
local lXRFWR = {
{40, 0, 0, 0},
{40, 0.0526316, 0, 0},
{40, 0.105263, 0, 0},
{40, 0.157895, 0, 0},
{39.5425, 0.210526, 24.8949, -1.12526},
{37.9867, 0.263158, 25.7035, 0.185351},
{37.7474, 0.315789, 25.4109, 1.53478},
{38.0054, 0.368421, 18.7635, 2.73215},
{38.0551, 0.421053, -5.69814, 3.02865},
{41.32, 0.473684, -40.7494, 1.85688},
{37.9758, 0.526316, -62.2078, -1.01772},
{38.0222, 0.578947, -53.4835, -4.09394},
{44.48, 0.631579, -32.1285, -6.451},
{49.68, 0.684211, -2.43113, -7.31522},
{47.64, 0.736842, 18.8218, -6.83066},
{38.0355, 0.789474, 23.8925, -5.68791},
{38.3399, 0.842105, 28.358, -4.27963},
{38.5076, 0.894737, 26.6962, -2.84166},
{47.24, 0.947368, 25.7123, -1.45739},
{41.64, 1, 29.3305, 0.0210956}
}

local lYRFWR = {
{40, 0, 0, 0},
{40, 0.0526316, 0, 0},
{40, 0.105263, 0, 0},
{40, 0.157895, 0, 0},
{39.5435, 0.210526, -13.713, 0.619837},
{37.9896, 0.263158, -14.1585, -0.102098},
{37.751, 0.315789, -13.9744, -0.84522},
{38.0149, 0.368421, -9.93974, -1.4843},
{38.0361, 0.421053, -4.28527, -1.84486},
{38.1317, 0.473684, -1.56333, -1.98223},
{37.9745, 0.526316, 6.11604, -1.91539},
{38.0521, 0.578947, 25.2379, -1.02544},
{37.9311, 0.631579, 31.0436, 0.545874},
{50.32, 0.684211, 25.2646, 2.1073},
{38.1389, 0.736842, 3.15474, 2.75313},
{38.02, 0.789474, -6.02978, 2.67063},
{38.346, 0.842105, -12.5109, 2.11892},
{38.4978, 0.894737, -13.0512, 1.39957},
{44.96, 0.947368, -15.3264, 0.751348},
{46.68, 1, -11.7457, 0.0323482}
}

local lWRFWR = {
{40, 0, 0, 0},
{40, 0.0526316, 0, 0},
{40, 0.105263, 0, 0},
{40, 0.157895, 0, 0},
{39.544, 0.210526, 1.88648, -0.08527},
{37.9912, 0.263158, 1.94776, 0.0140455},
{37.7533, 0.315789, 2.00605, 0.117318},
{38.0197, 0.368421, 2.22426, 0.233698},
{38.0362, 0.421053, 0.782961, 0.300471},
{42.76, 0.473684, -1.74004, 0.319763},
{38.0017, 0.526316, -4.20505, 0.132295},
{38.0624, 0.578947, -4.8715, -0.0971176},
{46.68, 0.631579, -3.33118, -0.328092},
{38.1218, 0.684211, -1.29266, -0.441654},
{38.1199, 0.736842, 0.150161, -0.46833},
{38.0573, 0.789474, 1.89451, -0.404805},
{38.3421, 0.842105, 1.99322, -0.299465},
{38.5101, 0.894737, 1.9388, -0.195802},
{38.5927, 0.947368, 1.84437, -0.0965617},
{39.5742, 1, 1.83468, 0.000181697}
}

local ROUND_CYCLE = 20

function RoundBallTest(task)
	local lastCycle = 0
	local calCnt = 0
	local startP
	local endP
	local startDir
	local endDir
	local mirrorFlag

	execute = function(runner)
		if type(task.pos) == "function" then
			endP = task.pos()
		else
			endP = task.pos
		end

		if type(task.dir) == "function" then
			endDir = task.dir(runner)
		else
			endDir = task.dir
		end

		if vision:Cycle() - lastCycle > 6 or lastCycle == 0 then
			calCnt = 0
			startP = CGeoPoint:new_local(player.posX(runner), player.posY(runner))
			startDir = player.dir(runner)
			mirrorFlag = ball.antiY()
		end

		lastCycle = vision:Cycle()
		local stepX   = endP:x() - startP:x()
		local stepY   = (endP:y() - startP:y())*mirrorFlag
		local stepDir = (endDir - startDir)*mirrorFlag

		local posX = learn.calRFWRRes(calCnt, lXRFWR, la, lTau, lK, 0, stepX,   lNum)
		local posY = learn.calRFWRRes(calCnt, lYRFWR, la, lTau, lK, 0, stepY, 	lNum)
		local posW = learn.calRFWRRes(calCnt, lWRFWR, la, lTau, lK, 0, stepDir, lNum)

		calCnt = calCnt + 1

		--return CGetBall(runner, endDir)

		if calCnt > ROUND_CYCLE then
			return CGetBall(runner, endDir)
		else
			return SimpleGotoPos(runner, posX+startP:x(), posY*mirrorFlag+startP:y(), Utils.Normalize(posW*mirrorFlag+startDir), 0)	
		end
	end

	matchPos = function()
		if type(task.pos) == "function" then
			mpos = task.pos()
		else
			mpos = task.pos
		end
		return mpos
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "RoundBallTest",
	execute = function (self)
		print("This is in learn skill"..self.name)
	end
}