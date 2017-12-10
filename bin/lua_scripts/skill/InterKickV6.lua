function InterKickV6(task)
	local mdir
	local mpower
	local mflag = task.flag or 0
	local mkick = task.kickway or false
	execute = function(runner)
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end
		if type(task.kickpower) == "function" then
			mpower = task.kickpower(runner)
		else
			mpower = task.kickpower
		end
		return CInterKickV6(runner, mdir, mkick, mpower)
	end

	matchPos = function()
		return ball.pos()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "InterKickV6",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}
