function GoAroundRobot(task)
	local mpos
	local mdir
	local mrotdir = task.rotdir or 0
	local mradius = task.radius or 25
	execute = function(runner)
		if type(task.dir) == "function" then
			mdir = task.dir(runner)
		else
			mdir = task.dir
		end

		if type(task.pos) == "function" then
			mpos = task.pos(runner)
		else
			mpos = task.pos
		end

		return CGoAroundRobot(runner, mdir, mpos:x(), mpos:y(), mrotdir, mradius)
	end

	matchPos = function()
		return ball.pos()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "GoAroundRobot",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}