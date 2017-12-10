local StartPosWithX    = CGeoPoint:new_local(200, 200)
local startPosWithoutX = CGeoPoint:new_local(280, 100)
local startDirWithX    = - math.pi * 1 --/ 2
local startDirWithoutX = math.pi * 1 / 2
local endPos   = CGeoPoint:new_local(-100, -100)

local StartPos = startPosWithoutX
local startDir = startDirWithoutX

local StartPos2 = CGeoPoint:new_local(200, 100)
local endPos2   = CGeoPoint:new_local(-100, -200)

local posFileName	  = "./compensation/data/pos.data"
local relFileName 	  = "./compensation/data/addSpeed.rel"

local initX = 0
local initY = 100
local initDir = 0

local stepY = 0

local posXs = {}
local posYs = {}
local posDirs = {}

-- for 15-10
-- Y_COMPENSATION_GEARS = 21, 221,	 250,	270, 300
-- W_COMPENSATION_VALUE = 0,  0.025, 0.05, 0.075, 0.1

-- for 15-9  // 待定
-- Y_COMPENSATION_GEARS = 10, 91,	 111,  151, 151,  171, 201,  251, 271, 290, 300
-- W_COMPENSATION_VALUE = 0,  0.025, 0.05, 0.1, 0.15, 0.2, 0.25, 0.3,  0.35, 0.4, 0.425

-- for 15-8
-- Y_COMPENSATION_GEARS = 10, 51,	 80,	120, 141,  171, 201,  251, 271, 290, 300
-- W_COMPENSATION_VALUE = 0,  0.05, 0.075, 0.1, 0.15, 0.2, 0.25, 0.3,  0.35, 0.4, 0.425

-- for 15-7
-- Y_COMPENSATION_GEARS = 10, 51,	 80,	120, 141,  161, 231,  251, 290,  300
-- W_COMPENSATION_VALUE = 0,  0.05, 0.075, 0.1, 0.15, 0.2, 0.25, 0.35,  0.375, 0.425

-------------------------------------------------------------------------------
-- for 14-6

-- for 14-1
-- Y_COMPENSATION_GEARS =  52,	  90,  120,   130, 170,  190,   240, 260,   270, 280,    290, 300
-- W_COMPENSATION_VALUE =  0, 0.075, 0.1, 0.15,  0.2, 0.225, 0.25, 0.3,  0.325, 0.35, 0.375, 0.4
-- =============================================================================
-- for 15-3
-- use 15-5

-- for 15-4
-- use 14-5

-- for 15-3, 15-4, 15-5
-- 10, 51,	 80,	120, 141,  161, 231,  251, 270,  290,  300,
-- 0,  0.05, 0.075, 0.1, 0.15, 0.2, 0.25, 0.3, 0.325, 0.35, 0.375,
-- Y_COMPENSATION_GEARS = 10, 51,	 80,	120, 141,  161, 231,  251, 270,  290,  300
-- W_COMPENSATION_VALUE = 0,  0.05, 0.075, 0.1, 0.15, 0.2, 0.25, 0.3, 0.325, 0.35, 0.375


-- for 15-1(尽量不用)
-- 10, 51,	 80,	120, 141,  161, 180,  200,   220, 230,   240,  250, 260
-- 0,  0.05, 0.075, 0.1, 0.15, 0.2, 0.25, 0.275, 0.3, 0.325, 0.35, 0.4, 0.5

-- for 15-6
-- 	10 	61	 	111  141   201 	221 	251 	271 	285 	300 	320
--	0 	0.05 	0.1  0.15  0.2  0.25 	0.30 	0.325 	0.35 	0.375 	0.4
-- Y_COMPENSATION_GEARS = 10, 61,	111,  141,  201, 221,	251, 	271,   285,  300, 320
-- W_COMPENSATION_VALUE = 0,0.05,	0.1,  0.15, 0.2, 0.25, 	0.30, 	0.325, 0.35, 0.375,	0.4

-- for 15-2
--	51-116	116-150	150-175	175-200 200-225	225-250	250-260 260-270 270-275	275-280 280-285 285-290 290-295 295-300
--	0.1 	0.2 	0.225 	0.25 	0.275 	0.3 	0.325  	0.35 	0.375 	0.4 	0.425 	0.45 	0.475 	
-- Y_COMPENSATION_GEARS = 51, 116, 150, 175,   200,  225,   250, 260,   270,  275,   280, 285,   290,  295,  300
-- W_COMPENSATION_VALUE = 0,  0.1,	0.2, 0.225,	0.25, 0.275, 0.3, 0.325, 0.35, 0.375, 0.4, 0.425, 0.45, 0.475, 0.5
local speedX = function()
	return initX
end
local speedY = function()
	return initY
end
local speedW = function()
	return initDir
end


local worldViewToRobot = function(_vx, _vy, _dir)

	angle = math.deg(math.atan(_vy / _vx))
	-- io.write("angle before: ", angle, '\n')
	if (_vx < 0 and _vy > 0) then
		angle = angle + 180
	elseif _vx < 0 and _vy < 0 then
		angle = angle - 180
	end

	-- io.write("_vy / _vx = ", _vy / _vx, '\n')
	-- io.write("angle: ", angle, '\n')
	tmpDir = math.deg(_dir)
	-- io.write("carDir: ", tmpDir, '\n')
	deltaAngle = angle - tmpDir
	if math.abs(deltaAngle) > 180 then
		if angle > 0 and tmpDir < 0 then
			deltaAngle = deltaAngle - 360
		elseif angle < 0 and tmpDir > 0 then
			deltaAngle = 360 + deltaAngle
		end
	end

	-- io.write("deltaAngle: ", deltaAngle, '\n')
	robotVX = math.sqrt(_vx^2 + _vy^2) * math.cos(math.rad(deltaAngle))
	robotVY = math.sqrt(_vx^2 + _vy^2) * math.sin(math.rad(deltaAngle))
	return robotVX, robotVY
end

local calculateSpeed = function(_dataPosXs, _dataPosYs, _dataPosDirs)
	if type(_dataPosXs) ~= "table" or type(_dataPosYs) ~= "table" or type(_dataPosDirs) ~= "table" then
		print("error in calculateSpeed() : type wrong")
		return
	end

	-- print("if 1")
	-- if #_dataPosXs ~= #_dataPosYs or #_dataPosXs ~= #_dataPosDirs or #_dataPosYs ~= #dataPosDirs then
	-- 	print("error in calculateSpeed() : count wrong")
	-- 	return
	-- end

	-- print("if 2")
	dataCount = #_dataPosXs
	if dataCount <= 100 then
		print("error in calculateSpeed() : datas < 100")
		return
	end
	-- print ("before loop")
	vx, vy, vdir, posDir= {}, {}, {}, {}
	resultX, resultY, resultDir = 0, 0, 0
	for i = 30, #_dataPosXs - 30, 1 do
		-- print("in lood")
		table.insert(vx, 60 * (_dataPosXs[i] - _dataPosXs[i-1]))
		table.insert(vy, 60 * (_dataPosYs[i] - _dataPosYs[i-1]))
		table.insert(posDir, (_dataPosDirs[i] + _dataPosDirs[i-1]) / 2)

		if math.abs(_dataPosDirs[i] - _dataPosDirs[i-1]) > math.pi then
			if _dataPosDirs[i] > 0 and _dataPosDirs[i-1] < 0 then
				table.insert(vdir, 60 *(_dataPosDirs[i] - _dataPosDirs[i-1] - 2 * math.pi))
			else
				table.insert(vdir, 60 *(_dataPosDirs[i] - _dataPosDirs[i-1] + 2 * math.pi))
				-- resultDir = resultDir + 60 * ((_dataPosDirs[i] -_dataPosDirs[i-1]) + 2 * math.pi)
			end
		else
			table.insert(vdir, 60 *(_dataPosDirs[i] - _dataPosDirs[i-1]))
		end
	end

	for i = 1, #vx do
		tmpX, tmpY = worldViewToRobot(vx[i], vy[i], posDir[i])
		resultX = resultX + tmpX
		resultY = resultY + tmpY
		resultDir = resultDir + vdir[i]
		-- io.write(tmpY, '\n')
	end

	return resultX / #vx, resultY / #vy, resultDir / #vdir
end

local doCalculate = function(setVX, setVY, setVW, posXs, posYs, posDirs)
	local sizeCount = #posXs - 60
	local startX = posXs[31]
	local startY = posYs[31]
	local startDir = posDirs[31]
	local realEndX, realEndY, realEndDir = 0, 0, 0
	if #posXs > 120 then
		realEndX = posXs[#posXs - 30]
		realEndY = posYs[#posYs - 30]
		realEndDir = posDirs[#posDirs - 30]
	else
		print("data size error")
		realEndX = posXs[#posXs - 30]
		realEndY = posYs[#posYs - 30]
		realEndDir = posDirs[#posDirs - 30]
	end
	-- local supposedX = startX + (#posXs - 60) * setVX / 60
	-- local supposedY = startY + (#posYs - 60) * setVY / 60
	-- local supposedDir = startDir + (#posDirs - 60) * setVW / 60
	local deltaX = realEndX - startX
	local deltaY = realEndY - startY
	local realVX, realVY = worldViewToRobot(deltaX, deltaY, startDir)
	realVX = realVX * 60 / sizeCount
	realVY = realVY * 60 / sizeCount
	local realVW = (realEndDir - startDir) * 60 / (#posDirs - 60)

	realVX, realVY, realVW = calculateSpeed(posXs, posYs, posDirs)


	local addVX = setVX - realVX --(supposedX - realEndX) * 60 / (sizeCount)
	local addVY = setVY - realVY --(supposedY - realEndY) * 60 / sizeCount
	local addVW = setVW - realVW --(supposedDir - realEndDir) * 60 / sizeCount

	addSpeedFile = io.open(relFileName, 'a')
	io.output(addSpeedFile)
	io.write(setVX, '\t', setVY, '\t', setVW, '\n')
	io.write(realVX, '\t', realVY, '\t', realVW, '\n')
	io.write(addVX, '\t', addVY, '\t', addVW, '\n')
	io.close(addSpeedFile)
end



gPlayTable.CreatePlay{

	firstState = "goStartPoint",

	["goStartPoint"] = {
		switch = function()
			print("goStartPoint")
			if bufcnt(player.toPointDist("Kicker", StartPos) < 2 and math.abs(player.dir("Kicker") - startDir) < 0.03 
				--and player.toPointDist("Tier", StartPos2) < 2 and math.abs(player.dir("Tier") - startDir) < 0.03
				 , 30) then
				posFile = io.open(posFileName, 'a')
				io.output(posFile)
				io.write(speedX(), '\t', speedY(), '\t', speedW(), '\n')
				return "runWithOpenSpeed"
				-- return "goEndPoint"
			end
		end,
		Kicker = task.goCmuRush(StartPos, startDir, 300),
		-- Tier   = task.goCmuRush(StartPos2, startDir, 300),
		match = ""
	},

	["goEndPoint"] = {
		switch = function()
			if bufcnt(player.toPointDist("Kicker", endPos) < 2  and player.toPointDist("Tier", endPos2) < 2 , 60) then
				return "goStartPoint"
			end
		end,
		Kicker = task.goCmuRush(endPos, startDir, 300),
		Tier   = task.goCmuRush(endPos2, startDir, 300),
		match = ""
	},

	["runWithOpenSpeed"] = {
		switch = function( )
			-- print("runWithOpenSpeed")
			table.insert(posXs, player.posX("Kicker"))
			table.insert(posYs, player.posY("Kicker"))
			table.insert(posDirs, player.dir("Kicker"))
			io.write(player.posX("Kicker"), '\t', player.posY("Kicker"), '\t', player.dir("Kicker"), '\n')
			if bufcnt(math.abs(player.posX("Kicker")) > 350 or math.abs(player.posY("Kicker")) > 222 , 1, 350) then
				io.write("-------------------------------------------------",'\n')
				io.close(posFile)
				doCalculate(speedX(), speedY(), speedW(), posXs, posYs, posDirs)
				posXs, posYs, posDirs = {}, {}, {}
				initY = initY + stepY
				return "goStartPoint"
			end
		end,
		Kicker = task.openSpeedLocalVersion(speedX, speedY, speedW),
		match = ""
	},

	["goStop"] = {
		switch = function()
			if dataFile ~= nil then
			end
		end,

		Kicker = task.stop(),
		match = ""
	},

	name = "OpenSpeedTest",
	applicable = {
		exp = "a",
		a = true
	},
	attribute = "sampling",
	timeout = 99999
}