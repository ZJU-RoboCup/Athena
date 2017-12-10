function CrazyPush(task)
	local dir = task.dir or dir.shoot()

	execute = function(runner)
		return CCrazyPush(runner, dir)
	end

	matchPos = function()
		return ball.pos()
	end

	return execute, matchPos
end

gSkillTable.CreateSkill{
	name = "CrazyPush",
	execute = function (self)
		print("This is in skill"..self.name)
	end
}
