function ReceivePass(task)
	local mdir
	local mflag = task.flag or 0

	execute = function(runner)
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end
		return CReceivePass(runner, mdir, mflag)
	end

	matchPos = function()
		return pos.playerBest()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "ReceivePass",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}
