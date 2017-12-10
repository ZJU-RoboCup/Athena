function GoAndTurnKickV4(task)
	local mdir
	local mflag = task.flag or 0

	execute = function(runner)
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end

		return CGoAndTurnKickV4(runner, mdir)
	end

	matchPos = function()
		return ball.pos()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "GoAndTurnKickV4",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}