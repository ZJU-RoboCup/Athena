function TouchKick(task)
	local mdir
	
	execute = function(runner)
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end
		return CTouchKick(runner, mdir)
	end

	matchPos = function()
		return ball.pos()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "TouchKick",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}
