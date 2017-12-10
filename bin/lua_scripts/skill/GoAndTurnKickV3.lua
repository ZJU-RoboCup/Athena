function GoAndTurnKickV3(task)
	local mpos
	local mdir
	local mflag = task.flag or 0
	local mcircle = task.circle or 0
	local fixAngle = math.pi / 3.3
	local maxAcc = task.acc or 650
	local radius = 19
	local numPerCircle = 5
	local goToPre = math.pi / 6
	local goToDist = 19
	local adjustPre = math.pi / 72

	execute = function(runner)
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end
		CSetPassDir(player.dir(runner))
		local mpre = task.pre()
		return CGoAndTurnKickV3(runner, mdir, mpre, mcircle, fixAngle, maxAcc, radius, numPerCircle, goToPre, goToDist, adjustPre)
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
	name = "GoAndTurnKickV3",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}
