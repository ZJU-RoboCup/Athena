function InterKick(task)
	local mdir
	local mflag = task.flag or 0
	local mkick = task.kickway or false
	execute = function(runner)
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end
		return CInterKick(runner, mdir, mkick)
	end

	matchPos = function()
		return ball.pos()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "InterKick",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}
